/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef PULSEGEN_H
#define PULSEGEN_H

#include <string>
#include "core/states.h"

/**
   \addtogroup basedrivers
   @{
*/

/**
   \brief pulse generation related error handling
 */
class pulse_exception: public std::string {
 public:
  pulse_exception(const std::string& s): std::string(s){}
};

/**
   \brief the pulse generator is the destination of the states sequences
 */

class pulsegen {
 public:
#if 1
  /**
     \brief a very simple pulse program, that could be hardcoded
     this pulse program is quite useful to see first results for hardware
   */
  virtual void single_pulse_program(double before, double length, double after)=0;
#endif

  /**
     compiles and runs the pulse program
   */
  virtual void run_pulse_program(state& exp)=0;

  /**
     wait till end of pulseprogram
   */
  virtual void wait_till_end()=0;

  virtual ~pulsegen(){}
};
/**
   @}
*/
#endif
