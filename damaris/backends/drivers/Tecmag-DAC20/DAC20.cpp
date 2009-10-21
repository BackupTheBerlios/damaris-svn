/**
 * \file
 * \brief Implementation of the Driver for the DAC20
 *   Digital to Analog Converter Board
 *
 * This board was first used in the PFG, and gets now
 * adopted by the Field-Cycling spectrometers.
 *
 * The board inverts all digital inputs.
 * This driver partially honors this.
 * Especially the data input is not yet inverted.
 * -# Inverting the data line is equivalent to inverting
 *    the value and add/substracting one.
 * -# There are different versions of the board around,
 *    where one board "undoes" the inversion by having an
 *    inverting op-amp after the DAC.
 * -# The digital units have to be calibrated to physical
 *    units anyway. A sign doesn't hurt much.
 *
 * This is still a current discussion topic and might
 * change.
 */

#include "DAC20.h"
#include <cstdio>
#include <cmath>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

using std::reverse;
using std::cout;
using std::vector;
#ifndef TIMING
#define TIMING 9e-8
#endif

// The bit depth of the DAC
#define DAC_BIT_DEPTH 20

// The channel configuration
#define DATA_BIT 18//18	
#define CLK_BIT 16//16

DAC20::DAC20(int myid): id(myid) {
	dac_value = 0;
	set_latch_bit(17);
}

DAC20::~DAC20() {}

// This sets the dac_value
void DAC20::set_dac(signed dw) {
	dac_value = dw;	
}

void DAC20::set_latch_bit(int le_bit)
{
	latch_bit = le_bit;
}

// This sets the DAC
void DAC20::set_dac(state& experiment) {
	
	state_sequent* exp_sequence=dynamic_cast<state_sequent*>(&experiment);
	if (exp_sequence == NULL)
		// is a very state on top level, todo: change interface
		throw pfg_exception( "cannot work on a single state, sorry (todo: change interface)");
	else {
		for(state_sequent::iterator child_state = exp_sequence->begin(); child_state != exp_sequence->end(); ++child_state)
			set_dac_recursive(*exp_sequence, child_state);
//		std::cout << "first state"<< std::endl;
		// Initialize the DAC to "0"
		state s(TIMING);
		ttlout* le=new ttlout();
		le->id=0;
		s.push_front(le);
		state::iterator my_state_iterator = exp_sequence->begin();
                state_sequent* rep_sequence=new state_sequent();
                rep_sequence->repeat=DAC_BIT_DEPTH;
		le->ttls=( 1 << latch_bit) + ( 1 << CLK_BIT );
		rep_sequence->push_back(s.copy_new());
		le->ttls=( 1 << latch_bit);
	        rep_sequence->push_back(s.copy_new());
                exp_sequence->insert(my_state_iterator, rep_sequence);
		//read in the word (41st pulse)
		le->ttls=0;
		exp_sequence->insert(my_state_iterator, s.copy_new());
		// 42nd pulse
		// the state should be 2ms long
		s.length = 2e-3-41*TIMING;
		le->ttls= ( 1 << latch_bit );
		exp_sequence->insert(my_state_iterator, s.copy_new());
	}
}

// This loops recursive through the state tree

