// tst2.ino - TCA9548A I2C scanner
//
// Diagnostic sketch. For every channel (0-7) on the TCA9548A multiplexer it
// scans the full 7-bit I2C address range and prints any device that
// acknowledges. Used during bring-up to confirm each BNO055 is actually
// present on the channel it is supposed to be on, and to catch address
// clashes early.

#include "Wire.h"

#define TCAADDR 0x70

// Select channel `i` on the multiplexer (0-7). Writing a one-hot byte to
// the TCA9548A's control register enables that channel and disables the rest.
void tcaselect(uint8_t i) {
  if (i > 7) return;

  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
}


void setup(){
  Wire.begin();
  delay(1000);
  Serial.begin(115200);
  Serial.println("\nTCAScanner ready!");

  // For each mux channel, probe every possible I2C address and print hits.
  for (uint8_t t=0; t<8; t++) {
    tcaselect(t);
    Serial.print("TCA Port #"); Serial.println(t);
    for (uint8_t addr = 0; addr<=127; addr++) {
      if (addr == TCAADDR) continue;   // skip the mux itself
      Wire.beginTransmission(addr);
      if (!Wire.endTransmission()) {
        // endTransmission returns 0 on ACK - device present at this address.
        Serial.print("Found I2C 0x");  Serial.println(addr,HEX);
      }
    }
  }
  Serial.println("\ndone");
}

void loop()
{
  // One-shot diagnostic - nothing to do after setup().
}
