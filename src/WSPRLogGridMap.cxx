#include "WSPRLog.hxx"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <math.h>
#include <set>

// build a heat map for the major grid divisions (AA to RR)
// from the maidenhead grid locator
class myWSPRLog : public WSPRLog {
public:
  myWSPRLog() : WSPRLog() {
    for(char f = 'A'; f <= 'R'; f++) {
      for(char s = 'A'; s <= 'R'; s++) {
	std::pair<char, char> k(f, s);
	rx_grid_map[k] = 0; 
	tx_grid_map[k] = 0; 
      }
    }
  }

  ~myWSPRLog() { 
  }

  bool processEntry(WSPRLogEntry * ent) {
    int fdiff; 
    if(ent == NULL) return false;

    std::string rxgrid, txgrid; 
    ent->getField(WSPRLogEntry::RXGRID, rxgrid);
    ent->getField(WSPRLogEntry::RXGRID, txgrid);     

    std::pair<char, char> rxkey(std::toupper(rxgrid[0]), std::toupper(rxgrid[1]));
    std::pair<char, char> txkey(std::toupper(txgrid[0]), std::toupper(txgrid[1]));    
    if(rx_grid_map.find(rxkey) != rx_grid_map.end()) {
      rx_grid_map[rxkey] += 1; 
    }
    else {
      std::cerr << boost::format("Couldn't find key for rx grid [%s]\n") % rxgrid; 
    }
    if(tx_grid_map.find(txkey) != tx_grid_map.end()) {
      tx_grid_map[txkey] += 1; 
    }
    else {
      std::cerr << boost::format("Couldn't find key for tx grid [%s]\n") % txgrid; 
    }
    return false;
  }

  void report(std::string & ofname) {
    std::ofstream os(ofname); 
    for(char f = 'A'; f <= 'R'; f++) {
      for(char s = 'A'; s <= 'R'; s++) {      
	std::pair<char, char> k(f, s); 
	int fi = f - 'A'; 
	int si = s - 'A'; 
	os << boost::format("%c %c %d %d %d %d\n")
	  % f % s % fi % si % rx_grid_map[k] % tx_grid_map[k]; 
      }
      os << std::endl; 
    }
    os.close(); 
  }

private:
  std::map<std::pair<char, char>, int> rx_grid_map;
  std::map<std::pair<char, char>, int> tx_grid_map;   
}; 

int main(int argc, char * argv[])
{
  std::string in_name, out_name;
  bool input_gzipped; 
  namespace po = boost::program_options;


  po::options_description desc("Options:");

  desc.add_options()
    ("help", "help message")
    ("log", po::value<std::string>(&in_name)->required(), "Input log file (csv) in WSPR log format")
    ("out", po::value<std::string>(&out_name)->required(), "Output table")
    ("igz", po::value<bool>(&input_gzipped)->default_value(false), "if true, input file is gzip compressed");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("log", 1);
  pos_opts.add("out", 1);
    
  po::variables_map vm; 

  try {
    po::store(po::command_line_parser(argc, argv).options(desc)
	      .positional(pos_opts).run(), vm);
    
    if(vm.count("help")) {
      std::cout << "Build heat map by major grid\n"
		<< desc << std::endl; 
      exit(-1);
    }

    po::notify(vm);
  }
  catch(po::required_option & e) {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
    std::cerr << "Build heat map by major grid\n"
	      << desc << std::endl;
    exit(-1);    
  }
  catch(po::error & e) {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
    std::cerr << "Build heat map by major grid\n"
	      << desc << std::endl;
    exit(-1);        
  }


  myWSPRLog wlog;

  wlog.readLog(in_name, input_gzipped);
  

  wlog.report(out_name); 
}
