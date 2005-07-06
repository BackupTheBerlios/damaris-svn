/* ***************************************************************************

 Author: Achim Gädke
 Date: October 2004

**************************************************************************** */

#include "PTS.h"
#include <cstdio>
#include <cmath>

PTS::PTS(int myid): id(myid) {
  frequency=0;
  phase=0;
  negative_logic=1;
}

PTS::~PTS() {

}

unsigned int PTS::phase_ttl_values(double phase) const {
  // break down to [0;360[
  phase-=floor(phase/360.0)*360.0;
  unsigned int phasesteps=(unsigned int)fabs(round(phase/0.225));
  int bcd_part=phasesteps/100;
  unsigned int ttl_value=bcd_part<<8;
  phasesteps-=bcd_part*100;
  bcd_part=phasesteps/10;
  ttl_value|=bcd_part<<4;
  phasesteps-=bcd_part*10;
  ttl_value|=phasesteps;
  return ttl_value;
}

void PTS::phase_add_ttls(state& the_state,double phase) const {
  unsigned int binary_code=phase_ttl_values(phase);
  std::vector<ttlout>::const_iterator mask=ttl_masks.begin();
  while (mask!=ttl_masks.end()) {
    /* obeye negative logic */
    if ((binary_code & 1<<11)==0 ^ negative_logic==0) the_state.push_back(mask->copy_new());
    binary_code<<=1;
    binary_code&=0xFFF;
    ++mask;
  }
  if (binary_code!=0) fprintf(stderr,"Warning! Insufficient phase precision for %f\n",phase);
}

long long unsigned int PTS::frequency_ttl_values(double frequency) const {
  long long unsigned int frequency_int=(long long unsigned int)fabs(floor(frequency));
  long long unsigned int freq_bcd=0;
  for (long long unsigned int divider=100000000; divider>0; divider/=10) {
    int part_freq=frequency_int/divider;
    freq_bcd<<=4;
    freq_bcd|=part_freq;
    frequency_int-=part_freq*divider;
  }
  return freq_bcd;
}

void PTS::set_frequency(double f) {
  frequency=f;
  /* todo switch PMD Module to frequency */
}

void PTS::set_frequency(state& experiment) {
  // find the frequency informations...
  // and exchange the phase informations
  frequency=0;
  phase=0;
  state_sequent* exp_sequence=dynamic_cast<state_sequent*>(&experiment);
  if (exp_sequence==NULL)
    set_frequency_ttls(experiment);
  else {
    state_iterator si(*exp_sequence);
    state* a_state=(state*)si.get_state();
    while (NULL!=a_state) {
      set_frequency_ttls(*a_state);
      a_state=(state*)si.next_state();
    } /* state loop */
  }
}

void PTS::set_frequency_ttls(state& the_state) {

  // find the frequency informations...
  // and exchange the phase informations
  analogout* pts_aout=NULL;
  /* find a analogout section with suitable id */
  state::iterator i=the_state.begin();
  while(i!=the_state.end()) {
    analogout* aout=dynamic_cast<analogout*>(*i);
    if (aout!=NULL && aout->id==id) {
      if (pts_aout==NULL) {
	/* save the informations */
	pts_aout=aout;
      }
      else {
	fprintf(stderr, "found another pts decade section, ignoring\n");
	delete aout;
      }
      /* remove the analog out section */
      the_state.erase(i++);
    }
    else
      ++i;
  } /* state members loop */

  /* now, add the ttl information*/
  if (pts_aout!=NULL) {
    phase_add_ttls(the_state, pts_aout->phase);
    if (pts_aout->frequency!=0) {
      if (frequency==0) {
	set_frequency(pts_aout->frequency);
      }
      /* different frequencies are forbidden */
      else if (frequency!=pts_aout->frequency) {
	fprintf(stderr, "ignoring frequency %g at analogout %d\n",pts_aout->frequency,id);
      }
    }
    delete pts_aout;
  }
  else {
    /* because we use transparent mode, we have to set phase everywhere */
    phase_add_ttls(the_state, phase);
  }

}
