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
  myWSPRLog(WSPRLogEntry::Field _sel) : WSPRLog() {
    sel = _sel; 
  }


  bool processEntry(WSPRLogEntry * ent) {
    int el; 

    ent->getField(sel, el); 

    if(histogram.find(el) == histogram.end()) {
      histogram[el] = 1; 
    }
    else {
      histogram[el] += 1; 
    }
    
    return true; 
  }


  void printHistogram(std::ostream & os) {
    for(auto he: histogram) {
      os << he.first << " " << he.second << std::endl; 
    }
  }
private:
  std::map<int, int> histogram;
  WSPRLogEntry::Field sel;
}; 

int main(int argc, char * argv[])
{
  std::string in_name, out_name, field_selector;
  bool input_gzipped; 
  namespace po = boost::program_options;


  po::options_description desc("Options:");

  desc.add_options()
    ("help", "help message")
    ("log", po::value<std::string>(&in_name)->required(), "Input log file (csv) in WSPR log format")
    ("out", po::value<std::string>(&out_name)->required(), "Histogram table suitable for gnuplot")
    ("field", po::value<std::string>(&field_selector)->required(), "Numeric field to use for histogram buckets")
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


  WSPRLogEntry::Field sel; 
  sel = WSPRLogEntry::str2Field(field_selector); 
  if(sel == WSPRLogEntry::UNDEFINED) {
    std::cerr << "Bad field selected.\n";
    WSPRLogEntry::printFieldChoices(std::cerr); 
    exit(-1); 
  }

  myWSPRLog wlog(sel); 

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

  std::ofstream ofs(out_name);
  wlog.printHistogram(ofs);
}
