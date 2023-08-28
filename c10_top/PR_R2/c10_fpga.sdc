set_time_format -unit ns -decimal_places 3

derive_pll_clocks -create_base_clocks
derive_clock_uncertainty

#create_clock -name {iCLK_50M} -period 20.000 -waveform { 0.000 10.000 } [get_ports {iCLK_50M}]

#**************************************************************
# Set Input Delay
#**************************************************************
set_min_delay 0.0 -from [get_ports {iFSCTS iFSDO }] -to *
set_max_delay 10.0 -from [get_ports {iFSCTS iFSDO }] -to *
set_min_delay 0.0 -from [get_ports {ioDRAM_DQ[*] iADC_MISO iADC_INTBn }] -to *
set_max_delay 10.0 -from [get_ports {ioDRAM_DQ[*] iADC_MISO iADC_INTBn }] -to *

#**************************************************************
# Set Output Delay
#**************************************************************
set_min_delay  0.0 -from * -to [get_ports { oFSCLK oFSDI }]
set_max_delay  10.0 -from * -to [get_ports { oFSCLK oFSDI }]
set_min_delay  0.0 -from * -to [get_ports {oDRAM_ADDR[*] oDRAM_BA[*] oDRAM_CASn oDRAM_RASn oDRAM_CLK oDRAM_CSn}]
set_max_delay  10.0 -from * -to [get_ports {oDRAM_ADDR[*] oDRAM_BA[*] oDRAM_CASn oDRAM_RASn oDRAM_CLK oDRAM_CSn }]
set_min_delay  0.0 -from * -to [get_ports {ioDRAM_DQ[*] oDRAM_DQM[*] oDRAM_RASn oDRAM_WEn}]
set_max_delay  10.0 -from * -to [get_ports {ioDRAM_DQ[*] oDRAM_DQM[*] oDRAM_RASn oDRAM_WEn }]
set_min_delay  0.0 -from * -to [get_ports {oADC_CSn oADC_MOSI oADC_SCLK oADC_CNVTBn }]
set_max_delay  10.0 -from * -to [get_ports {oADC_CSn oADC_MOSI oADC_SCLK oADC_CNVTBn }]


#**************************************************************
# Set False Path Outputs
#**************************************************************

set_false_path -from [get_clocks iCLK_12M] -to [get_clocks altera_reserved_tck]
set_false_path -from [get_clocks altera_reserved_tck] -to [get_clocks iCLK_12M]
set_false_path -from * -to [get_ports {oTP* oSTDOUT_UART_TX}]
set_false_path -from * -to [get_ports {oLED*}]
set_false_path -from * -to [get_ports {ioGPX*}]
set_false_path -from [get_registers {*watchdog*}] -to *

set_false_path -from * -to [get_ports {*ALTERA_DCLK}]
set_false_path -from * -to [get_ports {*ALTERA_SCE}]
set_false_path -from * -to [get_ports {*ALTERA_SDO}]

#**************************************************************
# Set False Path Inputs
#**************************************************************

set_false_path -from [get_ports {ioGPX* iGPX* iRSTn iSTDOUT_UART_RX}] -to *
set_false_path -from [get_registers { sys_rst_n }] -to *

set_false_path -from *ALTERA_DATA0 -to *

#**************************************************************
# JTAG
#**************************************************************
#create_clock -period 10MHz {altera_reserved_tck}
set_clock_groups -asynchronous -group {altera_reserved_tck}
set_input_delay -clock {altera_reserved_tck} 20 [get_ports altera_reserved_tdi]
set_input_delay -clock {altera_reserved_tck} 20 [get_ports altera_reserved_tms]
set_output_delay -clock {altera_reserved_tck} 20 [get_ports altera_reserved_tdo]
