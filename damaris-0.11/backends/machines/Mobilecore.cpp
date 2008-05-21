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

  PTS* my_pts;
  SpinCorePulseBlaster24Bit* my_pulseblaster;
  SpectrumMI40xxSeries* my_adc;

public:
  Mobile_hardware(){
      ttlout trigger;
      trigger.id=0;
      trigger.ttls=1<<17; /* line 2 */
      my_adc=new SpectrumMI40xxSeries(trigger);
      // device_id=0, clock=100MHz, sync_mask: Bit 16
      my_pulseblaster=new SpinCorePulseBlaster24Bit(0,1e8,1<<16);
      my_pts=new PTS_latched(0);

      // publish devices
      the_pg=my_pulseblaster;
      the_adc=my_adc;
      the_fg=my_pts;
  }

  result* experiment(const state& exp) {
    result* r=NULL;
    for(size_t tries=0; r==NULL && core::term_signal==0 &&  tries<102; ++tries) {
      state* work_copy=exp.copy_flat();
      if (work_copy==NULL) return new error_result(1,"could create work copy of experiment sequence");
      try {
	if (the_fg!=NULL)
	  the_fg->set_frequency(*work_copy);
	if (the_adc!=NULL)
	  the_adc->set_daq(*work_copy);
	if (the_gradientpg!=NULL)
	  the_gradientpg->set_dac(*work_copy);
	// the pulse generator is necessary
	my_pulseblaster->run_pulse_program_w_sync(*work_copy, my_adc->get_sample_clock_frequency());
	// wait for pulse generator
	the_pg->wait_till_end();
	// after that, the result must be available
	if (the_adc!=NULL)
	  r=the_adc->get_samples();
	else
	  r=new adc_result(1,0,NULL);
      }
      catch (frequ_exception e) {
	r=new error_result(1,"frequ_exception: "+e);
      }
      catch (ADC_exception e) {
	if (e!="ran into timeout!" || tries>=100)
	  r=new error_result(1,"ADC_exception: "+e);
      }
      catch (pulse_exception p) {
	r=new error_result(1,"pulse_exception: "+p);
      }
      delete work_copy;
      if (core::quit_signal!=0) break;
    }
    return r;
  }

  virtual ~Mobile_hardware() {
    if (the_adc!=NULL) delete the_adc;
    if (the_fg!=NULL) delete the_fg;
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
