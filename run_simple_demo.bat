@echo off
setlocal enabledelayedexpansion
set /a randHigh=%RANDOM% * 32768
set /a randLow=%RANDOM%
set /a seed=!randHigh!+!randLow!
echo Generated seed: !seed!
pause

.\PreliminaryJudge.exe -d log.txt -s !seed! -m maps\map5.txt .\Demo\main.exe

endlocal