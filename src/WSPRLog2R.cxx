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

// filter based on line frequencies.
// delete all 0, 50*n and 60*n Hz offset reports.
// mark offset reports with snr != main_snr
// write the output as a CSV file suitable for input into R
class myWSPRLog : public WSPRLog {
public:
  myWSPRLog(const std::string outf_name) : WSPRLog() {
    last_time = 0; 

    rx_suspect_threshold = 4;
    
    os.open(outf_name);

    printRHeader();

    fmt = new boost::format("%ld,%8.4f,%8.4f,%8.4f,%f,%f,%f,%f,%f,%f,%f,%s,%s,%s,%s\n");

  }

  bool isKeeper(WSPRLogEntry * ent) {
    return true; 
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
      if(ent != NULL) {
	clearMap();            	
	last_time = ent->dtime; 
	addEntry(ent); 
	return true; 
      }
      return false;       
    }
    return false; 
  }

  // if anyone ever reports more than 4 image pairs in one cycle, we mark
  // them as "bad"
  void dumpPairs() {
    std::map<std::string, int> rep_counts; 

    // at this point, pair_map is full of all the signal reports for 
    // the current time frame.  We'll cycle through each rx/tx pair
    // and write the log entries back out, but mark the "image" reports
    // that are not clearly caused by 60Hz or 50Hz hum as images (fdiff != 0)


    for(auto & mapent : pair_map) {
      if(bad_guys.find(mapent.second.front()->rxcall) != bad_guys.end()) 
	continue;
      
      if(mapent.second.size() > 1) {
	// two or more reports for the same station pair. 	
	// mapent.second is a list of all log reports in this 
	// timestep for mapent.first station pair (rxcall,txcall)
	mapent.second.sort(WSPRLogEntry::compareSNR);
	WSPRLogEntry * fle = mapent.second.front();
	if(fle == NULL) {
	  std::cerr << boost::format("Some kind of problem with key [%s]\n")
	    % mapent.first;
	  continue; 
	}		       

	fle->main_snr = fle->snr;
	
	// remember the reporting station
	if(rep_counts.find(fle->rxcall) == rep_counts.end()) rep_counts[fle->rxcall] = 1; 
	else rep_counts[fle->rxcall] += 1; 

	// skip reports where every entry is on exactly the same frequency.
	int printed_count = 0; 
	for(auto & le : mapent.second) {
	  if(le != fle) {
	    le->calcDiff(fle); 
	  }
	  else {
	    le->freq_diff = 0.0; 
	  }
	  printREntry(le); 
	}
      }
      else {
	// this is a singleton report.  don't throw it out, just
	// report it as a normal log entry. 
	WSPRLogEntry * fle = mapent.second.front();
	if(fle == NULL) {
	  std::cerr << boost::format("Some kind of problem with key on short list [%s]\n")
	    % mapent.first;
	  continue; 
	}		       
	fle->main_snr = fle->snr; 
	printREntry(fle); 
      }
    }
    // at this point the rep_counts map contains a count of repeat reports
    // for each rx station.  If an rx station has image reports from more than
    // one tx station, this is a hint that it may have duplicate receivers.
    // we should eliminate all reports from that rx for the remainder of the log. 

    // now check all the folks in the rep_counts map.  record bad guys in 
    // the badguy set. 
    for(auto & repent : rep_counts) {
      if(repent.second > rx_suspect_threshold) {
	bad_guys.insert(repent.first); 
      }
    }
  }

  void clearMap() {
    // empty maps, and free up records. 
    for(auto & mapent : pair_map) {
      while(!pair_map[mapent.first].empty()) {
	auto le = pair_map[mapent.first].front(); 
	delete le;
	pair_map[mapent.first].pop_front();
      }
    }
    pair_map.clear();
  }

  void addEntry(WSPRLogEntry * ent) {
    std::string key = ent->txcall + "," + ent->rxcall; 
    pair_map[key].push_back(ent); 
  }

  void printRHeader() {
    os << "\"EpocTime\", "
       << "\"RXSolarTime\", "
       << "\"TXSolarTime\", "
       << "\"MidSolarTime\", "

       << "\"FreqDiff\", "
       << "\"Freq\", "
       << "\"SNR\", "
       << "\"MainSNR\", "

       << "\"TxPower\", "
       << "\"AZ\", "
       << "\"DIST\", "
       << "\"RXCall\", "
       << "\"RXGrid\", "
       << "\"TXCall\", "
       << "\"TXGrid\" "
       << std::endl; 
  }

  bool isInRange(float v, float l, float h) { 
    return ((v > l) && (v < h)) ||
      ((v > -h) && (v < -l)); 
  }
  
  bool freqDiffIsBad(WSPRLogEntry * le) {
    float fd = le->freq_diff; 
    float l5 = 48.0; 
    float h5 = 52.0;
    float l6 = 58.0;
    float h6 = 62.0; 
      
    for(int i = 1; i < 4; i++) {
      if(isInRange(fd, i * l5, i * h5)) return true;
      if(isInRange(fd, i * l6, i * h6)) return true;       
    }

    return false; 
  }

  void printREntry(WSPRLogEntry * le) {
    // don't print entries whose freq_diff is "bad" 
    if(freqDiffIsBad(le)) return;
    return; 
    // print the log entry in a form suitable for R
    SolarTime tx_time(le->dtime, le->txgrid);
    SolarTime rx_time(le->dtime, le->rxgrid);    
    float mid_hour = TimeCorr::circularMean(24.0, tx_time.getFHour(), rx_time.getFHour()); 
    
    os << *fmt 
      % le->dtime % rx_time.getFHour() % tx_time.getFHour() % mid_hour
      % le->freq_diff % le->freq % le->snr % le->main_snr 
      % le->power % le->az % le->dist
      % le->rxcall % le->rxgrid % le->txcall % le->txgrid; 
  }

private:
  boost::format * fmt;   
  std::ofstream os; 
  unsigned long last_time; 
  std::map<std::string, std::list<WSPRLogEntry *> > pair_map; 

  double f_lo, f_hi; 
  std::set<std::string> bad_guys;
  int rx_suspect_threshold; 
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
    ("out", po::value<std::string>(&out_name)->required(), "Filtered output log file (csv) Formatted for input to R")
    ("igz", po::value<bool>(&input_gzipped)->default_value(false), "if true, input file is gzip compressed");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("log", 1);
  pos_opts.add("out", 1);
    
  po::variables_map vm; 

  std::string what_am_i("Format band log for use in R system -- include all but 50/60 Hz reports\n"); 
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
  
  // call processEntry one last time, to see if we've
  // got something stuck in the pipeline. 
  wlog.processEntry(NULL); 
}
