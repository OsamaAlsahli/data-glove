"""bridge.py - Serial-to-gRPC bridge between the ESP32 firmware and FSGlove.

The ESP32 streams sixteen quaternions per sample over USB serial in the form:
    w,x,y,z|w,x,y,z|...|w,x,y,z\n

FSGlove expects to consume IMU samples over gRPC on localhost:18890 using the
IMUPacketService schema defined in imu_packet.proto. This script:

    1. Opens /dev/ttyUSB0 at 115200 baud.
    2. Continuously reads frames from the ESP32 in a background thread and
       caches the latest quaternion for each of the 16 sensors.
    3. Runs a gRPC server that streams those cached quaternions to FSGlove
       whenever it subscribes via GetPacketArrayStream.

A simulated-data path (generate_fake_serial) is included for working on the
visualisation pipeline when the physical hardware is not connected.
"""

import serial
import threading
import time
import math
from concurrent import futures

import grpc
import imu_packet_pb2
import imu_packet_pb2_grpc


# Serial port and shared state. ttyUSB0 is the default ESP32 enumeration on
# Linux; change this if your board appears as a different /dev node.
ser = serial.Serial('/dev/ttyUSB0', 115200)

# Cache of the most recent quaternion for each sensor. Initialised to the
# identity rotation (no rotation) so a subscriber that connects before the
# first serial frame arrives still gets a valid response.
quats = [[0.0, 0.0, 0.0, 1.0]] * 16

# Guards `quats` against concurrent read/write between the serial reader
# thread and the gRPC stream handler.
lock = threading.Lock()


def generate_fake_serial():
    """Stand-in for read_serial used when no hardware is connected.

    Produces a smoothly varying quaternion for each sensor so the downstream
    visualisation can be exercised without an ESP32 on the bench.
    """
    t = 0
    while True:
        with lock:
            for i in range(16):
                # Each sensor gets a different phase so they animate
                # independently rather than all moving in lockstep.
                angle = math.sin(t * 0.5 + i * 0.3) * 0.5
                quats[i] = [math.cos(angle/2), math.sin(angle/2), 0.0, 0.0]
        t += 0.01
        time.sleep(0.01)


def read_serial():
    """Read one line per loop from the ESP32 and update the quaternion cache.

    Each line is expected to contain up to 16 quaternions separated by '|',
    with each quaternion as four comma-separated floats (w,x,y,z). Partial
    frames are tolerated - only the entries that actually parse correctly
    are written into the cache.
    """
    while True:
        try:
            line = ser.readline().decode('utf-8').strip()
            if not line:
                continue

            # Split by the pipe character
            sensor_quat = line.split('|')

            with lock:
                # Use len(sensor_quat) so we never go "out of range"
                for i in range(len(sensor_quat)):
                    # Safety: don't exceed the 16 slots we allocated for 'quats'
                    if i >= 16:
                        break

                    values = sensor_quat[i].split(',')
                    if i==0:
                        # Sensor 0 is the wrist (dorsum) - log it for sanity
                        # checking while the bridge is running.
                        quats[0] = [float(v) for v in values]
                        print(f"Wrist Quat: {quats[0]}")
                    if len(values) == 4:
                        quats[i] = [float(v) for v in values]

        except Exception as e:
            # A malformed frame should not kill the bridge; log and continue.
            print(f"Serial Error: {e}")

class IMUBridge(imu_packet_pb2_grpc.IMUPacketServiceServicer):
    """gRPC servicer implementing the slice of IMUPacketService that FSGlove
    actually calls. The other methods are stubs that return success/empty
    responses so FSGlove's startup probes don't error out."""

    def GetPacketArrayStream(self, request, context):
        """Stream the latest cached quaternions to FSGlove until it
        disconnects. Yields one IMUPacketArrayStreamResponse per ~10 ms."""
        while context.is_active():
            # Snapshot the shared cache under the lock so the gRPC payload
            # is consistent even if the serial thread updates mid-iteration.
            with lock:
                current_quats = list(quats)
            packets = {}
            for i in range(16):
                packets[f"imu_{i}"] = imu_packet_pb2.IMUPacketResponseRaw(
                    id = f"imu_{i}",
                    quat_w=current_quats[i][0],
                    quat_x=current_quats[i][1],
                    quat_y=current_quats[i][2],
                    quat_z=current_quats[i][3]
                )
            synced = imu_packet_pb2.IMUPacketResponseSynced(
                packets=packets,
                sys_ticks=int(time.time() * 1000)
            )
            response = imu_packet_pb2.IMUPacketArrayStreamResponse(
                packets = [synced],
                valid = True
            )
            yield response
            time.sleep(0.01)   # ~100 Hz output cap

    # The remaining methods are required by the proto but unused by the
    # current FSGlove flow - return minimal success responses.
    def SetFIFOStatus(self, request, context):
        return imu_packet_pb2.IMUStatusResponse(status=True)

    def GetFIFOStatus(self, request, context):
        return imu_packet_pb2.IMUStatusResponse(status=True)

    def GetPacketArray(self, request, context):
        return imu_packet_pb2.IMUPacketArrayResponse()

    def ListDev(self, request, context):
        return imu_packet_pb2.IMUInfoResponse()

def main():
    # Start the serial reader as a daemon thread so Ctrl-C exits cleanly.
    # Swap read_serial for generate_fake_serial here to run without hardware.
    serial_thread = threading.Thread(target=read_serial)
    serial_thread.daemon=True
    serial_thread.start()

    # Stand up the gRPC server. Port 18890 matches what FSGlove dials by default.
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=10))
    imu_packet_pb2_grpc.add_IMUPacketServiceServicer_to_server(IMUBridge(), server)
    server.add_insecure_port('[::]:18890')
    server.start()
    server.wait_for_termination()

if __name__ == '__main__':
    print("Server running on port 18890...")
    main()
