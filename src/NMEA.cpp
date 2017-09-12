#include "NMEA.h"

NMEA::NMEA(Stream* _serial)
:
serial(_serial)
{}

void NMEA::onFix(std::function<void(const Fix&)> f)
{
  onFixCallback = f;
}

void NMEA::onData(std::function<void(const String&)> f)
{
  onDataCallback = f;
}

void NMEA::handle()
{
  while (serial->available())
  {
    char c = serial->read();
    buffer[index++] = c;
    if(index >= BUFFER_SIZE)
    {
      index = 0;
      Serial.println("Buffer overflow");
    }
    if (c == '\n')
    {
      buffer[index - 1] = 0;
      parse();
      index = 0;
    }
  }
}

void NMEA::next(String& value, String& data, uint8_t& start, uint8_t& end)
{
  start = end + 1;
  end = data.indexOf(',', start);
  value = data.substring(start, end);  
}

void NMEA::next(int& value, String& data, uint8_t& start, uint8_t& end)
{
  start = end + 1;
  end = data.indexOf(',', start);
  value = data.substring(start, end).toInt();
}

void NMEA::next(uint8_t& value, String& data, uint8_t& start, uint8_t& end)
{
  start = end + 1;
  end = data.indexOf(',', start);
  value = data.substring(start, end).toInt();
}

void NMEA::next(float& value, String& data, uint8_t& start, uint8_t& end)
{
  start = end + 1;
  end = data.indexOf(',', start);
  value = data.substring(start, end).toInt();
}

void NMEA::next(String& data, uint8_t& start, uint8_t& end)
{
  start = end + 1;
  end = data.indexOf(',', start);
}

void NMEA::nextGeo(float& value, String& data, uint8_t& start, uint8_t& end)
{
  start = end + 1;
  end = data.indexOf(',', start);
  int dend = data.indexOf('.', start) - 2;
  int degrees = data.substring(start, dend).toInt();
  float minutes = data.substring(dend, end).toFloat();
  value = degrees + minutes / 60.;
  start = end + 1;
  end = data.indexOf(',', start);
  char w = data.charAt(start);
  if (w == 'W' || w == 'S')
    value = -value;
}

/*
$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39

Where:
    GSA      Satellite status
    A        Auto selection of 2D or 3D fix (M = manual) 
    3        3D fix - values include: 1 = no fix
                                      2 = 2D fix
                                      3 = 3D fix
    04,05... PRNs of satellites used for fix (space for 12) 
    2.5      PDOP (dilution of precision) 
    1.3      Horizontal dilution of precision (HDOP) 
    2.1      Vertical dilution of precision (VDOP)
    *39      the checksum data, always begins with *  
*/
void NMEA::parseGSA()
{
  String data(buffer);
  if (!checksum(data)) return;

  Status status;
  uint8_t start = data.indexOf(',');
  uint8_t end = start;

  String mode;
  next(mode, data, start, end);
  status.mode = mode.charAt(0);
  
  next(status.fixType, data, start, end);
  for (uint8_t i = 0 ; i < 12 ; ++i) next(status.prn[i], data, start, end);
  next(status.delution, data, start, end);
  next(status.horizontalDelution, data, start, end);
  next(status.verticalDelution, data, start, end);

  onStatusCallback(status);
}

void NMEA::parseGGA()
{
  String data(buffer);
  if (!checksum(data)) return;

  Fix fix;
  uint8_t start = data.indexOf(',');
  uint8_t end = start;

  String time;
  next(time, data, start, end);
  int hours = time.substring(0, 2).toInt();
  int minus = time.substring(2, 4).toInt();
  int secos = time.substring(4, 6).toInt();
  fix.time = hours * 3600 + minus * 60 + secos;

  nextGeo(fix.latitude, data, start, end);
  nextGeo(fix.longitude, data, start, end);
  next(fix.quality, data, start, end);
  next(fix.satallites, data, start, end);
  next(fix.dilution, data, start, end);
  next(fix.altitude, data, start, end);
  next(data, start, end);
  next(fix.height, data, start, end);
  next(data, start, end);
  next(data, start, end);

  start = end + 1;
  end = data.indexOf(0, start);
  int checksum = data.substring(start+1, end).toInt();
  
  onFixCallback(fix);
}

bool NMEA::checksum(String& data)
{
  char sum = 0;
  for(uint8_t i = 1; i < data.indexOf('*'); ++i)
    sum ^= data.charAt(i);
  unsigned long check = strtoul(data.substring(data.indexOf('*')+1, data.length()).c_str(), NULL, 16);
  return check == sum;
}

void NMEA::parse()
{
  String data(buffer);
  if (onDataCallback) onDataCallback(data);

  if (strncmp(buffer, "$GPGGA", 6) == 0 && onFixCallback)
  {
    parseGGA();
    return;
  }

  if (strncmp(buffer, "$GPGSA", 6) == 0 && onStatusCallback)
  {
    parseGSA();
    return;
  }
}
