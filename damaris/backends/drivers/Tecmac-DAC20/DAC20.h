/*
 Markus Rosenstihl 2005 Nov
 */
#ifndef DAC20_H
#define DAC20_H

#include "core/states.h"

class PFG {
public:
 	int id;
 	// default constructor
 	PFG(int myid=0);
	
 	virtual void set_dac(state& experiment, signed dac_value);
	virtual void set_dac_recursive(state_sequent& the_sequence, state::iterator& the_state, signed dac_value);
 	
	// destructor
 	virtual ~PFG();
	
};

class pfg_exception: public std::string {
public:
	pfg_exception(const std::string& s): std::string(s){}
};    
#endif
