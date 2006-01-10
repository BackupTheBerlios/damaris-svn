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
 	//
 	unsigned int dac_ttl_values(double dac_value) const;
 	// set the DAC to the required values
 	virtual void set_dac(state& experiment);
	virtual void set_dac_ttls(state& experiment);
 	// destructor
 	virtual ~PFG();

};
#endif
