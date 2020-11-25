TITLE a2_server
cd C:\
cd Allods2\
del /q time.info
cd Server%1
:a
del /q time.info
del /q chr\*.sck
del /q chr\info.map
del /q ctl\*

echo "[%date%, %time%] Allods Server # %1 is starting" >> ..\allods2.log
start /BELOWNORMAL /w a2serv%1.exe -cfg"C:\Allods2\Server%1\server.cfg" --cfg2="srvmgr.cfg"
echo "[%date%, %time%] Allods Server # %1 stopped" >> ..\allods2.log
sleep 15
goto a
