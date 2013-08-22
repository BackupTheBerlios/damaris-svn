#include <cmath>
#include <stack>
#include <cerrno>
#include <cstring>
#include "pthread.h"
#include "core/core.h"
#include "core/stopwatch.h"
#include "core/result.h"
#include "core/xml_states.h"
#include "Spectrum-MI40xxSeries.h"

#ifndef SPC_DEBUG
# define SPC_DEBUG 0
#endif
#ifndef SIZETPRINTFLETTER
#  ifndef __LP64__
#    define SIZETPRINTFLETTER "u"
#  else
#    define SIZETPRINTFLETTER "lu"
#  endif
#endif

void SpectrumMI40xxSeries::Configuration::print(FILE* f) {
    fprintf(f, "Spectrum MI40xx Series Configuration:\n");
    fprintf(f, "  Sample Frequency %g Hz, Acquisition Timeout %g\n",samplefreq, timeout);
    if (data_structure!=NULL)
        data_structure->print(f,2);
    else
        fprintf(f, "  No Data to acquire\n");

    for (int i=0; i < lSetChannels; i++) {
        fprintf(f, "  Channel %i: Impedance %f Ohm, Sensitivity %f V, Coupling %s\n", i, impedance[i], sensitivity[i], (coupling==0)?"DC":"AC");
    }
}


