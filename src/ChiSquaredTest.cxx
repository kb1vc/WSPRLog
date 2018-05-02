#include "ChiSquared.hxx"

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

  std::string what_am_i("Calculate chi-squared significance from normative and experimental histograms");

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
    std::cerr << "Filter WSPR logs by frequency range\n\t "
	      << desc << std::endl;
    exit(-1);    
  }
  catch(po::error & e) {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
    std::cerr << "Filter WSPR logs by frequency range\n\t "
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

  // now calculate the chi-sqared score
  
  std::cout << boost::format("X^2 = %g, significance = %g\n")
    % ChiSquared::pearsonCTS(S, X) % ChiSquared::test(S, X); 
}
