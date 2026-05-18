// webserial_3d.ino - Single BNO055 driver for the Adafruit WebSerial 3D viewer
//
// Used during early validation to confirm the BNO055 was outputting sensible
// orientation data, visualised in a browser via Adafruit's WebSerial 3D Model
// Viewer (https://adafruit.github.io/Adafruit_WebSerial_3DModelViewer/). The
// viewer expects three specific text lines per sample:
//   Orientation: heading, pitch, roll
//   Quaternion: w, x, y, z
//   Calibration: sys, gyro, accel, mag

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

/* Set the delay between fresh samples */
#define BNO055_SAMPLERATE_DELAY_MS (100)

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29);

// Print the BNO055's self-reported metadata once at startup - mostly a sanity
// check that the library is talking to the chip correctly.
void displaySensorDetails(void)
{
  sensor_t sensor;
  bno.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" xxx");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" xxx");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" xxx");
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}


void setup(void)
{
  Serial.begin(115200);
  Serial.println("WebSerial 3D Firmware"); Serial.println("");

  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }

  delay(1000);

  /* Use external crystal for better accuracy */
  bno.setExtCrystalUse(true);

  /* Display some basic information on this sensor */
  displaySensorDetails();
}

void loop(void)
{
  /* Get a new sensor event */
  sensors_event_t event;
  bno.getEvent(&event);

  /* The WebSerial 3D Model Viewer expects data as heading, pitch, roll */
  // Heading is inverted (360 - x) to match the viewer's coordinate convention.
  Serial.print(F("Orientation: "));
  Serial.print(360 - (float)event.orientation.x);
  Serial.print(F(", "));
  Serial.print((float)event.orientation.y);
  Serial.print(F(", "));
  Serial.print((float)event.orientation.z);
  Serial.println(F(""));

  /* The WebSerial 3D Model Viewer also expects data as roll, pitch, heading */
  // Raw quaternion - lets the viewer drive the 3D model directly without
  // gimbal-lock issues from Euler angles.
  imu::Quaternion quat = bno.getQuat();

  Serial.print(F("Quaternion: "));
  Serial.print((float)quat.w(), 4);
  Serial.print(F(", "));
  Serial.print((float)quat.x(), 4);
  Serial.print(F(", "));
  Serial.print((float)quat.y(), 4);
  Serial.print(F(", "));
  Serial.print((float)quat.z(), 4);
  Serial.println(F(""));

  /* Also send calibration data for each sensor. */
  // Each value is 0 (uncalibrated) to 3 (fully calibrated). All four should
  // reach 3 by moving the sensor through the BNO055's calibration routine.
  uint8_t sys, gyro, accel, mag = 0;
  bno.getCalibration(&sys, &gyro, &accel, &mag);
  Serial.print(F("Calibration: "));
  Serial.print(sys, DEC);
  Serial.print(F(", "));
  Serial.print(gyro, DEC);
  Serial.print(F(", "));
  Serial.print(accel, DEC);
  Serial.print(F(", "));
  Serial.print(mag, DEC);
  Serial.println(F(""));

  delay(BNO055_SAMPLERATE_DELAY_MS);
}
