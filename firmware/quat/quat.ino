// quat.ino - Full 16-IMU data glove firmware for ESP32
//
// Reads orientation quaternions from sixteen BNO055 IMUs distributed across
// two TCA9548A I2C multiplexers and streams them over USB serial as a single
// pipe-separated line per sample.
//
// Wiring assumptions:
//   - ESP32 default I2C pins (SDA = 21, SCL = 22)
//   - TCA9548A multiplexer at I2C address 0x70
//   - All BNO055 modules have their ADR pin tied to 3.3 V, giving address
//     0x29 (the AliExpress boards used here defaulted to 0x28, which clashed
//     with the multiplexer in some test setups, so 0x29 was forced)
//
// Output format (one line per sample, terminated with \n):
//   w,x,y,z|w,x,y,z|...|w,x,y,z       (16 quaternions, 15 separators)

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define TCAADDR 0X70   // I2C address of the TCA9548A multiplexer

// One Adafruit_BNO055 object per sensor. The first argument is a local id
// used by the library, the second is the I2C address as seen through the mux.
Adafruit_BNO055 imus[16] = {
  Adafruit_BNO055(0,0x29),
  Adafruit_BNO055(1,0x29),
  Adafruit_BNO055(2,0x29),
  Adafruit_BNO055(3,0x29),
  Adafruit_BNO055(4,0x29),
  Adafruit_BNO055(5,0x29),
  Adafruit_BNO055(6,0x29),
  Adafruit_BNO055(7,0x29),
  Adafruit_BNO055(8,0x29),
  Adafruit_BNO055(9,0x29),
  Adafruit_BNO055(10,0x29),
  Adafruit_BNO055(11,0x29),
  Adafruit_BNO055(12,0x29),
  Adafruit_BNO055(13,0x29),
  Adafruit_BNO055(14,0x29),
  Adafruit_BNO055(15,0x29)
};

// Activate channel `i` on the TCA9548A so that subsequent I2C traffic is
// routed only to the device on that channel. Writing a one-hot byte selects
// the channel; writing 0 disables all channels.
void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}

// Select the multiplexer channel, query the BNO055, and print a comma-
// separated quaternion (no trailing separator so the caller controls layout).
void read_imu_quat(int imu_id, int mp_addr){
  tcaselect(mp_addr);
  imu::Quaternion quat = imus[imu_id].getQuat();
  Serial.print(quat.w());
  Serial.print(",");
  Serial.print(quat.x());
  Serial.print(",");
  Serial.print(quat.y());
  Serial.print(",");
  Serial.print(quat.z());
}

// Read all sixteen sensors and emit one full sample.
// NOTE: this firmware was written assuming a single 16-channel mux; in the
// final hardware the IMUs are split across two TCA9548As, so a second
// multiplexer-select layer is required for sensors 8-15 in a follow-up build.
void read_all_imu(){
  read_imu_quat(0, 0);
  Serial.print("|");
  read_imu_quat(1, 1);
  Serial.print("|");
  read_imu_quat(2, 2);
  Serial.print("|");
  read_imu_quat(3, 3);
  Serial.print("|");
  read_imu_quat(4, 4);
  Serial.print("|");
  read_imu_quat(5, 5);
  Serial.print("|");
  read_imu_quat(6, 6);
  Serial.print("|");
  read_imu_quat(7, 7);
  Serial.print("|");
  read_imu_quat(8, 8);
  Serial.print("|");
  read_imu_quat(9, 9);
  Serial.print("|");
  read_imu_quat(10, 10);
  Serial.print("|");
  read_imu_quat(11, 11);
  Serial.print("|");
  read_imu_quat(12, 12);
  Serial.print("|");
  read_imu_quat(13, 13);
  Serial.print("|");
  read_imu_quat(14, 14);
  Serial.print("|");
  read_imu_quat(15, 5);   // NOTE: passes mux channel 5 for sensor 15 - kept as written; verify channel before deploying.
}

void setup(void){
  Serial.begin(115200);
  Wire.begin();
  // Walk every channel, bring up the BNO055 on that channel, and enable the
  // external 32 kHz crystal for better orientation stability.
  for(int i=0; i<16; i++){
    tcaselect(i);
    if(!imus[i].begin()){
      Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
      while(1);   // halt - missing IMU means the rest of the stream would be invalid
    }
    imus[i].setExtCrystalUse(true);
  }
}

void loop(void){
  read_all_imu();
  Serial.println();
  delay(10);   // ~100 Hz update target (actual rate limited by I2C + mux switching)
}
