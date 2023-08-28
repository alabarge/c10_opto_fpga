do adc_run_msim_rtl_vhdl.do

vcom -2008 -work work {./adc_top_TB.vhd}

vsim -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L cycloneiv_ver -L rtl_work -L work -voptargs="+acc"  adc_top_TB

do wave.do
