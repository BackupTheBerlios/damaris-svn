#include <cstdio>
#include "core/xml_states.h"
#include "drivers/Tecmag-DAC20/DAC20.h"


int main(int argc, char** argv) 
	
{
	int return_result=0;
	try {
		
		signed dac_value=2;
		state_atom* a=xml_state_reader().read_from_file("/dev/stdin");
		if (a==NULL) {
			fprintf(stderr, "%s: could not read a state tree from stdin\n", argv[0] );
			return 1;
		}
		state* a_state=dynamic_cast<state*>(a);
		if (a_state==NULL) {
			fprintf(stderr, "%s: did not find a state in input\n", argv[0] );
			delete a;
			return 1;
		}
		DAC20().set_dac(dac_value);
		DAC20().set_dac(*a_state);
		xml_state_writer().write_states(stdout,*a,1);
		delete a;  
		return 0;
	}
	catch(pfg_exception pfg_e){
		fprintf(stderr,"DAC20: %s\n",pfg_e.what());
		return_result=1;           
	}
}
