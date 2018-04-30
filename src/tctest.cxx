#include "TimeCorr.hxx"
#include <boost/format.hpp>
#include <iostream>

int main(int argc, char * argv[]) {
  int i, j; 

  if(argc < 3) {
    std::cout << "tctest <fromgrid> <togrid>\n"; 
    exit(-1); 
  }

  float flto = TimeCorr::localTimeOffset(argv[1]);
  float tlto = TimeCorr::localTimeOffset(argv[2]);  
  float ftlto = TimeCorr::localTimeOffset(argv[1], argv[2]);
  float tflto = TimeCorr::localTimeOffset(argv[2], argv[1]);    
  std::cout << boost::format("from: %s to: %s flto = %f  tlto = %f ftlto = %f tflto = %f\n")
    % argv[1] % argv[2] % flto % tlto % ftlto % tflto;

  long epoch_time = 1522549800L;
  
  for(long i = 0; i < 24; i++) {
    long et = i * 3600 + epoch_time; 
    float fto = TimeCorr::localTime(argv[1], et);
    float tto = TimeCorr::localTime(argv[2],  et);
    float mto = TimeCorr::localTime(argv[1], argv[2],  et);


    struct tm ts, * tsp;
    tsp = gmtime_r(&et, &ts);
    if(tsp == NULL) {
      std::cout << "bad gmtime_r call\n";
    }
    float utchr = (float) ts.tm_hour + ((float) ts.tm_min) / 60.0;
    std::cout << boost::format("fto hrs %5.2f tto hrs %5.2f mto hrs %5.2f utchr %5.2f\n")
      % fto % tto % mto % utchr;
  }
}
