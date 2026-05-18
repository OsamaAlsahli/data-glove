// adafruit_test.ino - Bring-up sketch for a single BNO055
//
// First sketch flashed to the ESP32 during hardware bring-up. Verifies that
// one BNO055 connected directly (no multiplexer) responds and produces sane
// Euler-angle orientation data. Used as a "is the sensor alive?" smoke test
// before introducing the TCA9548A multiplexer.

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

// id=55 is an arbitrary tag for the Adafruit library; 0x29 is the BNO055
// address with ADR tied high.
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x29);

void setup(void)
{
  Serial.begin(115200);
  Serial.println("Orientation Sensor Test"); Serial.println("");
  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  delay(1000);
  bno.setExtCrystalUse(true);   // use the external 32 kHz crystal for stability
}

void loop(void)
{
  /* Get a new sensor event */
  sensors_event_t event;
  bno.getEvent(&event);
  /* Display the floating point data - Euler angles in degrees */
  Serial.print("X: ");
  Serial.print(event.orientation.x, 4);
  Serial.print("\tY: ");
  Serial.print(event.orientation.y, 4);
  Serial.print("\tZ: ");
  Serial.print(event.orientation.z, 4);
  Serial.println("");
  delay(100);
}
