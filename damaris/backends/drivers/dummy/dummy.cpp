/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/

#include "dummy.h"
#include <list>
#include "core/states.h"

dummy::dummy() {
    new_results=NULL;
    set_history_stepsize(1);
}

result* dummy::get_samples(double timeout) {
  if (new_results==NULL) return new result(0);
  if (new_results->empty()) {
    delete new_results;
    new_results=NULL;
    return new result(0);
  }
  if (new_results->size()==1) {
    adc_result* res=new_results->front();
    new_results->pop_front();
    delete new_results;
    new_results=NULL;
    return res;
  }
  adc_results* res=new_results;
  new_results=NULL;
  return res;
}

void dummy::set_frequency(state& exp) {
  return;
}

void dummy::sample_after_external_trigger(double rate, size_t no, double sens, size_t res) {
  // generate artificial result
  short int* data=(short int*)malloc(no*sizeof(short int)*2);
  const double amplitude=1<<10;
  const double noise=1<<9;
  const double intermediate_f=1e4;
  const double time_offset=1e-5;
  const double decay_constant=1e-4;
  const double mixing_phase=0;
  for (double i=0; i<no; i++) {
    double t=time_offset+i/rate;
    data[(size_t)i*2]=(short int)floor(amplitude*sin(intermediate_f*t*2*M_PI+mixing_phase)*exp(-t/decay_constant)+
				       noise*((double)rand()/(double)RAND_MAX-0.5));
    data[(size_t)i*2+1]=(short int)floor(amplitude*cos(intermediate_f*t*2*M_PI+mixing_phase)*exp(-t/decay_constant)+
					 noise*((double)rand()/(double)RAND_MAX-0.5));
  }
  new_results->push_back(new adc_result(0, no, data, rate));
}

void dummy::set_daq(state& exp) {
  // deallocate all old results
  if (new_results!=NULL) {
    delete new_results;
  }
  new_results=new adc_results(0);
}

/* now run the pulse program */
void dummy::run_pulse_program(state& exp) {
  /* found a state sequence */
  state_sequent* ss=dynamic_cast<state_sequent*>(&exp);
  if (ss!=NULL) {
    for (size_t i=0; i<ss->repeat;++i)
      for (state::iterator j=ss->begin(); j!=ss->end(); j++) {
	state* substate=dynamic_cast<state*>(*j);
	if (substate==NULL) 
	  throw pulse_exception(std::string(__FUNCTION__)+"unknown substate");
	run_pulse_program(*substate);
      }
    return;
  }

  /* can not handle a parallel state */
  state_parallel* sp=dynamic_cast<state_parallel*>(&exp);
  if (sp!=NULL)
    throw pulse_exception(std::string(__FUNCTION__)+": state_parallel is not implemented");

  /* found a state */
  const state* s=dynamic_cast<const state*>(&exp);
  if (s!=NULL) {
    fprintf(stdout,"state: t=%g",s->length);
    for (state::const_iterator j=s->begin(); j!=s->end(); ++j) {
      const analogin* ai=dynamic_cast<const analogin*>(*j);
      if (ai!=NULL) {
	fprintf(stdout,"\n - analogin id=%d channels=%lu",ai->id,ai->channels.to_ulong());
	sample_after_external_trigger(ai->sample_frequency, ai->samples);
	continue;
      }
      const ttlout* to=dynamic_cast<const ttlout*>(*j);
      if (to!=NULL) {
	fprintf(stdout,"\n - ttlout id=%d ttls=%lu",to->id,to->ttls.to_ulong());
	continue;
      }
      const analogout* ao=dynamic_cast<const analogout*>(*j);
      if (ao!=NULL) {
	fprintf(stdout,"\n - analogout id=%d frequency=%g phase=%g",ao->id,ao->frequency,ao->phase);
	continue;
      }
      
      // other state atoms...
      fprintf(stdout, "\n unknown state");
    }
    fprintf(stdout,"\n");
  }
}


void dummy::run_pulse_program_w_sync(state& exp, double sync_freq) {
	dummy::run_pulse_program(exp);
}
double dummy::get_temperature() const {
  pthread_mutex_lock((pthread_mutex_t*)&device_lock);
  pthread_mutex_unlock((pthread_mutex_t*)&device_lock);
  return 0.0;
}

double dummy::set_setpoint(double temperature){
  pthread_mutex_lock(&device_lock);
  pthread_mutex_unlock(&device_lock);
  return 0.0;
}

double dummy::get_setpoint() const {
  pthread_mutex_lock((pthread_mutex_t*)&device_lock);
  pthread_mutex_unlock((pthread_mutex_t*)&device_lock);
  return 0.0;
}
