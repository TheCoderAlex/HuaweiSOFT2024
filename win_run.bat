@echo off
setlocal enabledelayedexpansion

:: Step 1: Compile the CPP file
set "cpp_file=%~1"
set "exe_name=%~n1"
echo Compiling %cpp_file%...
g++ -std=c++17 -O2 "%cpp_file%" -o "Demo\%exe_name%.exe"
if %errorlevel% neq 0 (
    echo Compilation failed.
    pause
    exit /b %errorlevel%
)

:: Step 2: Ask for map choice
echo Available maps:
for /l %%i in (1,1,8) do (
    echo map%%i
)
set /p map_choice="Choose a map (1-8): "

:: Step 3: Ask for seed
set /p seed="Enter a seed: "

:: Step 4: Execute PreliminaryJudge with chosen map and seed
echo Running with seed %seed% and map %map_choice%...
.\PreliminaryJudge.exe -l ERR -d log.txt -s %seed% -m "maps\map%map_choice%.txt" .\Demo\%exe_name%.exe

pause
endlocal
