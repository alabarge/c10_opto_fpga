..\..\utils\fw_ver.exe build.inc build.h ..\..\ ..\..\c10_top\PR_R2\c10_fpga.qsf
robocopy . share build.h /NFL /NDL /NJH /NJS /NS /NC /NP
robocopy cp_srv share cp_msg.h /NFL /NDL /NJH /NJS /NS /NC /NP
robocopy daq_srv share daq_msg.h /NFL /NDL /NJH /NJS /NS /NC /NP
robocopy ..\c10_bsp share system.h /NFL /NDL /NJH /NJS /NS /NC /NP
robocopy ..\c10_bsp share linker.h /NFL /NDL /NJH /NJS /NS /NC /NP
