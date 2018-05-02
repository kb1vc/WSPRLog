#include "KolmogorovSmirnov.hxx"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>

int main(int argc, char * argv[])
{
  std::string S_fname, X_fname; 

  namespace po = boost::program_options;

  po::options_description desc("Options:");
  desc.add_options()
    ("help", "help message")
    ("standard", po::value<std::string>(&S_fname)->required(), "Histogram file for null hypothesis")
    ("experiment", po::value<std::string>(&X_fname)->required(), "Histogram file for experimental result");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("standard", 1);
  pos_opts.add("experiment", 1);
    
  po::variables_map vm; 

  std::string what_am_i("Calculate Kolmogorov-Smirnov significance from normative and experimental histograms");

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

  // now calculate the KS score
  
  float a = 0.5; 
  float sv[] = {5.0, 2.0, 1.0}; 
  for(float a = 0.1; a > 0.0001; a = 0.1 * a) {
    for(int i = 0; i < 3; i++) {
      float aa = a * sv[i] ; 
      bool reject = KolmogorovSmirnov::test(aa, S, X); 
      std::cout << boost::format("alpha = %f  reject = %c\n")
	% aa % ((char) (reject ? 'T' : 'F'));
    }
  }
}
