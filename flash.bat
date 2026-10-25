@echo off
cls
echo ======================================================================
echo             ESP32 ONE-TOUCH FLASH HELPER
echo ======================================================================
echo.
echo ESP32 boards need to be put into Bootloader mode before flashing.
echo.
echo  STEP 1: Press and HOLD the 'BOOT' (or 'IO0') button on the ESP32.
echo  STEP 2: While holding 'BOOT', tap the 'EN' (or 'RST') button once.
echo  STEP 3: Release the 'BOOT' button.
echo.
echo The ESP32 is now in Download Mode and ready!
echo.
pause
echo.
echo Flashing firmware to COM15...
python -m platformio run --target upload
echo.
pause
