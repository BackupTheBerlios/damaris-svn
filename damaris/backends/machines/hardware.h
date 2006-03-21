/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef HARDWARE_H
#define HARDWARE_H
#include "drivers/pfggen.h"
#include "drivers/ADC.h"
#include "drivers/pulsegen.h"
#include "drivers/frequgen.h"
#include "drivers/tempcont.h"
#include "core/states.h"

/**
   \defgroup machines Different Machines
   Implementation of different machine setups
*/

/**
   \addtogroup machines
   \brief provides necessary hardware interfaces for a NMR experiment
 */
class hardware {
 public:
  /// adc component
  ADC* the_adc;
  /// frequency generator
  frequgen* the_fg;
  /// pulse generator
  pulsegen* the_pg;
  /// the temperature control, optional
  tempcont* the_tc;
  /// the  gradient pulse control
  pfggen* the_gradientpg;

  /**
     set all pointers to zero to signal missing hardware components
   */
  hardware() {the_adc=NULL; the_fg=NULL; the_pg=NULL; the_tc=NULL; the_gradientpg=NULL;}
  /**
     simple experiment
   */
  virtual result* single_pulse_experiment(double frequency, double t_before, double t, double sample_freq, size_t samples, signed dac_value);
  /**
     initialise the main components with the experiment
  */
  virtual result* experiment(const state& exp);

  /**
     \brief shuts down hardware
   */
  virtual ~hardware() {}
};

#endif
