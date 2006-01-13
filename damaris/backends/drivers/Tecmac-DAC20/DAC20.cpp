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

void PFG::dac_ttl_values(int dac_value) const {
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
}

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
    } /* state loop */
  }
}

void PFG::set_dac_ttls(state& the_state) 
{

    state::iterator i=the_state.begin();
    while(i!=the_state.end()) {
      ttlout* ttl_o=dynamic_cast<ttlout*>(*i);
      if (ttl_o!=NULL && ttl_o->id==id) {
	if (ttl_o==NULL) {
	  /* save the informations */
	  //pts_ttl_o=ttl_o;
	  ;
	}
	else {
	  fprintf(stderr, "found another pts decade section, ignoring\n");
	  delete ttl_o;
	}
	/* remove the analog out section */
	the_state.erase(i++);
      }
      else {
	++i;
      } /* state members loop */
      
      
    }
}
