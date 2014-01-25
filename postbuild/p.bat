hi %1 %2
IF ERRORLEVEL 0 (
call log %1 - OK!
) else (
call log %1 - failed!
)
