#include "WSPRLog.hxx"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

std::map<std::string, WSPRLogEntry::Field> WSPRLogEntry::field_map;
boost::format * WSPRLogEntry::fmt = NULL; 

void WSPRLogEntry::initMaps() {
  if(field_map.size() != 0) return; 
  
  field_map["SPOT"] = WSPRLogEntry::SPOT;
  field_map["DRIFT"] = WSPRLogEntry::DRIFT;
  field_map["BAND"] = WSPRLogEntry::BAND;
  field_map["VERSION"] = WSPRLogEntry::VERSION;
  field_map["CODE"] = WSPRLogEntry::CODE;
  field_map["TXCALL"] = WSPRLogEntry::TXCALL;
  field_map["RXCALL"] = WSPRLogEntry::RXCALL;
  field_map["TXGRID"] = WSPRLogEntry::TXGRID;
  field_map["RXGRID"] = WSPRLogEntry::RXGRID;
  field_map["FREQ"] = WSPRLogEntry::FREQ;
  field_map["DTIME"] = WSPRLogEntry::DTIME;
  field_map["DIST"] = WSPRLogEntry::DIST;
  field_map["SNR"] = WSPRLogEntry::SNR;
  field_map["POWER"] = WSPRLogEntry::POWER;
  field_map["AZ"] = WSPRLogEntry::AZ;
  field_map["FREQ_DIFF"] = WSPRLogEntry::FREQ_DIFF;
}

WSPRLogEntry::WSPRLogEntry(const std::string line)
{
  std::vector<std::string> logent;

  if(fmt == NULL) {
    fmt = new boost::format("%d,%ld,%s,%s,%f,%12.6f,%s,%s,%f,%f,%f,%f,%d,%s,%d,%d\n");
  }

  boost::algorithm::split(logent, line, boost::is_any_of(","));

  // just in case.  
  initMaps(); 

  // now use the hints from http://wsprnet.org/drupal/downloads
  spot_id = std::stoul(logent[0]);
  dtime = std::stoul(logent[1]);
  rxcall = boost::algorithm::trim_copy(logent[2]);
  rxgrid = boost::algorithm::trim_copy(logent[3]);
  snr = std::stof(logent[4]);
  freq = std::stod(logent[5]);
  txcall = boost::algorithm::trim_copy(logent[6]); 
  txgrid = boost::algorithm::trim_copy(logent[7]);
  power = std::stof(logent[8]);
  drift = std::stof(logent[9]);
  dist = std::stof(logent[10]);
  az = std::stof(logent[11]);
  band = std::stoi(logent[12]);
  version = boost::algorithm::trim_copy(logent[13]); 
  if(logent.size() > 14) {
    code = std::stoi(logent[14]);
  }
  else code = 0; 
  if (logent.size() > 15) {
    freq_diff = std::stoi(logent[15]);
  }
  else freq_diff = 0; 
}

WSPRLog::WSPRLog()
{
  update_count_interval = 10000;
  line_count = 0; 
}

void WSPRLog::readLog(std::istream & inf) 
{
  std::string line; 

  while(1) {
    WSPRLogEntry *le = WSPRLogEntry::get(inf);
    if(le == NULL) break;
    if(isKeeper(le)) {
      processEntry(le); 
    }
    updateCheck(); 
  }
}



std::ostream &  WSPRLogEntry::print(std::ostream & os)
{
  std::string ver = version; 
  if(version == "") ver = "UNKNOWN";

#if 0  
  os << spot_id << "," << dtime << "," << rxcall << "," << rxgrid 
     << "," << snr << "," 
     << std::setw(12)  << std::setprecision(6) << freq 
     << "," << txcall << "," << txgrid << "," << power << "," << drift << "," << dist << "," << az << "," << band << "," << ver << "," << code << "," << freq_diff << std::endl; 
#else
  os << *fmt
    % spot_id
    % dtime
    % rxcall
    % rxgrid
    % snr
    % freq
    % txcall
    % txgrid
    % power
    % drift
    % dist
    % az
    % band
    % ver
    % code
    % freq_diff;
#endif
  
  return os;     
}

