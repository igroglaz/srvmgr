@echo
cls
taskkill /F /T /FI "IMAGENAME eq cmd.exe" /FI "WINDOWTITLE eq a2_*"
taskkill /F /T /IM  redhat.exe
taskkill /F /T /FI "IMAGENAME eq a2serv*"
exit
