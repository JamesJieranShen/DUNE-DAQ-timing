local moo = import "moo.jsonnet";

// A schema builder in the given path (namespace)
local ns = "dunedaq.timing.timinghardwareinfo";
local s = moo.oschema.schema(ns);

// A temporary schema construction context.
local timinghardwareinfo = {
    
    bool_data: s.boolean("BoolData", doc="A bool"),

    text_data: s.string("TextData", moo.re.ident_only, 
        doc="A string field"),

    uint: s.number("RegValue", "u4", 
        doc="32 bit uint"),

    l_uint: s.number("LongUint", "u8",
        doc="64 bit uint"),

    l_int: s.number("LongInt", "i8",
        doc="64 bit uint"),

    double_val: s.number("DoubleValue", "f8", 
        doc="A double"),

    // hardware monitor structures
    timing_pll_mon_data: s.record("TimingPLLMonitorData", 
   	[
        s.field("config_id", self.text_data,
                doc="PLL config ID"),
        s.field("cal_pll", self.bool_data, 0,
                doc="Cal pll"),
        s.field("hold", self.bool_data, 0,
                doc="Holdover flag"),
        s.field("lol", self.bool_data, 0,
                doc="Loss of lock flag"),
        s.field("los", self.uint,
                doc="Loss of signal flag"),
        s.field("los_xaxb", self.uint,
                doc="Loss of signal flag XAXB"),
        s.field("los_xaxb_flg", self.uint,
                doc="Loss of signal flag XAXB stricky"),
        s.field("oof", self.uint,
                doc="Out of frequency flags"),
        s.field("oof_sticky", self.uint,
                doc="Out of frequency flags sticky"),
        s.field("smbus_timeout", self.bool_data, 0,
                doc="SMBUS timeout"),
        s.field("smbus_timeout_flg", self.bool_data, 0,
                doc="SMBUS timeout sticky"),
        s.field("sys_in_cal", self.bool_data, 0,
               doc="In calibration flag"),
        s.field("sys_in_cal_flg", self.bool_data, 0,
                doc="In calibration flag sticky"),
        s.field("xaxb_err", self.bool_data, 0,
                doc="XA-XB error flag"),
        s.field("xaxb_err_flg", self.bool_data, 0,
                doc="XA-XB error flag sticky"),
    ], 
    doc="Timing PLL monitor structure for data read over I2C"),

    timing_sfp_mon_data: s.record("TimingSFPMonitorData", 
    [
        s.field("vendor_name", self.text_data,
                doc="Vendor name"),
        s.field("vendor_pn", self.text_data,
                doc="Vendor part number"),
        s.field("sfp_fault", self.bool_data, 0,
               doc="SFP fault flag"),
        s.field("ddm_supported", self.bool_data, 0,
                doc="DDM supported flag"),
        s.field("tx_disable_hw", self.bool_data, 0,
                doc="Hardware transmit disable pin value"),
        s.field("tx_disable_sw_supported", self.bool_data, 0,
                doc="Soft transmit disable bit control supported"),
        s.field("tx_disable_sw", self.bool_data, 0,
                doc="Soft transmit disable bit value"),
        s.field("temperature", self.double_val,
                doc="SFP temperature"),
        s.field("supply_voltage", self.double_val,
                doc="SFP supply voltage"),
        s.field("rx_power", self.double_val,
                doc="SFP receive power"),
        s.field("tx_power", self.double_val,
                doc="SFP transmit power"),
        s.field("laser_current", self.double_val,
                doc="SFP laser current"),
    ], 
    doc="Timing SFP monitor structure for data read over I2C"),
//
    timing_fmc_mon_data: s.record("TimingFMCMonitorData", 
    [
        s.field("class_name", self.text_data, "TimingFMCMonitorData",
                doc="Info class name"),
        s.field("cdr_lol", self.bool_data,
                doc="CDR LOL flag"),
        s.field("cdr_los", self.bool_data, 0,
                doc="CDR LOS flag"),
        s.field("mmcm_ok", self.bool_data, 0,
                doc="MMCM OK flag"),
        s.field("mmcm_sticky", self.bool_data, 0,
                doc="Sticky MMCM OK flag"),
        s.field("sfp_flt", self.bool_data, 0,
                doc="SFP fault pin value"), 
        s.field("sfp_los", self.bool_data, 0,
                doc="SFP LOS pin value"),
    ], 
    doc="Timing FMC monitor data"),
//
    timing_fmc_mon_data_debug: s.record("TimingFMCMonitorDataDebug", 
    [
        s.field("class_name", self.text_data, "TimingFMCMonitorDataDebug",
                doc="Info class name"),
        s.field("pll_mon_data", self.timing_pll_mon_data,
                doc="PLL monitoring data"),
        s.field("sfp_mon_data", self.timing_sfp_mon_data,
                doc="SFP monitoring data"),
        s.field("cdr_freq", self.double_val,
                doc="CDR frequency"),
        s.field("pll_freq", self.double_val,
                doc="PLL frequency"),
    ],
    bases=self.timing_fmc_mon_data,
    doc="Extended timing FMC monitor data"),
//
    timing_tlu_mon_data: s.record("TimingTLUMonitorData", 
    [
        s.field("class_name", self.text_data, "TimingTLUMonitorData",
                doc="Info class name"),
        s.field("cdr_lol", self.bool_data,
                doc="CDR LOL flag"),
        s.field("cdr_los", self.bool_data, 0,
                doc="CDR LOS flag"),
        s.field("mmcm_ok", self.bool_data, 0,
                doc="MMCM OK flag"),
        s.field("mmcm_sticky", self.bool_data, 0,
                doc="Sticky MMCM OK flag"),
        s.field("pll_ok", self.bool_data, 0,
                doc="PLL OK flag"),
        s.field("pll_sticky", self.bool_data, 0,
                doc="Sticky PLL OK flag"),
        s.field("sfp_flt", self.bool_data, 0,
                doc="SFP fault pin value"),
        s.field("sfp_los", self.bool_data, 0,
                doc="SFP LOS pin value"),
    ], 
    doc="Timing TLU monitor data"),

    timing_tlu_mon_data_debug: s.record("TimingTLUMonitorDataDebug", 
    [   
        s.field("class_name", self.text_data, "TimingTLUMonitorDataDebug",
                doc="Info class name"),
        s.field("pll_mon_data", self.timing_pll_mon_data,
                doc="PLL monitoring data"),
    ], 
    doc="Extended timing TLU monitor data"),
};

// Output a topologically sorted array.
moo.oschema.sort_select(timinghardwareinfo, ns)