WSPRLogEntry * WSPRLogEntry::get(std::istream & is)
{
  std::string line; 

  while(1) {
    getline(is, line); 
    if(!is.good()) return NULL; 
    if(line == "") continue;
    
    auto * ret = new WSPRLogEntry(line); 
  
    return ret; 
  }

  return NULL;
}

WSPRLogEntry::Field WSPRLogEntry::str2Field(const std::string & str)
{
  if(field_map.find(str) == field_map.end()) return UNDEFINED;
  else return field_map[str]; 
}

bool WSPRLogEntry::getField(WSPRLogEntry::Field sel, unsigned long & val)
{

  switch (sel) {
  case WSPRLogEntry::SPOT:
    val = spot_id; 
    break; 
  case WSPRLogEntry::BAND:
    val = band;
    break; 
  case WSPRLogEntry::CODE:
    val = code; 
    break; 
  case WSPRLogEntry::DTIME:
    val = dtime; 
    break; 

  case WSPRLogEntry::VERSION:
  case WSPRLogEntry::TXCALL:
  case WSPRLogEntry::RXCALL:
  case WSPRLogEntry::TXGRID:
  case WSPRLogEntry::RXGRID:

  case WSPRLogEntry::DRIFT:
  case WSPRLogEntry::FREQ:
  case WSPRLogEntry::DIST:
  case WSPRLogEntry::SNR:
  case WSPRLogEntry::POWER:
  case WSPRLogEntry::AZ:
  case WSPRLogEntry::FREQ_DIFF:
  default: 
    return false;
    break; 
  }
  return true; 
}

bool WSPRLogEntry::getField(WSPRLogEntry::Field sel, std::string & val)
{

  switch (sel) {
  case WSPRLogEntry::SPOT:
  case WSPRLogEntry::BAND:
  case WSPRLogEntry::CODE:
  case WSPRLogEntry::DTIME:
    return false; 
    break; 

  case WSPRLogEntry::VERSION:
    val = version; 
    break; 
  case WSPRLogEntry::TXCALL:
    val = txcall; 
    break; 
  case WSPRLogEntry::RXCALL:
    val = rxcall; 
    break; 
  case WSPRLogEntry::TXGRID:
    val = txgrid; 
    break; 
  case WSPRLogEntry::RXGRID:
    val = rxgrid; 
    break;

  case WSPRLogEntry::DRIFT:
  case WSPRLogEntry::FREQ:
  case WSPRLogEntry::DIST:
  case WSPRLogEntry::SNR:
  case WSPRLogEntry::POWER:
  case WSPRLogEntry::AZ:
  case WSPRLogEntry::FREQ_DIFF:
  default: 
    return false;
    break; 
  }
  return true; 
}

bool WSPRLogEntry::getField(WSPRLogEntry::Field sel, double & val)
{

  switch (sel) {
  case WSPRLogEntry::SPOT:
  case WSPRLogEntry::BAND:
  case WSPRLogEntry::CODE:
  case WSPRLogEntry::DTIME:

  case WSPRLogEntry::VERSION:
  case WSPRLogEntry::TXCALL:
  case WSPRLogEntry::RXCALL:
  case WSPRLogEntry::TXGRID:
  case WSPRLogEntry::RXGRID:
    return false; 
    break; 

  case WSPRLogEntry::DRIFT:
    val = drift;
    break; 
  case WSPRLogEntry::FREQ:
    val = freq; 
    break; 
  case WSPRLogEntry::DIST:
    val = dist; 
    break; 
  case WSPRLogEntry::SNR:
    val = snr; 
    break; 
  case WSPRLogEntry::POWER:
    val = power; 
    break; 
  case WSPRLogEntry::AZ:
    val = az; 
    break; 

  default: 
    return false;
    break; 
  }
  return true; 
}


bool WSPRLogEntry::getField(WSPRLogEntry::Field sel, int & val)
{

  if(sel == FREQ_DIFF) {
    val = freq_diff; 
    return true; 
  }

  return false; 
}

