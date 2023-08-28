#!/bin/sh
#
# This file was automatically generated.
#
# It can be overwritten by nios2-flash-programmer-generate or nios2-flash-programmer-gui.
#

bin2flash.exe --input="../../../nios/c10_fw/zipfs.zip" --output="../output_files/zipfs_epcs.flash" --location=0x100000 --verbose

nios2-flash-programmer.exe "../output_files/zipfs_epcs.flash" --base=0x10020000 --epcs --device=1 --instance=0 --program --verbose --override=nios2-flash-override.txt --verify
