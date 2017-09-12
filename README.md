# NMEA

A NMEA serial parser with callbacks.

# Example
```
#include "NMEA.h"
#include <HardwareSerial.h>

HardwareSerial gpsSerial(2);
NMEA gps(&gpsSerial);

void setup()
{
  Serial.begin(230400);
  Serial.println();
  
  gps.onData([](const String& data){
    Serial.println(data);
  });
  gps.onFix([](const Fix& fix){
    Serial.print("Fix:");
    Serial.print(fix.latitude,7);
    Serial.print(" , ");
    Serial.print(fix.longitude,7);
    Serial.print(" @ ");
    Serial.print(fix.timeString());
    Serial.println();
  });
  
  gpsSerial.begin(9600);
}

void loop()
{
  gps.handle();
}
```