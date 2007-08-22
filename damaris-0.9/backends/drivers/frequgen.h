/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef FREQUGEN_H
#define FREQUGEN_H
#include "core/states.h"

/**
   \addtogroup basedrivers
   @{
*/

/**
   \brief frequency generation related error handling
 */
class frequ_exception: public std::string {
 public:
  frequ_exception(const std::string& s): std::string(s){}
};


/**
   \brief the frequency generator can set the reference frequency and pulse output
   also phase shifts and different channels are possible
 */
class frequgen {
 protected:
  /** reference frequency output */
  double frequency;
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
  virtual void set_frequency(double f)=0;
  virtual void set_frequency(state& experiment)=0;
  virtual ~frequgen() {}
};

/**
   @}
*/
#endif
