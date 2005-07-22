#include <cmath>
#include "pthread.h"
#include "core/stopwatch.h"
#include "core/result.h"
#include "Spectrum-MI40xxSeries.h"

SpectrumMI40xxSeries::SpectrumMI40xxSeries(const ttlout& t_line) {
  sampleno=0;
  samplefreq=0;
  // to be configured
  device_id=0;
  trigger_line=t_line;
  impedance=1e6; // default 1MOhm
  
#     if defined __linux__
      // ----- open driver -----
      deviceno = open ("/dev/spc0", O_RDWR);
      if (deviceno <= 0) throw SpectrumMI40xxSeries_error("device not found");
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
      
      /* make a check, whether spectrum board is running */
      int actual_status=0;
      SpcGetParam(deviceno, SPC_STATUS, &actual_status);
      if (actual_status!=SPC_READY) {
	fprintf(stderr, "Warning: Spectrum board was is running before reset.\n");
      }

      int ErrorOccurred=1;
      SpcSetParam(deviceno, SPC_COMMAND,        SPC_RESET)==ERR_OK &&       // reset device first
	  SpcSetParam (deviceno, SPC_GATE,          1)==ERR_OK &&             // Gated Triggering
	  SpcSetParam (deviceno, SPC_PLL_ENABLE,    1)==ERR_OK &&             // Internal PLL enabled for clock
	  SpcSetParam (deviceno, SPC_CLOCKOUT,      0)==ERR_OK &&             // No clock output
	  SpcSetParam (deviceno, SPC_REFERENCECLOCK,100000000)==ERR_OK &&     // external reference clock
	  SpcSetParam (deviceno, SPC_EXTERNALCLOCK, 0)==ERR_OK &&             //  but no external sample clock
	  SpcSetParam (deviceno, SPC_CLOCK50OHM,    1)==ERR_OK &&             // clock input with 50Ohm impedance
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


}

