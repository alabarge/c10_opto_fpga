if exist ".\release\" rmdir /s /q .\release
if exist ".\output_files\" del /F .\output_files\c10*.* .\output_files\*.h
..\..\utils\fpga_pre.exe fpga_build.inc fpga_build.h fpga_build.vhd ..\..\.git
echo f|xcopy fpga_build.h output_files\fpga_build.h /F /Y /R
echo f|xcopy fpga_build.h ..\..\nios\c10_fw\share\fpga_build.h /F /Y /R
