TITLE a2_hat
:a
echo "[%date%, %time%] Starting A2 hat" >> allods2.log
start /BELOWNORMAL /w C:\Allods2\redhat.exe
echo "[%date%, %time%] A2 hat stopped" >> allods2.log
sleep 5
goto a
