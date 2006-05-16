/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/

#include "drivers/device.h"
#include "drivers/ADC.h"
#include "drivers/frequgen.h"
#include "drivers/pulsegen.h"
#include "drivers/tempcont.h"
#include <cmath>

class dummy: public ADC, public frequgen, public pulsegen, public tempcont, public device {
  double frequency;
  /**
     save artificial results while running pulse program
  */
  adc_results* new_results;

 public:
  dummy();

  virtual void set_frequency(double f) {frequency=f;}

  virtual void set_frequency(state& exp);

  /**
     sample once after trigger signal
   */
  virtual void sample_after_external_trigger(double rate, size_t no, double sens=5.0, size_t res=12);

  /**
     emulate a simple single pulse program...
   */
  virtual void single_pulse_program(double before, double length, double after);

  virtual result* get_samples(double timeout=0.0);

  /**
     gets information about daq settings
   */
  virtual void set_daq(state& exp);

  /**
     print states of the subprogram and generate ADC results
   */
  virtual void run_pulse_program(state& exp);

  /**
     do not wait, just continue
   */
  virtual void wait_till_end(){}

  /**
     temperature control: get the actual temperature
   */
  virtual double get_temperature() const;

  /**
     temperature control: set temperature control value
     \return temperature, that is set
  */
  virtual double set_setpoint(double temperature);

  /**
     temperature control: get temperature control value
     \return temperature, that is set
  */
  virtual double get_setpoint() const;

  /**
   */
  virtual configuration_result* configure(const configuration_device_section& conf, int run) {
    if (run<10) {
      fprintf(stdout, "%d\n",run);
      return NULL;
    }
    conf.print();
    return new configuration_result();
  }

  virtual ~dummy() {}
};
