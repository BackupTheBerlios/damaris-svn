/* **************************************************************************

 Author: Achim Gaedke
 Created: August 2004

****************************************************************************/

#include "hardware.h"

result* hardware::single_pulse_experiment(double frequency, double t_before, double t, double sample_freq, size_t samples) {
# if 1
  the_fg->set_frequency(frequency);
  the_adc->sample_after_external_trigger(sample_freq,samples,sample_freq,samples);
  the_pg->single_pulse_program(t_before,t,((double)samples)/sample_freq);
  return the_adc->get_samples();
#else
  throw ADC_exception("no longer supported");
  return NULL;
#endif
}

result* hardware::experiment(const state& exp) {
  result* r=NULL;
  for(size_t tries=0; r==NULL; ++tries) {
    state* work_copy=exp.copy_flat();
    if (work_copy==NULL) return new error_result(1,"could create work copy of experiment sequence");
    try {
      if (the_fg!=NULL)
	the_fg->set_frequency(*work_copy);
      if (the_adc!=NULL)
	the_adc->set_daq(*work_copy);
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
  }
  return r;
}