void DAC20::set_dac_recursive(state_sequent& the_sequence, state::iterator& the_state) {
	
	state_sequent* a_sequence = dynamic_cast<state_sequent*>(*the_state);
	// Am I a sequence? Yes? Go one sequence further
	if (a_sequence != NULL) { 
		for(state_sequent::iterator child_state = a_sequence->begin(); child_state != a_sequence->end(); ++child_state)
			set_dac_recursive(*a_sequence, child_state);
	}
	// I am not a sequence, but a state
	else {
		state* this_state = dynamic_cast<state*>(*the_state);
		if (this_state == NULL) 
			throw pfg_exception( "state_atom in state_sequent not expected");
		analogout* PFG_aout = NULL;
		// find an analogout section with suitable id
		state::iterator pos = this_state->begin();
		while(pos!=this_state->end()) {  // state members loop
			analogout* aout = dynamic_cast<analogout*>(*pos); // initialize new analogout
			// This is for me, analogout is != NULL (there is an analogout) and has my ID
			if (aout!=NULL && aout->id == id) {
				if (PFG_aout == NULL) {
					// save the informations 
					PFG_aout = aout;
				}
				// there is no place for me here
				else {
					throw pfg_exception( "found another DAC section, ignoring");
					delete aout;
				}
				// remove the analog out section
				this_state->erase(pos++);
			}
			else {
			    ++pos;
			}
		} // state members loop
		
		if (PFG_aout != NULL) { // state modifications
                        //std::cout<<"found a analog out section, value="<<PFG_aout->dac_value<<std::endl;
			// check the length of the state
			if (this_state->length < TIMING*41.0)
				throw pfg_exception( "time is too short to save DAC information");
			else {
				// copy of original state
				state* register_state = new state(*this_state);
				ttlout* register_ttls = new ttlout();
				register_ttls->id = 0;
				register_state->length = TIMING;
				register_state->push_back(register_ttls);
				if (PFG_aout->dac_value > (pow(2.0, int(DAC_BIT_DEPTH-1))-1) )
					throw pfg_exception("dac_value too high");
				if ( abs(PFG_aout->dac_value) > pow(2.0, int(DAC_BIT_DEPTH-1)) )
					throw pfg_exception("dac_value too low");
				// now, insert the ttl information
				vector<int> dac_word;
				for (int j = 0; j < DAC_BIT_DEPTH ; j++)	{
					int bit = PFG_aout->dac_value & 1;
					// // invert the bit
					// bit = (bit == 0);
					dac_word.push_back(bit);
					//cout << dac_word[j];
					PFG_aout->dac_value >>= 1;
				}
				// need one clock cycle to read in bit
				// latch enable (LE) should always be high while doing so
				// except for the last bit
				// reverse the bit pattern
				reverse(dac_word.begin(), dac_word.end());

                                // do run length encoding, grouping same bit values in loops
				int last_seen_bit=dac_word[0];
                                int last_seen_bit_count=1;
				for (int i = 1; i < DAC_BIT_DEPTH+1; i++) {
                                        if (i==DAC_BIT_DEPTH || last_seen_bit!=dac_word[i]) {
                                           // so we have to write the bits
                                           // either because the previous were different or we are finished
                                           if (last_seen_bit_count>1) {
                                                // insert a loop
                                                state_sequent* rep_sequence=new state_sequent();
                                                rep_sequence->repeat=last_seen_bit_count;
        	                                register_ttls->ttls = (1 << DATA_BIT)*last_seen_bit + (1 << CLK_BIT) + (1 << latch_bit);
                                		rep_sequence->push_back(register_state->copy_new());
                                                register_ttls->ttls = (1 << DATA_BIT)*last_seen_bit + (1 << latch_bit);
                                		rep_sequence->push_back(register_state->copy_new());
                                                the_sequence.insert(the_state, rep_sequence);
                                           } else {
                                                   // no loop necessary, insert two states
        	                                   register_ttls->ttls = (1 << DATA_BIT)*last_seen_bit + (1 << CLK_BIT) + (1 << latch_bit);
	                                           the_sequence.insert(the_state, register_state->copy_new());
					           register_ttls->ttls = (1 << DATA_BIT)*last_seen_bit + (1 << latch_bit);
					           the_sequence.insert(the_state, register_state->copy_new());
                                            }
                                            // reset counter and bits if we are not finished
                                            if (i<DAC_BIT_DEPTH) {
                                                last_seen_bit=dac_word[i];
                                                last_seen_bit_count=1;
                                            }
        	           		} // finished writing
                                        else
                                            last_seen_bit_count+=1; // same bit value, so continue
                                } // end of bit loop
				register_ttls->ttls = 0;
				the_sequence.insert(the_state,register_state->copy_new());
				
				// shorten the remaining state 
				// and add LE high to this state
				ttlout* ttls=new ttlout();
				// 42nd pulse
				this_state->length -= TIMING*41;
				ttls->ttls = 1 << latch_bit;
				this_state->push_front(ttls);

                                // cleanup
				delete register_state;
				delete PFG_aout;
			} // state was long enough to work on
		}
		else {
			ttlout* le_ttls=new ttlout();
			le_ttls->ttls = 1 << latch_bit;
			this_state->push_back(le_ttls);
		}
		// end of state modifications 
	} // I was a state
}
