@echo off

IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

ctime -begin pathingman.ctm

set CompilerFlags=-nologo -O2 -FC -Z7 -W4 -WX
set WarningFlags=-wd4996 -wd4505 -wd4201 -wd4189 -wd4100
cl %CompilerFlags% %WarningFlags% ..\code\pathingman.cpp 

ctime -end pathingman.ctm

popd