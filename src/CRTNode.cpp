#include "pdt/CRTNode.hpp"

namespace pdt {

UHAL_REGISTER_DERIVED_NODE(CRTNode)

//-----------------------------------------------------------------------------
CRTNode::CRTNode(const uhal::Node& aNode) : TimingNode(aNode) {
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
CRTNode::~CRTNode() {
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void
CRTNode::enable(uint32_t aPartition, uint32_t aCmd) const {

	getNode("csr.ctrl.tgrp").write(aPartition);
    getNode("pulse.ctrl.cmd").write(aCmd);
    getNode("pulse.ctrl.en").write(0x1);
	getClient().dispatch();
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
void
CRTNode::disable() const {
	getNode("pulse.ctrl.en").write(0x0);
    getClient().dispatch();
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
std::string
CRTNode::getStatus(bool aPrint) const {
	std::stringstream lStatus;
	auto lCRTRegs = readSubNodes(getNode(""));
    lStatus << formatRegTable(lCRTRegs, "CRT state", {"", ""}) << std::endl;

    const uint64_t lLastPulseTimestamp = ((uint64_t)lCRTRegs.at("pulse.ts_h").value() << 32) + lCRTRegs.at("pulse.ts_l").value();
    lStatus << "Last Pulse Timestamp: 0x" << std::hex << lLastPulseTimestamp << std::endl;

    if (aPrint) std::cout << lStatus.str();
    return lStatus.str();
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
uint64_t
CRTNode::readLastPulseTimestamp() const {

    auto lTimestampRegLow = getNode("pulse.ts_l").read();
    auto lTimestampRegHigh = getNode("pulse.ts_h").read();
    getClient().dispatch();

    return ((uint64_t)lTimestampRegHigh.value() << 32) + lTimestampRegLow.value();	
}
//-----------------------------------------------------------------------------

} // namespace pdt