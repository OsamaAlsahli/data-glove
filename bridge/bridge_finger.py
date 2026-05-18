"""bridge_finger.py - Early 4-sensor variant of bridge.py.

NOTE: This file is an incomplete work-in-progress kept for historical
reference. It was the first attempt at the serial-to-gRPC bridge when only
one finger (four BNO055s) was wired up. The gRPC servicer class below is
truncated and will not run as-is - the finished, working bridge is bridge.py
in this same directory.
"""

import serial
import threading
import time
from concurrent import futures

import grpc
import imu_packet_pb2
import imu_packet_pb2_grpc


# Open the ESP32 serial port. Same baud rate as the full bridge.
ser = serial.Serial('/dev/ttyUSB0', 115200)

# Four-finger shared state plus a lock to keep the reader and the gRPC
# thread from racing on it.
quats = [0.0, 0.0, 0.0, 1.0]* 4
lock = threading.Lock()

def read_serial():
    """Read one frame per loop and parse the first four pipe-separated
    quaternions out of it."""
    while True:
        line = ser.readline().decode('utf-8').strip()
        sensor_quat = line.split('|')
        with lock:
            for i in range(4):
                values = sensor_quat[i].split(',')
                quats[i] = [float(v)  for v in values]

class IMUBridge(imu_packet_pb2_grpc.IMUpacketServiceServicer):
    # NOTE: incomplete - left as written during development.

    def GetPacketArrayStream(self, request, context):
        while context.

    def SetFIFOStatust(self, request, context):
