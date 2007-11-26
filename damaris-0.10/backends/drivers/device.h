#include <string>
#include "core/job.h"
#include "core/result.h"

#ifndef DEVICE_H
#define DEVICE_H


class device_error: public std::string {
 public:
  device_error(std::string message): std::string(message) {}
  device_error(const device_error& orig): std::string(orig) {}
};


/**
   base class for devices
   devices can be configured   
 */
class device {
 public:

  /**
     configuration interface
     if no result: call that section again
     run is incremented for each call, starting with 0
   */
  virtual configuration_result* configure(const configuration_device_section& conf, int run)=0;

  virtual ~device() {}
};


#endif
