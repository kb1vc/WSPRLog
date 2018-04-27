#include "WSPRLog.hxx"

#include <boost/format.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <math.h>

class myWSPRLog : public WSPRLog {
public:
  myWSPRLog() : WSPRLog() {
    last_time = 0; 
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

  void addToHistogram(int diff) {
    if(histo_map.find(diff) != histo_map.end()) {
      histo_map[diff] += 1; 
    }
    else {
      histo_map[diff] = 1; 
    }
  }

  void dumpPairs() {
    for(auto & mapent : pair_map) {
      if(mapent.second.size() > 1) {
	mapent.second.sort(WSPRLogEntry::compareSNR); 
	double base_freq = mapent.second.front()->freq; 
	// two reporters in the same segment
	WSPRLogEntry * fle = mapent.second.front();
	fle->calcDiff(*fle);
	std::cout << "\n";
	
	std::string prefix = (boost::format("tx: %10s %6s time: %ld  dist: %d  pwr: %g ") 
			      % fle->txcall % fle->txgrid % fle->dtime % fle->dist % fle->power).str();
	for(auto & le : mapent.second) {
	  le->calcDiff(*fle); 
	  double fdiff = 0.0;
	  le->getField(WSPRLogEntry::DRIFT, fdiff);
	  addToHistogram((int) floor(fdiff * 1e6 + 0.5)); 

	  le->print(std::cout); 
	}
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

  void writeHisto(std::ostream & os) {
    int sum = 0; 
    for(auto hent : histo_map) {    
      if(hent.first != 0) sum += hent.second; 
    }

    float fsum = float(sum); 
    for(auto hent : histo_map) {
      // there is one ZERO offset report for every image report. 
      // don't count it...
      int count = (hent.first == 0) ? 0 : hent.second; 
      float fcount = float(count); 
      if(hent.first == 0) continue;
      os << boost::format("%d  %g\n") % hent.first % (fcount / fsum); 
    }
  }
private:
  unsigned long last_time; 
  std::map<std::string, std::list<WSPRLogEntry *> > pair_map; 
  std::map<int, int> histo_map; 
}; 

int main(int argc, char * argv[])
{
  std::string fna(argv[1]);
  myWSPRLog wlog;

  std::ifstream inf(fna);
  wlog.readLog(inf);

  std::string hna(argv[2]);
  std::ofstream os(hna);
  wlog.writeHisto(os);
}
