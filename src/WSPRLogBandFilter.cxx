#include "WSPRLog.hxx"

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
	fle->calcDiff(*fle);

	// remember the reporting station
	if(repcounts.find(fle->rxcall) == repcounts.end()) repcounts[fle->rxcall] = 1; 
	else repcounts[fle->rxcall] += 1; 

	std::string prefix = (boost::format("tx: %10s %6s time: %ld  dist: %d  pwr: %g ") 
			      % fle->txcall % fle->txgrid % fle->dtime % fle->dist % fle->power).str();
	for(auto & le : mapent.second) {
	  le->calcDiff(*fle); 
	  le->print(out); 
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


  void dumpMultiRx(std::ostream & os) {
    for(auto se: multi_rx_reporters) {
      os << se << std::endl; 
    }
  }
private:
  std::ofstream out; 
  unsigned long last_time; 
  std::map<std::string, std::list<WSPRLogEntry *> > pair_map; 
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
    ("multi", po::value<std::string>(&multi_name)->required(), "List of stations that may have multiple receivers")
   ("flo", po::value<double>(&lo_freq)->required(), "lower bound of frequency range")
    ("fhi", po::value<double>(&hi_freq)->required(), "upper bound of frequency range")
    ("igz", po::value<bool>(&input_gzipped)->default_value(false), "if true, input file is gzip compressed");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("log", 1);
  pos_opts.add("out", 1);
  pos_opts.add("multi", 1);  
    
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

  std::cerr << boost::format("infile: [%s] outfile: [%s] f_lo = %g f_hi = %g\n") % in_name % out_name % lo_freq % hi_freq; 

  myWSPRLog wlog(out_name, lo_freq, hi_freq);

  if(input_gzipped) {
    std::ifstream gzfile(in_name, std::ios_base::in | std::ios_base::binary);
    boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf; 
    inbuf.push(boost::iostreams::gzip_decompressor());
    inbuf.push(gzfile);
    std::istream inf(&inbuf);
    wlog.readLog(inf);
  }
  else {
    std::ifstream inf(in_name);
    wlog.readLog(inf);
  }

  // call processEntry one last time, to see if we've
  // got something stuck in the pipeline. 
  wlog.processEntry(NULL); 

  std::ofstream rep(multi_name);
  wlog.dumpMultiRx(rep);
}