SpectrumMI40xxSeries::SpectrumMI40xxSeries(const ttlout& t_line, float impedance, int ext_reference_clock) {
    // print neat string to inform the user
    fprintf(stderr, "Initializing ADC card\n");

    // to be configured
    device_id=0;
    trigger_line=t_line;

    default_settings.qwSetChEnableMap = channel_array(ADC_MI_DEFAULT_CHANNELS);
    default_settings.lSetChannels = default_settings.qwSetChEnableMap.count();
    default_settings.impedance = new double[default_settings.lSetChannels];
    default_settings.sensitivity = new double[default_settings.lSetChannels];
    default_settings.offset = new int[default_settings.lSetChannels];

    for(int i = 0; i < default_settings.lSetChannels; i++) {
        default_settings.impedance[i] = ADC_MI_DEFAULT_IMPEDANCE;

        default_settings.sensitivity[i] = ADC_MI_DEFAULT_SENSITIVITY; // Set sensitivity in Volts to the default (maximum) value
        default_settings.offset[i] = ADC_MI_DEFAULT_OFFSET; // Set offsets in % of sensitivity to default (0)
    }


    default_settings.ext_reference_clock=ext_reference_clock; // Hz
    effective_settings=NULL;

#     if defined __linux__
    // ----- open driver -----
    deviceno = open ("/dev/spc0", O_RDWR);
    if (deviceno == -1) {
        std::string error_message="could not open /dev/spc0";
        if (errno==0) throw SpectrumMI40xxSeries_error( error_message+" (unknown error)");
        else throw SpectrumMI40xxSeries_error( error_message+" ("+strerror(errno)+")");
    }
#     endif
#     if defined __CYGWIN__
    // open library dll
    spectrum_driver_dll = LoadLibrary("spectrum.dll");
    if (spectrum_driver_dll==NULL)
        throw SpectrumMI40xxSeries_error("could not open driver dll");
    deviceno=0;
    // load driver functions
#     define load_spectrum_function(name) name = ( name##_type)GetProcAddress(spectrum_driver_dll, #name); \
if (name == NULL) { FreeLibrary(spectrum_driver_dll); \
 throw SpectrumMI40xxSeries_error("failed to load function" #name "" ); \
}
    load_spectrum_function(SpcInitPCIBoards);
    load_spectrum_function(SpcGetParam);
    load_spectrum_function(SpcSetParam);
    load_spectrum_function(SpcGetData);

    // find spectrum boards
    boardcount=0;
    SpcInitPCIBoards(&boardcount,&PCIVersion);
    if (boardcount<=0) {
        FreeLibrary(spectrum_driver_dll);
        throw SpectrumMI40xxSeries_error("no spectrum boards found");
    }

#     endif
    /* calculate the sample number that can be saved without fifo */
    int memory_size=0;
    SpcGetParam(deviceno,SPC_PCIMEMSIZE,&memory_size);
    fifo_minimal_size=memory_size/(sizeof(short int)*2);

    /* get the device type */
    SpcGetParam(deviceno, SPC_PCITYP, &cardType);
    if (cardType != TYP_MI4020 && cardType != TYP_MI4021 && cardType != TYP_MI4022 && cardType != TYP_MI4030 && cardType != TYP_MI4031 && cardType != TYP_MI4032) {
        throw SpectrumMI40xxSeries_error("Board type not supported");
    }

    /* make a check, whether spectrum board is running */
    int actual_status=0;
    SpcGetParam(deviceno, SPC_STATUS, &actual_status);
    if (actual_status!=SPC_READY) {
        fprintf(stderr, "Warning: Spectrum board was is running before reset.\n");
    }
#if SPC_DEBUG
    fprintf(stderr,"External reference clock: %i\n",default_settings.ext_reference_clock);
#endif
    int ErrorOccurred=1;
    SpcSetParam(deviceno, SPC_COMMAND,        SPC_RESET)==ERR_OK &&       // reset device first
    SpcSetParam (deviceno, SPC_GATE,          1)==ERR_OK &&             // Gated Triggering
    SpcSetParam (deviceno, SPC_PLL_ENABLE,    1)==ERR_OK &&             // Internal PLL enabled for clock
    SpcSetParam (deviceno, SPC_CLOCKOUT,      0)==ERR_OK &&             // No clock output
    SpcSetParam (deviceno, SPC_REFERENCECLOCK, default_settings.ext_reference_clock)==ERR_OK &&     // external reference clock
    SpcSetParam (deviceno, SPC_EXTERNALCLOCK, 0)==ERR_OK &&             //  but no external sample clock
    SpcSetParam (deviceno, SPC_CLOCK50OHM,    0)==ERR_OK &&             // clock input NOT with 50Ohm impedance
    SpcSetParam (deviceno, SPC_TRIGGERMODE,   TM_TTLPOS)==ERR_OK &&     // ttl trigger is used
    SpcSetParam (deviceno, SPC_TRIGGEROUT,    0)==ERR_OK &&             // No trigger output
    SpcSetParam (deviceno, SPC_TRIGGER50OHM,  0)==ERR_OK &&             // Trigger to 1MOhm, necessary for steep slopes
    (ErrorOccurred=0);                                                  // done, really no error occurred

    // ----- driver error: request error and end program -----
    if (ErrorOccurred!=0) {
        int32   lErrorCode, lErrorReg, lErrorValue;
        SpcGetParam (deviceno, SPC_LASTERRORCODE,   &lErrorCode);
        SpcGetParam (deviceno, SPC_LASTERRORREG,    &lErrorReg);
        SpcGetParam (deviceno, SPC_LASTERRORVALUE,  &lErrorValue);
        char error_message[256];
        snprintf(error_message, sizeof(error_message),"Configuration error %d in register %d at value %d", lErrorCode, lErrorReg, lErrorValue);
        throw SpectrumMI40xxSeries_error(error_message);
    }

    /* print lines with useful information: */
    fprintf(stderr, "Spectrum MI40xx series board with %d byte memory\n", memory_size);
    fprintf(stderr,
            " impedance set to %g Ohm\n expecting trigger on id=%d ttls=0x%lx\n external clock frequency set to %g Hz\n",
            impedance,
            t_line.id,
            t_line.ttls.to_ulong(),
            (float)ext_reference_clock );
}

void SpectrumMI40xxSeries::sample_after_external_trigger(double rate, size_t samples, double sensitivity, size_t resolution) {
    throw SpectrumMI40xxSeries_error("SpectrumMI40xxSeries::sample_after_external_trigger is not jet implemented");
}



void SpectrumMI40xxSeries::collect_config_recursive(state_sequent& exp, SpectrumMI40xxSeries::Configuration& settings) {

    /* start with dummy node */
    DataManagementNode* new_branch=new DataManagementNode(NULL);
    DataManagementNode* where_to_append=new_branch;
    double parent_timeout=settings.timeout;
    settings.timeout=0.0;

    /* loop over all states and sequences */
    for (state_sequent::iterator i=exp.begin(); i!=exp.end(); ++i) {

        state* a_state=dynamic_cast<state*>(*i);
        if (a_state==NULL)
            throw SpectrumMI40xxSeries_error("state_atom not expected in sate_sequent");
        if (dynamic_cast<state_parallel*>(*i)!=NULL)
            throw SpectrumMI40xxSeries_error("state_parallel handling is not jet implemented");
        state_sequent* a_sequence=dynamic_cast<state_sequent*>(a_state);
        if (a_sequence!=NULL) {
            // found a sequence
            DataManagementNode* tmp_structure=settings.data_structure;
            settings.data_structure=where_to_append;
            collect_config_recursive(*a_sequence, settings);
            settings.data_structure=tmp_structure;
        } /* end working on sequence */
        else {
            // found a state, not a sequence
            settings.timeout+=a_state->length;
#if SPC_DEBUG
            fprintf(stderr,"SETTINGS %e %e\n",settings.timeout,a_state->length);
#endif
            // collect analogin sections in state
            std::list<analogin*> inputs;
            /* loop over all device definitions in a state */
            state::iterator j=a_state->begin();
            while (j!=a_state->end()) {
                analogin* input=dynamic_cast<analogin*>(*j);
                if (input!=NULL && input->id==device_id) {
                    /* collect appropiate analogin sections, forget others */
                    if (input->samples<=0 || input->sample_frequency<=0)
                        delete input;
                    else
                        inputs.push_back(input);
                    j=a_state->erase(j);
                } else
                    ++j;
            }
            if (!inputs.empty()) {
                /* evaluate the found analogin definitions */
                if (inputs.size()>1) {
                    while (!inputs.empty()) {
                        delete inputs.front();
                        inputs.pop_front();
                    }
                    throw ADC_exception("can not handle more than one analogin section per state");
                }
                /* save sampling frequency */
                if (settings.samplefreq<=0)
                    settings.samplefreq=inputs.front()->sample_frequency;
                else if (settings.samplefreq != inputs.front()->sample_frequency) {
                    while (!inputs.empty()) {
                        delete inputs.front();
                        inputs.pop_front();
                    }
                    throw ADC_exception("Sorry, but gated sampling requires same sampling frequency in all analogin sections");
                }

                /* save channel mask and number of channels */
                if (settings.lSetChannels > 0 && settings.qwSetChEnableMap.to_ulong() > 0) {
                    if (settings.qwSetChEnableMap != inputs.front()->channels) {
                        fprintf(stderr, "Warning! different channels enabled in input %lu and in config %lu, setting to default \n",
                                settings.qwSetChEnableMap.to_ulong(),
                                inputs.front()->channels.to_ulong());
                        settings.qwSetChEnableMap = channel_array(ADC_MI_DEFAULT_CHANNELS);
                        settings.lSetChannels = settings.qwSetChEnableMap.count();
                    }
                } else {
                    settings.qwSetChEnableMap = inputs.front()->channels;
                    settings.lSetChannels = inputs.front()->nchannels;
                }

                /* save sensitivity */
                if (settings.sensitivity != NULL) { // if sensitivity is set, make sure it's valid (i.e. the same for all inputs)
                    for (int k = 0; k < inputs.front()->nchannels; k++) {
                        if (settings.sensitivity[k] != inputs.front()->sensitivity[k]) {
                            fprintf(stderr, "Warning! different sensitivity specified (here %f, elsewhere %f), choosing higher voltage\n",
                                    settings.sensitivity[k],
                                    inputs.front()->sensitivity[k]);
                            if (settings.sensitivity[k] < inputs.front()->sensitivity[k]) {
                                settings.sensitivity[k] = inputs.front()->sensitivity[k];
                            }
                        }
                    }
                } else {
                    settings.sensitivity = inputs.front()->sensitivity;
                }
                // check if sensitivity is valid
                for (int k = 0; k < inputs.front()->nchannels; k++) {
                    bool sensAllowed = false;
                    for (int l = 0; l < ADC_MI_ALLOWED_SENSITIVITY_LENGTH; l++) {
                        if (settings.sensitivity[k] == ADC_MI_ALLOWED_SENSITIVITY[l] ) {
                            sensAllowed = true;
                            break;
                        }
                    }
                    if (!sensAllowed) {
                        fprintf(stderr, "Warning! Illegal sensitivity specified for channel %i: %f", k, inputs.front()->sensitivity[k]);
                        settings.sensitivity[k] = ADC_MI_DEFAULT_SENSITIVITY;
                    }
                }

                /* save impedance */
                if (settings.impedance != NULL) {
                    for (int k = 0; k < inputs.front()->nchannels; k++) {
                        if (settings.impedance[k] != inputs.front()->impedance[k]) {
                            fprintf(stderr, "Warning! different impedance specified (here %f, elsewhere %f), setting to default\n",
                                    settings.impedance[k],
                                    inputs.front()->impedance[k]);
                            settings.impedance[k] = ADC_MI_DEFAULT_IMPEDANCE;
                        }
                        if (settings.impedance[k] != ADC_MI_DEFAULT_IMPEDANCE && settings.impedance[k] != ADC_MI_ALLOWED_IMPEDANCE) {
                            fprintf(stderr, "Warning! Illegal impedance specified for channel %i: %f",k, inputs.front()->impedance[k]);
                            settings.offset[k] = 0;
                        }
                    }
                } else {
                    settings.impedance = inputs.front()->impedance;
                }

                /* save offsets */
                if (settings.offset != NULL) {
                    for (int k = 0; k < inputs.front()->nchannels; k++) {
                        if (settings.offset[k] != inputs.front()->offset[k]) {
                            fprintf(stderr, "Warning! different impedance specified (here %i, elsewhere %i), setting to default\n",
                                    settings.offset[k],
                                    inputs.front()->offset[k]);
                            settings.offset[k] = ADC_MI_DEFAULT_OFFSET;
                        }
                        if (inputs.front()->offset[k] > 100 || inputs.front()->offset[k] < -100) {
                            fprintf(stderr, "Warning! Illegal offset specified for channel %i: %i", k, inputs.front()->offset[k]);
                            settings.offset[k] = 0;
                        }
                    }
                } else {
                    settings.offset = inputs.front()->offset;
                }

                if (inputs.front()->samples%4 != 0) {
                    throw ADC_exception("Number of samples must be a multiple of four");
                }

		    // calculate the time required
		    double delayed_gating_time=0.0;
		    // the gating time has an offset, which was found to be 1.5 dwelltimes for <2.5MHz and 4.5 dwelltimes for >=2.5MHz
		    double gating_time;
		    
#if SPC_DEBUG
		    fprintf(stderr, "Channels: %lu\n", settings.qwSetChEnableMap.to_ulong());
#endif
		    
		    /* check if channel mask is legal for the card */
	        if (this->IsChannelMaskLegal(inputs.front()->channels.to_ulong())) {
    		    settings.qwSetChEnableMap = inputs.front()->channels;
			    settings.lSetChannels = inputs.front()->nchannels;
		    } else {
		        throw SpectrumMI40xxSeries_error("Selected channels combination not allowed for this card type");
		    }
		    /* apply proper timing */
		    if ( settings.qwSetChEnableMap.to_ulong()==(CHANNEL0|CHANNEL1) || settings.qwSetChEnableMap.to_ulong()==(CHANNEL0|CHANNEL1|CHANNEL2|CHANNEL3) )  {
#if SPC_DEBUG		    
                fprintf(stderr, "Default Channels\n");
#endif
		        if (settings.samplefreq<2.5e6) {
		            // if sampling rate is <2.5MHz, there is another data handling mode,
		            // see MI4021 manual page 79: "Accquisition Delay: -6 Samples"
		            // it might be necessary to add 0.1 dwelltime to shift the sampling start a little more...
		            
		            // edit by Stefan Reutter @2013-06, it seems that the MI4021 cards actually have a second 
		            // threshold at 500 kHz that is not mentioned in the manual.
		            // this can be tested by disabling the if below and switching over the threshold
		            gating_time=(inputs.front()->samples)/settings.samplefreq;
		            if (settings.samplefreq >= 5e5) {
    		            delayed_gating_time=ceil(1e8*6.0/settings.samplefreq)/1e8;
		            } else {
    		            delayed_gating_time=0.0;
		            }
		        } else {
		            gating_time=(inputs.front()->samples+3)/settings.samplefreq;
		            delayed_gating_time=0.0;
		        }
		    }
		    // disabled the more exotic channel setup as it is untested with the updated timings and probably not used anyways
		    /* else if (settings.qwSetChEnableMap.to_ulong()==CHANNEL0 || settings.qwSetChEnableMap.to_ulong()==(CHANNEL0|CHANNEL2) )  {
#if SPC_DEBUG		    
                fprintf(stderr, "Weird Channels\n");
#endif
		        if (settings.samplefreq<5e6) {
		            gating_time=(inputs.front()->samples+1.5)/settings.samplefreq;
		            delayed_gating_time=ceil(1e8*6.0/settings.samplefreq)/1e8;
        		} else {
		            gating_time=(inputs.front()->samples+4.5)/settings.samplefreq;
		            delayed_gating_time=-ceil(1e8*7.0/settings.samplefreq)/1e8;

            } */else {
		        throw SpectrumMI40xxSeries_error("Selected channels combination not allowed");
		    }
		
		    gating_time=ceil(1e8*gating_time)/1e8; 
		    double time_required=delayed_gating_time+gating_time;
		    // check time requirements
		    if (a_state->length < gating_time) {
		        char parameter_info[512];
		        snprintf(parameter_info,sizeof(parameter_info),
			        "(%" SIZETPRINTFLETTER " samples, %g samplerate, %e time required, %e state time)",
			        inputs.front()->samples,
			        settings.samplefreq,
			        time_required,
			        a_state->length);
#if SPC_DEBUG	
			    fprintf(stderr, "state is shorter than acquisition time %e time required, %e state time\n", gating_time, a_state->length);
#endif
			    // update the state length if it's shorter than the gate. this is usually due to rounding to 10 ns for the pulseblaster
		        if (ceil(1e8*a_state->length)/1e8 < time_required) {
				    throw ADC_exception(std::string("state is shorter than acquisition time")+parameter_info);
			    } else {
			        a_state->length = time_required;
			    }
		    }
            // if necessary, add the gating pulse delay...
            if (delayed_gating_time>0.0) {
                state* delayed_gating_state=new state(*a_state);
                delayed_gating_state->length=delayed_gating_time;
                // insert before me
                exp.insert(i,(state_atom*)delayed_gating_state);
		    } else if (delayed_gating_time < 0.0)  {
                /*
		        For +samples delays
                    1. get the previous state
                    2. if the length is not long enough (6*dw) add the state before
                    3. split the state(s) so that we have the gating on 6*dw before the actual recording
                */
                double rest_length = delayed_gating_time;
                state_sequent::iterator i_prev;
                state* prev_state;
                i_prev = i;
                do {
                    i_prev--;
                    prev_state = dynamic_cast<state*>(*(i_prev));
                    rest_length -= prev_state->length;
                    fprintf(stderr, "DW rest_length: %g\n", rest_length);
                    fprintf(stderr, "DW state_length: %g\n", prev_state->length);
                    if (rest_length >= 0)
                        prev_state->push_back(trigger_line.copy_new()); // add trigger to this state
                    else { // split final state
                        state* prev_state_1 = prev_state->copy_flat(); //create copy of current state
                        prev_state_1->length += rest_length; // adjust 1st part length
                        exp.insert(i_prev,(state_atom*) prev_state_1); // insert AFTER prev_state
                        prev_state->length = -rest_length; // adjust 2nd part length
                        prev_state->push_back(trigger_line.copy_new()); // add trigger to 2nd part
                        break;
                    }

                } while (i_prev!=exp.begin() || rest_length > 0.0);
            }
# if SPC_DEBUG
            fprintf(stderr, "sequence after pre_trigger correction:\n");
            xml_state_writer().write_states(stderr, exp);
# endif
			
		    // adapt the pulse program for gated sampling
		    if (a_state->length == gating_time) {
		        // state has proper length
		        a_state->push_back(trigger_line.copy_new());
	        } else {
# if SPC_DEBUG
	            fprintf(stderr, "state too long, length %e, time required %e\n", a_state->length, time_required);
# endif
		        // state is too long... 
		        // create new one with proper time and gated sampling pulse
		        state* gated_sampling_pulse=new state(*a_state);
		        gated_sampling_pulse->length=gating_time;
		        gated_sampling_pulse->push_back(trigger_line.copy_new());
		        // insert gate pulse state before remaining (original) state
		        exp.insert(i,(state_atom*)gated_sampling_pulse);
		        // shorten this state
		        a_state->length-=time_required;
		    }
		    
# if SPC_DEBUG
                fprintf(stderr, "sequence after correcting trigger state:\n");
                xml_state_writer().write_states(stderr, exp);
# endif
		    
		    /* save sampleno */
		    DataManagementNode* new_one=new DataManagementNode(new_branch);
		    new_one->n=inputs.front()->samples;
		    new_one->child=NULL;
		    new_one->next=where_to_append->next;
		    where_to_append->next=new_one;
		    where_to_append=new_one;
		    while (!inputs.empty()) {delete inputs.front(); inputs.pop_front();}
	    } /* !inputs.empty() */



        } /* end working on state */


    } /* i */

    /* something happened? */
    if (new_branch->next!=NULL) {
        /* make dummy node to a loop */
        new_branch->n=exp.repeat;
        new_branch->child=new_branch->next;

        /* if possible, append it */
        if (settings.data_structure!=NULL) {
            new_branch->parent=settings.data_structure->parent;
            new_branch->next=settings.data_structure->next;
            settings.data_structure->next=new_branch;
        }
        else {
            new_branch->parent=NULL;
            new_branch->next=NULL;
            settings.data_structure=new_branch;
        }
    }
    else
        delete new_branch;

    settings.timeout*=exp.repeat;
    settings.timeout+=parent_timeout;
#ifdef SPC_DEBUG
    fprintf(stderr,"setting.timout %g\n",settings.timeout);
#endif
    return;
}

static void* SpectrumMI40xxSeries_TimeoutThread(void* data) {
	int *ret = new int;
	*ret = ((SpectrumMI40xxSeries*)data)->TimeoutThread();
    return (void*) ret;
}

void SpectrumMI40xxSeries::set_daq(state & exp) {

    // cleanup?!
    if (!fifobuffers.empty()) {
        fprintf(stderr, "normally there should be a cleanup at the end of acquisition\n");
        for (std::vector<short int*>::iterator i=fifobuffers.begin(); i!=fifobuffers.end(); ++i) free(*i);
        fifobuffers.clear();
    }

    /* what could be done: PCIBIT_GATE, PCIBIT_MULTI */
    int features;
    SpcGetParam(deviceno,SPC_PCIFEATURES,&features);

    state_sequent* exp_sequence=dynamic_cast<state_sequent*>(&exp);
    if (exp_sequence==NULL)
        throw ADC_exception("Spectrum-MI40xxSeries::set_daq only working on sequences");

# if SPC_DEBUG
    fprintf(stderr, "working on sequence:\n");
    xml_state_writer().write_states(stderr, *exp_sequence);
# endif

    /* find out what to do */
    Configuration* conf=new Configuration();
    collect_config_recursive(*exp_sequence, *conf);
    if (conf->samplefreq<=0) conf->samplefreq=default_settings.samplefreq;
    if (conf->impedance == NULL) conf->impedance = default_settings.impedance;
    if (conf->sensitivity  == NULL) conf->sensitivity = default_settings.sensitivity;
    if (conf->offset  == NULL) conf->offset = default_settings.offset;

    if (conf->lSetChannels == 0) {
        conf->lSetChannels = default_settings.lSetChannels;
        conf->qwSetChEnableMap = default_settings.qwSetChEnableMap;
    }

    size_t sampleno=(conf->data_structure==NULL)?0:conf->data_structure->size();

    /* nothing to do! */
    if (sampleno==0) {
        delete conf;
        effective_settings=NULL;
        return;
    }


    if (sampleno<16 || sampleno%16!=0) {
        delete conf;
        throw SpectrumMI40xxSeries_error("Total number of samples must be multiple of 16 and at least 16");
    }

    effective_settings=conf;

#if SPC_DEBUG
    /* settings for this experiment */
    conf->print(stderr);
#endif

    /* make a check, whether spectrum board is running */
    int actual_status=0;
    SpcGetParam (deviceno, SPC_STATUS, &actual_status);
    if (actual_status!=SPC_READY) {
        fprintf(stderr, "Warning: Spectrum board was/is running before starting data aquisition.\n");
        SpcSetParam (deviceno, SPC_COMMAND,      SPC_STOP);     // stop the board
    }

    /* and the dirty things there...
       ----- setup board for recording -----
    */
    for (unsigned int j=0; j<(unsigned int)effective_settings->lSetChannels; j++) {
        SpcSetParam (deviceno, SPC_AMP0 + 100*j,    (int)floor(effective_settings->sensitivity[j]*1000));          // +/- 5V input range
        SpcSetParam (deviceno, SPC_50OHM0 + 100*j,  ((effective_settings->impedance[j]==50.0)?1:0));             // 1 = 50 Ohm input impedance, 0 = 1MOhm input impedance
        SpcSetParam (deviceno, SPC_OFFS0 +  100*j, effective_settings->offset[j]); // set offset to zero
    }

    SpcSetParam (deviceno, SPC_CHENABLE, effective_settings->qwSetChEnableMap.to_ulong()); // Enable channels for recording
    int activated_channels;
    SpcGetParam (deviceno, SPC_CHENABLE, &activated_channels);

    SpcSetParam (deviceno, SPC_SAMPLERATE,          (int)floor(effective_settings->samplefreq));      // Samplerate

    // the MI4021 card doesn't accept all sampling frequency settings. check if the sampling rate is set correctly. if not, throw an exception
    int setSamplingRate = 0;
    SpcGetParam (deviceno, SPC_SAMPLERATE,          &setSamplingRate);
    if (setSamplingRate != (int)floor(effective_settings->samplefreq)) {
        char parameter_info[16];
        snprintf(parameter_info,sizeof(parameter_info), "%d", setSamplingRate);
        throw ADC_exception(std::string("DAC sampling rate not available. Try setting to: ")+parameter_info);
    }

  // decide for aquisition mode and start it
  int16 nErr=ERR_OK;
  if (sampleno<fifo_minimal_size) {

#if SPC_DEBUG
        fprintf(stderr, "expecting %" SIZETPRINTFLETTER " samples in normal mode\n",sampleno);
#endif

      SpcSetParam (deviceno, SPC_MEMSIZE,             sampleno);      // Memory size
	  
      // ----- start the board -----
      nErr = SpcSetParam (deviceno, SPC_COMMAND,      SPC_START);     // start the board
  } else {

#if SPC_DEBUG
        fprintf(stderr, "expecting %" SIZETPRINTFLETTER " samples in fifo mode\n",sampleno);
#endif
        // todo: fifo should really write directly to recursive gated sampling structures
        fifo_adc_data=(short int*)malloc(sampleno*sizeof(short int)*2);
        if (fifo_adc_data==NULL) {
            throw SpectrumMI40xxSeries_error("could not allocate adc data memory for fifo recording");
        }
        // ToDo: do some magic calculations
        fifobufferno=16;
        fifobufferlen=1<<20;
#if SPC_DEBUG
        fprintf(stderr, "configuring for %" SIZETPRINTFLETTER " buffers, each of size %" SIZETPRINTFLETTER "\n", fifobufferno, fifobufferlen);
#endif
        SpcSetParam(deviceno,SPC_FIFO_BUFFERS,fifobufferno);
        SpcSetParam(deviceno,SPC_FIFO_BUFLEN,fifobufferlen);
        // allocate FIFO buffers
        fifobuffers.resize(fifobufferno,(short int*)NULL);
        for (size_t i=0; i!=fifobufferno; ++i) {
            void* newbuffer;
#if SPC_DEBUG
            fprintf(stderr, "allocating fifobuffer %" SIZETPRINTFLETTER "...", i);
#endif
            newbuffer=malloc(fifobufferlen);
            if (newbuffer==NULL) {
                // free the buffers, if there is not enough memory
                for (size_t j=0; j!=i; ++j) free(fifobuffers[j]);
                fifobuffers.clear();
                throw SpectrumMI40xxSeries_error("could not allocate buffers for fifo mode");
            }
#if SPC_DEBUG
            fprintf(stderr, "and registering with ADC driver....");
#endif
            fifobuffers[i]=(short int*)newbuffer;
            // todo: check for errors
#ifndef _LINUX64
            SpcSetParam (deviceno, SPC_FIFO_BUFADR0+i, (int32) newbuffer);
#else
            // 64 bit Linux needs SetAdr function, 32 bit linux can also use this if driver build > 1093
            SpcSetAdr (deviceno, SPC_FIFO_BUFADR0+i, newbuffer);
#endif
#if SPC_DEBUG
            fprintf(stderr, "success.\n");
#endif
        }
#if SPC_DEBUG
        fprintf(stderr, "starting fifo thread\n");
#endif
        // to do: append a new state to overcome problems with fifo...
        // need another thread, that collects the data
        pthread_attr_init(&timeout_pt_attrs);
        pthread_create(&timeout_pt, &timeout_pt_attrs, SpectrumMI40xxSeries_TimeoutThread,(void*)this);
    }
    // ----- driver error: request error and end program -----
    if (nErr != ERR_OK) {
        if (effective_settings!=NULL) delete effective_settings;
        effective_settings=NULL;
        for (std::vector<short int*>::iterator i=fifobuffers.begin(); i!=fifobuffers.end(); ++i) free(*i);
        fifobuffers.clear();
        // create an error message
        int32   lErrorCode, lErrorReg, lErrorValue;
        SpcGetParam (deviceno, SPC_LASTERRORCODE,   &lErrorCode);
        SpcGetParam (deviceno, SPC_LASTERRORREG,    &lErrorReg);
        SpcGetParam (deviceno, SPC_LASTERRORVALUE,  &lErrorValue);
        char error_message[256];
        snprintf(error_message, sizeof(error_message),"Configuration error %d in register %d at value %d", lErrorCode, lErrorReg, lErrorValue);
        throw SpectrumMI40xxSeries_error(error_message);
    }

}

double SpectrumMI40xxSeries::get_sample_clock_frequency() const {

    if (effective_settings==NULL ||
            effective_settings->samplefreq<=0 ||
            effective_settings->data_structure==NULL ||
            effective_settings->data_structure->size()==0)
        return 0;

    return effective_settings->samplefreq;
}

int SpectrumMI40xxSeries::TimeoutThread() {
    // now collect data
    assert(effective_settings!=NULL);
    size_t sampleno=effective_settings->data_structure->size();
    size_t buff_number_expected=(sampleno*2*sizeof(short int)+fifobufferlen-1)/fifobufferlen;
    SpcSetParam(deviceno, SPC_FIFO_BUFMAXCNT, buff_number_expected);
    size_t buff_number=0;
    short int* buff_pointer=fifo_adc_data;

    // start fifo aquisition
#if SPC_DEBUG
    fprintf(stderr, "SPC_FIFOSTART\n");
#endif
    SpcSetParam(deviceno, SPC_COMMAND, SPC_FIFOSTART);

    do {
        // wait for the buffers
        pthread_testcancel();
#if SPC_DEBUG
        fprintf(stderr, "reading buffer no %" SIZETPRINTFLETTER "/%" SIZETPRINTFLETTER "\n", buff_number+1, buff_number_expected);
#endif
        if (buff_number+1==buff_number_expected) {
            // the last one is special, copy only expected bytes
            memcpy(buff_pointer, fifobuffers[buff_number%(fifobuffers.size())], sampleno*2*sizeof(short int)-fifobufferlen*buff_number);
            break;
        }
        else
            buff_pointer=(short int*)mempcpy(buff_pointer, fifobuffers[buff_number%(fifobuffers.size())], fifobufferlen);
        SpcSetParam(deviceno,SPC_FIFO_BUFREADY, buff_number%(fifobuffers.size()));
        buff_number++;
        int buff_available;
        SpcGetParam(deviceno, SPC_FIFO_BUFCOUNT, &buff_available);
        if (((size_t)buff_available)>buff_number) {
            // get also the next buffer...
#if SPC_DEBUG
            fprintf(stderr, "read  %" SIZETPRINTFLETTER " buffers, %d buffers gained by the hardware sofar\n", buff_number, buff_available);
#endif
            continue;
        }

        int adc_status;
#if SPC_DEBUG
        fprintf(stderr, "SPC_STATUS\n");
#endif
        SpcGetParam (deviceno, SPC_STATUS, &adc_status);
        if (adc_status==SPC_READY) {
            // this must be a timeout!
            //pthread_cancel(timeout_pt);
            //pthread_join(timeout_pt,NULL);
            //pthread_attr_destroy(&timeout_pt_attrs);
            free(fifo_adc_data);
            fifo_adc_data=NULL;
            throw SpectrumMI40xxSeries_error("timout occured while collecting data");
            return 0;
        }
#if SPC_DEBUG
        fprintf(stderr, "waiting for next fifo buffer\n");
        fprintf(stderr, "SPC_FIFOWAIT\n");
#endif
        SpcSetParam(deviceno, SPC_COMMAND, SPC_FIFOWAIT);
    }
    while (buff_number<buff_number_expected);
#if SPC_DEBUG
    fprintf(stderr, "SPC_STOP\n");
#endif
    SpcSetParam(deviceno, SPC_COMMAND, SPC_STOP);

    return 1;
}

short int* SpectrumMI40xxSeries::split_adcdata_recursive(short int* data, const DataManagementNode& structure, adc_results& result_splitted) {
    short nchannels = effective_settings->lSetChannels;
    if (structure.child==NULL) {
        // simple case: do real work
        // todo: Channel selection
        short int* datachunk=(short int*)malloc(sizeof(short int)*structure.n*nchannels);
        if (datachunk==NULL) {
            throw SpectrumMI40xxSeries_error("not enough memory to create results");
        }
        // todo: error checking
        memcpy(datachunk, data, sizeof(short int)*structure.n*nchannels);
        data+=structure.n*nchannels;
        adc_result* the_result=new adc_result(0, structure.n, datachunk, effective_settings->samplefreq,nchannels);
        result_splitted.push_back(the_result);
        if (structure.next!=NULL)
            data=split_adcdata_recursive(data, *(structure.next), result_splitted);
    }
    else
        for (size_t i=0; i<structure.n; ++i) {
            data=split_adcdata_recursive(data, *(structure.child), result_splitted);
        }
    return data;

}

result* SpectrumMI40xxSeries::get_samples(double _timeout) {

    if (core::term_signal!=0) return NULL;
    size_t sampleno=(effective_settings==NULL || effective_settings->data_structure==NULL)?0:effective_settings->data_structure->size();
    if (sampleno==0) return new adc_result(1,0,NULL);
    short nchannels=(effective_settings==NULL)?0:effective_settings->lSetChannels;
    if (nchannels==0) throw SpectrumMI40xxSeries_error("No channels specified");

//    double timeout=_timeout;

#if SPC_DEBUG
    fprintf(stderr,"effective_settings in get_samples: %g\n",effective_settings->timeout);
#endif
    // allocate a lot of space
    // todo: use GatedSampling buffers directly!

    if (fifobuffers.empty()) {
#if SPC_DEBUG
        printf("ADC not in FIFO mode\n");
#endif
        short int* adc_data=(short int*)malloc(sampleno*sizeof(short int)*nchannels);
        if (adc_data==NULL) {
            for (std::vector<short int*>::iterator i=fifobuffers.begin(); i!=fifobuffers.end(); ++i) free(*i);
            fifobuffers.clear();
            throw SpectrumMI40xxSeries_error("could not allocate adc data memory");
        }

        // simple version: standard acquisition, wait and get...
#if SPC_DEBUG
        fprintf(stderr, "fetching %" SIZETPRINTFLETTER " samples in normal mode (timeout is %g)\n",sampleno,effective_settings->timeout);
#endif
        stopwatch adc_timer;
        adc_timer.start();
        int adc_status, i;
        double elapsed_time=0;
        /*
        The gate-end signal can only be recognized at an eigth samples alignement.
        The board can record up to 7 more samples, the timeout has to be allowed to be longer
        */
        double post_gate_maxtime=(1e8*7.5/effective_settings->samplefreq)/1e8;
        i = 0;
        SpcGetParam(deviceno, SPC_STATUS, &adc_status);
        /*
        Sometimes the timeout is not enough, the reason could be that the timer
        starts running before the actual start of the pulse program because the OS is not loading and starting
               the pulseblaster immediatly. It is not possible to
        synchronize the timers for now.
        Thus, we add a generous 1s to the timeout to be on the safe side. One second ought to be enoug for everybody!
        */
        while (core::term_signal==0 && adc_status!=SPC_READY && elapsed_time<=(effective_settings->timeout + post_gate_maxtime + 0.5) ) {
            timespec sleeptime;
            sleeptime.tv_nsec=10*1000*1000; // 10 ms
            sleeptime.tv_sec=0;
            nanosleep(&sleeptime,NULL);
            SpcGetParam(deviceno, SPC_STATUS, &adc_status);
            elapsed_time=adc_timer.elapsed();
#if SPC_DEBUG
            fprintf(stderr, "(nofifo) loop %i (adc_status, t, t_out, post_gate, core): %i %g %g %g %i\n",i,adc_status, elapsed_time, effective_settings->timeout, post_gate_maxtime, core::term_signal);
#endif
            i++;
        }
#if SPC_DEBUG
        fprintf(stderr, "(nofifo) loop stopped  (adc_status, t, t_out, core): %i %g %g %i\n",adc_status, elapsed_time, effective_settings->timeout, core::term_signal);
#endif
        /* card ready to transfer samples */
        if (adc_status!=SPC_READY) {
            free(adc_data);
#if SPC_DEBUG
            fprintf(stderr, "ERROR: adc_status not ready (%i) \n",adc_status );
#endif
            throw SpectrumMI40xxSeries_error("timeout occured while collecting data");
        }
        SpcSetParam(deviceno, SPC_COMMAND, SPC_STOP);
        if (core::term_signal!=0) {
#if SPC_DEBUG
            fprintf(stderr, "received core::term_signal %i\n",core::term_signal);
#endif
            free(adc_data);
            return NULL;
        }

# if defined __linux__
        size_t data_length=SpcGetData (deviceno, 0, 0, nchannels * sampleno, 2, (dataptr) adc_data);
# if SPC_DEBUG
        fprintf(stderr, "SpcGetData returned %" SIZETPRINTFLETTER " bytes, got %" SIZETPRINTFLETTER " out of %" SIZETPRINTFLETTER " expected samples\n", data_length, data_length/sizeof(short int)/2, sampleno);
# endif
        if (nchannels*sizeof(short int)*sampleno!=data_length) {
            free(adc_data);
            throw SpectrumMI40xxSeries_error("wrong amount of data from adc card");
        }
        // current position in adc data array
# elif defined __CYGWIN__
        int16 read_result=SpcGetData (deviceno, 0, 0, nchannels * sampleno,(void*) adc_data);
# if SPC_DEBUG
        fprintf(stderr, "SpcGetData returned %d\n", read_result);
# endif
        if (read_result!=0) {
            free(adc_data);
            throw SpectrumMI40xxSeries_error("wrong amount of data from adc card");
        }
# endif
        short int* data_position=adc_data;
        // produced results
        adc_results* the_results=new adc_results(0);
        data_position=split_adcdata_recursive(data_position, *(effective_settings->data_structure), *the_results);
        if (data_position==0 || (size_t)(data_position-adc_data)!=(sampleno*nchannels)) {
            fprintf(stderr,"something went wrong while spliting data\n");
        }
        free(adc_data);
        delete effective_settings;
        effective_settings=NULL;
        return the_results;
    }
    else {
#if SPC_DEBUG
        printf("ADC in FIFO mode!\n");
        fprintf(stderr, "fetching %" SIZETPRINTFLETTER " samples in fifo mode (timeout is %g)\n",sampleno,effective_settings->timeout);
#endif
        // FIFO method
        stopwatch timer;
        timer.start();
        do {
            int32 lStatus;
            timespec sleeptime;
            sleeptime.tv_sec=0;
            sleeptime.tv_nsec=100*1000*1000;
            nanosleep(&sleeptime,NULL);
            //pthread_testcancel();
            SpcGetParam (deviceno, SPC_STATUS, &lStatus);
#if SPC_DEBUG
            fprintf(stderr, "while loop FIFO (lStatis, timer, effective_timeout): %i %g %g\n",lStatus, timer.elapsed(), effective_settings->timeout );
#endif
            if (lStatus == SPC_READY) break;
        } while (timer.elapsed()<effective_settings->timeout); //rosi _timeout
        SpcSetParam(deviceno, SPC_COMMAND, SPC_STOP);

#if SPC_DEBUG
        fprintf(stderr, "waiting for read thread terminating\n");
#endif
        /* wait for read thread */
        pthread_cancel(timeout_pt);
        pthread_join(timeout_pt,NULL);
        pthread_attr_destroy(&timeout_pt_attrs);
        /* free it's resources */
#if SPC_DEBUG
        fprintf(stderr, "freeing fifo data buffers\n");
#endif
        for (std::vector<short int*>::iterator i=fifobuffers.begin(); i!=fifobuffers.end(); ++i) free(*i);
        fifobuffers.clear();

        adc_results* the_results=NULL;
        if (fifo_adc_data!=NULL) {
            // split data to (small) pieces
            short int* data_position=fifo_adc_data;
            the_results=new adc_results(0);
            data_position=split_adcdata_recursive(data_position, *(effective_settings->data_structure), *the_results);
            if (data_position==0 || (size_t)(data_position-fifo_adc_data)!=(sampleno*nchannels)) {
                fprintf(stderr,"something went wrong while spliting data from fifo mode\n");
            }
            free(fifo_adc_data);
            fifo_adc_data=NULL;
        }
        else
            fprintf(stderr,"something went wrong while collecting data with fifo mode\n");

        delete effective_settings;
        effective_settings=NULL;
        return the_results;
    }
}

/* check whether a channel mask is legal for this board */
bool SpectrumMI40xxSeries::IsChannelMaskLegal(int mask) {
    if ( (cardType == TYP_MI4020 || cardType == TYP_MI4030) && mask != CHANNEL0) {
        return false;
    } else if ( (cardType == TYP_MI4021 || cardType == TYP_MI4031)  && mask != (CHANNEL0 | CHANNEL1) && mask != CHANNEL0) {
        return false;
    } else if ((cardType == TYP_MI4022 || cardType == TYP_MI4032) && mask != (CHANNEL0 | CHANNEL1 | CHANNEL2 | CHANNEL3) && mask != (CHANNEL0 | CHANNEL1) && mask != (CHANNEL0 | CHANNEL2) && mask != CHANNEL0) {
        return false;
    }
    return true;
}

SpectrumMI40xxSeries::~SpectrumMI40xxSeries() {
    SpcSetParam (deviceno, SPC_COMMAND,      SPC_STOP);     // stop the board
# if defined __linux__
    close (deviceno);
# elif defined __CYGWIN__
    FreeLibrary(spectrum_driver_dll);
# endif
}

