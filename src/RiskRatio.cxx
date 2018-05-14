
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <cmath>

int main(int argc, char * argv[])
{
  std::string S_fname, X_fname, out_fname; 

  namespace po = boost::program_options;

  po::options_description desc("Options:");
  desc.add_options()
    ("help", "help message")
    ("standard", po::value<std::string>(&S_fname)->required(), "Histogram file for null hypothesis")
    ("experiment", po::value<std::string>(&X_fname)->required(), "Histogram file for experimental result")
    ("out", po::value<std::string>(&out_fname)->required(), "Output file to contain PDF, CDF, pdf(X)/pdf(S)");

  
  po::positional_options_description pos_opts ;
  pos_opts.add("standard", 1);
  pos_opts.add("experiment", 1);
  pos_opts.add("out", 1);     
    
  po::variables_map vm; 

  std::string what_am_i("Calculate \"risk ratio\" statistic");

  try {
    po::store(po::command_line_parser(argc, argv).options(desc)
	      .positional(pos_opts).run(), vm);
    
    if(vm.count("help")) {
      std::cout << what_am_i
		<< desc << std::endl; 
      exit(-1);
    }

    po::notify(vm);
  }
  catch(po::required_option & e) {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
    std::cerr << what_am_i
	      << desc << std::endl;
    exit(-1);    
  }
  catch(po::error & e) {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
    std::cerr << what_am_i
	      << desc << std::endl;
    exit(-1);        
  }

  std::ifstream Sst(S_fname); 
  std::ifstream Xst(X_fname); 
  
  std::vector<int> S; 
  std::vector<int> X; 
  
  for(int n; Sst >> n; ) {
    S.push_back(n); 
  }
  for(int n; Xst >> n; ) {
    X.push_back(n); 
  }


  std::ofstream out(out_fname); 
  
  int x_sum, s_sum; 
  x_sum = s_sum = 0; 
  int i; 
  for(i = 0; i < S.size(); i++) {
    s_sum += S[i];
    x_sum += X[i]; 
  }


  float r_x_sum = 1.0 / ((float) x_sum);
  float r_s_sum = 1.0 / ((float) s_sum); 

  float x_cdf = 0.0; 
  float s_cdf = 0.0; 

  float norm = ((float) x_sum) / ((float) s_sum);
  for(i = 0; i < S.size(); i++) {
    float s = ((float) S[i]) * r_s_sum;
    float x = ((float) X[i]) * r_x_sum;     
    
    float r = ((float) X[i]) / ((float) S[i]);
    float rr = r / norm; 

    s_cdf += s; 
    x_cdf += ((float) X[i]) * r_x_sum; 
    out << boost::format("%d %f %f %f %f %f\n")
      % i % s % s_cdf % x % x_cdf % rr; 
  }
}
