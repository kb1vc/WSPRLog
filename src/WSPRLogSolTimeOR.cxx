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

// accumulate number of reports per solar hour for a
// image and normal log reports.  Print the odds ratio
//  associated with each hour of the day for
// solar time at the receiver, transmitter, and halfway point
// 
// That is OR = [(events at time = T) * (non-events over 24 hours)] /
//              [(events in 24 hours) * (non-events at time = T)]
// 
const int MINUTES_PER_BUCKET = 15;
const int BUCKETS_PER_TABLE = ((24 * 60) / MINUTES_PER_BUCKET);
const int BUCKETS_PER_HOUR = (60 / MINUTES_PER_BUCKET);

class myWSPRLog : public WSPRLog {
public:
  myWSPRLog() : WSPRLog() {
    exc_mode = false;
    initCounts();
  }

  ~myWSPRLog() {
  }

  enum Position { RX, TX, MID };
  // counts [0] is standard, [1] is exception
  unsigned int rx_counts[2], tx_counts[2], mid_counts[2]; 

  std::vector<unsigned int> rx_histo[2];
  std::vector<unsigned int> tx_histo[2];
  std::vector<unsigned int> mid_histo[2];

  void initCounts() {
    rx_counts[0] = rx_counts[1] = 0;
    tx_counts[0] = tx_counts[1] = 0;    
    mid_counts[0] = mid_counts[1] = 0;    

    for(int m = 0; m < 2; m++) {
      rx_histo[m].resize(BUCKETS_PER_TABLE, 0);
      tx_histo[m].resize(BUCKETS_PER_TABLE, 0);
      mid_histo[m].resize(BUCKETS_PER_TABLE, 0);            
    }
  }

  void bump(Position pos, float hour) {
    int mode = exc_mode ? 1 : 0; 
    int tim = (int) (floor(hour * ((float) BUCKETS_PER_HOUR)));
    if(tim > BUCKETS_PER_TABLE) {
      std::cerr << boost::format(" hour = %f  Time index is %d. How did that happen?\n") % hour % tim; 
      exit(-1);
    }

    switch (pos) {
    case RX:
      rx_histo[mode][tim]++; 
      rx_counts[mode]++; 
      break; 
    case TX:
      tx_histo[mode][tim]++; 
      tx_counts[mode]++; 
      break; 
    case MID:
      mid_histo[mode][tim]++; 
      mid_counts[mode]++; 
      break; 
    }
  }



  void setExcMode(bool fl) { exc_mode = fl; }

  bool processEntry(WSPRLogEntry * ent) {
    if(ent == NULL) return false; 
    
    // calculate solar time for rx, tx, and midpoint 
    SolarTime tx_time(ent->dtime, ent->txgrid);
    SolarTime rx_time(ent->dtime, ent->rxgrid);    
    float mid_hour = TimeCorr::circularMean(24.0, tx_time.getFHour(), rx_time.getFHour());

    std::cerr << boost::format("rx time  %f  tx_time %f  mid_time = %f\n")
      % rx_time.getFHour() % tx_time.getFHour() % mid_hour; 

    bump(RX, rx_time.getFHour());
    bump(TX, tx_time.getFHour());
    bump(MID, mid_hour);

    return true; 
  }

// accumulate number of reports per solar hour for a
// image and normal log reports.  Print the odds ratio
//  associated with each hour of the day for
// solar time at the receiver, transmitter, and halfway point
// 
// That is OR = [(events at time = T) * (non-events over 24 hours)] /
//              [(events in 24 hours) * (non-events at time = T)]
// 

  void calcOR(Position pos, std::vector<float> & ort) {
    std::vector<unsigned int> & exc_histo = rx_histo[1];
    std::vector<unsigned int> & norm_histo = rx_histo[0];     
    float exc_count;
    float norm_count; 
    for(int i = 0; i < BUCKETS_PER_TABLE; i++) {
      ort.push_back(0.0);
    }

    switch (pos) {
    case RX:
      exc_histo = rx_histo[1]; 
      norm_histo = rx_histo[0]; 
      exc_count = (float) rx_counts[1];
      norm_count = (float) (rx_counts[0] - rx_counts[1]) ;
      break; 
    case TX:
      exc_histo = tx_histo[1]; 
      norm_histo = tx_histo[0]; 
      exc_count = (float) tx_counts[1];
      norm_count = (float) (tx_counts[0] - tx_counts[1]) ;
      break; 
    case MID:
      exc_histo = mid_histo[1]; 
      norm_histo = mid_histo[0]; 
      exc_count = (float) mid_counts[1];
      norm_count = (float) (mid_counts[0] - mid_counts[1]) ;
      break; 
    }

    for(int i = 0; i < BUCKETS_PER_TABLE; i++) {
      float De = (float) exc_histo[i]; 
      float He = (float) (norm_histo[i] - exc_histo[i]);

      ort[i] = (De * norm_count) / (He * exc_count);
    }
  }

  void dumpTables(const std::string & fname) {

    std::ofstream os(fname); 

    std::vector<float> rx_or, tx_or, mid_or; 
    calcOR(RX, rx_or);
    calcOR(TX, tx_or);
    calcOR(MID, mid_or);

    // we go by quarter hour.  
    os << "# solar hour, rximage reports, rxall reports, tximage reps, txall reps, midimage reps, imgall reps, rxOR, txOR, midOR\n";

    for(int i = 0; i < BUCKETS_PER_TABLE; i++) {
      os << boost::format("%5.2f, ") % (((float) i) / 4.0);
      os << boost::format(" %8d, %8d, %8d, %8d, %8d, %8d, ") 
	% rx_histo[1][i] % rx_histo[0][i] 
	% tx_histo[1][i] % tx_histo[0][i]
	% mid_histo[1][i] % mid_histo[0][i];
      os << boost::format("%g %g %g\n")
	% rx_or[i] % tx_or[i] % mid_or[i]; 
    }

    os.close();
  }

private:
  bool exc_mode; 
}; 

int main(int argc, char * argv[])
{
  bool input_gzipped; 
  std::string std_name, exc_name, report_name; 
  namespace po = boost::program_options;

  po::options_description desc("Options:");

  desc.add_options()
    ("help", "help message")
    ("standard", po::value<std::string>(&std_name)->required(), "WSPR log for baseline reports")
    ("exception", po::value<std::string>(&exc_name)->required(), "WSPR log for baseline reports")
    ("report", po::value<std::string>(&report_name)->required(), "Output data file containing calls, and risk ratios");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("standard", 1);
  pos_opts.add("exception", 1);  
  pos_opts.add("report", 1);

  po::variables_map vm; 

  std::string what_am_i("Relative Probability of Exceptional Report by RX and TX Call Sign\n");
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



  myWSPRLog wlog;

  wlog.setExcMode(true);
  wlog.readLog(exc_name, input_gzipped);
  
  wlog.setExcMode(false);
  wlog.readLog(std_name, input_gzipped);
  
  wlog.dumpTables(report_name);
}