void SpectrumMI40xxSeries::sample_after_external_trigger(double rate, size_t samples, double sensitivity, size_t resolution) {
  throw SpectrumMI40xxSeries_error("SpectrumMI40xxSeries::sample_after_external_trigger is not jet implemented");  
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

  /* find out what to do */

  double sensitivity=5.0;
  sampleno=0;
  samplefreq=0;


  state_sequent* exp_sequence=dynamic_cast<state_sequent*>(&exp);
  if (exp_sequence==NULL)
    throw ADC_exception("Spectrum-MI40xxSeries: only working on sequences");

  // todo: cleanup before exception...
  state_iterator si(*exp_sequence);
  state* a_state=(state*)si.get_state();
  /* loop over all states */
  while (NULL!=a_state) {
    // collect analogin sections in state
    std::list<analogin*> inputs;
    /* loop over all device definitions in a state */
    state::iterator i=a_state->begin();
    while (i!=a_state->end()) {
      analogin* input=dynamic_cast<analogin*>(*i);
      if (input!=NULL && input->id==device_id) {
	/* collect appropiate analogin sections, forget others */
	if (input->samples<=0 || input->sample_frequency<=0)
	  delete input;
	else
	  inputs.push_back(input);
	i=a_state->erase(i);
      }
      else
	++i;
    }
    if (!inputs.empty()) {
      /* evaluate the found analogin definitions */
      if (inputs.size()>1) {
	while (!inputs.empty()) {delete inputs.front(); inputs.pop_front();}
	throw ADC_exception("can not handle more than one analogin section per state");
      }
      if (sampleno==0) {
	sampleno=inputs.front()->samples*si.get_count();
	samplefreq=inputs.front()->sample_frequency;
      }
      else if (samplefreq==inputs.front()->sample_frequency) {
        sampleno+=inputs.front()->samples*si.get_count();
      }
      else {
	while (!inputs.empty()) {delete inputs.front(); inputs.pop_front();}
	throw ADC_exception("Sorry, but gated sampling requires same sampling frequency in all analogin sections");
      }
      // adapt the pulse program for gated sampling
      if (a_state->length<1.0*inputs.front()->samples/samplefreq) {
	throw ADC_exception("state is shorter than acquisition time");
      }
      if (a_state->length>1.0*inputs.front()->samples/samplefreq) {
	// state is too long... create new one with proper time
	state* gated_sampling_pulse=new state(*a_state);
	double new_length=(inputs.front()->samples+3)/samplefreq;
	// align to lower 10ns step
	new_length=floor(1e8*new_length)/1e8;
	gated_sampling_pulse->length=new_length;
	gated_sampling_pulse->push_back(trigger_line.copy_new());
	// insert after me... well we have a const iterator, so we have to do a workaround...
	state_sequent* sseq=(state_sequent*)si.subsequence_stack.back().subsequence;
	state_sequent::iterator sseqi=si.subsequence_stack.back().subsequence_pos;
	sseq->insert(sseqi,(state_atom*)gated_sampling_pulse);
	// shorten this state
	a_state->length-=new_length;
      }
      else {
	// state has proper length
	a_state->push_back(trigger_line.copy_new());
      }

      // to do: more analogin sections per state
      while (!inputs.empty()) {delete inputs.front(); inputs.pop_front();}
    }
    // send a trigger pulse
    // todo only short trigger pulse
    a_state=(state*)si.next_state();
  }

  /* nothing to do! */
  if (sampleno==0) return;

  /* make a check, whether spectrum board is running */
  int actual_status=0;
  SpcGetParam(deviceno, SPC_STATUS, &actual_status);
  if (actual_status!=SPC_READY) {
      fprintf(stderr, "Warning: Spectrum board was is running before starting data aquisition.\n");
      SpcSetParam (deviceno, SPC_COMMAND,      SPC_STOP);     // start the board
  }



  /* and the dirty things there...
     ----- setup board for recording -----
  */
  size_t nChannels = 2;
  for (unsigned int j=0; j<nChannels; j++) {
    SpcSetParam (deviceno, SPC_AMP0 + 100*j,    (int)floor(sensitivity*1000));          // +/- 5V input range
    SpcSetParam (deviceno, SPC_50OHM0 + 100*j,  ((impedance==50.0)?1:0));             // 1 = 50 Ohm input impedance, 0 = 1MOhm input impedance
  }

  SpcSetParam (deviceno, SPC_CHENABLE,            CHANNEL0 | CHANNEL1); // Enable channels for recording
  SpcSetParam (deviceno, SPC_SAMPLERATE,          (int)floor(samplefreq));      // Samplerate: 20 MHz

  // decide for aquisition mode and start it
  int16 nErr=ERR_OK;
  if (sampleno<fifo_minimal_size) {
#if 0
      fprintf(stderr, "expecting %u samples in normal mode\n",sampleno);
#endif
      SpcSetParam (deviceno, SPC_MEMSIZE,             sampleno);      // Memory size
      // ----- start the board -----
      nErr = SpcSetParam (deviceno, SPC_COMMAND,      SPC_START);     // start the board
  }
  else {
      fprintf(stderr, "expecting %u samples in fifo mode\n",sampleno);
    // ToDo: do some magic calculations
    fifobufferno=16;
    fifobufferlen=1<<20;
    SpcSetParam(deviceno,SPC_FIFO_BUFFERS,fifobufferno);
    SpcSetParam(deviceno,SPC_FIFO_BUFLEN,fifobufferlen);
    // allocate FIFO buffers
    fifobuffers.resize(fifobufferno,(short int*)NULL);
    for (size_t i=0; i!=fifobufferno; ++i) {
      void* newbuffer=malloc(fifobufferlen);
      if (newbuffer==NULL) {
	// free the buffers, if there is not enough memory
	for (size_t j=0; j!=i; ++j) free(fifobuffers[j]);
	fifobuffers.clear();
	throw SpectrumMI40xxSeries_error("could not allocate buffers for fifo mode");
      }
      fifobuffers[i]=(short int*)newbuffer;
      SpcSetParam(deviceno,SPC_FIFO_BUFADR0+i,(int)newbuffer);
    }
    // start fifo aquisition
    nErr=SpcSetParam(deviceno, SPC_COMMAND, SPC_FIFOSTART);
    // to do: append a new state to overcome problems with fifo...
    
  }
  // ----- driver error: request error and end program -----
  if (nErr != ERR_OK) {
    int32   lErrorCode, lErrorReg, lErrorValue;
    SpcGetParam (deviceno, SPC_LASTERRORCODE,   &lErrorCode);
    SpcGetParam (deviceno, SPC_LASTERRORREG,    &lErrorReg);
    SpcGetParam (deviceno, SPC_LASTERRORVALUE,  &lErrorValue);
    char error_message[256];
    snprintf(error_message, sizeof(error_message),"Configuration error %d in register %d at value %d", lErrorCode, lErrorReg, lErrorValue);
    for (std::vector<short int*>::iterator i=fifobuffers.begin(); i!=fifobuffers.end(); ++i) free(*i);
    fifobuffers.clear();
    throw SpectrumMI40xxSeries_error(error_message);
  }
  
}

static void* SpectrumMI40xxSeries_TimeoutThread(void* data) {
  return (void*)((SpectrumMI40xxSeries*)data)->TimeoutThread();
}

int SpectrumMI40xxSeries::TimeoutThread() { 
  stopwatch timer;
  timer.start();
  do {
    int32 lStatus;
    timespec sleeptime;
    sleeptime.tv_sec=0;
    sleeptime.tv_nsec=100000000;
    nanosleep(&sleeptime,NULL);
    pthread_testcancel();
    SpcGetParam (deviceno, SPC_STATUS,      &lStatus);    
    pthread_testcancel();
    if (lStatus == SPC_READY) return 0;
  } while (timer.elapsed()<timeout);
  SpcSetParam(deviceno, SPC_COMMAND, SPC_STOP);
  return 1;
}

