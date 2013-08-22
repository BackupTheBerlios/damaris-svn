/*  Spectrum-M2i40xxSeries.cpp
    Author: Stefan Reutter (2011)
    *****************************************************************
    Driver for the Spectrum M2i Series ADC cards. Based on the driver for the old MI40xx cards.
 */


#include <cmath>
#include <stack>
#include <cerrno>
#include <cstring>
#include "pthread.h"
#include "core/core.h"
#include "core/stopwatch.h"
#include "core/result.h"
#include "core/xml_states.h"
#include "Spectrum-M2i40xxSeries.h"

#ifndef SIZETPRINTFLETTER
#  ifndef __LP64__
#    define SIZETPRINTFLETTER "u"
#  else
#    define SIZETPRINTFLETTER "lu"
#  endif
#endif

bool SpectrumM2i40xxSeries::Configuration::bOpenCard() {
	// open ADC handle and read information to be displayed
	char szDrvName[20];
	sprintf (szDrvName, "/dev/spcm%d", lCardID);
	hDrv = spcm_hOpen(szDrvName);

	if (!hDrv) {
		return false;
	}

	// read information to be displayed
	spcm_dwGetParam_i32(hDrv, SPC_PCITYP, 					&lCardType);
	spcm_dwGetParam_i32(hDrv, SPC_PCISERIALNO,				&lSerialNumber);
	spcm_dwGetParam_i32(hDrv, SPC_PCIFEATURES,            	&lFeatureMap);
	spcm_dwGetParam_i64(hDrv, SPC_PCIMEMSIZE,             	&llInstMemBytes);
	spcm_dwGetParam_i32(hDrv, SPC_MIINST_MINADCLOCK,      	&lMinSampleRate);
	spcm_dwGetParam_i32(hDrv, SPC_MIINST_MAXADCLOCK,      	&lMaxSampleRate);
	spcm_dwGetParam_i32(hDrv, SPC_MIINST_MODULES,         	&lModulesCount);
	spcm_dwGetParam_i32(hDrv, SPC_MIINST_CHPERMODULE,     	&lMaxChannels);
	spcm_dwGetParam_i32(hDrv, SPC_MIINST_BYTESPERSAMPLE,  	&lBytesPerSample);
	spcm_dwGetParam_i32(hDrv, SPC_GETDRVVERSION,          	&lLibVersion);
	spcm_dwGetParam_i32(hDrv, SPC_GETKERNELVERSION,       	&lKernelVersion);

	int32 lTmp;

	spcm_dwGetParam_i32 (hDrv, SPC_PCIVERSION,             &lTmp);
	lBaseHwVersion = (lTmp >> 16) & 0xffff;
	lCtrlFwVersion = lTmp  & 0xffff;

	spcm_dwGetParam_i32 (hDrv, SPC_PCIMODULEVERSION,       &lTmp);
	lModHwVersion = (lTmp >> 16) & 0xffff;
	lModFwVersion = lTmp  & 0xffff;

	// we need to recalculate the channels value as the driver returns channels per module
	lMaxChannels *= lModulesCount;

	int32 lAIFeatures;

	spcm_dwGetParam_i32(hDrv, SPC_MIINST_BITSPERSAMPLE,  &lResolution);
	spcm_dwGetParam_i32(hDrv, SPC_READAIPATHCOUNT,  &lPathCount);
	spcm_dwGetParam_i32(hDrv, SPC_READIRCOUNT, &lRangeCount);
	for (int i=0; (i<lRangeCount) && (i<SPCM_MAX_AIRANGE); i++) {
		spcm_dwGetParam_i32(hDrv, SPC_READRANGEMIN0 + i, &lRangeMin[i]);
		spcm_dwGetParam_i32(hDrv, SPC_READRANGEMAX0 + i, &lRangeMax[i]);
	}
	spcm_dwGetParam_i32(hDrv, SPC_READAIFEATURES, &lAIFeatures);

	bInputTermAvailable = (lAIFeatures & SPCM_AI_TERM) != 0;
	bDiffModeAvailable =  (lAIFeatures & SPCM_AI_DIFF) != 0;
	bACCouplingAvailable =(lAIFeatures & SPCM_AI_ACCOUPLING) != 0;
	bBWLimitAvailable =   (lAIFeatures & SPCM_AI_LOWPASS) != 0;
	bOffsPercentMode =    (lAIFeatures & SPCM_AI_OFFSPERCENT) != 0;

	spcm_dwGetParam_i32(hDrv, SPC_MIINST_MAXADCVALUE, &lMaxADCValue);

	// set up gated sampling mode
	spcm_dwSetParam_i32(hDrv, SPC_CARDMODE, SPC_REC_STD_GATE);
	//spcm_dwSetParam_i32(hDrv, SPC_CARDMODE, SPC_REC_STD_SINGLE);
	
	// set up external TTL trigger	
	spcm_dwSetParam_i32(hDrv, SPC_TRIG_ORMASK, SPC_TMASK_EXT0);
	spcm_dwSetParam_i32(hDrv, SPC_TRIG_EXT0_MODE, SPC_TM_POS);	
	//spcm_dwSetParam_i32(hDrv, SPC_PRETRIGGER, 16384);
	//spcm_dwSetParam_i32(hDrv, SPC_TRIG_CH_ORMASK0, SPC_TMASK0_CH0);		
	//spcm_dwSetParam_i32(hDrv, SPC_TRIG_CH0_MODE, SPC_TM_POS);
	//spcm_dwSetParam_i32(hDrv, SPC_TRIG_TERM, 1);
	//spcm_dwSetParam_i32(hDrv, SPC_TRIG_DELAY, 2000);	
	
	
    //spcm_dwSetParam_i32(hDrv, SPC_TIMEOUT, 5000);

	return true;
}

