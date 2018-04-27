#ifndef HISTOGRAM_HDR
#define HISTOGRAM_HDR
#include <map>
#include <iostream> 

template <typename K> class Histogram {
public:
  Histogram() {
  }

  void addEntry(const K & key) {
    if(hist.find(key) == key.end()) {
      hist[key] = 1; 
    }
    else {
      hist[key] = hist[key] + 1; 
    }
  }

  std::ostream & print(std::ostream & os) {
    for(auto kvp: hist) {
      os << kvp.first << " " << kvp.second << std::endl; 
    }
    return os; 
  }

protected:
  std::map<K, unsigned int> hist; 
};



#endif

