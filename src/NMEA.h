#include <Arduino.h>

#include <Stream.h>
#include <functional>

#define BUFFER_SIZE 160

struct Fix
{
  int time;
  float latitude;
  float longitude;
  uint8_t quality = quality;
  uint8_t satallites = satallites;
  float dilution = dilution;
  float altitude = altitude;
  float height = height;
  
  String timeString() const
  {
    String str;
    int h = time / 3600;
    int m = time / 60 % 60;
    int s = time % 60;

    str += (char)('0' + h / 10);
    str += (char)('0' + h % 10);
    str += ':';
    str += (char)('0' + m / 10);
    str += (char)('0' + m % 10);
    str += ':';
    str += (char)('0' + s / 10);
    str += (char)('0' + s % 10);

    return str;
  }
};

struct Status
{
  char mode;
  uint8_t fixType;
  uint8_t prn[12];
  float delution;
  float horizontalDelution;
  float verticalDelution;
};

class NMEA
{
  private:
    Stream* serial;
    uint8_t index;
    char buffer[BUFFER_SIZE];
    std::function<void(const String&)> onDataCallback;
    std::function<void(const Fix&)> onFixCallback;
    std::function<void(const Status&)> onStatusCallback;
    
  public:
    NMEA(Stream* serial);
    void onFix(std::function<void(const Fix&)> f);
    void onData(std::function<void(const String&)> f);
    void onStatus(std::function<void(const Status&)> f);
    void handle();

  private:
    void parse();
    void parseGGA();
    void parseGSA();
    
    bool checksum(String& data);
    
    void next(String& value, String& data, uint8_t& start, uint8_t& end);
    void next(uint8_t& value, String& data, uint8_t& start, uint8_t& end);
    void next(int& value, String& data, uint8_t& start, uint8_t& end);
    void next(float& value, String& data, uint8_t& start, uint8_t& end);
    void next(String& data, uint8_t& start, uint8_t& end);
    void nextGeo(float& value, String& data, uint8_t& start, uint8_t& end);
};
