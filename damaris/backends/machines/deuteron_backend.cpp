/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include "machines/hardware.h"
#include "core/core.h"
#include "drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h"
#include "drivers/SpinCore-PulseBlasterDDSIII/SpinCore-PulseBlasterDDSIII.h"

/**
   \defgroup deuteronmachine Magnex Static Gradient NMR Spectrometer
   \ingroup machines
   Uses Spincore Pulseblaster DDSIII and Spectrum MI4021


   \par Starting the hardware
   Switch on the amplifier
   @{
*/

/**
   line 0 for gate
   line 1 for pulse
   line 2 for trigger
   line 3 for sync active
 */
class deuteron_hardware: public hardware {

  SpinCorePulseBlasterDDSIII* dds;
  SpectrumMI40xxSeries* my_adc;


public:
 deuteron_hardware(){
      ttlout trigger;
      trigger.id=0;
      trigger.ttls=4; /* line 2 */
      my_adc=new SpectrumMI40xxSeries(trigger);
      dds=new SpinCorePulseBlasterDDSIII(0,     // ttlout id
					 100e6, //clock
					 0x8);  // sync active: line 3
      
      the_adc=my_adc;
      the_pg=dds;
      the_fg=dds;
      the_tc=NULL;

  }

  virtual result* experiment(const state& exp) {
    result* r=NULL;
    for(size_t tries=0; r==NULL && core::term_signal==0 &&  tries<102; ++tries) {
      state* work_copy=exp.copy_flat();
      if (work_copy==NULL) return new error_result(1,"could create work copy of experiment sequence");
      try {
	if (the_fg!=NULL)
	  the_fg->set_frequency(*work_copy);
	if (the_adc!=NULL)
	  the_adc->set_daq(*work_copy);
	// the pulse generator is necessary
	dds->run_pulse_program_w_sync(*work_copy, my_adc->get_sample_clock_frequency());
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


  virtual ~deuteron_hardware() {
    if (the_adc!=NULL) {delete the_adc; the_adc=my_adc=NULL;}
    if (the_pg!=NULL) {delete the_pg; the_pg=dds=NULL;}
    if (the_tc!=NULL) delete the_tc;
  }

};

/**
   \brief brings standard core together with the deuteron spectrometer hardware
*/
class deuteron_core: public core {
  std::string the_name;
public:
    deuteron_core(const core_config& conf): core(conf) {
      the_hardware=new deuteron_hardware();
      the_name="deuteron";
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
      deuteron_core my_core(my_conf);
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
