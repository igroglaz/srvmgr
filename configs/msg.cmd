@echo off
set repeat=3
set delay=100
FOR /L %%i IN (1, 1, %repeat%) DO (
	echo %2 > C:\Allods2\Server%1\ctl\say.8
	sleep -m %delay%
)
exit