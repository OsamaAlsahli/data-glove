# Data Glove

Low-cost, high-accuracy data glove with real-time hand motion tracking and
fingertip force sensing.

Final Year Project — **Osama Alsahli**, BEng Electronic Engineering, University
of York, 2026.

## What this is

A wearable glove that captures finger and hand orientation using sixteen
BNO055 9-axis IMUs (one per finger phalanx plus one on the dorsum), with
soft optical tactile sensors (STOF) at the fingertips for contact force.
Quaternion data is streamed from an ESP32 over USB serial to a Python
bridge, which serves it to the FSGlove visualisation framework via gRPC.

Headline hardware bill of materials: 16× BNO055 IMUs, 2× TCA9548A I2C
multiplexers, one ESP32 dev board, soft Ecoflex 00-30 STOF sensors at the
fingertips. Total cost of purchased electronics ≈ £105.

For the full report (motivation, lit review, design, evaluation), see the
accompanying BEng final report.

## Repository layout

```
firmware/         ESP32 Arduino sketches (C++)
  adafruit_test/  Single-BNO055 smoke test
  tst/            Single sensor through TCA9548A
  tst2/           TCA9548A I2C scanner
  webserial_3d/   Single sensor for the Adafruit WebSerial 3D viewer
  quat_finger/    4-IMU single-finger demo
  quat/           Full 16-IMU glove firmware
  fsglove-main/   FSGlove visualisation framework (third-party, included
                  for reproducibility - see Licence section)

bridge/           Python serial-to-gRPC bridge
  bridge.py            Full 16-sensor bridge (the one you actually run)
  bridge_finger.py     Incomplete 4-sensor variant, kept for reference
  read_imu.py          Minimal serial quaternion printer
  visualise_imu.py     Live OpenGL "bone" visualiser

images/           Prototype photos, kinematic model, Gantt chart
schematics/       PCB / schematic designs (KiCad) for STOF and main board
```

## How to flash the firmware

1. Open `firmware/quat/quat.ino` in the Arduino IDE.
2. Install the **Adafruit BNO055** and **Adafruit Unified Sensor** libraries
   from the Library Manager.
3. Select the ESP32 board, the matching USB serial port, and upload.

The single-sensor bring-up sketches (`adafruit_test`, `tst`, `tst2`,
`webserial_3d`) are useful for verifying hardware before flashing the full
build.

## How to run the bridge

```bash
cd bridge
pip install pyserial grpcio
# Generate imu_packet_pb2.py / imu_packet_pb2_grpc.py from the FSGlove proto
# (imu_packet.proto lives inside firmware/fsglove-main).
python bridge.py
```

The bridge listens on `localhost:18890`; start FSGlove afterwards and it
will subscribe to the stream.

If you want to drive the visualiser without hardware, swap the
`target=read_serial` line in `bridge.py`'s `main()` for
`target=generate_fake_serial`.

## Licence

The code I wrote (everything under `firmware/` except `fsglove-main/`, and
everything under `bridge/`) is released for academic and personal use.

`firmware/fsglove-main/` is included for reproducibility only and is the
work of its original authors — see its own README and licence inside that
folder. The SMPLH model weights (`SMPLH_male.pkl`, `SMPLH_female.pkl`)
required by FSGlove are **not** included in this repo. To obtain them and
get FSGlove running, follow the setup instructions in the upstream FSGlove
repository (<https://github.com/davidliyutong/fsglove>); the project page
is at <https://sites.google.com/view/fsglove/>.

## Notes

- `firmware/quat/quat.ino` has a known issue on line 77 where sensor 15
  is read through multiplexer channel 5 instead of channel 15. Flagged in
  the source with a `NOTE:` comment; verify the channel before deploying.
- `bridge/bridge_finger.py` is an unfinished early version of the bridge,
  kept for historical reference. The working bridge is `bridge/bridge.py`.
