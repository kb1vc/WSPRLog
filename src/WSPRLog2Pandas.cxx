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

class myWSPRLog : public WSPRLog {
public:
  myWSPRLog(const std::string outf_name, 
	    double _f_lo, double _f_hi) : WSPRLog() {
    last_time = 0; 

    //create a list of "bands" that we exclude 
    // because they are close to multiples of the 
    // power line frequency. 
    int i, j; 
    for(i = -4; i < 5; i++) {
      if(i == 0) continue; 
      for(j = -2; j < 3; j++) {
	line_freqs.insert(i * 50 + j);
	line_freqs.insert(i * 60 + j);	
      }
    }
    
    out.open(outf_name);

    printHeader();
  }

  void finish() {
    out.close();
  }
  
  bool processEntry(WSPRLogEntry * ent) {
    if((ent != NULL) && (last_time == 0)) last_time = ent->dtime; 

    if((ent != NULL) && (ent->dtime == last_time)) {
      // add entry to tx list
      addEntry(ent); 
      return true; 
    }
    else {
      // we are done.  Dump the pairs, if any
      dumpPairs();
      clearMap();
      if(ent != NULL) {
	last_time = ent->dtime; 
	addEntry(ent); 
	return true; 
      }
      return false; 
    }

    return false; 
  }

  void printHeader() {
    // print column labels
    out << "RXSOL,TXSOL,MIDSOL,"; 
    WSPRLogEntry::printHeader(out); 
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

    out << boost::format("%ld,%ld,%ld,")
      % tx_time.getFHour() % rx_time.getFHour() % mid_hour;
    
    fle->print(out);
  }

  // if anyone ever reports more than 4 image pairs in one cycle, we mark
  // them as "bad"
  // we report pairs and non-pairs.  
  // But we don't report pairs where the difference frequency is 
  // in the line_freqs set. 
  void dumpPairs() {
    for(auto & mapent : pair_map) {
      if(mapent.second.size() > 1) {
	// two or more reports for the same station pair. 	
	mapent.second.sort(WSPRLogEntry::compareSNR); 
	WSPRLogEntry * fle = mapent.second.front();
	fle->main_snr = fle->snr;

	for(auto & le : mapent.second) {
	  // print them all. 
	  if(le != fle) le->calcDiff(fle);
	  if(line_freqs.find(le->freq_diff) == line_freqs.end()) {
	    printExtended(le); 
	  }
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

  void addEntry(WSPRLogEntry * ent) {
    std::string key = ent->txcall + "," + ent->rxcall; 
    pair_map[key].push_back(ent); 
  }

private:
  std::ofstream out; 
  unsigned long last_time; 
  std::map<std::string, std::list<WSPRLogEntry *> > pair_map;
  std::set<int> line_freqs; 
}; 

int main(int argc, char * argv[])
{
  std::string in_name, out_name, multi_name;
  double lo_freq, hi_freq; 
  bool input_gzipped; 
  namespace po = boost::program_options;


  po::options_description desc("Options:");

  desc.add_options()
    ("help", "help message")
    ("log", po::value<std::string>(&in_name)->required(), "Input log file (csv) in WSPR log format")
    ("out", po::value<std::string>(&out_name)->required(), "Filtered output log file (csv) in CSV log format")
    ("igz", po::value<bool>(&input_gzipped)->default_value(false), "if true, input file is gzip compressed");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("log", 1);
  pos_opts.add("out", 1);
    
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

  myWSPRLog wlog(out_name, lo_freq, hi_freq);

  wlog.readLog(in_name, input_gzipped);
  
  // call processEntry one last time, to see if we've
  // got something stuck in the pipeline. 
  wlog.processEntry(NULL); 

  wlog.finish();
}
