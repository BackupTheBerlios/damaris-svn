#include "Spectrum-M2i40xxSeries.h"
#include "../../machines/hardware.h"
#include "include/regs.h"
#include "core/states.h"

#include <typeinfo>

int main() {
	int i = 3;
	fprintf(stderr, "a"+i);

	try {
		SpectrumM2i40xxSeries* my_adc;
		ttlout trigger;
		trigger.id=0;
		trigger.ttls=0x400000; /* line 22 *///
		my_adc=new SpectrumM2i40xxSeries(trigger, 50.0, 1e7);

		state_sequent* test_sequence = new state_sequent();
		state* test_state = new state(1);
		analogin* test_in = new analogin();
		test_in->id = 0;
		//test_in->sensitivity = 5;
		test_in->sample_frequency = 2e7;
		test_in->resolution = 14;
		test_in->samples = 4096;
		test_in->channels = channel_array(15);
		test_in->nchannels = test_in->channels.count();

		test_in->sensitivity = new double[4];
		test_in->sensitivity[0] = 10.0;
		test_in->sensitivity[1] = 10.0;
		test_in->sensitivity[2] = 10.0;
		test_in->sensitivity[3] = 10.0;

		test_in->impedance = new double[4];
		test_in->impedance[0] = 50.0;
		test_in->impedance[1] = 50.0;
		test_in->impedance[2] = 50.0;
		test_in->impedance[3] = 50.0;

		test_in->offset = new int[4];
		test_in->offset[0] = -50;
		test_in->offset[1] = 0;
		test_in->offset[2] = 0;
		test_in->offset[3] = 0;

		test_state->push_back(test_in);

		test_sequence->push_back(test_state);

		my_adc->set_daq(*test_sequence);

		adc_results* resultat = dynamic_cast<adc_results*>(my_adc->get_samples(1));
		FILE* outputFile = fopen("./out.txt","w");
		int j;
		for(j = 0; j < 4*resultat->front()->samples; j += 4) {
			fprintf(outputFile, "%i\t%i\t%i\t%i\n", resultat->front()->data[j], resultat->front()->data[j+1], resultat->front()->data[j+2], resultat->front()->data[j+3]);
		}
		fclose(outputFile);


		delete my_adc;
	}  catch(ADC_exception ae) {
		fprintf(stderr,"adc: %s\n",ae.c_str());
	}
}
