../../utils/fw_ver.exe build.inc build.h ../../ ../../c10_top/PR_R2/c10_fpga.qsf
cp -f build.h share/build.h
cp -f cp_srv/cp_msg.h share/cp_msg.h
cp -f daq_srv/daq_msg.h share/daq_msg.h
cp -f ../c10_bsp/system.h share/system.h
cp -f ../c10_bsp/linker.h share/linker.h
