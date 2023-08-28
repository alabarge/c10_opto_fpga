onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate /adc_top_tb/clk
add wave -noupdate -group SLAVE /adc_top_tb/reset_n
add wave -noupdate -group SLAVE /adc_top_tb/irq
add wave -noupdate -group SLAVE -radix hexadecimal /adc_top_tb/address
add wave -noupdate -group SLAVE /adc_top_tb/read_n
add wave -noupdate -group SLAVE -radix hexadecimal /adc_top_tb/readdata
add wave -noupdate -group SLAVE /adc_top_tb/write_n
add wave -noupdate -group SLAVE -radix hexadecimal /adc_top_tb/writedata
add wave -noupdate -group SLAVE -radix hexadecimal /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_REGS_I/adc_ADR_BEG
add wave -noupdate -group SLAVE -radix hexadecimal /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_REGS_I/adc_ADR_END
add wave -noupdate -group SLAVE -radix hexadecimal /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_REGS_I/adc_PKT_CNT
add wave -noupdate -group SLAVE /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_REGS_I/adc_POOL_CNT
add wave -noupdate -group SLAVE -radix hexadecimal /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_REGS_I/adc_ADC_RATE
add wave -noupdate -group SLAVE -radix hexadecimal /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_REGS_I/adc_CONTROL
add wave -noupdate -group MASTER -radix hexadecimal /adc_top_tb/m1_wr_address
add wave -noupdate -group MASTER -radix hexadecimal /adc_top_tb/m1_writedata
add wave -noupdate -group MASTER /adc_top_tb/m1_write
add wave -noupdate -group MASTER /adc_top_tb/m1_wr_waitreq
add wave -noupdate -group MASTER -radix hexadecimal /adc_top_tb/m1_wr_burstcount
add wave -noupdate -expand -group CHIP -radix hexadecimal /adc_top_tb/ramp
add wave -noupdate -expand -group ADC /adc_top_tb/ADC_TOP_I/sclk
add wave -noupdate -expand -group ADC /adc_top_tb/ADC_TOP_I/cs_n
add wave -noupdate -expand -group ADC /adc_top_tb/ADC_TOP_I/mosi
add wave -noupdate -expand -group ADC /adc_top_tb/ADC_TOP_I/miso
add wave -noupdate -expand -group ADC /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/intb_n
add wave -noupdate -expand -group ADC /adc_top_tb/ADC_TOP_I/cnvtb_n
add wave -noupdate -expand -group ADC /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/convert
add wave -noupdate -expand -group ADC -radix hexadecimal /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/cnvst_cnt
add wave -noupdate -expand -group ADC -childformat {{/adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/ad.pkt_cnt -radix hexadecimal} {/adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/ad.in_ptr -radix hexadecimal} {/adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/ad.out_dat -radix hexadecimal} {/adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/ad.adc_dat -radix hexadecimal} {/adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/ad.seq_id -radix hexadecimal}} -expand -subitemconfig {/adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/ad.pkt_cnt {-height 15 -radix hexadecimal} /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/ad.in_ptr {-height 15 -radix hexadecimal} /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/ad.out_dat {-height 15 -radix hexadecimal} /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/ad.adc_dat {-height 15 -radix hexadecimal} /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/ad.seq_id {-height 15 -radix hexadecimal}} /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/ad
add wave -noupdate -expand -group ADC -childformat {{/adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/wr.addr -radix hexadecimal} {/adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/wr.wrd_cnt -radix hexadecimal} {/adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/wr.pool_cnt -radix hexadecimal} {/adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/wr.head_addr -radix hexadecimal} {/adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/wr.burstcnt -radix hexadecimal}} -expand -subitemconfig {/adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/wr.addr {-height 15 -radix hexadecimal} /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/wr.wrd_cnt {-height 15 -radix hexadecimal} /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/wr.pool_cnt {-height 15 -radix hexadecimal} /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/wr.head_addr {-height 15 -radix hexadecimal} /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/wr.burstcnt {-height 15 -radix hexadecimal}} /adc_top_tb/ADC_TOP_I/ADC_CORE_I/ADC_CTL_I/wr
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {{Cursor 1} {5996292625 ps} 0} {{Cursor 2} {1419793836 ps} 0} {{Cursor 3} {100761516 ps} 0}
quietly wave cursor active 2
configure wave -namecolwidth 205
configure wave -valuecolwidth 72
configure wave -justifyvalue left
configure wave -signalnamewidth 1
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
configure wave -gridoffset 0
configure wave -gridperiod 1
configure wave -griddelta 40
configure wave -timeline 0
configure wave -timelineunits us
update
WaveRestoreZoom {1409062720 ps} {1480603456 ps}
