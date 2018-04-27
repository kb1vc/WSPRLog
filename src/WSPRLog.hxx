#ifndef WSRPLOG_HDR
#define WSRPLOG_HDR

#include <map>
#include <string>
#include <list>
#include <vector>
#include <iostream>
#include <boost/format.hpp>
#include <fstream>

class WSPRLogEntry {
public: 
  WSPRLogEntry(const std::string line); 
  
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
  float power;
  float az; 
  double freq_diff; 

  enum Field { SPOT, DRIFT, BAND, VERSION, CODE, TXCALL, RXCALL, TXGRID, RXGRID, 
	       FREQ, DTIME, DIST, SNR, POWER, AZ, FREQ_DIFF, UNDEFINED } ;

  Field str2Field(const std::string & str); 
  
  bool getField(Field sel, unsigned long & val); 
  bool getField(Field sel, double & val); 
  bool getField(Field sel, std::string & val); 
  
  
  void calcDiff(const WSPRLogEntry & ot) { freq_diff = 1e6*(freq - ot.freq); }

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
  void initMaps(); 

  static std::map<std::string, Field> field_map;
}; 


class WSPRLog {
public:
  WSPRLog(); 

  void readLog(std::istream & in); 

  /// use this for filtering by some property (like a field value)
  virtual bool isKeeper(WSPRLogEntry * ent) { return true; }

  virtual bool processEntry(WSPRLogEntry * ent) = 0; 

  virtual void updateCheck() {
    line_count++;
    if((line_count % update_count_interval) == 0) {
      std::cerr << boost::format("Line_Count = %d\r") % line_count;
    }
  }
private: 
  int update_count_interval; 
  int line_count;
}; 

#endif

