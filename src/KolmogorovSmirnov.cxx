#include "KolmogorovSmirnov.hxx"
#include <boost/format.hpp>
#include <cmath>
#include <iostream>

std::vector<float> KolmogorovSmirnov::cdf(const std::vector<int> & S, int & N)
{
  std::vector<float> ret; 

  N = 0;
  for(int i = 0; i < S.size(); i++) {
    N += S[i]; 
  }

  float fscale = 1.0 / ((float) N); 

  float cdf = 0.0;   
  for(int i = 0; i < S.size(); i++) {
    cdf += ((float) S[i]) * fscale; 
    ret.push_back(cdf); 
  }

  return ret; 
}

bool KolmogorovSmirnov::test(float alpha, 
			     const std::vector<int> & S, 
			     const std::vector<int> & X) 
{
  int N, M; 
  std::vector<float> Scdf = cdf(S, N);
  std::vector<float> Xcdf = cdf(X, M);
  
  float max_sep = 0.0; 
  for(int i = 0; i < S.size(); i++) {
    float sep = fabs(Scdf[i] - Xcdf[i]);
    max_sep = (max_sep > sep) ? max_sep : sep; 
  }

  // now is the difference significant? 
  float n = (float) N;
  float m = (float) M; 

  float calpha = sqrt(-0.5 * log(alpha / 2.0));
  
  float Dmin = calpha * sqrt((n + m) / (n*m));

  std::cerr << boost::format("Dnm = %f n = %f m = %f calpha = %f Dmin = %f\n")
    % max_sep % n % m % calpha % Dmin; 

  return max_sep  > Dmin; 
}
