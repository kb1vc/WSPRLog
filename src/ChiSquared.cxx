// functions to calculate X^2 (Pearson's cumulative test statistic)
// and significance (1 - P(X^2/2, k/2)) for k categories. 

#include <boost/math/special_functions/gamma.hpp>
#include <boost/format.hpp>

#include <iostream>
#include <cmath>
#include <vector>
#include "ChiSquared.hxx"

double ChiSquared::pearsonCTS(const std::vector<int> & S, 
		  const std::vector<int> & X)
{
  // The inputs S[i] and X[i] are in the form of "number of samples in
  // category [i] for the "standard" case (the histogram that fits
  // the null hypothesis) and the sample to be tested. 
  
  // first make the p[i] (normative probability of an occurance in
  // bucket [i].
  int num_Ssamps = 0;
  int N = 0;   
  for(int i = 0; i < S.size(); i++) {
    num_Ssamps += S[i];
    N += X[i];     
  }

  std::vector<double> p; 
  double rtsamps = 1.0 / ((double) num_Ssamps); 
  for(int i = 0; i < S.size(); i++) {
    double v = (double) S[i]; 
    p.push_back(v * rtsamps);
  }

  double X2 = 0.0; 
  for(int i = 0; i < S.size(); i++) {
    double m = N * p[i]; 
    double diff = ((double) X[i]) - m;
    double a = diff * diff / m;
    X2 += a; 
  }

  return X2; 
}

double ChiSquared::test(const std::vector<int> & S, const std::vector<int> X)
{
  int k = S.size(); 
  
  return 1.0 - boost::math::gamma_p(((double) k) * 0.5, pearsonCTS(S, X) * 0.5);
}


