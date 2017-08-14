@echo off

set build_path=win
set source_path=..

if NOT EXIST "%build_path%" (
    mkdir "%build_path%"
)

rem mkdir "%build_path%"
pushd "%build_path%"

CALL cmake -G "Visual Studio 14 2015" %source_path%

popd