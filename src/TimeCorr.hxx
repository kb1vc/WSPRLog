#ifndef TIMECORR_HDR
#define TIMECORR_HDR
#include <string>
namespace TimeCorr {
  // offset from UTC for the specified grid
  float localTimeOffset(const std::string & grid);

  float circularMean(float maxpos, float a, float b);

  float localTimeOffset(const std::string & from, 
			const std::string & to);

  float localTime(const std::string & from, 
		  const std::string & to, 
		  long epoch_time);

  float localTime(const std::string & grid, 
		  long epoch_time);
}
#endif
