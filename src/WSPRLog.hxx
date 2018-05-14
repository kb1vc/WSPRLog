#ifndef WSRPLOG_HDR
#define WSRPLOG_HDR

#include <map>
#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <boost/format.hpp>
#include <fstream>
#include <cmath>

class WSPRLogEntry {
public: 
  WSPRLogEntry(const std::string & line); 
  
  unsigned long spot_id; 
  float drift;
  int band;
  std::string version;
  int code;
  std::string txcall; 
  std::string rxcall; 
  std::string txgrid; 
  std::string rxgrid; 
  double freq; 
  unsigned long dtime; // time since epoch in seconds. 
  float dist; // length of path
  float snr; 
  float main_snr; // If this is an image, snr reported by the "zero offset" entry
  float power;
  float az; 
  int freq_diff; 

  enum Field { SPOT, DRIFT, BAND, VERSION, CODE, TXCALL, RXCALL, TXGRID, RXGRID, 
	       FREQ, DTIME, DIST, SNR, POWER, AZ, FREQ_DIFF, MAINSNR, UNDEFINED } ;

  static Field str2Field(const std::string & str); 

  static void printFieldChoices(std::ostream & os) { 
    os << "One of: "; 
    int i = 0; 
    for(auto sel: field_map) {
      os << sel.first << " ";  
      i++; 
      if((i % 6) == 0) os << std::endl; 
    }
  }
  
  bool getField(Field sel, unsigned long & val); 
  bool getField(Field sel, double & val); 
  bool getField(Field sel, std::string & val);
  bool getField(Field sel, int & val);   
  
  
  void calcDiff(const WSPRLogEntry * ot) { 
    freq_diff = (int) floor(1e6*(freq - ot->freq)); 
    if(this != ot) {
      snr = snr - ot->snr; 
      main_snr = ot->snr;
    }
  }

  std::ostream & print(std::ostream & os);

  static WSPRLogEntry * get(std::istream & is);

  static bool compareTimestamps(const WSPRLogEntry * first, const WSPRLogEntry * second)
  {
    return first->dtime < second->dtime; 
  }

  static bool compareFreq(const WSPRLogEntry * first, const WSPRLogEntry * second)
  {
    return first->freq < second->freq; 
  }

  static bool compareSNR(const WSPRLogEntry * first, const WSPRLogEntry * second)
  {
    return first->snr > second->snr; 
  }

private:
  static void initMaps(); 
  static boost::format * fmt; 

  static std::map<std::string, Field> field_map;
}; 


class WSPRLog {
public:
  WSPRLog(); 

  void readLog(std::istream & in); 
  void readLog(std::string infname, bool is_gzipped = false); 

  /// use this for filtering by some property (like a field value)
  virtual bool isKeeper(WSPRLogEntry * ent) { return true; }

  virtual bool processEntry(WSPRLogEntry * ent) = 0; 

  virtual void updateCheck() {
    line_count++;
    if((line_count % update_count_interval) == 0) {
      std::cerr << boost::format("Line_Count = %d\n") % line_count;
    }
  }
private: 
  int update_count_interval; 
  int line_count;
}; 

#endif

 
