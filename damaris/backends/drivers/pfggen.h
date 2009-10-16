/* **************************************************************************

 Author: Markus Rosenstihl, Achim Gaedke
 Created: Nov 2005

****************************************************************************/
#ifndef PFGGEN_H
#define PFGGEN_H
#include "core/states.h"

/**
   \addtogroup basedrivers
   @{
*/

/**
   \brief pulsed field gradient related error handling
 */
class pfg_exception: public std::string {
 public:
  pfg_exception(const std::string& s): std::string(s){}
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
