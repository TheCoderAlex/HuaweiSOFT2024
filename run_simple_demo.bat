@echo off
setlocal enabledelayedexpansion
set /a randHigh=%RANDOM% * 32768
set /a randLow=%RANDOM%
set /a seed=!randHigh!+!randLow!
echo Generated seed: !seed!
pause

.\PreliminaryJudge.exe -l ERR -s 123 -m maps\map10.txt .\Demo\avoid_main.exe

endlocal