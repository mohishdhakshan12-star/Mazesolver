@echo off
cls
echo ======================================================================
echo             MAZEBOT ESP32 FIRMWARE UPLOADER (COM15)
echo ======================================================================
echo.
echo  STEP 1: Make sure your robot power switch is ON.
echo  STEP 2: Press and HOLD the physical 'BOOT' button on your ESP32.
echo  STEP 3: While holding 'BOOT', tap the 'EN' (or 'RST') button once.
echo  STEP 4: Release the 'BOOT' button.
echo.
echo (The board is now in download mode and ready!)
echo.
pause
echo.
echo Flashing firmware to COM15...
python -m platformio run --target upload
echo.
pause
