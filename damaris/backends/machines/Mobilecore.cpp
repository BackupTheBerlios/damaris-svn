/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include "machines/hardware.h"
#include "core/core.h"
#include "drivers/PTS-Synthesizer/PTS.h"
#include "drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h"
#include "drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h"

/**
   \defgroup mobilemachine Mobile NMR Spectrometer
   \ingroup machines
   Uses Spincore Pulseblaster 24 Bit and Spectrum MI4021, but there is no cable driver with phase control

   \par Starting the hardware
   This procedure should assure the correct initialisation of the hardware:
   \li Switch off main switches of SpinCore Pulseblaster and Computer (the main switch of the computer is at the rear)
   \li Switch on Computer and start Windows or linux

   @{
*/



/**
   line 0 for gate
   line 1 for pulse
   line 2 for trigger
   line 3 free
 */
class Mobile_hardware: public hardware {

public:
  Mobile_hardware(){
      ttlout trigger;
      trigger.id=0;
      trigger.ttls=4; /* line 2 */
      the_adc=new SpectrumMI40xxSeries(trigger);
      the_pg=new SpinCorePulseBlaster24Bit();
      PTS* my_pts=new PTS(0);
      my_pts->negative_logic=0;
      for (int i=23; i>=15; --i) {
	ttlout phase_out;
	phase_out.id=0;
	phase_out.ttls=1<<i;
	my_pts->ttl_masks.push_back(phase_out);
      }	
      the_fg=my_pts;
  }

  virtual ~Mobile_hardware() {
    if (the_adc!=NULL) delete the_adc;
    if (the_pg!=NULL) delete the_pg;
  }

};

/**
   \brief brings standard core together with the Mobile NMR hardware
*/
class Mobile_core: public core {
  std::string the_name;
public:
    Mobile_core(const core_config& conf): core(conf) {
	the_hardware=new Mobile_hardware();
	the_name="Mobile core";
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
      Mobile_core my_core(my_conf);
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
  return return_result;
}
