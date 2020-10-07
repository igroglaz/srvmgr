@echo off

SET PATH=%PATH%;%CD%

call log building a2server.exe
if exist a2server.exe del /Q /F a2server.exe >nul

copy original\a2server.exe a2server.exe >nul

rem if exist srvmgr.dll del /Q /F srvmgr.dll >nul
rem copy /Y ..\srvmgr\Release\srvmgr.dll .\ >nul

call log adding section .anet...
add_sect a2server.exe .anet 60000060 4000 >nul

call log building server...

call log patching...
for /f %%A in (arena.txt) do call p patches\%%A a2server.exe
cd patches
for %%A in (bug_0*.xck) do call p %%A ..\a2server.exe
cd ..
for /f %%A in (srvlist.txt) do call p patches\%%A a2server.exe

call log adding srvmgr...
rem add_dll a2server.exe srvmgr.dll server.mp >nul
DLLInject server.dis

del /Q /F release
mkdir release
move /Y a2server.exe release >nul
move /Y srvmgr.dll release >nul

call log building world.res...
if exist world.res del /Q /F world.res >nul
if exist world\data.bin del /Q /F world\data.bin >nul
if exist tmp_world rmdir /S /Q tmp_world >nul
databin2xml.exe "world\data.xml" world\data.bin
mkdir tmp_world
mkdir tmp_world\data
copy world\*.* tmp_world\data >nul
del /Q /F tmp_world\data\data.xml >nul
res.exe ar world.res world tmp_world >nul
del /Q /F world\data.bin >nul

move /Y world.res release >nul
move /Y srvmgr.pdb release >nul

call log done!
rem pause
exit
