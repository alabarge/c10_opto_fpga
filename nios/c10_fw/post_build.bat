..\..\utils\ci_check.exe de0_fw_ci.txt zipfs/c10_fw_ci.txt zipfs/c10_fw_ci.bin
rm -f zipfs.zip
cd zipfs
"C:\Program Files\7-Zip\7z.exe" a -tzip -mx0 ..\zipfs.zip
cd ..
c:\altera\25.1std\nios2eds\bin\bin2flash.exe --input="zipfs.zip" --output="zipfs.flash" --location=0xF0000 --verbose
riscv32-unknown-elf-objcopy --input-target srec --output-target ihex zipfs.flash zipfs.hex