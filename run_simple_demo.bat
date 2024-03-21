@echo off
setlocal enabledelayedexpansion
set /a randHigh=%RANDOM% * 32768
set /a randLow=%RANDOM%
set /a seed=!randHigh!+!randLow!
echo Generated seed: !seed!
pause

.\PreliminaryJudge.exe -l ERR -d log.txt -s 999999 -m maps\map6.txt .\Demo\hihihi.exe

endlocal