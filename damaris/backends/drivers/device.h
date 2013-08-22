#include <string>
#include "core/job.h"
#include "core/result.h"
#include "core/core_exception.h"

#ifndef DEVICE_H
#define DEVICE_H


/**
 * device exception
 */
class device_error: public RecoverableException
{
public:
    explicit device_error(const std::string& msg) throw (): RecoverableException(msg) {}
    explicit device_error(const char* msg) throw (): RecoverableException(msg) {}
    virtual ~device_error() throw () {}
protected:
    virtual const std::string prefix() const { return "ERROR (core_exception): "; }
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
