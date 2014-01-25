@echo off
echo %DATE% %TIME% %1 %2 %3 %4 %5 %6 %7 %8 %9 > t
type t
type t >>log.txt
del t