char* SpectrumM2i40xxSeries::Configuration::PrintInfo() {
	char *sInfo = new char[10000];
	char *sTmp = new char[100];

	// the card type + serial number
	switch (lCardType & TYP_SERIESMASK){
		case TYP_M2ISERIES:     sprintf(sInfo, "M2i.%04x sn %05d\n",(unsigned int)  (lCardType & TYP_VERSIONMASK), lSerialNumber); break;
		case TYP_M2IEXPSERIES:  sprintf(sInfo, "M2i.%04x-Exp sn %05d\n", (unsigned int)( lCardType & TYP_VERSIONMASK), lSerialNumber); break;
		default:                sprintf(sInfo, "Type: %x not supported so far\n", lCardType); break;
	}
	sprintf(sTmp, "  Installed memory:  %lld MByte\n", llInstMemBytes / 1024 / 1024);
	strcat(sInfo, sTmp);
	sprintf(sTmp, "  Max sampling rate: %.1f MS/s\n", (double) lMaxSampleRate / 1000000);
	strcat(sInfo, sTmp);
	sprintf(sTmp, "  Channels:          %d\n", lMaxChannels);
	strcat(sInfo, sTmp);
	sprintf(sTmp, "  Kernel Version:    %d.%02d build %d\n", lKernelVersion >> 24, (lKernelVersion >> 16) & 0xff, lKernelVersion & 0xffff);
	strcat(sInfo, sTmp);
	sprintf(sTmp, "  Library Version    %d.%02d build %d\n", lLibVersion >> 24, (lLibVersion >> 16) & 0xff, lLibVersion & 0xffff);
	strcat(sInfo, sTmp);

#if SPC_DEBUG
	sprintf(sTmp, "Debug Information:\n");
	strcat(sInfo, sTmp);
	sprintf(sTmp, "  Bytes per sample: %i\n", lBytesPerSample);
	strcat(sInfo, sTmp);
#endif

	return sInfo;
}


