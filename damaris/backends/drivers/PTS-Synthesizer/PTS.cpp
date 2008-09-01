/* ***************************************************************************

 Author: Achim Gädke
 Date: October 2004

**************************************************************************** */

#include "PTS.h"
#include <cstdio>
#include <cmath>
#include "core/xml_states.h"

#ifdef __CYGWIN__
# define floorl floor
#endif

PTS::PTS(int myid): id(myid) {
  frequency=0;
  phase=0;
  negative_logic=1;
}

PTS::~PTS() {

}

unsigned int PTS::phase_ttl_values(double p) const {
  // break down to [0;360[
  p-=floor(p/360.0)*360.0;
  unsigned int phasesteps=(unsigned int)fabs(round(p/0.225));
  int bcd_part=phasesteps/100;
  unsigned int ttl_value=bcd_part<<8;
  phasesteps-=bcd_part*100;
  bcd_part=phasesteps/10;
  ttl_value|=bcd_part<<4;
  phasesteps-=bcd_part*10;
  ttl_value|=phasesteps;
  return ttl_value;
}

void PTS::phase_add_ttls(state& the_state, double p) const {
  unsigned int binary_code=phase_ttl_values(p);
  std::vector<ttlout>::const_iterator mask=ttl_masks.begin();
  while (mask!=ttl_masks.end()) {
    /* obeye negative logic */
    if ((binary_code & 1<<11)==0 ^ negative_logic==0) the_state.push_back(mask->copy_new());
    binary_code<<=1;
    binary_code&=0xFFF;
    ++mask;
  }
  if (binary_code!=0) fprintf(stderr,"Warning! Insufficient phase precision for %f\n",p);
}

long unsigned int PTS::frequency_ttl_values(double f) const {
  long unsigned int frequency_int=(long unsigned int)fabs(floor(f));
  long unsigned int freq_bcd=0;
  if (sizeof(freq_bcd)<8) {
    fprintf(stderr, "warning! possible overflow (f=%f)\n", f);
  }
  for (long unsigned int divider=100000000; divider>0; divider/=10) {
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
    phase=pts_aout->phase;
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


/* **************************************************************************

PTS_latched

************************************************************************** */

void PTS_latched::set_frequency(state& experiment) {
  // find the frequency informations...
  // and exchange the phase informations
  frequency=0;
  phase=0;
  state_sequent* exp_sequence=dynamic_cast<state_sequent*>(&experiment);
  if (exp_sequence==NULL)
    // is a very state on top level, todo: change interface
    throw frequ_exception("cannot work on a single state, sorry (todo: change interface)");
  else {
    for(state_sequent::iterator child_state=exp_sequence->begin(); child_state!=exp_sequence->end(); ++child_state)
      set_frequency_recursive(*exp_sequence, child_state);
  }
}

void PTS_latched::set_frequency_recursive(state_sequent& the_sequence, state::iterator& the_state) {

  state_sequent* a_sequence=dynamic_cast<state_sequent*>(*the_state);
  if (a_sequence!=NULL) {
    for(state_sequent::iterator child_state=a_sequence->begin(); child_state!=a_sequence->end(); ++child_state)
      set_frequency_recursive(*a_sequence, child_state);
  }
  else {
    // find the frequency informations...
    state* this_state=dynamic_cast<state*>(*the_state);
    if (this_state==NULL) throw frequ_exception("state_atom in state_sequent not expected");
    // and exchange the phase informations
    analogout* pts_aout=NULL;
    /* find a analogout section with suitable id */
    state::iterator i=this_state->begin();
    while(i!=this_state->end()) {
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
	this_state->erase(i++);
      }
      else
	++i;
    } /* state members loop */

    if (pts_aout!=NULL) {
      if (this_state->length<9e-8*4.0) throw frequ_exception("time is too short to save phase information");
      /* now, insert the ttl information*/
      /* set phase everytime */
      pts_aout->phase-=floor(pts_aout->phase/360.0)*360.0;
      int phase_int=(int)floor(pts_aout->phase/0.225+0.5);
      /* copy of original state */
      state* register_state=new state(*this_state);
      ttlout* register_ttls=new ttlout();
      register_ttls->id=0;
      register_state->length=9e-8;
      register_state->push_back(register_ttls);
      /* set frequency if necessary */
      if (pts_aout->frequency!=0) {
	if (this_state->length<9e-8*12.0) throw frequ_exception("time is too short to save frequency information");
	// enable remote frequency control
	register_ttls->ttls=0x8010;
	the_sequence.insert(the_state,register_state->copy_new());
	register_ttls->ttls&=0x7fff;
	the_sequence.insert(the_state,register_state->copy_new());
	// in 0.1 Hz units !
	unsigned long int frequency_int=(unsigned long int)floorl(fabs(pts_aout->frequency)*10.0+0.5);
	if (sizeof(frequency_int)<8 && pts_aout->frequency>4e8) {
	  fprintf(stderr, "warning! possible overflow (f=%f)\n",pts_aout->frequency);
	}
	/* 100MHz and 10MHz */
	register_ttls->ttls=(frequency_int/1000000000)%10<<8|((frequency_int/100000000)%10)<<4|15<<12;
	the_sequence.insert(the_state,register_state->copy_new());
	register_ttls->ttls&=0x7fff;
	the_sequence.insert(the_state,register_state->copy_new());
	/* 1MHz and 100kHz */
	register_ttls->ttls=(frequency_int/10000000)%10<<8|((frequency_int/1000000)%10)<<4|14<<12;
	the_sequence.insert(the_state,register_state->copy_new());
	register_ttls->ttls&=0x7fff;
	the_sequence.insert(the_state,register_state->copy_new());
	/* 10kHz and 1kHz */
	register_ttls->ttls=(frequency_int/100000)%10<<8|((frequency_int/10000)%10)<<4|13<<12;
	the_sequence.insert(the_state,register_state->copy_new());
	register_ttls->ttls&=0x7fff;
	the_sequence.insert(the_state,register_state->copy_new());
	/* 100Hz and 10Hz */
	register_ttls->ttls=(frequency_int/1000)%10<<8|((frequency_int/100)%10)<<4|12<<12;
	the_sequence.insert(the_state,register_state->copy_new());
	register_ttls->ttls&=0x7fff;
	the_sequence.insert(the_state,register_state->copy_new());
	/* 1Hz and 0.1Hz (not used for all models) */
	//register_state->length=0.5e-6;
	register_ttls->ttls=(frequency_int/10)%10<<8|((frequency_int)%10)<<4|11<<12;
	the_sequence.insert(the_state,register_state->copy_new());
	register_ttls->ttls&=0x7fff;
	the_sequence.insert(the_state,register_state->copy_new());
	//register_state->length=9e-8;
	/* and shorten the remaining state */
	this_state->length-=9e-8*12;
	//this_state->length-=9e-8*11+0.5e-6;
      }
      if (1) {
	/* first entry for phase */      
	register_ttls->ttls=((phase_int/100)%16)<<4|9<<12;
	the_sequence.insert(the_state,register_state->copy_new());
	register_ttls->ttls&=0x7fff;
	the_sequence.insert(the_state,register_state->copy_new());
	/* second entry for phase */
	register_ttls->ttls=(phase_int%10)<<8|((phase_int/10)%10)<<4|10<<12;
	the_sequence.insert(the_state,register_state->copy_new());
	register_ttls->ttls&=0x7fff;
	the_sequence.insert(the_state,register_state->copy_new());
	/* and shorten the remaining state */
	this_state->length-=9e-8*4;
      }
      delete register_state;
      delete pts_aout;

    }


  /* end of state modifications */
  }
}
