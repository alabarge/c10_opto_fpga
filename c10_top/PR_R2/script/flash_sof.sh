#!/bin/sh
#
# This file was automatically generated.
#
# It can be overwritten by nios2-flash-programmer-generate or nios2-flash-programmer-gui.
#

sof2flash.exe --input="../output_files/c10_fpga.sof" --output="../output_files/c10_fpga_epcs.flash" --epcs --verbose

nios2-flash-programmer.exe "../output_files/c10_fpga_epcs.flash" --base=0x10020000 --epcs --device=1 --instance=0 --program --verbose --override=nios2-flash-override.txt --verify

