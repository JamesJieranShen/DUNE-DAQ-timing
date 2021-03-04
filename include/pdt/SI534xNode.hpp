/**
 * @file SI534xNode.hpp
 *
 * SI534xSlave and SI534xNode are classes providing an interface
 * over I2C to SI53xx PLL devices.
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TIMING_BOARD_SOFTWARE_INCLUDE_PDT_SI534XNODE_HPP_
#define TIMING_BOARD_SOFTWARE_INCLUDE_PDT_SI534XNODE_HPP_

#include "pdt/SIChipSlave.hpp"
#include "pdt/I2CMasterNode.hpp"
#include "pdt/timingmon/Structs.hpp"

#include "ers/Issue.hpp"

#include <map>
#include <vector>
#include <string>

namespace dunedaq {
ERS_DECLARE_ISSUE(pdt,                               ///< Namespace
                  SI534xConfigError,                 ///< Issue class name
                  " SI534xConfigError: " << message, ///< Message
                  ((std::string)message)             ///< Message parameters
)
ERS_DECLARE_ISSUE(pdt,                                       ///< Namespace
                  SI534xMissingConfigSectionError,           ///< Issue class name
                  " Missing configuration section: " << tag, ///< Message
                  ((std::string)tag)                         ///< Message parameters
)
ERS_DECLARE_ISSUE(pdt,                                                             ///< Namespace
                  SI534xRegWriteFailed,                                            ///< Issue class name
                  " Failed to write Si53xx reg: " << reg << "with data: " << data, ///< Message
                  ((std::string)reg)((std::string)data)                            ///< Message parameters
)
ERS_DECLARE_ISSUE(pdt,                                       ///< Namespace
                  SI534xRegWriteRetry,                       ///< Issue class name
                  "Retry " << attempt << " for reg " << reg, ///< Message
                  ((std::string)attempt)((std::string)reg)   ///< Message parameters
)
namespace pdt 
{

/**
 * @class      SI534xSlave
 *
 * @brief      I2C slave class to control SI5345 chips.
 * @author     Alessandro Thea
 * @date       August 2017
 */
class SI534xSlave : public SIChipSlave {
public:
    SI534xSlave(const I2CMasterNode* i2c_master, uint8_t i2c_device_address);
    virtual ~SI534xSlave();

    void configure(const std::string& filename) const;

    std::map<uint16_t, uint8_t> registers() const;

    std::string read_config_id() const;

    void get_info(timingmon::TimingPLLMonitorData& mon_data) const;

private:
    typedef boost::tuple<uint16_t, uint8_t>  RegisterSetting_t;

    std::string seek_header(std::ifstream& file) const;
    std::vector<RegisterSetting_t> read_config_section( std::ifstream& file, std::string tag ) const;

    void upload_config( const std::vector<SI534xSlave::RegisterSetting_t>& config ) const;
};

/**
 * @class      SI534xNode
 *
 * @brief      uhal::Node implementing single I2C Master Slave connection to
 *             control SI5345 chips
 * @author     Alessandro Thea
 * @date       August 2013
 */
class SI534xNode : public I2CMasterNode, public SI534xSlave {
    UHAL_DERIVEDNODE(SI534xNode)
public:
    explicit SI534xNode(const uhal::Node& node);
    SI534xNode(const SI534xNode& node);
    virtual ~SI534xNode();

};

    
} // namespace pdt
} // namespace dunedaq

#endif  // TIMING_BOARD_SOFTWARE_INCLUDE_PDT_SI534XNODE_HPP_