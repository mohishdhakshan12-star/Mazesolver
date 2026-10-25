import subprocess
import time
import sys

print("=" * 60)
print("  ESP32 AUTO-DETECTING FLASH LOOP")
print("=" * 60)
print("\nInstructions:")
print(" 1. Press and HOLD the physical 'BOOT' button on your ESP32.")
print(" 2. While holding 'BOOT', tap 'EN' (or 'RST') once, then release.")
print(" 3. This script will automatically catch the connection and upload!\n")

cmd = [sys.executable, "-m", "platformio", "run", "--target", "upload"]

for attempt in range(1, 15):
    print(f"\n[Attempt {attempt}/15] Connecting to ESP32 on COM15...")
    proc = subprocess.run(cmd)
    if proc.returncode == 0:
        print("\n" + "=" * 60)
        print(" [SUCCESS] Entry-to-Exit Firmware with Pushbuttons uploaded successfully!")
        print("=" * 60)
        sys.exit(0)
    print("Connection not caught yet. Retrying in 1.5 seconds...")
    time.sleep(1.5)

print("\nAttempts finished. If it did not connect, make sure the USB cable is fully plugged in and the bot power switch is ON.")
