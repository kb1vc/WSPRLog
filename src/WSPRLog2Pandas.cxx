#include "WSPRLog.hxx"
#include "SolarTime.hxx"
#include "TimeCorr.hxx"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <math.h>
#include <set>
#include <iomanip>

class myWSPRLog : public WSPRLog {
public:
  myWSPRLog(double _f_lo, double _f_hi, bool print_header = false) : WSPRLog() {
    last_time = 0; 

    freq_min = _f_lo; 
    freq_max = _f_hi; 

    //create a list of "bands" that we exclude 
    // because they are close to multiples of the 
    // power line frequency. 
    int i, j; 
    
    if(print_header) printHeader();
  }

  void finish() {

  }
  
  bool processEntry(WSPRLogEntry * ent) {
    if((ent != NULL) && (last_time == 0)) last_time = ent->dtime; 

    if((ent != NULL) && (ent->dtime == last_time)) {
      // add entry to tx list
      return addEntry(ent); 
    }
    else {
      // we are done.  Dump the pairs, if any
      dumpPairs();
      clearMap();
      if(ent != NULL) {
	last_time = ent->dtime; 
	return addEntry(ent); 
      }
      return false; 
    }

    return false; 
  }

  void printHeader() {
    // print column labels
    std::cout << "RXSOL,TXSOL,MIDSOL,"; 
    WSPRLogEntry::printHeader(std::cout); 
    // SPOT,DTIME,RXCALL,RXGRID,SNR,REFSNR,FREQ,TXCALL,TXGRID,POW,DRIFT,DIST,AZ,BAND,VER,CODE,FREQDIFF\n";
  }

  void printExtended(WSPRLogEntry * fle) {
    // prefix the record with the solar time for TX, RX, and midpoint
    // then print the actual record

    SolarTime tx_time(fle->dtime, fle->txgrid);
    SolarTime rx_time(fle->dtime, fle->rxgrid);    
    float mid_hour = TimeCorr::circularMean(24.0, 
					    tx_time.getFHour(), 
					    rx_time.getFHour());

    std::cout << boost::format("%ld,%ld,%ld,")
      % tx_time.getFHour() % rx_time.getFHour() % mid_hour;
    
    fle->print(std::cout);
  }

  // if anyone ever reports more than 4 image pairs in one cycle, we mark
  // them as "bad"
  // we report pairs and non-pairs.  
  void dumpPairs() {
    for(auto & mapent : pair_map) {
      if(mapent.second.size() > 1) {
	// two or more reports for the same station pair. 	
	mapent.second.sort(WSPRLogEntry::compareSNR); 
	WSPRLogEntry * fle = mapent.second.front();
	fle->main_snr = fle->snr;

	for(auto & le : mapent.second) {
	  // print them all.
	  le->calcDiff(fle);
	  printExtended(le); 
	}
      }
      else {
	// this is a single -- non duplicate -- report. 
	WSPRLogEntry * fle = mapent.second.front(); 
	fle->main_snr = fle->snr; 
	fle->calcDiff(fle); // zero things out.
	printExtended(fle);
      }
    }
  }

  void clearMap() {
    // empty maps, and free up records. 
    for(auto & mapent : pair_map) {
      while(!pair_map[mapent.first].empty()) {
	delete pair_map[mapent.first].front();
	pair_map[mapent.first].pop_front();
      }
    }
    pair_map.clear();
  }

  bool addEntry(WSPRLogEntry * ent) {
    if((ent->freq <= freq_max) && (ent->freq >= freq_min)) {
      std::string key = ent->txcall + "," + ent->rxcall; 
      pair_map[key].push_back(ent); 
      return true; 
    }
    else {
      return false; 
    }
  }

private:
  unsigned long last_time; 
  std::map<std::string, std::list<WSPRLogEntry *> > pair_map;
  double freq_min, freq_max; 
}; 

int main(int argc, char * argv[])
{
  std::string in_name, multi_name;
  double freq_min, freq_max;   
  bool input_gzipped; 
  namespace po = boost::program_options;


  po::options_description desc("Options:");
  bool print_header; 

  desc.add_options()
    ("help", "help message")
    ("freq_min", po::value<double>(&freq_min)->required(), "Lower edge of band-of-interest (MHz)") 
    ("freq_max", po::value<double>(&freq_max)->required(), "Upper edge of band-of-interest (MHz)")
    ("log", po::value<std::string>(&in_name)->default_value("STDIN"), "Input log file (csv) in WSPR log format")
    ("igz", po::value<bool>(&input_gzipped)->default_value(false), "if true, input file is gzip compressed")
    ("header", po::value<bool>(&print_header)->default_value(true), "if true, input file is gzip compressed");  
  
  po::positional_options_description pos_opts ;
  pos_opts.add("log", 1);
    
  po::variables_map vm; 

  try {
    po::store(po::command_line_parser(argc, argv).options(desc)
	      .positional(pos_opts).run(), vm);
    
    if(vm.count("help")) {
      std::cerr << "Filter WSPR logs by frequency range\n\t"
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

  std::cerr << boost::format("freq_min = %f freq_max = %f\n") % freq_min % freq_max; 
  myWSPRLog wlog(freq_min, freq_max, print_header);

  // establish a default float precision that gets us 1Hz resolution
  
  if(in_name == "STDIN") {
    wlog.readLog(std::cin);
  }
  else {
    wlog.readLog(in_name, input_gzipped);
  }
  
  // call processEntry one last time, to see if we've
  // got something stuck in the pipeline. 
  wlog.processEntry(NULL); 

  wlog.finish();
}
