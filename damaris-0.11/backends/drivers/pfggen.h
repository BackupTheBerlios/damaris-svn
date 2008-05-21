/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef PFGGEN_H
#define PFGGEN_H
#include "core/states.h"

/**
   \addtogroup basedrivers
   @{
*/

/**
   \brief frequency generation related error handling
 */
class pfg_exception: public std::string {
 public:
  pfg_exception(const std::string& s): std::string(s){}
};


/**
   \brief the frequency generator can set the reference frequency and pulse output
   also phase shifts and different channels are possible
 */
class pfggen {
 protected:
  /** reference frequency output */
  signed dac_value;
  /**
     default phase offset of the frequency generator
   */
  double phase;
  /**
     should move to frequgen
   */
  int pulse_channel;
  /**
     should move to frequgen
   */
  int gate_channel;
  /**
     should move to frequgen
   */
  double gate_time;
 public:
  virtual void set_dac(signed d)=0;
  virtual void set_dac(state& experiment)=0;
  virtual ~pfggen() {}
};

/**
   @}
*/
#endif
