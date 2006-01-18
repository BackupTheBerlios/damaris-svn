#include "DAC20.h"
#include <cstdio>
#include <cmath>
#include "core/xml_states.h"
#include <iostream>
#define DAC_BIT_DEPTH 20

// The channel configuration
#define DATA_BIT 16	
#define CLK_BIT 17
#define LE_BIT 18

PFG::PFG(int myid): id(myid) {}

PFG::~PFG() {}


// creates an array with the word for the DAC
void PFG::dac_ttl_values(signed dac_value, int *my_ptr) {
	int bit_mask = int(pow(double(2),(DAC_BIT_DEPTH-1)));
	int data = int(pow(double(2), DATA_BIT));
	int clk = int(pow(double(2), CLK_BIT));
	int le = int(pow(double(2), LE_BIT));
	int bit;
	
	for (int j=0; j < DAC_BIT_DEPTH ; j++)	{
		if (dac_value & bit_mask)
			int bit = 1;
		else
			int bit = 1;
		bit_mask >>= 1;
		my_ptr[j]=bit;
	}
}


// This sets the dac

void PFG::set_dac(state& experiment, signed dac_value) {

	state_sequent* exp_sequence=dynamic_cast<state_sequent*>(&experiment);
	if (exp_sequence==NULL)
		// is a very state on top level, todo: change interface
		fprintf(stdout, "cannot work on a single state, sorry (todo: change interface)");
	else {
		for(state_sequent::iterator child_state=exp_sequence->begin(); child_state!=exp_sequence->end(); ++child_state)
			set_dac_recursive(*exp_sequence, child_state, dac_value);
	}
}

// This loops recursive through the state tree

void PFG::set_dac_recursive(state_sequent& the_sequence, state::iterator& the_state, signed dac_value) {
	
	state_sequent* a_sequence=dynamic_cast<state_sequent*>(*the_state);
	// Am I a sequence? Yes? Go one sequence further
	if (a_sequence!=NULL) { 
		for(state_sequent::iterator child_state=a_sequence->begin(); child_state!=a_sequence->end(); ++child_state)
			set_dac_recursive(*a_sequence, child_state, dac_value);
	}
	// I am not a sequence, but a state
	else {
		state* this_state=dynamic_cast<state*>(*the_state);
		if (this_state==NULL) fprintf(stdout, "state_atom in state_sequent not expected");
		analogout* PFG_aout=NULL;
		/* find a analogout section with suitable id */
		state::iterator i=this_state->begin();
		while(i!=this_state->end()) {
			analogout* aout=dynamic_cast<analogout*>(*i);
			// This is for me, state is != NULL (there is a state) and has my ID
			if (aout!=NULL && aout->id==id) {
				if (PFG_aout==NULL) {
					// save the informations 
					PFG_aout=aout;
				}
				// there is no place for me here
				else {
					fprintf(stderr, "found another DAC section, ignoring\n");
					delete aout;
				}
				// remove the analog out section
				this_state->erase(i++);
			}
			else
				++i;
		} // state members loop
		
		if (PFG_aout!=NULL) {
			if (this_state->length<9e-8*41.0) fprintf(stderr, "time is too short to save DAC information");
			// now, insert the ttl information
			// copy of original state
			state* register_state=new state(*this_state);
			ttlout* register_ttls=new ttlout();
			register_ttls->id=0;
			register_state->length=9e-8;
			register_state->push_back(register_ttls);
			
			// todo: remove int bit_mask and so on 
			int bit_mask = int(pow(double(2),(DAC_BIT_DEPTH-1)));
			int data = int(pow(double(2), DATA_BIT));
			int clk = int(pow(double(2), CLK_BIT));
			int le = int(pow(double(2), LE_BIT));		
			int dac_word[DAC_BIT_DEPTH];
			
			dac_ttl_values(dac_value, dac_word);
			
			for (int j=0; j < DAC_BIT_DEPTH ; j++)	{
				int bit=dac_word[j];
				register_ttls->ttls = (data*bit + le); // need one clock cycle to read in bit
				the_sequence.insert(the_state,register_state->copy_new());
				register_ttls->ttls = (data*bit + clk + le); // le should always be high
				the_sequence.insert(the_state,register_state->copy_new());
				if (j == (DAC_BIT_DEPTH-1)) {// last bit => LE low, tell DAC to read the word in 
					register_ttls->ttls = data*bit;
					the_sequence.insert(the_state,register_state->copy_new());
				}
			}
			
			// and shorten the remaining state 
			this_state->length-=9e-8*41;
			delete register_state;
			delete PFG_aout;		
		}
		// end of state modifications 
	}
}

