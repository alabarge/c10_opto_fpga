del ..\%3\*.dll /Q /F
xcopy %1\dll\%2\%3\*.dll ..\%3 /F /Y /E /R
xcopy *Msg.h %1\inc /F /Y /E /R
if "%3"=="Debug" (
  xcopy %1\lib\%2\%3\*.pdb ..\%3 /F /Y /E /R
)
if "%4"=="Install" (
  %1\Install\installer.bat %1 %2 %3
)
exit 0