result* SpectrumMI40xxSeries::get_samples(double _timeout) {

    if (sampleno==0) return new adc_result(1,0,NULL);
  timeout=_timeout;
  result* the_result=NULL;
  // allocate a lot of space
  short int* adc_data=(short int*)malloc(sampleno*sizeof(short int)*2);
  if (adc_data==NULL) {
    for (std::vector<short int*>::iterator i=fifobuffers.begin(); i!=fifobuffers.end(); ++i) free(*i);
    fifobuffers.clear();
    throw SpectrumMI40xxSeries_error("could not allocate adc data memory");
  }

  if (fifobuffers.empty()) {
    // simple version: standard acquisition, wait and get...
#if 0
      fprintf(stderr, "fetching %u samples in normal mode (timeout is %g)\n",sampleno,timeout);
#endif
    stopwatch adc_timer;
    adc_timer.start();
    int adc_status;
    do {
      timespec sleeptime;
      sleeptime.tv_nsec=100000000;
      sleeptime.tv_sec=0;
      nanosleep(&sleeptime,NULL);
      SpcGetParam(deviceno, SPC_STATUS, &adc_status);
    }
    while (adc_status!=SPC_READY && adc_timer.elapsed()<timeout);
    if (adc_status!=SPC_READY) {
      SpcSetParam(deviceno, SPC_COMMAND, SPC_STOP);
      free(adc_data);
      throw SpectrumMI40xxSeries_error("timout occured while collecting data");
    }
    
# if defined __linux__
    SpcGetData (deviceno, 0, 0, 2 * sampleno, 2, (dataptr) adc_data);
# elif defined __CYGWIN__
    SpcGetData (deviceno, 0, 0, 2 * sampleno,(void*) adc_data);
# endif
  
    the_result=new adc_result(0,sampleno,adc_data,samplefreq);
  }
  else {
      fprintf(stderr, "fetching %u samples in fifo mode (timeout is %g)\n",sampleno,timeout);
    // FIFO method
    // need another thread, that stops my process
    pthread_attr_t timeout_pt_attrs;
    pthread_attr_init(&timeout_pt_attrs); 
    pthread_t timeout_pt;
    pthread_create(&timeout_pt,&timeout_pt_attrs,SpectrumMI40xxSeries_TimeoutThread,(void*)this);
    
    // now collect data
    size_t buff_number_expected=(sampleno*2*sizeof(short int)+fifobufferlen-1)/fifobufferlen;
    SpcSetParam(deviceno, SPC_FIFO_BUFMAXCNT, buff_number_expected);
    size_t buff_number=0;
    size_t buff_index=0;
    short int* buff_pointer=adc_data;
    while (buff_number<buff_number_expected) {
      // wait for the buffers
      SpcSetParam(deviceno, SPC_COMMAND, SPC_FIFOWAIT);
      int adc_status;
      SpcGetParam (deviceno, SPC_STATUS, &adc_status);
      if (adc_status==SPC_READY) {
	// this must be a timeout!
	pthread_cancel(timeout_pt);
	pthread_join(timeout_pt,NULL);
	pthread_attr_destroy(&timeout_pt_attrs);
	free(adc_data);
	for (std::vector<short int*>::iterator i=fifobuffers.begin(); i!=fifobuffers.end(); ++i) free(*i);
	fifobuffers.clear();
	throw SpectrumMI40xxSeries_error("timout occured while collecting data");
      }
      if (buff_number+1==buff_number_expected)
	// the last one is special
	memcpy(buff_pointer, fifobuffers[buff_index], sampleno*2*sizeof(short int)-fifobufferlen*buff_number);
      else
	buff_pointer=(short int*)mempcpy(buff_pointer, fifobuffers[buff_index], fifobufferlen);
      SpcSetParam(deviceno,SPC_FIFO_BUFREADY,buff_index);
      buff_number++;
      buff_index++;
      if (buff_index==fifobuffers.size()) buff_index=0;
    }
    SpcSetParam(deviceno, SPC_COMMAND, SPC_STOP);

    /* wait for timeout thread */
    pthread_cancel(timeout_pt);
    pthread_join(timeout_pt,NULL);
    the_result=new adc_result(0,sampleno,adc_data,samplefreq);
    pthread_attr_destroy(&timeout_pt_attrs);
    
    for (std::vector<short int*>::iterator i=fifobuffers.begin(); i!=fifobuffers.end(); ++i) free(*i);
    fifobuffers.clear();
  }
  return the_result;
}

SpectrumMI40xxSeries::~SpectrumMI40xxSeries() {
# if defined __linux__
  close (deviceno);
# elif defined __CYGWIN__
  FreeLibrary(spectrum_driver_dll);  
# endif
}

