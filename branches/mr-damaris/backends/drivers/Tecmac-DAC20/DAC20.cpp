#include "DAC20.h"
#include <cstdio>
#include <cmath>
#include "core/xml_states.h"
#include <iostream>



#define DAC_BIT_DEPTH 20

// The channel configuration
#define DATA_BIT 2//18	
#define CLK_BIT 0//16
#define LE_BIT 1//17

PFG::PFG(int myid): id(myid) {
	dac_value=0;
}

PFG::~PFG() {}


// This sets the dac
void PFG::set_dac(signed dw) {
	dac_value=dw;	
}

void PFG::set_dac(state& experiment) {
	
	state_sequent* exp_sequence=dynamic_cast<state_sequent*>(&experiment);
	if (exp_sequence==NULL)
		// is a very state on top level, todo: change interface
		throw pfg_exception( "cannot work on a single state, sorry (todo: change interface)");
	else {
		for(state_sequent::iterator child_state=exp_sequence->begin(); child_state!=exp_sequence->end(); ++child_state)
			set_dac_recursive(*exp_sequence, child_state);
	}
}

// This loops recursive through the state tree

void PFG::set_dac_recursive(state_sequent& the_sequence, state::iterator& the_state) {
	
	state_sequent* a_sequence=dynamic_cast<state_sequent*>(*the_state);
	// Am I a sequence? Yes? Go one sequence further
	if (a_sequence!=NULL) { 
		for(state_sequent::iterator child_state=a_sequence->begin(); child_state!=a_sequence->end(); ++child_state)
			set_dac_recursive(*a_sequence, child_state);
	}
	// I am not a sequence, but a state
	else {
		state* this_state=dynamic_cast<state*>(*the_state);
		if (this_state==NULL) throw pfg_exception( "state_atom in state_sequent not expected");
		analogout* PFG_aout=NULL;
		// find a analogout section with suitable id
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
					throw pfg_exception( "found another DAC section, ignoring\n");
					delete aout;
				}
				// remove the analog out section
				this_state->erase(i++);
			}
			else {
				if(0) {
				state* my_state=NULL;
				state* register_state=new state(*this_state);
				ttlout* le_ttls=new ttlout();
				le_ttls->id=0;
				le_ttls->ttls=2000;//int(pow(2.0, LE_BIT));					
				my_state=register_state;
				// push le_ttls in the front of the state
				my_state->push_front(le_ttls);
				// delete the informations
				//register_state->erase(i++);
				//the_sequence.clear();
				//the_sequence.insert(the_state,register_state->copy_flat());
				//my_state->length=0.001;
				the_sequence.insert(the_state, my_state->copy_flat());
				//delete register_state;
				the_sequence.remove(this_state);
				
				//this_state->length=0.0;
				//this_state=NULL;
				//delete le_ttls;
				}
				++i;
			}
		} // state members loop
		
		if (PFG_aout!=NULL) {
			// check the length of the state
			if (this_state->length<9e-8*41.0)
				throw pfg_exception( "time is too short to save DAC information");
			else {
				// copy of original state
				state* register_state=new state(*this_state);
				ttlout* register_ttls=new ttlout();
				register_ttls->id=0;
				register_state->length=9e-8;
				register_state->push_back(register_ttls);
				std::cout << "DANGER!! Need warmup pulse on LE" <<std::endl;	
				// now, insert the ttl information
				// we need 2*DAC_BIT_DEPTH + 1 pulses to read the word in
				int dac_word[20];
				// std::cout<< PFG_aout->dac_value <<std::endl;
				for (int j = 0; j < DAC_BIT_DEPTH ; j++)	{
					int bit=PFG_aout->dac_value & 1;
					dac_word[j]=bit;
					PFG_aout->dac_value>>=1;
					}
				// need one clock cycle to read in bit
				// latch enable (LE) should always be high while doing so
				// except for the last bit
				// todo: may be we can save the last bit somehow
				// ugly: reverse the bit pattern
				for (int i=DAC_BIT_DEPTH-1; i >= 0; i--) {	
					register_ttls->ttls = (int(pow(2.0, DATA_BIT))*dac_word[i] + int(pow(2.0, LE_BIT)));
					the_sequence.insert(the_state,register_state->copy_new());
					register_ttls->ttls = (int(pow(2.0, DATA_BIT))*dac_word[i] + int(pow(2.0, CLK_BIT)) + int(pow(2.0, LE_BIT)));
					the_sequence.insert(the_state,register_state->copy_new());
					if (i == 0 /*(DAC_BIT_DEPTH-1)*/) {// last bit => LE low, tell DAC to read the word in 
						register_ttls->ttls = 0; //  int(pow(2.0, DATA_BIT))*bit;
						the_sequence.insert(the_state,register_state->copy_new());
					}
				}
				
				// and shorten the remaining state 
				this_state->length-=9e-8*41;
				delete register_state;
				delete PFG_aout;	
			}	
		}
		else {
			
			ttlout* le_ttls=new ttlout();
			le_ttls->id=0;
			le_ttls->ttls=2000;//int(pow(2.0, LE_BIT));
			this_state->push_front(le_ttls);
			/*if (this_state->length < 0.02) {
				this_state->length = 0.02;
			}*/
				
				
		}
		// end of state modifications 
	}
}
