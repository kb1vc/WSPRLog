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

  // check
  double sum_c = 0.0; 
  for(double pi: p) {
    sum_c += pi; 
  }
  std::cerr << "sum_c = " << sum_c << " (1-sum_c) = " << (1.0 - sum_c) << std::endl; 

  double X2 = -1.0 * ((double) N);
  
  for(int i = 0; i < S.size(); i++) {
    double m = N * p[i]; 
    double xi = ((double) X[i]); 
    
    X2 += (xi * xi / m); 
  }

  return X2; 
}

double ChiSquared::test(const std::vector<int> & S, const std::vector<int> X)
{
  int k = S.size(); 
  
  return 1.0 - boost::math::gamma_p(((double) k) * 0.5, pearsonCTS(S, X) * 0.5);
}


