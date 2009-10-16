/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef HARDWARE_H
#define HARDWARE_H

class configuration_result;
class device_section;

#include <string>
#include <map>
#include "drivers/device.h"
#include "drivers/pfggen.h"
#include "drivers/ADC.h"
#include "drivers/pulsegen.h"
#include "drivers/frequgen.h"
#include "drivers/tempcont.h"
#include "drivers/device.h"
#include "core/states.h"
#include "core/job.h"
#include "core/result.h"

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
  /**
   * List of all DACs in the system
   *
   * The destructor of this class cleans them up
   */
  std::list<GenericDAC*> list_dacs;

  /**
     configurable devices dictionary
   */
  std::map<std::string, device*> configurable_devices;

  /**
     set all pointers to zero to signal missing hardware components
   */
  hardware() {the_adc=NULL; the_fg=NULL; the_pg=NULL; the_tc=NULL;}

  /**
   */
  virtual configuration_results* configure(const std::list<configuration_device_section>& d);

  /**
     initialise the main components with the experiment
  */
  virtual result* experiment(const state& exp);

  /**
     \brief shuts down hardware
   */
  virtual ~hardware();

 protected:
  /**
   * Utility function: Prepare the DACs for the experiment
   *
   * In this base class, it calls the set_dac() method on
   * each DAC in the list_dacs.
   *
   * This might include sending configuration info to the
   * DAC, or rewriting the pulse program.
   */
  virtual void experiment_prepare_dacs(state* work_copy);
};

#endif
