xcopy %1\dll\%2\*.dll ..\%3 /F /Y
cd ..\Install
"C:\Program Files\NSIS\makensis.exe" install_%2.nsi
cd ..
exit 0
