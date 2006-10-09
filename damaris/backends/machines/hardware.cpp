/* **************************************************************************

 Author: Achim Gaedke
 Created: August 2004

****************************************************************************/

#include "core/core.h"
#include "core/result.h"
#include "hardware.h"

result* hardware::single_pulse_experiment(double frequency, double t_before, double t, double sample_freq, size_t samples, signed dac_value) {
# if 1
  the_fg->set_frequency(frequency);
  the_adc->sample_after_external_trigger(sample_freq,samples,sample_freq,samples);
  the_gradientpg->set_dac(dac_value);
  the_pg->single_pulse_program(t_before,t,((double)samples)/sample_freq);
  return the_adc->get_samples();
#else
  throw ADC_exception("no longer supported");
  return NULL;
#endif
}

result* hardware::experiment(const state& exp) {
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
      the_pg->run_pulse_program(*work_copy);
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

configuration_results* hardware::configure(const std::list<configuration_device_section>& d) {
  configuration_results* r=new configuration_results(0);
  
  // create a work list
  std::list<const configuration_device_section*> to_configure;
  for (std::list<configuration_device_section>::const_iterator i=d.begin(); i!=d.end(); ++i) {
    // if the device name is available, we will add a pointer, otherwise a remark about missing device
    if (configurable_devices.count(i->name)!=0) to_configure.push_back(&(*i));
    else {
      // todo generate a remark
      r->push_back(new configuration_result(0));
    }
  }

  // go through this list again and again, until all devices returned at least something...
  int run=0;
  for (int run=0; !to_configure.empty(); ++run) {

    std::list<const configuration_device_section*>::iterator i=to_configure.begin();
    while( i!=to_configure.end()) {
      configuration_result* config_result=NULL;
      std::map<const std::string, device*>::iterator dev_iterator=configurable_devices.find((*i)->name);
      device* dev=dev_iterator->second;
      if (dev!=NULL) {
	try {
	  config_result=dev->configure(**i, run);
	}
	catch (device_error e) {
	  // error to result...
	  config_result=new configuration_result(0);
	}
      }
      else {
	// error to result...
	config_result=new configuration_result(0);
      }
      if (config_result!=NULL) {
	// do not configure this device again...
	r->push_back(config_result);
	i=to_configure.erase(i);
      }
      else {
	// once more
	++i;
      }
    }

  }

  return r;
}
