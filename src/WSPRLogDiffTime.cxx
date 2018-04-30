#include "WSPRLog.hxx"
#include "TimeCorr.hxx"
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <math.h>
#include <set>

// plot distance, "local time" pairs as histogram
class myWSPRLog : public WSPRLog {
public:
  myWSPRLog(const std::string & out_base_name) : WSPRLog() {
    osrx.open(out_base_name + "_FD_T_rx.dat");
    ostx.open(out_base_name + "_FD_T_tx.dat");
    ostxrx.open(out_base_name + "_FD_T_txrx.dat");    
    
    for(int i = 0; i < 24; i++) {
      for(int j = 0; j < 200; j++) {
	rxhisto[i][j] = 0;
	txhisto[i][j] = 0;
	txrxhisto[i][j] = 0; 	
      }
    }
  }

  ~myWSPRLog() {
  }

  enum SEL { RX, TX, TXRX };

  bool processEntry(WSPRLogEntry * ent) {
    std::string from, to; 
    double ft, tt, ftt; 
    unsigned long et; 
    double freq_diff;

    if(ent == NULL) return false; 

    to = ent->rxgrid;
    from = ent->txgrid; 
    et = ent->dtime; 
    freq_diff = ent->freq_diff; 

    // calculate the "local time" for to, from, and midpath
    float fto = TimeCorr::localTime(from, et);
    float tto = TimeCorr::localTime(to,  et);
    float mto = TimeCorr::localTime(from, to,  et);
    
    int ifreq_diff = (int) freq_diff; 

    makeEntry(RX, tto, ifreq_diff); 
    makeEntry(TX, fto, ifreq_diff);
    makeEntry(TXRX, mto, ifreq_diff);     

    delete ent;
    return true; 
  }


  void makeEntry(SEL sel, float hr, int offset) {
    int off_idx = offset + 100; 
    if((off_idx < 0) || (off_idx >= 200)) return; 

    int ihr = (int) (hr + 0.5);
    if(ihr > 23) ihr -= 24;

    if((ihr < 0) || (ihr > 23)) {
      std::cerr << boost::format("makeEntry got hr = %f ihr = %d offset = %d off_idx = %d\n") % hr % ihr % offset % off_idx;
      return; 
    }

    
    switch (sel) {
    case RX:
      rxhisto[ihr][off_idx]++;
      break;
    case TX:
      txhisto[ihr][off_idx]++;
      break;
    case TXRX:
      txrxhisto[ihr][off_idx]++;
      break;
    }
  }

  void dumpTables() {
    dumpTable(RX, osrx);
    dumpTable(TX, ostx);
    dumpTable(TXRX, ostxrx);

    osrx.close();
    ostx.close();
    ostxrx.close();
  }

  void dumpTable(SEL sel, std::ostream & os) {
    // first find total 
    long total = 0;
    long smax = 1; 
    for(int i = 0; i < 24; i++) {
      for(int j = 0; j < 200; j++) {
	int val; 
	switch (sel) {
	case RX:
	  val = rxhisto[i][j];
	  break;
	case TX:
	  val = txhisto[i][j];
	  break;
	case TXRX:
	  val = txrxhisto[i][j];
	  break;
	}
	total += val; 
	smax = (smax > val) ? smax : val; 
      }
    }
    

    double ftotal = ((double) total) + 1.0e-6; 
    double fsmax = ((double) smax); 
    for(int i = 0; i < 24; i++) {
      for(int j = 0; j < 200; j++) {
	int val; 
	switch (sel) {
	case RX:
	  val = rxhisto[i][j];
	  break;
	case TX:
	  val = txhisto[i][j];
	  break;
	case TXRX:
	  val = txrxhisto[i][j];
	  break;
	}

	double fval = ((double) val) / ftotal; 
	double rmag = ((double) val) / fsmax;
	os << boost::format("%d %d %f %f\n") % i % (j - 100) % fval % rmag; 
      }
      os << std::endl; 
    }
  }

private:
  std::ofstream osrx, ostx, ostxrx;
  unsigned rxhisto[24][200];
  unsigned txhisto[24][200];
  unsigned txrxhisto[24][200]; 
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

  std::string what_am_i("Make [hr][delta f] histogram from  WSPR log\n\t");
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
