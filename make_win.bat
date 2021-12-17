@echo off

set build_path="build/vs2019"
set source_path="%CD%"

if NOT EXIST %build_path% (
    mkdir %build_path% || goto :do_exit
)

pushd %build_path%

CALL cmake.exe -G "Visual Studio 16 2019" %source_path%  || goto :do_exit

popd

:do_exit
exit /b %ERRORLEVEL%
