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

// plot distance, azimuth pairs. 
class myWSPRLog : public WSPRLog {
public:
  myWSPRLog(WSPRLogEntry::Field _xsel, WSPRLogEntry::Field _ysel,
	    const std::string & out_name) : WSPRLog() {
    xsel = _xsel; 
    ysel = _ysel;
    os.open(out_name);
  }


  bool processEntry(WSPRLogEntry * ent) {
    int x, y; 
    

    if(ent == NULL) return false; 

    ent->getField(xsel, x);
    ent->getField(ysel, y);

    os << x << " " << y << std::endl; 
    return false;
  }


private:
  std::ofstream os;
  WSPRLogEntry::Field xsel, ysel; 
}; 

int main(int argc, char * argv[])
{
  std::string in_name, out_name, x_field_selector, y_field_selector;
  bool input_gzipped; 
  namespace po = boost::program_options;


  po::options_description desc("Options:");

  desc.add_options()
    ("help", "help message")
    ("log", po::value<std::string>(&in_name)->required(), "Input log file (csv) in WSPR log format")
    ("out", po::value<std::string>(&out_name)->required(), "Output data file")
    ("x_field", po::value<std::string>(&x_field_selector)->required(), "Numeric field to use for x coordinate")
    ("y_field", po::value<std::string>(&y_field_selector)->required(), "Numeric field to use for y coordinate")
    ("igz", po::value<bool>(&input_gzipped)->default_value(false), "if true, input file is gzip compressed");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("log", 1);
  pos_opts.add("out", 1);

    
  po::variables_map vm; 

  std::string what_am_i("Filter WSPR logs by frequency range\n\t");
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


  WSPRLogEntry::Field xsel, ysel;

  xsel = WSPRLogEntry::str2Field(x_field_selector);
  ysel = WSPRLogEntry::str2Field(y_field_selector);   
  if((xsel == WSPRLogEntry::UNDEFINED) || (ysel == WSPRLogEntry::UNDEFINED)) {
    std::cerr << "Bad field selected [" << x_field_selector << "," << y_field_selector << "]\n";
    WSPRLogEntry::printFieldChoices(std::cerr); 
    exit(-1); 
  }

  myWSPRLog wlog(xsel, ysel, out_name); 

  wlog.readLog(in_name, input_gzipped);
}
