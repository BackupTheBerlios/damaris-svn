/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include "machines/hardware.h"
#include "core/core.h"
#include "drivers/Eurotherm-2000Series/Eurotherm-2000Series.h"
#include "drivers/PTS-Synthesizer/PTS.h"
#include "drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h"
#include "drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h"

/**
   \defgroup magnexgradmachine Magnex Static Gradient NMR Spectrometer
   \ingroup machines
   Uses Spincore Pulseblaster 24 Bit and Spectrum MI4021 together with PTS phase and frequency cable driver
   Also implements Eurotherm temperature control

   \par Starting the hardware
   Switch on the amplifier
   @{
*/

/**
   line 0 for gate
   line 1 for pulse
   line 2 for trigger
   line 3 free
 */
class magnexgrad_hardware: public hardware {

public:
  magnexgrad_hardware(){
      ttlout trigger;
      trigger.id=0;
      trigger.ttls=4; /* line 2 */
      the_adc=new SpectrumMI40xxSeries(trigger);
      the_pg=new SpinCorePulseBlaster24Bit();
      PTS* my_pts=new PTS_latched(0);
      the_fg=my_pts;
      the_tc=new Eurotherm2000Series("/dev/ttyS0",2,0x0);
  }


  /**
     print out a temperature line
   */
  virtual result* experiment(const state& exp) {
    result* r=hardware::experiment(exp);
    /**
       make a timestamp
     */
    time_t timestamp;
    time(&timestamp);
    tm timestamp_broken;
    localtime_r(&timestamp, &timestamp_broken);
    char timestamp_string[254];
    strftime(timestamp_string,254, "%F %T", &timestamp_broken);
    timeval exact_time;
    gettimeofday(&exact_time, NULL);

    if (the_tc!=NULL) {
      try {
	double temp=the_tc->get_temperature();
	fprintf(stdout, "%s.%03d temperature=%f\n",timestamp_string, exact_time.tv_usec/1000, temp);
      }
      catch (Eurotherm2000Series_error e) {
	fprintf(stdout, "%s.%03d temperature=0.0 (error %s)\n",timestamp_string, exact_time.tv_usec/1000, e.c_str());
      }
      fflush(stdout);
    }
    return r;
  }

  virtual ~magnexgrad_hardware() {
    if (the_adc!=NULL) delete the_adc;
    if (the_pg!=NULL) delete the_pg;
    if (the_fg!=NULL) delete the_fg;
    if (the_tc!=NULL) delete the_tc;
  }

};

/**
   \brief brings standard core together with the Mobile NMR hardware
*/
class magnexgrad_core: public core {
  std::string the_name;
public:
    magnexgrad_core(const core_config& conf): core(conf) {
	the_hardware=new magnexgrad_hardware();
	the_name="magnexgrad";
  }
  virtual const std::string& core_name() const {
  	return the_name;
  }
};

/**
   @}
 */

int main(int argc, const char** argv) {
  int return_result=0;
  try {
      core_config my_conf(argv, argc);
      // setup input and output
      magnexgrad_core my_core(my_conf);
      // start core application
      my_core.run();
  }
  catch(ADC_exception ae) {
    fprintf(stderr,"adc: %s\n",ae.c_str());
    return_result=1;
  }
  catch(core_exception ce) {
    fprintf(stderr,"core: %s\n",ce.c_str());
    return_result=1;
  }
  catch(pulse_exception pe) {
    fprintf(stderr,"pulse: %s\n",pe.c_str());
    return_result=1;
  }
  catch (tempcont_error te) {
    fprintf(stderr,"temperature control: %s\n",te.c_str());
    return_result=1;
  }
  return return_result;
}
