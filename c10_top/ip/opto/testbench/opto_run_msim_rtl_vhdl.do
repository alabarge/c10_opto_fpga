transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vcom -2008 -work work {../../../packages/de0_pkg.vhd}
vcom -2008 -work work {../../../packages/PCK_tb.vhd}
vcom -2008 -work work {../../../packages/PCK_print_utilities.vhd}
vcom -2008 -work work {../../../packages/PCK_FIO_1993.vhd}
vcom -2008 -work work {../../../packages/PCK_FIO_1993_BODY.vhd}
vcom -2008 -work work {../adc_4k.vhd}
vcom -2008 -work work {../adc_ctl.vhd}
vcom -2008 -work work {../adc_irq.vhd}
vcom -2008 -work work {../adc_regs.vhd}
vcom -2008 -work work {../adc_core.vhd}
vcom -2008 -work work {../adc_pixi.vhd}

