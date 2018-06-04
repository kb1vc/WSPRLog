#include "WSPRLog.hxx"
#include "SolarTime.hxx"
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <math.h>
#include <set>

// accumulate number of reports per hour of the day at TX, RX, and midpoint
class myWSPRLog : public WSPRLog {
public:
  myWSPRLog(const std::string & out_base_name) : WSPRLog() {
    osrx.open(out_base_name + "_TH_rx.dat");
    ostx.open(out_base_name + "_TH_tx.dat");
    
    for(int i = 0; i < 24; i++) {
	rxhisto[i] = 0;
	txhisto[i] = 0;
    }
  }

  ~myWSPRLog() {
  }


  bool processEntry(WSPRLogEntry * ent) {
    std::string from, to; 
    double ft, tt, ftt; 
    unsigned long et; 
    double freq_diff;

    if(ent == NULL) return false; 

    to = ent->rxgrid;
    from = ent->txgrid; 
    et = ent->dtime; 

    // calculate the "local time" for to, from, and midpath
    SolarTime tx_time(et, from);
    SolarTime rx_time(et, to);     

    makeEntry(rxhisto, rx_time.getFHour());
    makeEntry(txhisto, tx_time.getFHour());

    return false;
  }



  void makeEntry(unsigned *histo, float hr) 
  {
    int ihr = (int) hr;
    if(ihr > 23) ihr -= 24;

    if(ihr < 0) ihr += 24; 

    histo[ihr]++; 
  }

  void dumpTables() {
    dumpTable(rxhisto, osrx);
    dumpTable(txhisto, ostx);

    osrx.close();
    ostx.close();
  }

  void dumpTable(unsigned * histo, std::ostream & os) {
    // first find total 
    long total = 0;
    long smax = 1; 
    
    for(int i = 0; i < 24; i++) {
      os << histo[i] << std::endl; 
    }
  }

private:
  std::ofstream osrx, ostx;
  unsigned rxhisto[24];
  unsigned txhisto[24];
}; 

int main(int argc, char * argv[])
{
  bool input_gzipped; 
  std::string in_name, out_name; 
  namespace po = boost::program_options;

  po::options_description desc("Options:");

  desc.add_options()
    ("help", "help message")
    ("log", po::value<std::string>(&in_name)->required(), "Input log file (csv) in WSPR log format")
    ("out_base", po::value<std::string>(&out_name)->required(), "Output data file basename")
    ("igz", po::value<bool>(&input_gzipped)->default_value(false), "if true, input file is gzip compressed");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("log", 1);
  pos_opts.add("out_base", 1);

  po::variables_map vm; 

  std::string what_am_i("Create histogram of reports in each hour from  WSPR log\n\t");
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

  wlog.readLog(in_name, input_gzipped);
  
  wlog.dumpTables();
}
