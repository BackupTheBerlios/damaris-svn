/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include "machines/hardware.h"
#include "core/core.h"
#include "drivers/Tecmac-DAC20/DAC20.h"
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
class PFG_hardware: public hardware {

public:
  PFG_hardware(){
      ttlout trigger;
      trigger.id=0;
      trigger.ttls=4; /* line 2 */// 
      the_adc=new SpectrumMI40xxSeries(trigger);
      the_pg=new SpinCorePulseBlaster24Bit();
      PTS* my_pts=new PTS_latched(0); // ID of PTS_analogout 0
      the_fg=my_pts;
      PFG* my_pfg=new PFG(1);
      the_gradientpg=my_pfg;
  }

  virtual ~PFG_hardware() {
    if (the_adc!=NULL) delete the_adc;
    if (the_pg!=NULL) delete the_pg;
	if (the_gradientpg!=NULL) delete the_gradientpg;
  }

};

/**
   \brief brings standard core together with the Mobile NMR hardware
*/
class PFG_core: public core {
  std::string the_name;
public:
    PFG_core(const core_config& conf): core(conf) {
	the_hardware=new PFG_hardware();
	the_name="PFG core";
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
      PFG_core my_core(my_conf);
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
  catch(pfg_exception pfge) {
	  fprintf(stderr,"pfg: %s\n",pfge.c_str());
	  return_result=1;
  }
  
  return return_result;
}
