"""read_imu.py - Minimal serial-quaternion printer.

Reads the "Quaternion: w, x, y, z" lines produced by webserial_3d.ino (the
single-sensor WebSerial firmware) and prints each quaternion to the terminal
formatted to four decimal places. Used during bring-up to eyeball whether
the BNO055 was producing sensible numbers without spinning up a viewer.
"""

import serial
import time

# Open the ESP32. A short timeout means readline() returns promptly even when
# the device is briefly quiet between samples.
ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1)
time.sleep(3)        # let the ESP32 finish its boot/reset before reading
ser.flushInput()     # discard any garbage from the reset

print("Reading quaternion data...")

while True:
    try:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
    except:
        # Decoding errors can happen on serial noise; just retry.
        continue

    if not line:
        continue

    # The firmware also prints Orientation/Calibration lines; ignore anything
    # that is not the quaternion we care about.
    if line.startswith("Quaternion:"):
        data = line.split(":")[1].strip()
        try:
            w, x, y, z = [float(v) for v in data.split(',')]
            print(f"W: {w:.4f}  X: {x:.4f}  Y: {y:.4f}  Z: {z:.4f}")
        except ValueError:
            # Partial line received - skip it.
            pass
