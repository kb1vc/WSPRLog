#include "WSPRLog.hxx"
#include "SolarTime.hxx"
#include "TimeCorr.hxx"
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <math.h>
#include <set>
#include <vector>

// translate the timestamp field of each log entry into an INT
// representing the solar time in minutes (at a granularity of 15 min)

const int MINUTES_PER_BUCKET = 2;
const int BUCKETS_PER_TABLE = ((24 * 60) / MINUTES_PER_BUCKET);
const int BUCKETS_PER_HOUR = (60 / MINUTES_PER_BUCKET);

class myWSPRLog : public WSPRLog {
public:
  myWSPRLog(const std::string fname) : WSPRLog() {
    os.open(fname); 
  }

  ~myWSPRLog() {
    os.close();
  }

  bool processEntry(WSPRLogEntry * ent) {
    if(ent == NULL) return false; 
    
    // calculate solar time for rx, tx, and midpoint 
    SolarTime tx_time(ent->dtime, ent->txgrid);
    SolarTime rx_time(ent->dtime, ent->rxgrid);    
    float mid_hour = TimeCorr::circularMean(24.0, tx_time.getFHour(), rx_time.getFHour());

    ent->dtime = ((int) floor(mid_hour * 60.0)); // minutes past the hour

    ent->print(os);
    
    return false; 
  }


private:
  std::ofstream os; 
}; 

int main(int argc, char * argv[])
{
  bool input_gzipped; 
  std::string in_name, out_name; 
  namespace po = boost::program_options;

  po::options_description desc("Options:");

  desc.add_options()
    ("help", "help message")
    ("in", po::value<std::string>(&in_name)->required(), "WSPR log for baseline reports")
    ("out", po::value<std::string>(&out_name)->required(), "WSPR log for baseline reports");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("in", 1);
  pos_opts.add("out", 1);  

  po::variables_map vm; 

  std::string what_am_i("Convert epoch timestamp to minutes past solar midnight\n");
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

  myWSPRLog wlog(out_name);

  wlog.readLog(in_name);
}
