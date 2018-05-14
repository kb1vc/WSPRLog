#include "WSPRLog.hxx"

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <math.h>
#include <set>

class myWSPRLog : public WSPRLog {
public:
  myWSPRLog(const std::string outf_base_name): WSPRLog() {
    buildBandList(outf_base_name); 
    distance_threshold = 100; 
  }


  ~myWSPRLog() {
    for(auto bfe: band_list) {
      bfe->out.close();
    }
  }
  

  bool processEntry(WSPRLogEntry * ent) {
    // delete all records that are over a path less than 100km
    if(ent->dist < distance_threshold) {
      delete ent; 
      return false;       
    }
    std::ostream * osp = getBandFile(ent->freq); 
    if(osp != NULL) {
      ent->print(*osp);
    }
    delete ent;
    return false; 
  }


  std::ostream * getBandFile(double freq) {
    for(auto bfe: band_list) {
      if((freq >= bfe->lo) && (freq <= bfe->hi)) return &(bfe->out); 
    }
    return NULL; 
  }

  void buildBandList(const std::string & basename) {
    band_list.push_back(new BandFile(3.5, 4.0, basename + "_80m.csv"));
    band_list.push_back(new BandFile(7.0, 7.3, basename + "_40m.csv"));
    band_list.push_back(new BandFile(10.1, 10.15, basename + "_30m.csv"));
    band_list.push_back(new BandFile(14.0, 14.35, basename + "_20m.csv"));
    band_list.push_back(new BandFile(18.068, 18.168, basename + "_17m.csv"));
    band_list.push_back(new BandFile(21.0, 21.45, basename + "_15m.csv"));
    band_list.push_back(new BandFile(24.89, 24.99, basename + "_12m.csv"));
    band_list.push_back(new BandFile(28.0, 29.7, basename + "_10m.csv"));
    band_list.push_back(new BandFile(50.0, 54.0, basename + "_6m.csv"));
    band_list.push_back(new BandFile(144.0, 148.0, basename + "_2m.csv"));    
  }

private:
  class BandFile {
  public:
    BandFile(double _lo, double _hi, std::string fname) {
      lo = _lo; 
      hi = _hi; 
      out.open(fname); 
    }
    double lo; 
    double hi; 
    std::ofstream out; 
  }; 

  float distance_threshold; 
  std::list<BandFile *> band_list; 
}; 

int main(int argc, char * argv[])
{
  std::string in_name, out_base_name;
  bool input_gzipped; 
  namespace po = boost::program_options;


  po::options_description desc("Options:");

  desc.add_options()
    ("help", "help message")
    ("log", po::value<std::string>(&in_name)->required(), "Input log file (csv) in WSPR log format")
    ("out_base", po::value<std::string>(&out_base_name)->required(), "Basename for output logs in WSPR log format")
    ("igz", po::value<bool>(&input_gzipped)->default_value(false), "if true, input file is gzip compressed");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("log", 1);
  pos_opts.add("out_base", 1);

    
  po::variables_map vm; 

  try {
    po::store(po::command_line_parser(argc, argv).options(desc)
	      .positional(pos_opts).run(), vm);
    
    if(vm.count("help")) {
      std::cout << "Split WSPR logs into frequency bands\n\t"
		<< desc << std::endl; 
      exit(-1);
    }

    po::notify(vm);
  }
  catch(po::required_option & e) {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl; 
      std::cout << "Split WSPR logs into frequency bands\n\t"
	      << desc << std::endl;
    exit(-1);    
  }
  catch(po::error & e) {
    std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
    std::cout << "Split WSPR logs into frequency bands\n\t"
	      << desc << std::endl;
    exit(-1);        
  }


  myWSPRLog wlog(out_base_name);

  wlog.readLog(in_name, input_gzipped);  
}
