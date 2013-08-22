/* **************************************************************************

 Author: Markus Rosenstihl, Achim Gaedke
 Created: Nov 2005

****************************************************************************/
#ifndef PFGGEN_H
#define PFGGEN_H
#include "core/states.h"
#include "core/core_exception.h"

/**
   \addtogroup basedrivers
   @{
*/

/**
 * PFG exception
 */
class pfg_exception: public RecoverableException
{
public:
    explicit pfg_exception(const std::string& msg) throw (): RecoverableException(msg) {}
    explicit pfg_exception(const char* msg) throw (): RecoverableException(msg) {}
    virtual ~pfg_exception() throw () {}
protected:
    virtual const std::string prefix() const { return "ERROR (pfg_exception): "; }
};


/**
 * Generic base class for DAC drivers
 */
class GenericDAC {
 protected:
  /** no use by now... could be for default value at the end of an experiment?! */
  signed dac_value;

 public:
  /**
     no use by now! could be for default value at the end of an experiment?!
  */
  virtual void set_dac(signed d)=0;
  /**
     does the real work: transmits data to dac during pulse sequence
  */
  virtual void set_dac(state& experiment)=0;

  /**
    cleanup: nothing to do
  */
  virtual ~GenericDAC() {}
};

/**
   @}
*/
#endif
