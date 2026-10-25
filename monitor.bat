@echo off
echo ================================================================
echo  Starting MazeBot Serial Monitor at 115200 baud (COM15)...
echo ================================================================
python -m platformio device monitor --baud 115200 --port COM15
pause
