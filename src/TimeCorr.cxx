#include "TimeCorr.hxx"
#include <string>
#include <iostream>
#include <boost/format.hpp>
#include <cmath>
#include <ctype.h>
#include <time.h>

// offset from UTC for the specified grid
float TimeCorr::localTimeOffset(const std::string & grid) {
  char d1 = grid[0]; 
  char d2 = grid[2]; 

  float hrs_per_20deg = 20.0 * 12.0 / 180.0; 
  float fhrs = 0.0; 
  int d1_steps = (int) (toupper(d1) - 'J'); // approximately.. GMT
  float mult = (d1_steps < 0) ? -1.0 : 1.0; 
  int d2_steps = (int) (d2 - '0');

  float fhrs1 = hrs_per_20deg * ((float) d1_steps);   
  float fhrs2 = 0.1 * hrs_per_20deg * ((float) d2_steps); 

  return fhrs1 + fhrs2; 
}

float TimeCorr::circularMean(float maxpos, float a, float b)
{
  if(b < a) return circularMean(maxpos, b, a);

  if(((a < 0.0) && (b < 0.0)) ||
     ((a > 0.0) && (b > 0.0)) || 
     (a * b == 0)) {
    return 0.5 * (a + b); 
  }
  else {
    float m1, m2, ap; 
    m1 = 0.5 * (a + b);
    ap = a + maxpos; 
    m2 = 0.5 * (ap + b);
    if (m2 > (0.5 * maxpos)) {
      m2 = m2 - maxpos; 
    }
    
    float lim = maxpos * 0.25; 
    if(fabs(a - m1) <= lim) {
      return m1; 
    }
    else return m2; 
  }
}

float TimeCorr::localTimeOffset(const std::string & from, const std::string & to)
{
  float f_off = localTimeOffset(from);  // !
  float t_off = localTimeOffset(to); 
    
  return circularMean(24.0, f_off, t_off); 
}

float TimeCorr::localTime(const std::string & grid, 
			  long epoch_time)
{
  double h_off = (double) localTimeOffset(grid);

 
  struct tm ts; 
  gmtime_r(&epoch_time, &ts);

  
  float fhr = (float) ts.tm_hour;
  float fmin = (float) ts.tm_min; 
  float res = h_off + fhr + fmin / 60.0; // return hours + fraction
  if(res < -24.0) {
    std::cerr << boost::format("What is going on here.  Grid = [%s] epoch = %ld h_off = %f ts.tm_hour = %d  hr = %f ts.tm_min = %d fmin = %f res = %f\n")
      % grid % epoch_time % h_off % ts.tm_hour % fhr % ts.tm_min % fmin % res;
  }
  while(res > 24.0) res -= 24.0; 
  while(res < 0.0) res += 24.0;
  return res;  
}


float TimeCorr::localTime(const std::string & from, 
			  const std::string & to, 
			  long epoch_time)
{
  double h_off = (double) localTimeOffset(from, to);

  struct tm ts; 
  gmtime_r(&epoch_time, &ts);

  float fhr = (float) ts.tm_hour;
  float fmin = (float) ts.tm_min; 
  float res = h_off + fhr + fmin / 60.0; // return hours + fraction
  while(res > 24.0) res -= 24.0; 
  while(res < 0.0) res += 24.0;
  return res;
}
