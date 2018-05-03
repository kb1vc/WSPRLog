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

// accumulate number of reports per callsign -- this is
// useful in identifying stations that may have multiple
// receivers or other peculiar effects... RX stations that
// report more than 1% of their entries as "anomalous" are
// identified by an H in the first column of the report.
//
// extract a list of possible bad sources like this:
// CallRR raw_band_file.csv band_file_D.csv band_file_dups.rpt
// grep "^H" band_file_dups.rpt | awk '{ print $2; }' > band_excludes.lis
// grep -v -f band_excludes.lis raw_band_file.csv > clean_band_file.csv
//
// now do the D, 50Hz, 60Hz splits.
// and the rest.

class myWSPRLog : public WSPRLog {
public:
  myWSPRLog() : WSPRLog() {
    exc_mode = false;
  }

  ~myWSPRLog() {
  }


  class CountRec {
  public:
    CountRec() { 
      rx_standard = rx_exception = 0;
      tx_standard = tx_exception = 0;       
    }

    void bump(bool is_rx, bool is_exc) {
      if(is_rx) {
	if(is_exc) rx_exception++; 
	else rx_standard++; 
      }
      else {
	if(is_exc) tx_exception++; 
	else tx_standard++; 
      }
    }

    unsigned int rx_standard; 
    unsigned int rx_exception; 
    unsigned int tx_standard; 
    unsigned int tx_exception; 
  }; 


  void setExcMode(bool fl) { exc_mode = fl; }

  void incEntry(const std::string & call, bool is_rx)
  {
    if(call_table.find(call) == call_table.end()) {
      // we only collect exc reports
      if(!exc_mode) return; 
      CountRec * cr = new CountRec(); 
      call_table[call] = cr; 
    }

    call_table[call]->bump(is_rx, exc_mode);
    
    totals.bump(is_rx, exc_mode); 
  }


  bool processEntry(WSPRLogEntry * ent) {
    if(ent == NULL) return false; 
    
    incEntry(ent->rxcall, true); 
    incEntry(ent->txcall, false); 
    return true; 
  }

  void dumpTables(const std::string & fname) {
    std::ofstream rxos("rx_" + fname);
    std::ofstream txos("tx_" + fname);     

    float esum = (float) totals.rx_exception;
    float ssum = (float) totals.rx_standard;

    rxos << "# call std:rx_ct exc:rx_ct er/esum er/sr (er/sr)/(esum/ssum)\n";
    txos << "# call std:tx_ct exc:tx_ct et/esum et/st (et/st)/(esum/ssum)\n";
    for(auto ent: call_table) {
      CountRec *crp = ent.second; 
      float er = (float) crp->rx_exception;
      float et = (float) crp->tx_exception;
      float sr = (float) crp->rx_standard; 
      float st = (float) crp->tx_standard; 

      char excess_char = ((crp->rx_exception * 100) >  crp->rx_standard) ? 'H' : 'N';

      if (crp->rx_exception != 0) {
	rxos << boost::format("%c %12s %6d %6d %10g %10g %g\n")
	  % excess_char
	  % ent.first
	  % crp->rx_standard % crp->rx_exception 
	  % (er / esum) % (er / sr) 
	  % ((er/sr)/(esum/ssum));
      }

      excess_char = ((crp->tx_exception * 100) >  crp->tx_standard) ? 'H' : 'N';
      if (crp->tx_exception != 0) {
	txos << boost::format("%c %12s %6d %6d %10g %10g %g\n")
	  % excess_char
	  % ent.first
	  % crp->tx_standard % crp->tx_exception 
	  % (et / esum) % (et / st) 
	  % ((et/st)/(esum/ssum));
      }
    }

    rxos.close();
    txos.close();    
  }

private:
  bool exc_mode; 
  std::map<std::string, CountRec *> call_table;
  CountRec totals; 

}; 

int main(int argc, char * argv[])
{
  bool input_gzipped; 
  std::string std_name, exc_name, report_name; 
  namespace po = boost::program_options;

  po::options_description desc("Options:");

  desc.add_options()
    ("help", "help message")
    ("standard", po::value<std::string>(&std_name)->required(), "WSPR log for baseline reports")
    ("exception", po::value<std::string>(&exc_name)->required(), "WSPR log for baseline reports")
    ("report", po::value<std::string>(&report_name)->required(), "Output data file containing calls, and risk ratios");
  
  po::positional_options_description pos_opts ;
  pos_opts.add("standard", 1);
  pos_opts.add("exception", 1);  
  pos_opts.add("report", 1);

  po::variables_map vm; 

  std::string what_am_i("Relative Probability of Exceptional Report by RX and TX Call Sign\n");
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



  myWSPRLog wlog;

  wlog.setExcMode(true);
  wlog.readLog(exc_name, input_gzipped);
  
  wlog.setExcMode(false);
  wlog.readLog(std_name, input_gzipped);
  
  wlog.dumpTables(report_name);
}
