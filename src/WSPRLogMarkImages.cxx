#include "WSPRLog.hxx"

// Mark the "image" contacts by extending the WSPR log entry 
// with an additional field. 

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
    f_lo = _f_lo; 
    f_hi = _f_hi; 

    rx_suspect_threshold = 4;
    
    out.open(outf_name);
  }

  bool isKeeper(WSPRLogEntry * ent) {
    double freq; 
    bool s = ent->getField(WSPRLogEntry::FREQ, freq); 
    return (s && (freq <= f_hi) && (freq >= f_lo)); 
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

  // if anyone ever reports more than 4 image pairs in one cycle, we mark
  // them as "bad"
  void dumpPairs() {
    std::map<std::string, int> repcounts; 

    for(auto & mapent : pair_map) {
      if(mapent.second.size() > 1) {
	// two or more reports for the same station pair. 	
	mapent.second.sort(WSPRLogEntry::compareSNR); 
	WSPRLogEntry * fle = mapent.second.front();
	// don't do this -- 0 offset has the info... fle->calcDiff(fle);
	fle->main_snr = fle->snr;
	
	// remember the reporting station
	if(repcounts.find(fle->rxcall) == repcounts.end()) repcounts[fle->rxcall] = 1; 
	else repcounts[fle->rxcall] += 1; 

	addToCount(total_pair_reports, mapent.first);

	// skip reports where every entry is on exactly the same frequency.
	int printed_count = 0; 
	for(auto & le : mapent.second) {
	  if(le != fle) le->calcDiff(fle); 
	  if(le->freq_diff != 0.0) {
	    le->print(out); 
	    printed_count++; 
	  }
	}
	if(printed_count > 0) {
	  fle->print(out);
	  addToCount(img_rx_reports, fle->rxcall, printed_count);
	  addToCount(img_tx_reports, fle->txcall, printed_count);
	  addToCount(img_pair_reports, mapent.first, printed_count);
	}
      }
    }

    // now check all the folks in the repcounts map.  record bad guys in 
    // the badguy set. 
    for(auto & repent : repcounts) {
      if(repent.second > rx_suspect_threshold) {
	multi_rx_reporters.insert(repent.first);
      }
    }
  }

  void addToCount(std::map<std::string, int> & reps, const std::string & k, int count = 1) {
    if(reps.find(k) != reps.end()) {
      reps[k] += count; 
    }
    else {
      reps[k] = count; 
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
    // record the number of reports for each call
    addToCount(total_rx_reports, ent->rxcall);
    addToCount(total_tx_reports, ent->txcall);
    addToCount(total_pair_reports, key);
  }


  void dumpReportCounts(const std::string & ofn) {
    std::string bn = boost::filesystem::basename(ofn); 
    dumpRC(total_rx_reports, img_rx_reports, bn + "_rx.prop");
    dumpRC(total_tx_reports, img_tx_reports, bn + "_tx.prop");
    dumpRC(total_pair_reports, img_pair_reports, bn + "_pair.prop");    
  }

  void dumpRC(std::map<std::string, int> & tots,
	      std::map<std::string, int> & imgs,
	      const std::string & fn) {
    std::ofstream os(fn);
    for(auto tp: tots) {
      if(imgs.find(tp.first) != imgs.end()) {
	int tot = tots[tp.first];
	int img = imgs[tp.first]; 
	float prop = ((float) img)/((float) tot);
	os << tp.first << " " << tot << " " << img << " " << prop << std::endl;
      }
    }
    os.close();
  }
private:
  std::ofstream out; 
  unsigned long last_time; 
  std::map<std::string, std::list<WSPRLogEntry *> > pair_map; 

  std::map<std::string, int> total_rx_reports, total_tx_reports, total_pair_reports;
  std::map<std::string, int> img_rx_reports, img_tx_reports, img_pair_reports; 
  double f_lo, f_hi; 
  std::set<std::string> multi_rx_reporters; 
  int rx_suspect_threshold; 
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
    ("out", po::value<std::string>(&out_name)->required(), "Filtered output log file (csv) in WSPR log format")
   ("flo", po::value<double>(&lo_freq)->required(), "lower bound of frequency range")
    ("fhi", po::value<double>(&hi_freq)->required(), "upper bound of frequency range")
    ("igz", po::value<bool>(&input_gzipped)->default_value(false), "if true, input file is gzip compressed");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("log", 1);
  pos_opts.add("out", 1);
    
  po::variables_map vm; 

  try {
    po::store(po::command_line_parser(argc, argv).options(desc)
	      .positional(pos_opts).run(), vm);
    
    if(vm.count("help")) {
      std::cout << "Filter WSPR logs by frequency range and mark \"image\" reports.\n\t"
		<< desc << std::endl; 
      exit(-1);
    }

    po::notify(vm);
  }
  catch(po::required_option & e) {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
    std::cerr << "Filter WSPR logs by frequency range and mark \"image\" reports.\n\t"
	      << desc << std::endl;
    exit(-1);    
  }
  catch(po::error & e) {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
    std::cerr << "Filter WSPR logs by frequency range and mark \"image\" reports.\n\t"
	      << desc << std::endl;
    exit(-1);        
  }

  myWSPRLog wlog(out_name, lo_freq, hi_freq);

  wlog.readLog(in_name, input_gzipped);
  
  // call processEntry one last time, to see if we've
  // got something stuck in the pipeline. 
  wlog.processEntry(NULL); 

  wlog.dumpReportCounts(out_name); 
}