/*
    Initialize the card
*/
SpectrumM2i40xxSeries::SpectrumM2i40xxSeries(const ttlout& t_line, int ext_reference_clock) {
	// print neat string to inform the user
	fprintf(stderr, "Initializing ADC card\n");

	// check parameters
	trigger_line=t_line;

	default_settings.qwSetChEnableMap = channel_array(ADC_M2I_DEFAULT_CHANNELS);
	default_settings.lSetChannels = default_settings.qwSetChEnableMap.count();
	default_settings.impedance = new double[default_settings.lSetChannels];
	default_settings.sensitivity = new double[default_settings.lSetChannels];
	default_settings.offset = new int[default_settings.lSetChannels];

	for(int i = 0; i < default_settings.lSetChannels; i++) {
		default_settings.impedance[i] = ADC_M2I_DEFAULT_IMPEDANCE;

		default_settings.sensitivity[i] = ADC_M2I_DEFAULT_SENSITIVITY; // Set sensitivity in Volts to the default (maximum) value
		default_settings.offset[i] = ADC_M2I_DEFAULT_OFFSET; // Set offsets in % of sensitivity to default (0)
	}

	default_settings.ext_reference_clock = ext_reference_clock; // Hz
	effective_settings=NULL;

    fprintf(stderr, "clock %i\n", ext_reference_clock);
	// initializing adc info class with card ID at 0
	default_settings.lCardID = 0;

	if(default_settings.bOpenCard())
		fprintf(stderr, "%s", default_settings.PrintInfo());
	else
		throw SpectrumM2i40xxSeries_error("Could not open card");
	
		
    spcm_dwSetParam_i32(default_settings.hDrv, SPC_CLOCKMODE, SPC_CM_EXTREFCLOCK);
    spcm_dwSetParam_i32(default_settings.hDrv, SPC_REFERENCECLOCK, default_settings.ext_reference_clock);
    spcm_dwSetParam_i32(default_settings.hDrv, SPC_CLOCK50OHM, 1); // ToDo: test this

	fprintf(stderr, "\nADC card initialized\n");
}

