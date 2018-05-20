#include "WSPRLog.hxx"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <math.h>
#include <set>

// filter based on line frequencies.
// delete all 0 Hz offset reports.
// write +/-50 +/-100 Hz to <basename>_50Hz.csv
// write +/-60 +/-120 Hz to <basename>_60Hz.csv
class myWSPRLog : public WSPRLog {
public:
  myWSPRLog(const std::string outf_base_name) : WSPRLog() {
    std::string out50_nm = outf_base_name + "_50Hz.csv";
    std::string out60_nm = outf_base_name + "_60Hz.csv";
    std::string outDist_nm = outf_base_name + "_D.csv";
    out50.open(out50_nm);
    out60.open(out60_nm);
    outDist.open(outDist_nm);

    int i, j; 
    for(i = -4; i < 5; i++) {
      if(i == 0) continue; 
      for(j = -2; j < 3; j++) {
	set50.insert(i * 50 + j);
	set60.insert(i * 60 + j);
      }
    }
  }

  ~myWSPRLog() { 
    out50.close();
    out60.close();
    outDist.close();
  }

  bool processEntry(WSPRLogEntry * ent) {
    int fdiff; 
    if(ent == NULL) return false;

    ent->getField(WSPRLogEntry::FREQ_DIFF, fdiff); 

    if(fdiff == 0) return true; 
    
    if(set50.find(fdiff) != set50.end()) ent->print(out50);
    else if(set60.find(fdiff) != set60.end()) ent->print(out60);
    else ent->print(outDist);     

    return true; 
  }

private:
  std::ofstream outDist, out50, out60; 
  std::set<int> set50, set60; 
}; 

int main(int argc, char * argv[])
{
  std::string in_name, out_base_name;
  bool input_gzipped; 
  namespace po = boost::program_options;


  po::options_description desc("Options:");

  desc.add_options()
    ("help", "help message")
    ("log", po::value<std::string>(&in_name)->required(), "Input log file (csv) in WSPR log format")
    ("out_base", po::value<std::string>(&out_base_name)->required(), "Basename for output logs in WSPR log format")
    ("igz", po::value<bool>(&input_gzipped)->default_value(false), "if true, input file is gzip compressed");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("log", 1);
  pos_opts.add("out_base", 1);
    
  po::variables_map vm; 

  try {
    po::store(po::command_line_parser(argc, argv).options(desc)
	      .positional(pos_opts).run(), vm);
    
    if(vm.count("help")) {
      std::cout << "Filter WSPR logs by frequency range\n\t"
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


  myWSPRLog wlog(out_base_name);

  wlog.readLog(in_name, input_gzipped);
  
  // call processEntry one last time, to see if we've
  // got something stuck in the pipeline. 
  wlog.processEntry(NULL); 
}
