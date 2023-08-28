xcopy %3\%4.dll %1\dll\%2\%3\*.dll /F /Y
xcopy %3\%4.lib %1\lib\%2\%3\*.lib /F /Y
xcopy %3\%4.exp %1\lib\%2\%3\*.exp /F /Y
xcopy %3\%4.pdb %1\lib\%2\%3\*.pdb /F /Y
xcopy %4.h %1\inc /Y /F
xcopy x64\wpcap.dll %1\dll\%2\%3 /Y /F
exit 0