#include <iostream>
#include <boost/format.hpp>
#include <cmath>

bool bkt_is_ok(float bkt) {
  return (bkt != 60.0) 
    && (bkt != -60.0)
    && (bkt != 120.0)
    && (bkt != -120.0)
    && (bkt != 50.0) 
    && (bkt != -50.0)
    && (bkt != 100.0)
    && (bkt != -100.0);
}

int main(int argc, char * argv[]) 
{
  float bkt; 
  float prop; 
  int count = 0; 
  float sum = 0.0;
  float sumsq = 0.0;
  bool flag = true; 
  while(flag) {
    std::cin >> bkt >> prop;
    flag = !std::cin.eof();
    if(flag) {
      if(bkt_is_ok(bkt)) {
	count++;	
	sum += bkt * prop; 
	sumsq += (bkt * bkt * prop);
      }
    }
  }

  float fcount = float(count); 
  // variance is E[X^2] - (E[X])^2
  
  float var = sumsq - sum*sum; 
  float sdev = sqrt(var); 
  std::cout << boost::format("mean = %g  var = %g  sdev = %g\n")
    % sum % var % sdev; 
}

