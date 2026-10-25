import serial
import time
import sys

print("=" * 65)
print("     MAZEBOT LIVE SERIAL DIAGNOSTIC MONITOR (COM15)")
print("=" * 65)
print("\nInstructions:")
print(" >> Press the physical 'EN' (or 'RST') button on your ESP32 once <<")
print("    (or switch your robot power switch OFF then ON)\n")
print("Listening for boot output...")

try:
    ser = serial.Serial('COM15', 115200, rtscts=False, dsrdtr=False, timeout=0.1)
    start = time.time()
    while time.time() - start < 15:
        if ser.in_waiting:
            data = ser.read(ser.in_waiting).decode('utf-8', errors='replace')
            print(data, end='', flush=True)
            start = time.time() # Reset timeout on activity
        time.sleep(0.05)
    ser.close()
    print("\n--- Diagnostic monitor finished ---")
except Exception as e:
    print("\nSerial Error:", e)
