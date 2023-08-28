# TCL File Generated by Component Editor 12.0sp2
# Sat Nov 03 18:01:28 EDT 2012
# DO NOT MODIFY


#
# sdram "sdram" v1.0
# A.E. LaBarge 2012.11.03.18:01:28
#
#

#
# request TCL package from ACDS 12.0
#
package require -exact qsys 12.0


#
# module sdram
#
set_module_property NAME sdram
set_module_property VERSION 22.1
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property GROUP Omniware
set_module_property AUTHOR "A.E. LaBarge"
set_module_property DISPLAY_NAME "SDRAM Controller Intel FPGA IP, Deprecated"
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property ANALYZE_HDL AUTO
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false


#
# file sets
#
add_fileset quartus_synth QUARTUS_SYNTH "" "Quartus Synthesis"
set_fileset_property quartus_synth TOP_LEVEL sdram_top
set_fileset_property quartus_synth ENABLE_RELATIVE_INCLUDE_PATHS true
add_fileset_file sdram.v VERILOG PATH sdram.v

add_fileset sim_verilog SIM_VERILOG "" "Verilog Simulation"
set_fileset_property sim_verilog TOP_LEVEL sdram_top
set_fileset_property sim_verilog ENABLE_RELATIVE_INCLUDE_PATHS true
add_fileset_file sdram.v VERILOG PATH sdram.v


#
# parameters
#


#
# display items
#


#
# connection point s1
#
add_interface s1 avalon slave
set_interface_property s1 addressAlignment {DYNAMIC}
set_interface_property s1 addressGroup {0}
set_interface_property s1 addressSpan {8388608}
set_interface_property s1 addressUnits {WORDS}
set_interface_property s1 alwaysBurstMaxBurst {0}
set_interface_property s1 associatedClock {clk}
set_interface_property s1 associatedReset {reset}
set_interface_property s1 bitsPerSymbol {8}
set_interface_property s1 burstOnBurstBoundariesOnly {0}
set_interface_property s1 burstcountUnits {WORDS}
set_interface_property s1 constantBurstBehavior {0}
set_interface_property s1 explicitAddressSpan {0}
set_interface_property s1 holdTime {0}
set_interface_property s1 interleaveBursts {0}
set_interface_property s1 isBigEndian {0}
set_interface_property s1 isFlash {0}
set_interface_property s1 isMemoryDevice {1}
set_interface_property s1 isNonVolatileStorage {0}
set_interface_property s1 linewrapBursts {0}
set_interface_property s1 maximumPendingReadTransactions {7}
set_interface_property s1 minimumUninterruptedRunLength {1}
set_interface_property s1 printableDevice {0}
set_interface_property s1 readLatency {0}
set_interface_property s1 readWaitStates {1}
set_interface_property s1 readWaitTime {1}
set_interface_property s1 registerIncomingSignals {0}
set_interface_property s1 registerOutgoingSignals {0}
set_interface_property s1 setupTime {0}
set_interface_property s1 timingUnits {Cycles}
set_interface_property s1 transparentBridge {0}
set_interface_property s1 wellBehavedWaitrequest {0}
set_interface_property s1 writeLatency {0}
set_interface_property s1 writeWaitStates {0}
set_interface_property s1 writeWaitTime {0}

add_interface_port s1 az_addr address Input 22
add_interface_port s1 az_be_n byteenable_n Input 2
add_interface_port s1 az_cs chipselect Input 1
add_interface_port s1 az_data writedata Input 16
add_interface_port s1 az_rd_n read_n Input 1
add_interface_port s1 az_wr_n write_n Input 1
add_interface_port s1 za_data readdata Output 16
add_interface_port s1 za_valid readdatavalid Output 1
add_interface_port s1 za_waitrequest waitrequest Output 1

set_interface_assignment s1 embeddedsw.configuration.isMemoryDevice {1}

#
# connection point clk
#
add_interface clk clock end
set_interface_property clk clockRate 0
set_interface_property clk ENABLED true

add_interface_port clk clk clk Input 1


#
# connection point reset
#
add_interface reset reset end
set_interface_property reset associatedClock clk
set_interface_property reset synchronousEdges DEASSERT
set_interface_property reset ENABLED true

add_interface_port reset reset_n reset_n Input 1


#
# connection point wire
#
add_interface wire conduit end
set_interface_property wire associatedClock clk
set_interface_property wire associatedReset reset
set_interface_property wire ENABLED true

add_interface_port wire zs_addr export Output 12
add_interface_port wire zs_ba export Output 2
add_interface_port wire zs_cas_n export Output 1
add_interface_port wire zs_cke export Output 1
add_interface_port wire zs_cs_n export Output 1
add_interface_port wire zs_dq export Bidir 16
add_interface_port wire zs_dqm export Output 2
add_interface_port wire zs_ras_n export Output 1
add_interface_port wire zs_we_n export Output 1

#
# DTS Entry
#
set_module_assignment embeddedsw.dts.vendor "omni"
set_module_assignment embeddedsw.dts.group "sdram"
set_module_assignment embeddedsw.dts.name "sdram"
set_module_assignment embeddedsw.dts.compatible "generic-uio"

