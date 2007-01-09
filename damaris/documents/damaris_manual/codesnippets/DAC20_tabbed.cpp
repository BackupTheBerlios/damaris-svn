#include "DAC20.h"
#include <cstdio>
#include <cmath>
#include "core/xml_states.h"
#include <iostream>
#include <list>
#include <vector>

using std::reverse;
using std::cout;
using std::vector;
#ifndef TIMING
#define TIMING 9e-8
#endif

// The bit depth of the DAC
#define DAC_BIT_DEPTH 20

// The channel configuration
#define DATA_BIT 18
#define CLK_BIT 16
#define LE_BIT 17

PFG::PFG(int myid): id(myid) {
 dac_value = 0;
}

PFG::~PFG() {}

// This sets the dac_value
void PFG::set_dac(signed dw) {
 dac_value = dw; 
}

// This sets the DAC
void PFG::set_dac(state& experiment) {
 
 state_sequent* exp_sequence=dynamic_cast<state_sequent*>(&experiment);
 if (exp_sequence == NULL)
  // Is a very state on top level, todo: change interface
  throw pfg_exception( "cannot work on a single state, sorry (todo: change interface)");
 else {
  for(state_sequent::iterator child_state = exp_sequence->begin();\
   child_state != exp_sequence->end();\
   ++child_state)
   set_dac_recursive(*exp_sequence, child_state);
  // Initialize the DAC to "0"
  state s(TIMING);
  ttlout* le=new ttlout();
  le->id=0;
  le->ttls=( 1 << LE_BIT );
  s.push_front(le);
  state::iterator my_state_iterator = exp_sequence->begin();
  for ( int i = 0; i < DAC_BIT_DEPTH; i++ ) {
   le->ttls=( 1 << LE_BIT) + ( 1 << CLK_BIT );
   exp_sequence->insert(my_state_iterator, s.copy_new());
   le->ttls=( 1 << LE_BIT );
   exp_sequence->insert(my_state_iterator, s.copy_new());
   if ( i == DAC_BIT_DEPTH-1 ) {
   // Transfer the word (41st pulse)
   le->ttls=0;
   exp_sequence->insert(my_state_iterator, s.copy_new());
   }
  }
  // 42nd pulse
  // The state should be 2ms long
  s.length = 2e-3-41*TIMING;
  le->ttls= ( 1 << LE_BIT );
  exp_sequence->insert(my_state_iterator, s.copy_new());
 }
}

// This loops recursive through the state tree

void PFG::set_dac_recursive(state_sequent& the_sequence, state::iterator& the_state) {
 state_sequent* a_sequence = dynamic_cast<state_sequent*>(*the_state);
 // Am I a sequence? Yes? Go one sequence further
 if (a_sequence != NULL) { 
  for(state_sequent::iterator child_state = a_sequence->begin(); 
  		child_state != a_sequence->end(); 
  		++child_state)
   set_dac_recursive(*a_sequence, child_state);
 }
 // I am not a sequence, but a state
 else {
  state* this_state = dynamic_cast<state*>(*the_state);
  if (this_state == NULL) 
   throw pfg_exception( "state_atom in state_sequent not expected");
  analogout* PFG_aout = NULL;
  // Find an analogout section with suitable id
  state::iterator i = this_state->begin();
  while(i!=this_state->end()) {  // Begin state members loop
   analogout* aout = dynamic_cast<analogout*>(*i); // Initialize new analogout
   analogout* next_aout = dynamic_cast<analogout*>(*i++);
   i--;
   // This is for me, analogout is != NULL (there is an analogout) and has my ID
   if (aout!=NULL && aout->id == id) {
    if (PFG_aout == NULL) {
     // Save the informations 
     PFG_aout = aout;
     }
    // There is no place for me here
    else {
     throw pfg_exception( "found another DAC section, ignoring");
     delete aout;
    	 }
    
    // Remove the analogout section
    this_state->erase(i++);
   }
   // Nothing to see, move on
   else {
    ++i;
   		}
  } // End state members loop
  
  if (PFG_aout != NULL) { // Begin state modifications
   // Check the length of the state
   if (this_state->length < TIMING*41.0)
    throw pfg_exception( "time is too short to save DAC information");
   else {
    // Copy of original state
    state* register_state = new state(*this_state);
    ttlout* register_ttls = new ttlout();
    register_ttls->id = 0;
    register_state->length = TIMING;
    register_state->push_back(register_ttls);
    // Return error if dac_value is out of bounds
    if (PFG_aout->dac_value > (pow(2.0, int(DAC_BIT_DEPTH-1))-1) )
     throw pfg_exception("dac_value too high");
    if ( abs(PFG_aout->dac_value) > pow(2.0, int(DAC_BIT_DEPTH-1)) )
     throw pfg_exception("dac_value too low");
    // Now, create the bit pattern
    vector<int> dac_word;
    for (int j = 0; j < DAC_BIT_DEPTH ; j++) {
     int bit = PFG_aout->dac_value & 1;
     dac_word.push_back(bit);
     PFG_aout->dac_value >>= 1;
     }
    // Reverse the bit pattern 
    reverse(dac_word.begin(), dac_word.end());
    /*
    	Datasheet AD1862: 	
    		- Bit is read with rising edge of the CLK 
       		- Word is read with falling edge of LE
       	The opto-coupler in the DAC20 are inverting the signal!  
    	CLK is here inverted, the rest not. This does not affect functionality
    	because the inverse of a 2s complement is then just negative - 1:
    	Example:
    		5 bits:
    		 15 --> 01111 -->inverting--> 10000 = -16
    		  1 --> 00001 -->inverting--> 11110 = -2
    		  0 --> 00000 -->inverting--> 11111 = -1
    		 -1 --> 11111 -->inverting--> 00000 =  0
    		 -2 --> 11110 -->inverting--> 00001 =  1
    		-16 --> 10000 -->inverting--> 01111 =  15
    	Latch enable is going high after 41st bit.
    */
    
    // Transfer the bit pattern
    for (int i = 0; i < DAC_BIT_DEPTH; i++) {
     // Two states = one clock cycle to read in bit
     // State 1
     register_ttls->ttls=(1<<DATA_BIT)*dac_word[i]+(1<<CLK_BIT)+(1<<LE_BIT);
     the_sequence.insert(the_state,register_state->copy_new());
     // State 2
     register_ttls->ttls = (1<<DATA_BIT)*dac_word[i]+(1<<LE_BIT);
     the_sequence.insert(the_state,register_state->copy_new());
     if (i == (DAC_BIT_DEPTH-1)) {
      // Last bit => LE -> 0, prepare DAC to read the word in 
      register_ttls->ttls = 0;
      the_sequence.insert(the_state,register_state->copy_new());
     }
    }
    
    /* 
    	Shorten the remaining state 
		and add LE high to this state.
		The word is read in.
    */
    ttlout* ttls=new ttlout();
    // 42nd pulse
    this_state->length -= TIMING*41;
    // Here is the word read in
    ttls->ttls = 1 << LE_BIT;
    this_state->push_front(ttls);
    delete register_state;
    delete PFG_aout; 

   } 
  }
  else {
   ttlout* le_ttls=new ttlout();
   le_ttls->ttls = 1 << LE_BIT;
   this_state->push_back(le_ttls);
  }
  // End state modifications 
 } // I was a state
}
