#ifndef KS_HDR
#define KS_HDR

#include <vector>

namespace KolmogorovSmirnov {
  std::vector<float> cdf(const std::vector<int> & S, int & samples);
  bool test(float alpha, const std::vector<int> & S, const std::vector<int> & X);  
}
#endif