void SpectrumM2i40xxSeries::collect_config_recursive(state_sequent& exp, SpectrumM2i40xxSeries::Configuration& settings) {
	/* start with dummy node */
	DataManagementNode* new_branch = new DataManagementNode(NULL);
	DataManagementNode* where_to_append = new_branch;
	double parent_timeout=settings.timeout;
	settings.timeout=0.0;

	/* loop over all states and sequences within the current sequence */
	for (state_sequent::iterator i = exp.begin(); i != exp.end(); ++i) {

		state* a_state = dynamic_cast<state*>(*i); // cast into a state and check for illegal types
		if (a_state == NULL)
			throw SpectrumM2i40xxSeries_error("Expecting state or state_sequent object");
		if (dynamic_cast<state_parallel*>(*i)!=NULL)
			throw SpectrumM2i40xxSeries_error("State parallel is not implemented");

		state_sequent* a_sequence=dynamic_cast<state_sequent*>(a_state); // cast into a sequence of states
		if (a_sequence != NULL) { // if a sequence is found, recurse
			DataManagementNode* tmp_structure = settings.data_structure;
			settings.data_structure = where_to_append;
			collect_config_recursive(*a_sequence, settings);
			settings.data_structure = tmp_structure;
		} else { // otherwise, we are supposed to have a single analogin state
		    settings.timeout += a_state->length;

		    // collect analogin sections in state
		    std::list<analogin*> inputs;

		    // loop over analogin states for this device
		    state::iterator k = a_state->begin();
		    while (k != a_state->end()) {
		    	analogin* input = dynamic_cast<analogin*>(*k);
				if (input != NULL && input->id == device_id) { // check for validity
					if (input->samples<=0 || input->sample_frequency<=0) { // don't use an input if it doesn't make sense
						delete input;
					} else {
						inputs.push_back(input);
					}
					k=a_state->erase(k);
				} else {
					++k;
				}
			}


		    if (!inputs.empty()) {
				/* evaluate the found analogin definitions */
				if (inputs.size() > 1) { // only one analogin allowed for each state
					while (!inputs.empty()) { delete inputs.front(); inputs.pop_front();} // free list
					throw ADC_exception("can not handle more than one analogin section per state");
				}

				/* save sampling frequency */
				if (settings.samplefreq <= 0) {
					settings.samplefreq = inputs.front()->sample_frequency;
				} else if (settings.samplefreq != inputs.front()->sample_frequency) {
					while (!inputs.empty()) { delete inputs.front(); inputs.pop_front();} // free list
					throw ADC_exception("Sorry, but gated sampling requires same sampling frequency in all analogin sections");
				}

				/* save sensitvity */
				if (settings.sensitivity != NULL) { // if sensitivity is set, make sure it's valid (i.e. the same for all inputs)
					for (int j = 0; j < inputs.front()->nchannels; j++) {
						if (settings.sensitivity[j] != inputs.front()->sensitivity[j]) {
							fprintf(stderr, "Warning! different sensitivity specified (here %f, elsewhere %f), choosing higher voltage\n",
								 settings.sensitivity[j],
								 inputs.front()->sensitivity[j]);
							if (settings.sensitivity[j] < inputs.front()->sensitivity[j]) {
								settings.sensitivity[j] = inputs.front()->sensitivity[j];
							}
						}
					}
				} else {
					settings.sensitivity = inputs.front()->sensitivity;
				}
				// check if sensitivity is valid
				for (int j = 0; j < inputs.front()->nchannels; j++) {
				    bool sensAllowed = false;
				    for (int l = 0; l < ADC_M2I_ALLOWED_SENSITIVITY_LENGTH; l++) {
					    if (settings.sensitivity[j] == ADC_M2I_ALLOWED_SENSITIVITY[l] ) {
					        sensAllowed = true;
					        break;
					    }
				    }
				    if (!sensAllowed) {
				        fprintf(stderr, "Warning! Illegal sensitivity specified for channel %i: %f", j, inputs.front()->sensitivity[j]);
				        settings.sensitivity[j] = ADC_M2I_DEFAULT_SENSITIVITY;
				    }
				}
				
				/* save impedance */
				if (settings.impedance != NULL) {
					for (int j = 0; j < inputs.front()->nchannels; j++) {
						if (settings.impedance[j] != inputs.front()->impedance[j]) {
							fprintf(stderr, "Warning! different impedance specified (here %f, elsewhere %f), setting to default\n",
								 settings.impedance[j],
								 inputs.front()->impedance[j]);
							settings.impedance[j] = ADC_M2I_DEFAULT_IMPEDANCE;
						}
						if (settings.impedance[j] != ADC_M2I_DEFAULT_IMPEDANCE && settings.impedance[j] != ADC_M2I_ALLOWED_IMPEDANCE) {
						    fprintf(stderr, "Warning! Illegal impedance specified for channel %i: %f", j, inputs.front()->impedance[j]);
						    settings.offset[j] = 0;
						}
					}
				} else {
					settings.impedance = inputs.front()->impedance;
				}

				/* save offsets */
				if (settings.offset != NULL) {
					for (int j = 0; j < inputs.front()->nchannels; j++) {
						if (settings.offset[j] != inputs.front()->offset[j]) {
							fprintf(stderr, "Warning! different impedance specified (here %i, elsewhere %i), setting to default\n",
								 settings.offset[j],
								 inputs.front()->offset[j]);
							settings.offset[j] = ADC_M2I_DEFAULT_OFFSET;
						}
						if (inputs.front()->offset[j] > 100 || inputs.front()->offset[j] < -100) {
						    fprintf(stderr, "Warning! Illegal offset specified for channel %i: %i", j, inputs.front()->offset[j]);
						    settings.offset[j] = 0;
						}
					}
				} else {
					settings.offset = inputs.front()->offset;
				}

				if (inputs.front()->samples%4 != 0) {
					throw ADC_exception("Number of samples must be a multiple of four");
				}

				/* save channel mask and number of channels */
				if (settings.lSetChannels > 0) {
					if (settings.qwSetChEnableMap.to_ulong() > 0) {
						if (settings.qwSetChEnableMap != inputs.front()->channels) {
							fprintf(stderr, "Warning! different channels enabled in input %lu and in config %lu, setting to default \n",
									settings.qwSetChEnableMap.to_ulong(),
									inputs.front()->channels.to_ulong());
							settings.qwSetChEnableMap = channel_array(ADC_M2I_DEFAULT_CHANNELS);
							settings.lSetChannels = settings.qwSetChEnableMap.count();
						}
					} else {
						settings.qwSetChEnableMap = inputs.front()->channels;
						settings.lSetChannels = inputs.front()->nchannels;
					}
				} else {
					settings.qwSetChEnableMap = inputs.front()->channels;
					settings.lSetChannels = inputs.front()->nchannels;
				}

				// gating time offsets apparently were fixed in the M2i cards. todo: check


				// calculate the time required
				double time_required;
				time_required = (inputs.front()->samples)/settings.samplefreq;
				time_required = ceil(1e8*time_required)/1e8;

				// check time requirements
				if (a_state->length < time_required) {
				    char parameter_info[512];
				    snprintf(parameter_info,sizeof(parameter_info),
					   "(%" SIZETPRINTFLETTER " samples, %g samplerate, %e time required, %e state time)",
					   inputs.front()->samples,
					   settings.samplefreq,
					   time_required,
					   a_state->length);
					   
					// update the state length if it's shorter than the gate. this is usually due to rounding to 10 ns for the pulseblaster
			        if (ceil(1e8*a_state->length)/1e8 < time_required) {
    				    throw ADC_exception(std::string("state is shorter than acquisition time")+parameter_info);
				    } else {
				        a_state->length = time_required;
				    }
				}

				// adapt the pulse program for gated sampling
				if (a_state->length == time_required) { // state has proper length
				  a_state->push_back(trigger_line.copy_new());
				} else { // state is too long...
				  // create new one with proper time and gated sampling pulse
				  state* gated_sampling_pulse = new state(*a_state);
				  gated_sampling_pulse->length = time_required;
				  gated_sampling_pulse->push_back(trigger_line.copy_new());

				  // insert gate pulse state before remaining (original) state
				  exp.insert(i,(state_atom*)gated_sampling_pulse);

				  // shorten this state
				  a_state->length -= time_required;
				}
				
# if SPC_DEBUG
                fprintf(stderr, "state sequence:\n");
                xml_state_writer().write_states(stderr, exp);
# endif

				/* insert a new state */
				DataManagementNode* new_one = new DataManagementNode(new_branch);
				new_one->n = inputs.front()->samples;
				new_one->child = NULL;
				new_one->next = where_to_append->next;
				where_to_append->next = new_one;
				where_to_append = new_one;

				while (!inputs.empty()) {delete inputs.front(); inputs.pop_front();} // free inputs
		    } /* !inputs.empty() */
		} // end state
	} // i

	/* something happened? */
	if (new_branch->next != NULL) {
		/* make dummy node to a loop */
		new_branch->n=exp.repeat;
		new_branch->child=new_branch->next;

		/* if possible, append it */
		if (settings.data_structure!=NULL) {
			new_branch->parent=settings.data_structure->parent;
			new_branch->next=settings.data_structure->next;
			settings.data_structure->next=new_branch;
		} else {
			new_branch->parent=NULL;
			new_branch->next=NULL;
			settings.data_structure=new_branch;
		}
	}
	else
		delete new_branch;

	settings.timeout *= exp.repeat;
	settings.timeout += parent_timeout;
	fprintf(stderr,"setting.timeout %g\n",settings.timeout);
	return;
}

