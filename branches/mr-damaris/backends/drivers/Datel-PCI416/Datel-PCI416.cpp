/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include "Datel-PCI416.h"

DatelPCI416::DatelPCI416(const ttlout& t_line){
  trigger_line=t_line;
  PCI_DLL = LoadLibrary("PCI41632");
  if (PCI_DLL==NULL) {
    throw ADC_exception("found no library\n");
  }

    /* import all functions from dll and exit on error*/
#   define  pci41632dll_getproc(name) \
          name = (TFP_ ## name)GetProcAddress(PCI_DLL,#name); \
          if (name == NULL) {FreeLibrary(PCI_DLL); throw ADC_exception("found no " #name " function in DLL");}
#   include "PCI416_func_import.cpp"

    if (pci416_init(&brdcount)!=NOERROR) {
      brdcount=0;
      throw ADC_exception("initialisation failed");
    }

    // now hardcoded, use first board
    board=0;
    if (NOERROR!=pci416_getcaps(board,4,caps))
      throw ADC_exception("getcaps failed");
    fprintf(stderr,"Datel PCI416 caps: FIFO size: %d, DMA size: %d, index ADM %d, acqmode %d\n",caps[0],caps[1],caps[2],caps[3]);

    sample_count=0;
    trigger_count=0;
    sample_frequency=0;

    trigger_line.ttls=1<<2;
    trigger_line.id=0;

}

DatelPCI416::~DatelPCI416() {
  pci416_close(board);
  FreeLibrary(PCI_DLL);
}

/**
   here the data aquisition unit is configured and adds the necessary pusle components to the program
*/
void DatelPCI416::set_daq(state& exp) {

  sample_count=0;
  trigger_count=0;
  sample_frequency=0;

  // todo: cleanup before exception...
  state_iterator si(dynamic_cast<state_sequent&>(exp));
  state* a_state=(state*)si.get_state();
  /* loop over all states */
  while (NULL!=a_state) {
    // collect analogin sections in state
    std::list<analogin*> inputs;
    /* loop over all device definitions in a state */
    state::iterator i=a_state->begin();
    while (i!=a_state->end()) {
      analogin* input=dynamic_cast<analogin*>(*i);
      if (input!=NULL) {
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
      if (trigger_count==0) {
	sample_count=inputs.front()->samples;
	sample_frequency=inputs.front()->sample_frequency;
	trigger_count=si.get_count();
      }
      else if (	sample_count==inputs.front()->samples && sample_frequency==inputs.front()->sample_frequency ) {
	trigger_count+=si.get_count();
      }
      else {
	while (!inputs.empty()) {delete inputs.front(); inputs.pop_front();}
	throw ADC_exception("Sorry, but multittriggering requires same parameters in all analogin sections");
      }

      if (a_state->length<1.0*sample_count/sample_frequency) {
	fprintf(stderr,"WARINING: state is shorter than aquisition time\n");
      }
      a_state->push_back(trigger_line.copy_new());

      // to do: more analogin sections per state
      while (!inputs.empty()) {delete inputs.front(); inputs.pop_front();}
    }
    // send a trigger pulse
    // todo only short trigger pulse
    a_state=(state*)si.next_state();
  }

  /* nothing to do! */
  if (trigger_count==0 || sample_count==0) return;


  if (sample_count*trigger_count*4>caps[1])
    throw ADC_exception("required dma buffer size is too big for pci slot config");
  fprintf(stderr,"samples: %d, triggers: %d, frequency=%f\n", sample_count, trigger_count, sample_frequency);

  /* initialise board */
  if (NOERROR!=set_modes(board,
			 0, // clock source: 0=internal
			 sample_frequency, // sample rate
			 sample_count*2, // samples per trigger
			 1, // trigger src: 0=internal, 1=external digital
			 1, // channel (1= Ch1 and Ch2)
			 0, // pretrigger
			 1, // scan (multichannel=1)
			 0  // marker
			 ))
    throw ADC_exception("set_modes failed...");

  // clear fifo, even when using dma
  if (NOERROR!=pci416_clear_fifo(board)) throw ADC_exception("clear_fifo failed");

  DWORD dma_buffer_size[2];
  dma_buffer_size[0]=dma_buffer_size[1]=sample_count*trigger_count*4; //bytes
  dma_buffer_handle=0;
  if (NOERROR!=pci416_setup_dma(board,DMA_SINGLE,dma_buffer_size,&dma_buffer_handle))
    throw ADC_exception("setup_dma failed");

  // start single aquisition
  double freq=1.0;
  // start_daq(board, trigger_mode, frequency) trigger mode: 0 external
  if (NOERROR!=start_daq(board,0,&freq)) throw ADC_exception("start_daq failed");

}

adc_result* DatelPCI416::get_samples(double timeout) {
  // nothing to do here
  if (trigger_count==0 || sample_count==0) return new adc_result(1,0,NULL);

  const size_t poll_time=10000;
  double time_spent=0;
  // busy wait for end
  fprintf(stderr,"waiting for ADC");
  while (1) {
    DWORD dma_status=0;
    // status NOT DONE=0, Done=1
    if (pci416_dma_status(board,&dma_status)!=NOERROR)
      throw ADC_exception("dma_status failed");
    if (dma_status!=0) break;
    usleep(poll_time);
    time_spent+=1e-6*poll_time;
    fprintf(stderr,".",dma_status);
    fflush(stderr);
    if (time_spent>timeout) {
      fprintf(stderr," timeout!\n");
      throw ADC_exception("ran into timeout!");
    }
  }
  fprintf(stderr," done\n");

  if (NOERROR!=stop_daq(board))
    throw ADC_exception("stop daq failed");
  if(NOERROR!=pci416_stop_dma(board,NULL))
    throw ADC_exception("stop_dma failed");

  // copy results...
  DWORD resulting_bytes=sample_count*trigger_count*4; // bytes
  short int* data=(short int*)malloc(resulting_bytes);
  if (data==NULL) throw ADC_exception("Could not allocate enough memory");
  if (NOERROR!=pci416_copy_dmabuffer(board, DMA_SINGLE, 0, &resulting_bytes, (DWORD*)data)) {
    free(data);
    throw ADC_exception("copy_dmabuffer failed");
  }

#if 0
  fprintf(stderr,"got %d bytes back\n",resulting_bytes);
  FILE* binfile=fopen("/tmp/bla.bin","w+");
  if (binfile!=NULL) {
     fwrite(data,1,resulting_bytes, binfile);
     fclose(binfile);
  }
  else 
   fprintf(stderr,"could not open file\n");
#endif
  /* return them */
  return new adc_result(1, sample_count*trigger_count, data, sample_frequency);
}


void DatelPCI416::sample_after_external_trigger(const double rate, const size_t samples, double sensitivity, size_t resolution) {
    const int board=0;

    // try dma
    if (NOERROR!=set_modes(board,
			   0, // clock source: 0=internal
			   rate, // sample rate
			   samples*2, // samples per trigger
			   1, // trigger src: 0=internal, external digital =1
			   1, // channel (1= Ch1 and Ch2)
			   0, // pretrigger
			   1, // scan
			   0  // marker
			   ))
      throw ADC_exception("set_modes failed...");

    // clear fifo, even when using dma
    if (NOERROR!=pci416_clear_fifo(board)) throw ADC_exception("clear_fifo failed");

    DWORD dma_buffer_size[2];
    dma_buffer_size[0]=dma_buffer_size[1]=samples*trigger_count*4; //bytes
    DWORD dma_buffer_handle=0;
    if (NOERROR!=pci416_setup_dma(board,DMA_SINGLE,dma_buffer_size,&dma_buffer_handle))
      throw ADC_exception("setup_dma failed");
    fprintf(stderr,"buffersize %d\n",dma_buffer_size[0]);

    // start single aquisition
    double freq=1.0;
    if (NOERROR!=start_daq(board,0,&freq)) throw ADC_exception("start_daq failed");
}

