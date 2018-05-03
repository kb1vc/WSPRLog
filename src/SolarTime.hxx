#ifndef SOLARTIME_HDR
#define SOLARTIME_HDR
#include <time.h>
#include <string>


class SolarTime { 
public:
  SolarTime(long epoch_time, double longitude);
  SolarTime(long epoch_time, const std::string & grid);

  void init(long epoch_time, double longitude);    

  int getIHour() { return solar.tm_hour + ((solar.tm_min > 30) ? 1 : 0); }  
  float getFHour() { return ((float) solar.tm_hour) + ((float) solar.tm_min) / 60.0; }

  struct tm utc; 
  struct tm solar;
  long solar_epoch_time; 
}; 

#endif
