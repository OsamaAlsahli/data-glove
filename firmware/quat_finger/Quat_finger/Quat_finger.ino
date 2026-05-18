// Quat_finger.ino - Single-finger demo firmware (4 IMUs)
//
// Reduced-scope variant of quat.ino used while validating the I2C plumbing
// on one finger before scaling up to the full sixteen-sensor glove. Four
// real BNO055s are read on mux channels 0-3; the remaining twelve slots are
// padded with a static identity quaternion so the downstream Python bridge
// (which expects sixteen entries) still parses a complete frame.

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define TCAADDR 0X70   // TCA9548A multiplexer address
#define imu_no 4       // number of real sensors connected for this build

// Real sensors on mux channels 0-3, all at I2C address 0x29.
Adafruit_BNO055 imus[imu_no] = {
  Adafruit_BNO055(0,0x29),
  Adafruit_BNO055(1,0x29),
  Adafruit_BNO055(2,0x29),
  Adafruit_BNO055(3,0x29)
};

// Select one of channels 0-7 on the TCA9548A.
void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
  delay(20);   // brief settle time - mux switching was unreliable without it
}

// Read one sensor and print its quaternion as comma-separated floats.
void read_imu_quat(int imu_id, int mp_addr){
  tcaselect(mp_addr);
  delay(5);
  imu::Quaternion quat = imus[imu_id].getQuat();
  Serial.print(quat.w());
  Serial.print(",");
  Serial.print(quat.x());
  Serial.print(",");
  Serial.print(quat.y());
  Serial.print(",");
  Serial.print(quat.z());
}

// Emit one full 16-slot frame: 4 real quaternions + 12 identity placeholders.
// Keeping the frame width constant means the Python bridge does not need to
// know how many sensors are physically present.
void read_all_imu(){
  read_imu_quat(0, 0);
  delay(50);
  Serial.print("|");
  read_imu_quat(1, 1);
  delay(50);
  Serial.print("|");
  read_imu_quat(2, 2);
  delay(50);
  Serial.print("|");
  read_imu_quat(3, 3);
  delay(50);
  Serial.print("|");
  // Identity quaternion = "no rotation" - placeholder for unconnected sensors.
  Serial.print("1.00,0.00,0.00,0.00");
  Serial.print("|");
  Serial.print("1.00,0.00,0.00,0.00");
  Serial.print("|");
  Serial.print("1.00,0.00,0.00,0.00");
  Serial.print("|");
  Serial.print("1.00,0.00,0.00,0.00");
  Serial.print("|");
  Serial.print("1.00,0.00,0.00,0.00");
  Serial.print("|");
  Serial.print("1.00,0.00,0.00,0.00");
  Serial.print("|");
  Serial.print("1.00,0.00,0.00,0.00");
  Serial.print("|");
  Serial.print("1.00,0.00,0.00,0.00");
  Serial.print("|");
  Serial.print("1.00,0.00,0.00,0.00");
  Serial.print("|");
  Serial.print("1.00,0.00,0.00,0.00");
  Serial.print("|");
  Serial.print("1.00,0.00,0.00,0.00");
  Serial.print("|");
  Serial.print("1.00,0.00,0.00,0.00");
  Serial.println();
}

void setup(void){
  Serial.begin(115200);
  Wire.begin(21,22);        // explicit ESP32 I2C pins
  Wire.setClock(100000);    // 100 kHz - slower than default; more tolerant of long mux paths
  Wire.setTimeOut(50);

  for(int i=0; i<imu_no; i++){
    tcaselect(i);
    if(!imus[i].begin()){
      Serial.print("IMU NOT FOUND");   // soft fail so the loop still runs for the surviving sensors
    }
    delay(50);
  }
}

void loop(void){
  read_all_imu();
  delay(50);
}
