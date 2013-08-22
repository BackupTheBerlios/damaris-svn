/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef PULSEGEN_H
#define PULSEGEN_H

#include <string>
#include "core/states.h"
#include "core/core_exception.h"

/**
   \addtogroup basedrivers
   @{
*/

/**
 * pulse generator exception
 */
class pulse_exception: public RecoverableException
{
public:
    explicit pulse_exception(const std::string& msg) throw (): RecoverableException(msg) {}
    explicit pulse_exception(const char* msg) throw (): RecoverableException(msg) {}
    virtual ~pulse_exception() throw () {}
protected:
    virtual const std::string prefix() const { return "ERROR (pulse_exception): "; }
};

/**
   \brief the pulse generator is the destination of the states sequences
 */

class pulsegen {
 public:
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