void SpectrumM2i40xxSeries::set_daq(state & exp) {
	fprintf(stderr, "Setting up data acquisition\n");
	// check whether the experiment state is legal
	state_sequent* exp_sequence=dynamic_cast<state_sequent*>(&exp);
	if (exp_sequence==NULL)
		throw SpectrumM2i40xxSeries_error("Spectrum-M2i40xxSeries::set_daq only working on sequences");

	/* find out what to do */
	Configuration* conf=new Configuration();
	collect_config_recursive(*exp_sequence, *conf);
	if (conf->samplefreq <= 0) conf->samplefreq = default_settings.samplefreq;
	if (conf->impedance == NULL) conf->impedance = default_settings.impedance;
	if (conf->sensitivity  == NULL) conf->sensitivity = default_settings.sensitivity;
	if (conf->offset  == NULL) conf->offset = default_settings.offset;
	if (conf->hDrv  == NULL) conf->hDrv = default_settings.hDrv;
	if (conf->lBytesPerSample == 0) conf->lBytesPerSample = default_settings.lBytesPerSample;
	if (conf->lSetChannels == 0) { conf->lSetChannels = default_settings.lSetChannels; conf->qwSetChEnableMap = default_settings.qwSetChEnableMap; }
    
	size_t sampleno = (conf->data_structure == NULL) ? 0 : conf->data_structure->size();

	/* nothing to do! */
	if (sampleno == 0) {
	  delete conf;
	  effective_settings=NULL;
	  fprintf(stderr, "Warning: Nothing to do.\n");
	  return;
	}

	if (sampleno < 16 || sampleno%16 != 0) {
		delete conf;
		throw SpectrumM2i40xxSeries_error("total number of samples must be multiple of 16 and at least 16");
	}

	effective_settings=conf;
	// make sure the board is ready
	int actual_status = 0;
	spcm_dwGetParam_i32(effective_settings->hDrv, SPC_M2STATUS, &actual_status);

	if ((actual_status & M2STAT_CARD_READY) == 0) {
		fprintf(stderr, "Warning: Spectrum board was/is running before starting data aquisition. Status: %i\n", actual_status);
		spcm_dwSetParam_i32(effective_settings->hDrv, SPC_M2CMD, M2CMD_CARD_STOP);     // stop the board
	}
    
	// set sensitivity, channels, etc.
	for (int j = 0; j < effective_settings->lSetChannels; j++) {
		spcm_dwSetParam_i32(effective_settings->hDrv, SPC_AMP0 + 100*j,    (int)floor(effective_settings->sensitivity[j]*1000));          // +/- 10V input range
		spcm_dwSetParam_i32(effective_settings->hDrv, SPC_50OHM0 + 100*j,  ((effective_settings->impedance[j] == 50.0) ? 1 : 0));             // 1 = 50 Ohm input impedance, 0 = 1MOhm input impedance
		spcm_dwSetParam_i32(effective_settings->hDrv, SPC_OFFS0 +  100*j, effective_settings->offset[j]); // set offset in % of sensitivity
#if SPC_DEBUG
		fprintf(stderr, "Input impedance for channel %i is %f\n", j, effective_settings->impedance[j]);
#endif
	}
	spcm_dwSetParam_i32(effective_settings->hDrv, SPC_CHENABLE, effective_settings->qwSetChEnableMap.to_ulong()); // set channels
	spcm_dwSetParam_i32(effective_settings->hDrv, SPC_SAMPLERATE, (int)floor(effective_settings->samplefreq)); // set sample rate
	
	// check if frequency was set correctly
	int setSamplingRate = 0;
	spcm_dwGetParam_i32(effective_settings->hDrv, SPC_SAMPLERATE, &setSamplingRate);
    if (setSamplingRate != (int)floor(effective_settings->samplefreq)) {
        char parameter_info[16];
        snprintf(parameter_info,sizeof(parameter_info), "%d", setSamplingRate);
        throw ADC_exception(std::string("DAC sampling rate not available. Try setting to: ")+parameter_info);
    }
	

	spcm_dwSetParam_i32(effective_settings->hDrv, SPC_MEMSIZE, sampleno + ADC_M2I_PRETRIGGER + ADC_M2I_POSTTRIGGER);      // Memory size    * effective_settings->lSetChannels * effective_settings->lBytesPerSample
     
    spcm_dwSetParam_i32(effective_settings->hDrv, SPC_PRETRIGGER, ADC_M2I_PRETRIGGER);   
    spcm_dwSetParam_i32(effective_settings->hDrv, SPC_POSTTRIGGER, ADC_M2I_POSTTRIGGER);
    
	// ----- start the board -----
	spcm_dwSetParam_i32(effective_settings->hDrv, SPC_M2CMD, M2CMD_CARD_START | M2CMD_CARD_ENABLETRIGGER);     // start the board
    
    
	// check for error messages
	char szErrorText[ERRORTEXTLEN];
	if (spcm_dwGetErrorInfo_i32 (effective_settings->hDrv, NULL, NULL, szErrorText) != ERR_OK) {
		if (effective_settings!=NULL) delete effective_settings;
		effective_settings=NULL;

		fprintf(stderr, "%s", szErrorText);
		throw SpectrumM2i40xxSeries_error(szErrorText);
	}
	fprintf(stderr, "Data acquisition setup successful\n");
}

