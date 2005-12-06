#include "pfg_dac.h"
#include <cstdio>
#include <cmath>
#include "core/xml_states.h"

#define DAC_BIT_DEPTH 20

// The channel configuration
#define DATA_BIT 16	
#define CLK_BIT 17
#define LE_BIT 18

pfg::pfg(int myid): id(myid) {dac_value = 0, length = 0}

pfg::~pfg() {}

unsigned int pfg::dac_ttl_values(double dac_value, double length) const {
	int bit_mask = int(pow(double(2),(DAC_BIT_DEPTH-1)));
	int data = int(pow(double(2), DATA_BIT));
	int clk = int(pow(double(2), CLK_BIT));
	int le = int(pow(double(2), LE_BIT));
	int bit;

	for (int j=0; j<DAC_BIT_DEPTH ; j++)	{
			if (dac_value & bit_mask)
				bit = 1;
			else
				bit = 0;
			bit_mask >>= 1;
			ttl_out=(data*bit + le); // need one clock cycle to read in bit
			ttl_out=(data*bit + clk + le); // le should always be high
//			cout << (data*bit + le) << "  *  "<< (data*bit+clk+le)<< endl;
			if (j == 19) // LE low, tell DAC to read the word in 
				ttl_out = data*bit;
	}
}

void pfg::set_dac()