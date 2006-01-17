#include "DAC20.h"
#include <cstdio>
#include <cmath>
#include "core/xml_states.h"

#define DAC_BIT_DEPTH 20

// The channel configuration
#define DATA_BIT 16	
#define CLK_BIT 17
#define LE_BIT 18

PFG::PFG(int myid): id(myid) {}

PFG::~PFG() {}

int PFG::dac_ttl_values(int dac_value) const {
	int bit_mask = int(pow(double(2),(DAC_BIT_DEPTH-1)));
	int data = int(pow(double(2), DATA_BIT));
	int clk = int(pow(double(2), CLK_BIT));
	int le = int(pow(double(2), LE_BIT));
	int bit;
	int dac_word[DAC_BIT_DEPTH];

	for (int j=0; j < DAC_BIT_DEPTH ; j++)	{
			if (dac_value & bit_mask)
				int bit = 1;
			else
				int bit = 0;
			bit_mask >>= 1;
			dac_word[j]=bit;
			//ttl_out=(data*bit + le); // need one clock cycle to read in bit
			//ttl_out=(data*bit + clk + le); // le should always be high
			//if (j == 19) // last bit => LE low, tell DAC to read the word in 
			//	ttl_out = data*bit;
	}
	//return dac_word[i];
}

/*
void PFG::set_dac(state& experiment) 
  {
  state_sequent* exp_sequence=dynamic_cast<state_sequent*>(&experiment);
  if (exp_sequence==NULL)
    set_dac_ttls(experiment);
  else {
    state_iterator si(*exp_sequence);
    state* a_state=(state*)si.get_state();
    while (NULL!=a_state) {
      set_dac_ttls(*a_state);
      a_state=(state*)si.next_state();
    } // state loop
  }
}

void PFG::set_dac_ttls(state& the_state) 
{

    state::iterator i=the_state.begin();
    while(i!=the_state.end()) {
      ttlout* ttl_o=dynamic_cast<ttlout*>(*i);
      if (ttl_o!=NULL && ttl_o->id==id) {
	if (ttl_o==NULL) {
	  // save the informations 
	  //pts_ttl_o=ttl_o;
	  ;
	}
	else {
	  fprintf(stderr, "found another pts decade section, ignoring\n");
	  delete ttl_o;
	}
	// remove the analog out section 
	the_state.erase(i++);
      }
      else {
	++i;
      } // state members loop 
      
      
    }
}*/

/* **************************************************************************

PFG

************************************************************************** */

void PFG::set_dac(state& experiment, int dac_value) {
	// find the frequency informations...
	// and exchange the phase informations
	//frequency=0;
	//phase=0;
	state_sequent* exp_sequence=dynamic_cast<state_sequent*>(&experiment);
	if (exp_sequence==NULL)
		// is a very state on top level, todo: change interface
		fprintf(stdout, "cannot work on a single state, sorry (todo: change interface)");
	else {
		for(state_sequent::iterator child_state=exp_sequence->begin(); child_state!=exp_sequence->end(); ++child_state)
			set_dac_recursive(*exp_sequence, child_state, dac_value);
	}
}

void PFG::set_dac_recursive(state_sequent& the_sequence, state::iterator& the_state, int dac_value) {
	
	state_sequent* a_sequence=dynamic_cast<state_sequent*>(*the_state);
	// Am I a sequence? Yes? Go one sequence further
	if (a_sequence!=NULL) { 
		for(state_sequent::iterator child_state=a_sequence->begin(); child_state!=a_sequence->end(); ++child_state)
			set_dac_recursive(*a_sequence, child_state, dac_value);
	}
	// I am not a sequence, but a state
	else {
		// find the frequency informations...
		state* this_state=dynamic_cast<state*>(*the_state);
		if (this_state==NULL) fprintf(stdout, "state_atom in state_sequent not expected");
		// and exchange the phase informations
		analogout* PFG_aout=NULL;
		/* find a analogout section with suitable id */
		state::iterator i=this_state->begin();
		while(i!=this_state->end()) {
			analogout* aout=dynamic_cast<analogout*>(*i);
			// This is for me, state is != NULL and has my ID
			if (aout!=NULL && aout->id==id) {
				if (PFG_aout==NULL) {
					/* save the informations */
					PFG_aout=aout;
				}
				// there is no place for me here
				else {
					fprintf(stderr, "found another DAC section, ignoring\n");
					delete aout;
				}
				/* remove the analog out section */
				this_state->erase(i++);
			}
			else
				++i;
		} /* state members loop */

    if (PFG_aout!=NULL) {
		if (this_state->length<9e-8*42.0) fprintf(stdout, "time is too short to save DAC information");
		/* now, insert the ttl information*/
		
		PFG_aout->phase-=floor(PFG_aout->phase/360.0)*360.0;
		int phase_int=(int)floor(PFG_aout->phase/0.225+0.5);
		/* copy of original state */
		state* register_state=new state(*this_state);
		ttlout* register_ttls=new ttlout();
		register_ttls->id=0;
		register_state->length=9e-8;
		register_state->push_back(register_ttls);
		int bit_mask = int(pow(double(2),(DAC_BIT_DEPTH-1)));
		int data = int(pow(double(2), DATA_BIT));
		int clk = int(pow(double(2), CLK_BIT));
		int le = int(pow(double(2), LE_BIT));
		int bit;
		//int dac_word[DAC_BIT_DEPTH];
		
		for (int j=0; j < DAC_BIT_DEPTH ; j++)	{
			if (dac_value & bit_mask)
				int bit = 1;
			else
				int bit = 0;
			bit_mask >>= 1;
			//dac_word[j]=bit;
			register_ttls->ttls = (data*bit + le); // need one clock cycle to read in bit
			the_sequence.insert(the_state,register_state->copy_new());
			register_ttls->ttls = (data*bit + clk + le); // le should always be high
			the_sequence.insert(the_state,register_state->copy_new());
			if (j == 19) {// last bit => LE low, tell DAC to read the word in 
				register_ttls->ttls = data*bit;
				the_sequence.insert(the_state,register_state->copy_new());
			}
		}
		
		/* set frequency if necessary */
		if (PFG_aout->frequency!=0) {
			if (this_state->length<9e-8*12.0) fprintf(stdout, "time is too short to save frequency information");
			// enable remote frequency control
			register_ttls->ttls=0x8010;
			the_sequence.insert(the_state,register_state->copy_new());
			register_ttls->ttls&=0x7fff;
			the_sequence.insert(the_state,register_state->copy_new());
			// in 0.1 Hz units !
			size_t frequency_int=(int)floor(PFG_aout->frequency*10.0+0.5);
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
			this_state->length-=9e-8*42;
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
		delete PFG_aout;
		
    }


  /* end of state modifications */
	}
}