result* SpectrumM2i40xxSeries::get_samples(double _timeout) {
	if (core::term_signal != 0) return NULL;
	size_t sampleno = (effective_settings == NULL || effective_settings->data_structure == NULL) ? 0 : effective_settings->data_structure->size();
	if (sampleno == 0) return new adc_result(1,0,NULL);

#if SPC_DEBUG
	fprintf(stderr, "samples: %lu\tchannels: %i\tbytes/sample: %i\n", sampleno, effective_settings->lSetChannels, effective_settings->lBytesPerSample);
#endif
	int memSize = (sampleno + ADC_M2I_PRETRIGGER + ADC_M2I_POSTTRIGGER) * effective_settings->lSetChannels * effective_settings->lBytesPerSample;

#if SPC_DEBUG
	fprintf(stderr, "memory size: %i\n", memSize);
#endif
	short int* adc_data=(short int*)malloc(memSize);

	stopwatch adc_timer;
	adc_timer.start();
	int adc_status;
	spcm_dwGetParam_i32(effective_settings->hDrv, SPC_M2STATUS, &adc_status);
	fprintf(stderr, "card status: %x; waiting for trigger\n", adc_status);
	spcm_dwSetParam_i32(effective_settings->hDrv, SPC_TIMEOUT, 1000);
    
    if (spcm_dwSetParam_i32(effective_settings->hDrv, SPC_M2CMD, M2CMD_CARD_WAITTRIGGER) == ERR_TIMEOUT) {
	    fprintf(stderr, "no trigger detected, timing out and forcing one now\n");
	    spcm_dwSetParam_i32(effective_settings->hDrv, SPC_M2CMD, M2CMD_CARD_FORCETRIGGER);
	}
	
	spcm_dwSetParam_i32(effective_settings->hDrv, SPC_TIMEOUT, 0);
	spcm_dwSetParam_i32(effective_settings->hDrv, SPC_M2CMD, M2CMD_CARD_WAITREADY);
	

	/*while (core::term_signal == 0 && (adc_status & M2STAT_CARD_READY) == 0 && adc_timer.elapsed() <= effective_settings->timeout) {
		timespec sleeptime;
		sleeptime.tv_nsec = 10*1000*1000; // 10 ms
		sleeptime.tv_sec = 0;
		nanosleep(&sleeptime, NULL);
		spcm_dwGetParam_i32(effective_settings->hDrv, SPC_M2STATUS, &adc_status);
		fprintf(stderr, "card status: %x\n", adc_status);
	}*/
    fprintf(stderr, "ADC finished. Stopping\n");
	spcm_dwSetParam_i32(effective_settings->hDrv, SPC_M2CMD, M2CMD_CARD_STOP);
	if (core::term_signal != 0) {
		free(adc_data);
		return NULL;
	}
	spcm_dwGetParam_i32(effective_settings->hDrv, SPC_M2STATUS, &adc_status);
	if ((adc_status & M2STAT_CARD_READY) == 0) {
		free(adc_data);
		fprintf(stderr, "adc_status not ready: %i \n", adc_status);
		throw SpectrumM2i40xxSeries_error("timeout occured while collecting data");
	}

	fprintf(stderr, "Starting data transfer.\n");
	spcm_dwDefTransfer_i64 (effective_settings->hDrv, SPCM_BUF_DATA, SPCM_DIR_CARDTOPC, 0, adc_data, 0, memSize);
	spcm_dwSetParam_i32 (effective_settings->hDrv, SPC_M2CMD, M2CMD_DATA_STARTDMA | M2CMD_DATA_WAITDMA);
    
    
	char szErrorText[ERRORTEXTLEN];
	if (spcm_dwGetErrorInfo_i32 (effective_settings->hDrv, NULL, NULL, szErrorText)){
		delete adc_data;
		throw SpectrumM2i40xxSeries_error(szErrorText);
	}

	short int* data_position=adc_data + ADC_M2I_PRETRIGGER*effective_settings->lSetChannels; // drop first points due to pre trigger 
	// produced results
	adc_results* the_results = new adc_results(0);
	data_position = split_adcdata_recursive(data_position, *(effective_settings->data_structure), *the_results);
	if (data_position==0 || (size_t)(data_position - adc_data - ADC_M2I_PRETRIGGER * effective_settings->lSetChannels) != (sampleno * effective_settings->lSetChannels)) {
		fprintf(stderr,"something went wrong while splitting data\n");
	}
	free(adc_data);
	
	delete effective_settings;
	effective_settings=NULL;
	fprintf(stderr, "Finished data transfer.\n");
	return the_results;
}

