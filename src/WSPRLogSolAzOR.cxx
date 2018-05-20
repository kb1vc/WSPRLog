#include "WSPRLog.hxx"
#include "SolarTime.hxx"
#include "TimeCorr.hxx"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <math.h>
#include <set>

// plot distance, azimuth pairs.
const int NUM_TIME_BUCKETS = (24 * 10);
const int NUM_AZ_BUCKETS = (360 / 5);
  
class myWSPRLog : public WSPRLog {
public:
  myWSPRLog() : WSPRLog() {
    total_reports[0] = 0;
    total_reports[1] = 0;     
    mode_index = 0; 
    for(int tbucket = 0; tbucket < NUM_TIME_BUCKETS; tbucket++) {
      for(int azbucket = 0; azbucket < NUM_AZ_BUCKETS; azbucket++) {
	histo[0][tbucket][azbucket] = 0;
	histo[1][tbucket][azbucket] = 0; 	
      }
    }
  }

  void setExcMode(bool fl) {
    mode_index = fl ? 1 : 0; 
  }

  bool processEntry(WSPRLogEntry * ent) {
    int x, y; 
    

    if(ent == NULL) return false; 

    SolarTime tx_time(ent->dtime, ent->txgrid);
    SolarTime rx_time(ent->dtime, ent->rxgrid);    
    float mid_hour = TimeCorr::circularMean(24.0, tx_time.getFHour(), rx_time.getFHour());
    bump(mid_hour, ent->az); 
    return true; 
  }


  void bump(float t_hour, float az) {
    // quantize into 10 minute buckets
    int time_bucket = ((int) floor(t_hour * 6.0));
    // quantize into 5 degree segments
    int az_bucket = (int) (az / 5.0); 
    if(time_bucket >= NUM_TIME_BUCKETS) time_bucket = (NUM_TIME_BUCKETS - 1);
    if(az_bucket >= NUM_AZ_BUCKETS) az_bucket = (NUM_AZ_BUCKETS - 1);    
    histo[mode_index][time_bucket][az_bucket]++;
    total_reports[mode_index]++; 
  }

  void writeReport(const std::string & out_name) {
    std::ofstream os(out_name);    
    float count_ratio = ((float) total_reports[0]) / ((float) total_reports[1]); 

    for(int tbucket = 0; tbucket < NUM_TIME_BUCKETS; tbucket++) {
      for(int azbucket = 0; azbucket < NUM_AZ_BUCKETS; azbucket++) {
	float exc_val = (float) histo[1][tbucket][azbucket];
	float std_val = (float) histo[0][tbucket][azbucket]; 	

        float De = exc_val; 
	float He = std_val - exc_val; 
	float OR;
	if(He > 0.0) OR = (De / He) * count_ratio; 
	else OR = 0.0; 
	os << boost::format("%d %d %f\n")
	  % (tbucket * 10) % (azbucket * 5) % OR; 
      }
    }
    os.close();
  }

private:
  int histo[2][NUM_TIME_BUCKETS][NUM_AZ_BUCKETS]; 
  int total_reports[2]; 
  int mode_index; 
}; 

int main(int argc, char * argv[])
{
  std::string std_name, exc_name, report_name; 
  bool input_gzipped; 
  namespace po = boost::program_options;


  po::options_description desc("Options:");

  desc.add_options()
    ("help", "help message")
    ("standard", po::value<std::string>(&std_name)->required(), "WSPR log for baseline reports")
    ("exception", po::value<std::string>(&exc_name)->required(), "WSPR log for baseline reports")
    ("report", po::value<std::string>(&report_name)->required(), "Output data file containing calls, and risk ratios")
    ("igz", po::value<bool>(&input_gzipped)->default_value(false), "if true, input file is gzip compressed");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("standard", 1);
  pos_opts.add("exception", 1);  
  pos_opts.add("report", 1);

    
  po::variables_map vm; 

  std::string what_am_i("Write 2D histogram file (heat map) for Odds Ratio vs. solar time and azimuth\n\t");
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
    
  wlog.writeReport(report_name); 
}
