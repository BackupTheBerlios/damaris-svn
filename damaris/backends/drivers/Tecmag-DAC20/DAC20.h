/*
 Markus Rosenstihl 2005 Nov
 */
#ifndef DAC20_H
#define DAC20_H

#include "core/states.h"
#include "drivers/pfggen.h"

/**
 * \ingroup drivers
 */
class DAC20: public GenericDAC {
protected:
	/// The channel for the latch signal
	int latch_bit;

public:
 	int id;
 	// default constructor
 	DAC20(int myid=0);
	virtual void set_dac(signed dw);
//	virtual void set_dac_ttls(signed value);

	void set_latch_bit(int le_bit);

        /**
            inserts necessary serial data transmission to set the dac, at the end of experiment reset the dac

            assumes, the root sequence is not repeated, because the reset sequence is appended to the root sequence!
        */
 	virtual void set_dac(state& experiment);
 	
	// destructor
 	virtual ~DAC20();
	
    private:
	void set_dac_recursive(state_sequent& the_sequence, state::iterator& the_state);
	void set_dac_to_zero(state_sequent* exp_sequence, state::iterator where);
};

/*class pfg_exception: public std::string {
public:
	pfg_exception(const std::string& s): std::string(s){}
};
*/
#endif