short int* SpectrumM2i40xxSeries::split_adcdata_recursive(short int* data, const DataManagementNode& structure, adc_results& result_splitted) {

  if (structure.child==NULL) {
	  // simple case: do real work
	  // todo: Channel selection
	  short int* datachunk = (short int*) malloc(effective_settings->lBytesPerSample*structure.n*effective_settings->lSetChannels);
	  if (datachunk==NULL) {
		  throw SpectrumM2i40xxSeries_error("not enough memory to create results");
	  }
	  // todo: error checking
	  memcpy(datachunk, data, effective_settings->lBytesPerSample*structure.n*effective_settings->lSetChannels);
	  data += structure.n*effective_settings->lSetChannels;
	  adc_result* the_result = new adc_result(0, structure.n, datachunk, effective_settings->samplefreq, effective_settings->lSetChannels);
	  result_splitted.push_back(the_result);
	  if (structure.next != NULL)
		  data = split_adcdata_recursive(data, *(structure.next), result_splitted);
  } else {
	  for (size_t i=0; i<structure.n; ++i) {
		  data = split_adcdata_recursive(data, *(structure.child), result_splitted);
	  }
  }
  return data;

}

void SpectrumM2i40xxSeries::sample_after_external_trigger(double rate, size_t samples, double sensitivity, size_t resolution) {
  throw SpectrumM2i40xxSeries_error("SpectrumM2i40xxSeries::sample_after_external_trigger is not implemented");
}

SpectrumM2i40xxSeries::~SpectrumM2i40xxSeries() {
	spcm_vClose(default_settings.hDrv);
}

