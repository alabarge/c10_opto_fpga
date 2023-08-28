sof2flash.exe --input="../output_files/c10_fpga.sof" --output="../output_files/c10_fpga_epcs.flash" --epcs --verbose

nios2-flash-programmer.exe "../output_files/c10_fpga_epcs.flash" --base=0x10020000 --epcs --device=1 --instance=0 --program --verbose --override=nios2-flash-override.txt --verify

elf2flash.exe --input="../../../nios/c10_fw/c10_fw.elf" --output="../output_files/c10_epcs.flash" --epcs --after="../output_files/c10_fpga_epcs.flash" --verbose

nios2-flash-programmer.exe "../output_files/c10_epcs.flash" --base=0x10020000 --epcs --device=1 --instance=0 --program --verbose --override=nios2-flash-override.txt --verify

quartus_pgm.exe -m jtag -c 1 -o "p;../output_files/c10_fpga.sof@1"

