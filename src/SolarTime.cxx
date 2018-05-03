// Calculate solar time from UTC and longitude
#include "SolarTime.hxx"

// Solar time calculation from https://www.esrl.noaa.gov/gmd/grad/solcalc/solareqns.PDF
#include <cmath>
#include <ctype.h>

SolarTime::SolarTime(long epoch_time, double longitude)
{
  init(epoch_time, longitude); 
}

SolarTime::SolarTime(long epoch_time, const std::string & grid)
{
  double longitude; 
  char d1 = grid[0]; 
  char d2 = grid[2];
  int d1_steps = (int) (toupper(d1) - 'J'); // approximately.. GMT
  int d2_steps = (int) (d2 - '0');
  int d3_steps = 0; 
  if(grid.length() > 4) {
    char d3 = grid[4];       
    d3_steps = (int) (toupper(d3) - 'A'); 
  }

  longitude = 20.0 * ((float) d1_steps) + 2.0 * ((float) d2_steps)
    + ((float) d3_steps) / 12.0; 

  init(epoch_time, longitude); 
}



void SolarTime::init(long epoch_time, double longitude)
{
  // convert epoch time to UTC
  gmtime_r(&epoch_time, &utc); 

  // calculate fractional year -- gamma
  double f_day_of_year = (double) utc.tm_yday; 
  double f_hour = (double) utc.tm_hour;
  double gamma = (2.0 * M_PI / 365.0) * 
    (f_day_of_year - 1.0 + ((f_hour - 12.0)/24.0));

  // estimate "equation of time" in minutes
  double eqtime = 229.18 * (0.000075
			  + 0.001868 * cos(gamma)
			  - 0.032077 * sin(gamma)
			  - 0.014514 * cos(2.0 * gamma)
			  - 0.040849 * sin(2.0 * gamma)); 

  double offset_mins = eqtime + 4.0 * longitude; 

  long offset_seconds = (long) floor(offset_mins * 60.0); 

  // now setup the solar time... 
  solar_epoch_time = epoch_time + offset_seconds; 
  gmtime_r(&solar_epoch_time, &solar); 

}
