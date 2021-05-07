/**
 * @file OuroborosMuxDesign.hpp
 *
 * OuroborosMuxDesign is a class providing an interface
 * to the PD-I master design firmware block.
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef TIMING_INCLUDE_TIMING_OUROBOROSMUXDESIGN_HPP_
#define TIMING_INCLUDE_TIMING_OUROBOROSMUXDESIGN_HPP_

// PDT Headers
#include "timing/MasterMuxDesign.hpp"
#include "timing/EndpointDesign.hpp"
#include "timing/TLUIONode.hpp"
#include "timing/FMCIONode.hpp"
#include "timing/PDIMasterNode.hpp"
#include "timing/TriggerReceiverNode.hpp"
#include "timing/SIMIONode.hpp"

// uHal Headers
#include "uhal/DerivedNode.hpp"

// C++ Headers
#include <chrono>
#include <string>
#include <sstream>

namespace dunedaq {
namespace timing {

/**
 * @brief      Class for PDI timing master design (known as overlord).
 */
template <class IO>
class OuroborosMuxDesign : public MasterMuxDesign<IO,PDIMasterNode>, public EndpointDesign<IO> {

public:
    explicit OuroborosMuxDesign(const uhal::Node& node);
    virtual ~OuroborosMuxDesign();

    /**
     * @brief     Get status string, optionally print.
     */
    std::string get_status(bool print_out=false) const override;

    /**
     * @brief      Prepare the timing master for data taking.
     *
     */
    void configure() const override;


// In leiu of UHAL_DERIVEDNODE
protected:
    virtual uhal::Node* clone() const;
//
};

} // namespace timing
} // namespace dunedaq

#include "timing/detail/OuroborosMuxDesign.hxx"

#endif // TIMING_INCLUDE_TIMING_OUROBOROSMUXDESIGN_HPP_