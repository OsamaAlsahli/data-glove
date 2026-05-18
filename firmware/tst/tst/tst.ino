// tst.ino - Single-sensor-via-mux quaternion stream
//
// Bring-up step between adafruit_test (no mux) and the multi-sensor builds.
// Talks to one BNO055 on channel 0 of a TCA9548A and prints its quaternion
// each loop iteration, formatted so the Python bridge can parse it.

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

#define TCAADDR 0X70
#define imu_no 1

Adafruit_BNO055 imus[imu_no] = {
  Adafruit_BNO055(0,0x29)
};

// Activate one of channels 0-7 on the TCA9548A. The delay was added because
// the BNO055 occasionally missed the first transaction after a channel switch.
void tcaselect(uint8_t i) {
  if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
  delay(20);
}

// Print one quaternion followed by a pipe separator (matches the multi-sensor
// firmware's frame format).
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
  Serial.print("|");
}

void read_all_imu(){
  read_imu_quat(0, 0);
  delay(50);
  Serial.println();
}

void setup(void){
  Serial.begin(115200);
  Wire.begin(21,22);        // ESP32 default I2C pins
  Wire.setClock(100000);    // slower clock helps with long wires / breadboard parasitics
  Wire.setTimeOut(50);
  Serial.println("Initializing Sensors...");

  for(int i=0; i<imu_no; i++){
    tcaselect(i);
    if(!imus[i].begin()){
      Serial.print("IMU NOT FOUND ");
    }
    delay(50);
  }
}

void loop(void){
  read_all_imu();
  delay(50);
}
