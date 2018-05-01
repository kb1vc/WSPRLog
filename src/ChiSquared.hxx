#ifndef CHISQUARED_HDR
#define CHISQUARED_HDR

#include <vector>

namespace ChiSquared {
  double pearsonCTS(const std::vector<int> & S, 
		  const std::vector<int> & X);
  double test(const std::vector<int> & S, const std::vector<int> X);  
}
#endif
