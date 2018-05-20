#include "WSPRLog.hxx"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

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
  field_map["MAINSNR"] = WSPRLogEntry::MAINSNR;    
  field_map["POWER"] = WSPRLogEntry::POWER;
  field_map["AZ"] = WSPRLogEntry::AZ;
  field_map["FREQ_DIFF"] = WSPRLogEntry::FREQ_DIFF;
}

WSPRLogEntry::WSPRLogEntry(const std::string & line)
{
  std::vector<std::string> logent;
  std::vector<std::string> snrvec;
  
  if(fmt == NULL) {
    fmt = new boost::format("%d,%ld,%s,%s,%4.1f|%4.1f,%12.6f,%s,%s,%3.0f,%3.1f,%6f,%3f,%d,%s,%d,%d\n");
  }

  boost::algorithm::split(logent, line, boost::is_any_of(","));

  // just in case.  
  initMaps(); 

  // now use the hints from http://wsprnet.org/drupal/downloads
  spot_id = std::stoul(logent[0]);
  dtime = std::stoul(logent[1]);
  rxcall = boost::algorithm::trim_copy(logent[2]);
  rxgrid = boost::algorithm::trim_copy(logent[3]);
  boost::algorithm::split(snrvec, logent[4], boost::is_any_of("|"));
  if(snrvec.size() > 1) {
    snr = std::stof(snrvec[0]);
    main_snr = std::stof(snrvec[1]);            
  }
  else {
    snr = std::stof(logent[4]);
    main_snr = snr; 
  }
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
  update_count_interval = 250000;
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

  os << *fmt
    % spot_id
    % dtime
    % rxcall
    % rxgrid
    % snr
    % main_snr
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
  if(field_map.size() == 0) initMaps();

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
  case WSPRLogEntry::MAINSNR:    
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
  case WSPRLogEntry::MAINSNR:    
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
  case WSPRLogEntry::MAINSNR:
    val = main_snr; 
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

  switch (sel) {
  case WSPRLogEntry::SPOT:
  case WSPRLogEntry::BAND:
  case WSPRLogEntry::CODE:
  case WSPRLogEntry::DTIME:
    val = (int) dtime;

  case WSPRLogEntry::VERSION:
  case WSPRLogEntry::TXCALL:
  case WSPRLogEntry::RXCALL:
  case WSPRLogEntry::TXGRID:
  case WSPRLogEntry::RXGRID:
    return false; 
    break; 

  case WSPRLogEntry::DRIFT:
    val = (int) drift;
    break; 
  case WSPRLogEntry::FREQ:
    val = (int) freq; 
    break; 
  case WSPRLogEntry::DIST:
    val = (int) dist; 
    break; 
  case WSPRLogEntry::SNR:
    val = (int) snr; 
    break; 
  case WSPRLogEntry::MAINSNR:
    val = (int) main_snr; 
    break; 
  case WSPRLogEntry::POWER:
    val = (int) power; 
    break; 
  case WSPRLogEntry::AZ:
    val = (int) az; 
    break; 
  case WSPRLogEntry::FREQ_DIFF:
    val = freq_diff; 
    break; 
  default: 
    return false;
    break; 
  }
  return true; 
}

void WSPRLog::readLog(std::string infname, bool is_gzipped)
{
  if(is_gzipped) {
    std::ifstream gzfile(infname, std::ios_base::in | std::ios_base::binary);
    boost::iostreams::filtering_streambuf<boost::iostreams::input> inbuf;
    if(!gzfile.good()) {
      std::cerr << boost::format("Could not open input file [%s] for reading.\n") % infname; 
    }
     
    inbuf.push(boost::iostreams::gzip_decompressor());
    inbuf.push(gzfile);
    std::istream inf(&inbuf);
    readLog(inf);
    gzfile.close();
  }
  else {
    std::ifstream inf(infname);
    if(!inf.good()) {
      std::cerr << boost::format("Could not open input file [%s] for reading.\n") % infname; 
    }
    readLog(inf);
    inf.close();
  }
}
 
