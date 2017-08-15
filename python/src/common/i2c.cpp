// #include "pdt/python/registrators.hpp"


// Boost Headers
#include <boost/python/def.hpp>
#include <boost/python/wrapper.hpp>
#include <boost/python/overloads.hpp>
#include <boost/python/class.hpp>
#include <boost/python/enum.hpp>
#include <boost/python/copy_const_reference.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/operators.hpp>

#include "pdt/I2CBaseNode.hpp"
#include "pdt/I2CMasterNode.hpp"
#include "pdt/I2CSlave.hpp"
// #include "pdt/MiniPODMasterNode.hpp"
#include "pdt/SI5344Node.hpp"
// #include "pdt/SI570Node.hpp"

// Namespace resolution
using namespace boost::python;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(pdt_I2CBaseNode_writeI2C_overloads, writeI2C, 3, 4);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(pdt_I2CBaseNode_writeI2CArray_overloads, writeI2CArray, 3, 4);

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(pdt_I2CSlave_writeI2C_overloads, writeI2C, 2, 3);
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(pdt_I2CSlave_writeI2CArray_overloads, writeI2CArray, 2, 3);


namespace pdt {
namespace python {

void register_i2c() {
        // .def("hardReset", (void ( mp7::CtrlNode::*) (double)) 0, mp7_CTRLNODE_hardReset_overloads())

  // Wrap pdt::I2CMasterNode
  class_<pdt::I2CBaseNode, bases<uhal::Node> > ("I2CBaseNode", init<const uhal::Node&>())
      .def("getI2CClockPrescale", &pdt::I2CBaseNode::getI2CClockPrescale)
      .def("readI2C", &pdt::I2CBaseNode::readI2C)
      .def("writeI2C", &pdt::I2CBaseNode::writeI2C, pdt_I2CBaseNode_writeI2C_overloads())
      .def("readI2CArray", &pdt::I2CBaseNode::readI2CArray)
      .def("writeI2CArray", &pdt::I2CBaseNode::writeI2CArray, pdt_I2CBaseNode_writeI2CArray_overloads())
      .def("getSlaves", &pdt::I2CBaseNode::getSlaves)
      .def("getSlaveAddress", &pdt::I2CBaseNode::getSlaveAddress)
      ;

  // Wrap pdt::I2CMasterNode
  class_<pdt::I2CMasterNode, bases<pdt::I2CBaseNode> > ("I2CMasterNode", init<const uhal::Node&>())
      .def("getSlave", &pdt::I2CMasterNode::getSlave, return_internal_reference<>())
      ;

  // Wrap pdt::I2CSlave    
  class_<pdt::I2CSlave, boost::noncopyable>("I2CSlave", no_init)
      .def("getI2CAddress", &pdt::I2CSlave::getI2CAddress)
      .def("readI2C", &pdt::I2CSlave::readI2C)
      .def("writeI2C", &pdt::I2CSlave::writeI2C, pdt_I2CSlave_writeI2C_overloads())
      .def("readI2CArray", &pdt::I2CSlave::readI2CArray)
      .def("writeI2CArray", &pdt::I2CSlave::writeI2CArray, pdt_I2CSlave_writeI2CArray_overloads())
      ;

  // Wrap SI5344Slave
  class_<pdt::SI5344Slave, bases<pdt::I2CSlave>, boost::noncopyable > ("SI5344Slave", no_init)
      .def("readPage", &pdt::SI5344Slave::readPage)
      .def("switchPage", &pdt::SI5344Slave::switchPage)
      .def("readDeviceVersion", &pdt::SI5344Slave::readDeviceVersion)
      .def("readClockRegister", &pdt::SI5344Slave::readClockRegister)
      .def("writeClockRegister", &pdt::SI5344Slave::writeClockRegister)
      .def("configure", &pdt::SI5344Slave::configure)
      ;

  // Wrap SI5344Node
  class_<pdt::SI5344Node, bases<pdt::SI5344Slave, pdt::I2CBaseNode> > ("SI5344Node", init<const uhal::Node&>())
      ;

  // // Wrap SI5326Slave
  // class_<pdt::SI5326Slave, bases<pdt::I2CSlave>, boost::noncopyable > ("SI5326Slave", no_init)
  //     .def("configure", &pdt::SI5326Slave::configure)
  //     .def("reset", &pdt::SI5326Slave::reset)
  //     .def("intcalib", &pdt::SI5326Slave::intcalib)
  //     .def("sleep", &pdt::SI5326Slave::sleep)
  //     .def("debug", &pdt::SI5326Slave::debug)
  //     .def("registers", &pdt::SI5326Slave::registers)
  //     ;

  // // Wrap SI570Node
  // class_<pdt::SI5326Node, bases<pdt::SI5326Slave, pdt::I2CBaseNode> > ("SI5326Node", init<const uhal::Node&>())
  //     ;

}

} // namespace python
} // namespace pdt