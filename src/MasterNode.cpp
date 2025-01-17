/**
 * @file MasterNode.cpp
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "timing/MasterNode.hpp"
#include "timing/MasterGlobalNode.hpp"

#include "logging/Logging.hpp"

#include <string>

namespace dunedaq {
namespace timing {

UHAL_REGISTER_DERIVED_NODE(MasterNode)

//-----------------------------------------------------------------------------
MasterNode::MasterNode(const uhal::Node& node)
  : MasterNodeInterface(node)
{}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
MasterNode::~MasterNode() {}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
std::string
MasterNode::get_status_tables() const
{
  std::stringstream status;

  status << getNode<MasterGlobalNode>("global").get_status();
  status << std::endl;

  status << getNode<FLCmdGeneratorNode>("scmd_gen").get_cmd_counters_table();
  status << std::endl;

  getNode("cmd_ctrs.addr").write(0x0);
  auto counters = getNode("cmd_ctrs.data").readBlock(0xff);
  getClient().dispatch();

  std::vector<uint32_t> non_zero_counters;
  std::vector<std::string> counter_labels;

  for (uint i=0; i < counters.size(); ++i) 
  {
    auto counter = counters.at(i);
    if (counter > 0)
    {
      counter_labels.push_back(format_reg_value(i));
      non_zero_counters.push_back(counter);
    }
  }

  std::vector<std::vector<uint32_t>> counters_container = { non_zero_counters }; // NOLINT(build/unsigned)

  status << format_counters_table(counters_container, { "Sent cmd counters" }, "Master cmd counters (>0)", counter_labels);
  status << std::endl;

  auto acmd_buf = read_sub_nodes(getNode("acmd_buf.stat"));
  status << format_reg_table(acmd_buf, "Master acmd buffer");

  return status.str();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
std::string
MasterNode::get_status(bool print_out) const
{
  std::stringstream status;
  auto raw_timestamp = getNode<TimestampGeneratorNode>("tstamp").read_raw_timestamp();
  status << "Timestamp: 0x" << std::hex << tstamp2int(raw_timestamp) << std::endl << std::endl;
  status << get_status_tables();

  if (print_out)
    TLOG() << status.str();
  return status.str();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
std::string
MasterNode::get_status_with_date(uint32_t clock_frequency_hz, bool print_out) const // NOLINT(build/unsigned)
{
  std::stringstream status;
  auto raw_timestamp = getNode<TimestampGeneratorNode>("tstamp").read_raw_timestamp();
  status << "Timestamp: 0x" << std::hex << tstamp2int(raw_timestamp) << " -> " << format_timestamp(raw_timestamp, clock_frequency_hz) << std::endl
          << std::endl;
  status << get_status_tables();

  if (print_out)
    TLOG() << status.str();
  return status.str();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
MasterNode::switch_endpoint_sfp(uint32_t address, bool turn_on) const // NOLINT(build/unsigned)
{
    uint32_t sequence = 0xab;
    uint32_t address_mode = 1;
    
    std::vector<uint32_t> tx_packet = { address & 0xff, 
                                        address >> 8UL, 
                                        sequence,

                                        // packet to reset rx
                                        (0x1 << 7UL) | 0x70, // write transaction on 0x70
                                        (address_mode << 7UL) | 0x1, // transaction length of 0x1
                                        turn_on,
                                      };
  tx_packet.back() = tx_packet.back() | (0x1 << 8UL);

  auto result = transmit_async_packet(tx_packet, -1);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
MasterNode::enable_upstream_endpoint() const
{
  auto global = getNode<MasterGlobalNode>("global");
  global.enable_upstream_endpoint();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
MasterNode::send_fl_cmd(uint32_t command,
                        uint32_t channel,                  // NOLINT(build/unsigned)
                        uint32_t number_of_commands) const // NOLINT(build/unsigned)
{
  for (uint32_t i = 0; i < number_of_commands; i++) { // NOLINT(build/unsigned)
    getNode<FLCmdGeneratorNode>("scmd_gen").send_fl_cmd(command, channel);
    
    auto ts_l = getNode("cmd_log.tstamp_l").read();
    auto ts_h = getNode("cmd_log.tstamp_h").read();
    auto sent_cmd = getNode("cmd_log.cmd").read();
    getClient().dispatch();

    if (sent_cmd.value() != command)
    {
      TLOG() << "cmd in sent log: 0x" << std::hex << command << ", does not match requested 0x: " << sent_cmd.value();
      // TODO throw something
    }
    uint64_t timestamp = (uint64_t)ts_h.value() << 32 | ts_l.value();
    TLOG() << "Command sent " << "(" << format_reg_value(command) << ") from generator "
         << format_reg_value(channel) << " @time " << std::hex << std::showbase << timestamp;
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
uint32_t                                                                      // NOLINT(build/unsigned)
MasterNode::measure_endpoint_rtt(uint32_t address, bool control_sfp) const // NOLINT(build/unsigned)
{

  auto global = getNode<MasterGlobalNode>("global");
  auto echo = getNode<EchoMonitorNode>("echo_mon");

  if (control_sfp)
  {
    // Switch off all TX SFPs
    //switch_endpoint_sfp(0xffff, false);

    // Turn on the current target
    switch_endpoint_sfp(address, true);

    millisleep(100);

    try
    {
      global.enable_upstream_endpoint();
    }
    catch (const timing::ReceiverNotReady& e)
    {
      if (control_sfp) {
        switch_endpoint_sfp(address, false);
      }
      throw e;
    }
  }

  uint32_t endpoint_rtt = echo.send_echo_and_measure_delay(); // NOLINT(build/unsigned)

  if (control_sfp)
    switch_endpoint_sfp(address, false);

  return endpoint_rtt;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
MasterNode::apply_endpoint_delay(uint32_t address,      // NOLINT(build/unsigned)
                                    uint32_t coarse_delay, // NOLINT(build/unsigned)
                                    uint32_t /*fine_delay*/,   // NOLINT(build/unsigned)
                                    uint32_t /*phase_delay*/,  // NOLINT(build/unsigned)
                                    bool measure_rtt,
                                    bool control_sfp) const
{

  auto global = getNode<MasterGlobalNode>("global");
  auto echo = getNode<EchoMonitorNode>("echo_mon");

  if (measure_rtt) {
    if (control_sfp) {
      // Switch off all TX SFPs
      // switch_endpoint_sfp(0xffff, false);

      // Turn on the current target
      switch_endpoint_sfp(address, true);

      millisleep(100);
    }

    try
    {
      global.enable_upstream_endpoint();
    }
    catch (const timing::ReceiverNotReady& e)
    {
      if (control_sfp) {
        switch_endpoint_sfp(address, false);
      }
      throw e;
    }

    uint64_t endpoint_rtt = echo.send_echo_and_measure_delay(); // NOLINT(build/unsigned)
    TLOG() << "Pre delay adjustment RTT:  " << format_reg_value(endpoint_rtt, 10);
  }

  uint32_t sequence = 0xab;
  uint32_t address_mode = 1;
    
  std::vector<uint32_t> tx_packet = { address & 0xff, 
                                        address >> 8UL, 
                                        sequence,

                                        // packet to write coarse delay
                                        (0x1 << 7UL) | 0x72, // write transaction on 0x72
                                        (address_mode << 7UL) | 0x1, // transaction length of 0x1
                                        coarse_delay & 0xf,

                                        // packet to set skew done
                                        (0x1 << 7UL) | 0x70, // write transaction on 0x70
                                        (address_mode << 7UL) | 0x1, // transaction length of 0x1
				                                0x3, // deskew done

                                        // packet to resync
                                        (0x1 << 7UL) | 0x70, // write transaction on 0x70
                                        (address_mode << 7UL) | 0x1, // transaction length of 0x1
				                                0x4, // resync
                                    };

  tx_packet.back() = tx_packet.back() | (0x1 << 8UL);

  transmit_async_packet(tx_packet, -1);

  if (measure_rtt) {
    try
    {
      global.enable_upstream_endpoint();
    }
    catch (const timing::ReceiverNotReady& e)
    {
      if (control_sfp)
      {
        switch_endpoint_sfp(address, false);
      }
      throw e;
    }

    uint64_t endpoint_rtt = echo.send_echo_and_measure_delay(); // NOLINT(build/unsigned)
    TLOG() << "Post delay adjustment RTT: " << format_reg_value(endpoint_rtt, 10);

    if (control_sfp)
      switch_endpoint_sfp(address, false);
  }
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
MasterNode::sync_timestamp(uint32_t clock_frequency_hz) const // NOLINT(build/unsigned)
{
  const uint64_t old_timestamp = read_timestamp(); // NOLINT(build/unsigned)
  TLOG() << "Reading old timestamp: " << format_reg_value(old_timestamp) << ", " << format_timestamp(old_timestamp, clock_frequency_hz);

  const uint64_t now_timestamp = get_seconds_since_epoch() * clock_frequency_hz; // NOLINT(build/unsigned)
  TLOG() << "Setting new timestamp: " << format_reg_value(now_timestamp) << ", " << format_timestamp(now_timestamp, clock_frequency_hz);

  set_timestamp(now_timestamp);

  const uint64_t new_timestamp = read_timestamp(); // NOLINT(build/unsigned)
  TLOG() << "Reading new timestamp: " << format_reg_value(new_timestamp) << ", " << format_timestamp(new_timestamp, clock_frequency_hz);

  enable_timestamp_broadcast();
  TLOG() << "Timestamp broadcast enabled";
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
uint64_t // NOLINT(build/unsigned)
MasterNode::read_timestamp() const
{
  return getNode<TimestampGeneratorNode>("tstamp").read_timestamp();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
MasterNode::set_timestamp(uint64_t timestamp) const // NOLINT(build/unsigned)
{
  getNode<TimestampGeneratorNode>("tstamp").set_timestamp(timestamp);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
MasterNode::get_info(timingfirmwareinfo::MasterMonitorData& mon_data) const
{
  mon_data.timestamp = read_timestamp();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
MasterNode::get_info(opmonlib::InfoCollector& ic, int level) const
{
  timingfirmwareinfo::MasterMonitorData mon_data;
  this->get_info(mon_data);

  auto control = read_sub_nodes(getNode("global.csr.ctrl"), false);
  auto state = read_sub_nodes(getNode("global.csr.stat"), false);
  getClient().dispatch();

  mon_data.ts_en = control.at("ts_en").value();
  mon_data.ts_err = state.at("ts_err").value();
  mon_data.tx_err = state.at("tx_err").value();
  mon_data.ctrs_rdy = state.at("ctrs_rdy").value();

  ic.add(mon_data);

  uint number_of_commands = 0xff;

  getNode("cmd_ctrs.addr").write(0x0);
  auto counters = getNode("cmd_ctrs.data").readBlock(number_of_commands);
  getClient().dispatch();

  for (uint i = 0; i < number_of_commands; ++i) { // NOLINT(build/unsigned)

    timingfirmwareinfo::SentCommandCounter cmd_counter;
    opmonlib::InfoCollector cmd_counter_ic;

    cmd_counter.counts = counters.at(i);

    std::stringstream channel;
    channel << "cmd_0x" << std::hex << i;

    cmd_counter_ic.add(cmd_counter);
    ic.add(channel.str(), cmd_counter_ic);
  }

  getNode<FLCmdGeneratorNode>("scmd_gen").get_info(ic, level);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void MasterNode::reset_command_counters() const
{
  auto global = getNode<MasterGlobalNode>("global");
  global.reset_command_counters();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
std::vector<uint32_t>
MasterNode::transmit_async_packet(const std::vector<uint32_t>& packet, int timeout) const
{
  // TODO: check for valid packet

  reset_sub_nodes(getNode("acmd_buf.txbuf"));

  TLOG_DEBUG(11) << "tx packet: ";
  for (auto t : packet)
    TLOG_DEBUG(11) << std::hex << "0x" << t;

  getNode("acmd_buf.txbuf").writeBlock(packet);
  getClient().dispatch();

  // we do not expect a reply
  if (timeout < 0)
  {
    std::vector<uint32_t> empty_vector;
    return empty_vector;
  }

  uhal::ValWord<uint32_t> buffer_ready;  // NOLINT(build/unsigned)
  uhal::ValWord<uint32_t> buffer_timeout;  // NOLINT(build/unsigned)

  // start time counting
  auto start = std::chrono::high_resolution_clock::now();

  // Wait for the buffer to be happy
  while (true) {

    buffer_ready = getNode("acmd_buf.stat.ready").read();
    buffer_timeout = getNode("acmd_buf.stat.timeout").read();
    getClient().dispatch();
    
    TLOG_DEBUG(10) << "async buffer ready: 0x" << buffer_ready.value() << ", timeout: " << buffer_timeout.value();
  
    if (buffer_timeout)
      throw VLCommandReplyTimeout(ERS_HERE);
     
    if (buffer_ready)
      break;

    auto now = std::chrono::high_resolution_clock::now();
    auto us_since_start = std::chrono::duration_cast<std::chrono::microseconds>(now - start);

    if (us_since_start.count() > timeout)
      throw VLCommandReplyBufferFlagTimeout(ERS_HERE, timeout);

    std::this_thread::sleep_for(std::chrono::microseconds(50));
  }
    
  auto rx_packet = getNode("acmd_buf.rxbuf").readBlock(0x20);
  getClient().dispatch();

  if (rx_packet.at(0) != 0xff || rx_packet.at(1) != 0xff || rx_packet.at(2) != packet.at(2))
  {
    ers::warning(InvalidVLCommandReplyPacket(ERS_HERE, rx_packet.at(0), rx_packet.at(1), rx_packet.at(2)));
  }

  TLOG_DEBUG(11) << "async result: ";
  for (auto r : rx_packet)
    TLOG_DEBUG(11) << std::hex << "0x" << r;
  
  return rx_packet.value();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void
MasterNode::write_endpoint_data(uint16_t endpoint_address, uint8_t reg_address, std::vector<uint8_t> data, bool address_mode) const
{
  auto data_length = data.size();
  if (data_length > 0x3f || data_length == 0)
  {
    TLOG() << "invalid data length";
  }

  // TODO make sequence a function argument?
  uint32_t sequence = 0xab;
  
  std::vector<uint32_t> tx_packet = { static_cast<uint32_t>(endpoint_address & 0xff), 
                                      static_cast<uint32_t>(endpoint_address >> 8UL), 
                                      sequence, 
                                      // bit 7 = 1 -> write
                                      static_cast<uint32_t>((0x1 << 7UL) | reg_address),
                                       static_cast<uint32_t>((address_mode << 7UL) | (0x3f & data_length))
                                    };
  tx_packet.insert(tx_packet.end(), data.begin(), data.end());
  tx_packet.back() = tx_packet.back() | (0x1 << 8UL);

  auto result = transmit_async_packet(tx_packet);
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
std::vector<uint32_t>
MasterNode::read_endpoint_data(uint16_t endpoint_address, uint8_t reg_address, uint8_t data_length, bool address_mode) const
{
  if (data_length > 0x3f || data_length == 0)
  {
    TLOG() << "invalid data length";
    // TODO throw something
  }

  // TODO make sequence a function argument?
  uint32_t sequence = 0xab;
  std::vector<uint32_t> tx_packet = { static_cast<uint32_t>(endpoint_address & 0xff), 
                                      static_cast<uint32_t>(endpoint_address >> 8UL), 
                                      sequence,
                                      // bit 7 = 0 -> read
                                      reg_address,
                                      static_cast<uint32_t>((0x1 << 8UL) | (address_mode << 7UL) | (0x3f & data_length))
                                      };

  auto result = transmit_async_packet(tx_packet);

  // get parts we actually want
  std::vector<uint32_t> result_data (result.begin()+3, result.begin()+3+data_length);
  
  // strip off the bit 8 which is high for last byte
  result_data.back() = result_data.back() & 0xff;

  return result_data;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void MasterNode::disable_timestamp_broadcast() const
{
  getNode("global.csr.ctrl.ts_en").write(0x0);
  getClient().dispatch();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void MasterNode::enable_timestamp_broadcast() const
{
  getNode("global.csr.ctrl.ts_en").write(0x1);
  getClient().dispatch();
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
timingfirmware::EndpointCheckResult
MasterNode::scan_endpoint(uint16_t endpoint_address, bool control_sfp) const
{
  timingfirmware::EndpointCheckResult result;
  auto global = getNode<MasterGlobalNode>("global");
  auto echo = getNode<EchoMonitorNode>("echo_mon");

  timingfirmware::EndpointCheckResult endpoint_result;
  endpoint_result.address = endpoint_address;

  // is endpoint sfp switched on?
  // are any relevant muxes set to correct channel?
  if (control_sfp)
  {
    switch_endpoint_sfp(endpoint_address, true);

    millisleep(100);
  }

  try
  {
    global.enable_upstream_endpoint();
  }
  catch (const timing::ReceiverNotReady& e)
  {
      switch_endpoint_sfp(endpoint_address, false);

      ers::error(MonitoredEndpointDead(ERS_HERE, endpoint_address));

      return endpoint_result;
  }

  endpoint_result.alive = true;
  endpoint_result.round_trip_time = echo.send_echo_and_measure_delay();
  TLOG_DEBUG(5) << "Endpoint at address " << endpoint_address << " alive. RTT: " << endpoint_result.round_trip_time;

  auto ept_state = read_endpoint_data(endpoint_address, 0x71, 0x1, 0x1).at(0) & 0xf;
  TLOG_DEBUG(5) << "Endpoint at address " << endpoint_address << " state: 0x" << std::hex << ept_state;
  endpoint_result.state = ept_state;

  if (ept_state == 0x6)
  {
    TLOG_DEBUG(5) << "Endpoint at address " << endpoint_address << ", applying delays of: " << 0x0;
      
    apply_endpoint_delay(endpoint_address, 0x0, 0x0, 0x0, false, false);
      
    endpoint_result.applied_delay = 0x0;

    auto ept_state_after_delays = read_endpoint_data(endpoint_address, 0x71, 0x1, 0x1).at(0) & 0xf;
    TLOG_DEBUG(5) << "Endpoint at address " << endpoint_address << ", state after delays apply: " << ept_state_after_delays;
    endpoint_result.state_after_delay_apply = ept_state_after_delays;

    endpoint_result.round_trip_time_after_delay_apply = echo.send_echo_and_measure_delay();
    TLOG_DEBUG(5) << "Endpoint at address " << endpoint_address << ", RTT after delays apply: " << endpoint_result.round_trip_time_after_delay_apply;
  }
  else if (ept_state == 0x7 || ept_state == 0x8)
  {
    TLOG_DEBUG(5) << "Endpoint at address " << endpoint_address << ", delays not needed";
  }
  else
  {
    ers::error(MonitoredEndpointUnexpectedState(ERS_HERE, endpoint_address, ept_state));
  }

  if (control_sfp)
  {
    switch_endpoint_sfp(endpoint_address, false);
  }

  return endpoint_result;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
void MasterNode::configure_endpoint_command_decoder(uint16_t endpoint_address, uint8_t slot, uint8_t command) const
{
  write_endpoint_data(endpoint_address, 0x60+slot, {command}, true);
}
//-----------------------------------------------------------------------------
} // namespace timing
} // namespace dunedaq
