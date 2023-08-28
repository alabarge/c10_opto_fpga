ECHO F|xcopy %1%2\%3\%4.exe %1..\..\data\%4.exe /F /Y
ECHO F|xcopy %1driver\wpcap.dll %1..\..\data\wpcap.dll /F /Y
ECHO F|xcopy %1driver\libserialport.dll %1..\..\data\libserialport.dll /F /Y
exit 0