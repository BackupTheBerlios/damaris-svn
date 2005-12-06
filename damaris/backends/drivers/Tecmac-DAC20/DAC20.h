/*
Markus Rosenstihl 2005 Nov
*/
#ifndef DAC20_H
#define DAC20_H

#include "core/states.h"
class pfg: public pfgpulse {
 public:
 	int id;
 	// defualt constructor
 	pfg(int myid=0);
 	//
 	unsigned int dac_ttl_values(double dac_value, double length) const;
 	// set the DAC to the required values
 	virtual void set_dac(state& experiment);
 	// destructor
 	virtual ~pfg();
};


#endif