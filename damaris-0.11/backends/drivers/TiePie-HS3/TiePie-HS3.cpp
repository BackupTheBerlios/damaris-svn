/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include "drivers/TiePie-HS3/TiePie-HS3.h"
#include "tiepie.h"
#include <cmath>
#include <cstdio>
#include "core/xml_states.h"

TiePieHS3::TiePieHS3() {
  if (1!=OpenDLL(dtHandyscope3)){
    throw ADC_exception("could not find driver dll for HS3!");
  }
  if (NO_ERROR!=InitInstrument(0)) {
    ADC_exception("HS3 not available!");
  }
  unsigned int serialno=0;
  unsigned short int retval=GetSerialNumber(&serialno);
  //fprintf(stderr, "sizeof(unsigned int)=%d, sizeof(unsigned short int)=%d\n", sizeof(serialno), sizeof(retval));
  if (NO_ERROR!=retval) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "could not get device's serial number: GetSerialNumber returned %d", retval);
    throw ADC_exception(std::string(buffer));
  }
  fprintf(stderr, "successfully initialized HS3, SN=%d\n",serialno);
  trigger_line_id=0;
  trigger_line_mask=1<<2;
}

void TiePieHS3::sample_after_external_trigger(double _rate, size_t _samples, double _sensitivity ,size_t _resolution) {
  ADC_Abort();
  /* set autoranging off */
  SetAutoRanging(Ch1,0);
  SetAutoRanging(Ch2,0);

  samples=_samples;
  if (samples==0 || _rate<=0) return;
  unsigned short int retval;
  //samples=1024*1024*1024; // impossible value... provoke error
  retval=SetRecordLength(samples);
  if (0!=retval) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "could not set record length: SetRecrodLength(%d) returned %d", samples, retval);
    throw ADC_exception(std::string(buffer));
  }
  if (0!=SetPostSamples(samples)){
    throw ADC_exception("could not set post sample number");
  }
  fprintf(stderr,"set sample number to %d\n",samples);

  /* set sampling frequency */
  unsigned int freq=(unsigned int)fabs(floor(_rate)+0.5);
  unsigned int freq_req=freq;
  SetSampleFrequency(&freq_req);
  if (freq!=freq_req)
    throw ADC_exception("requested frequency could not be set");
  rate=freq_req;
  fprintf(stderr,"set rate to %g\n",rate);

  /* set resolution */
  if (0!=SetResolution(_resolution))
    throw ADC_exception("could not set resolution");
  unsigned char resolution_set;
  GetResolution(&resolution_set);
  if (_resolution!=resolution_set)
    throw ADC_exception("requested resolution not supported");
  resolution=_resolution;
  fprintf(stderr,"set resolution to %d\n",resolution);

#if 0
  /* set DC level value to zero */
  const double dclevel_req=-2.0;
  if (E_NO_ERRORS!=SetDcLevel(1, dclevel_req))
    throw ADC_exception("could not set dc level for channel 1");
  if (E_NO_ERRORS!=SetDcLevel(2, dclevel_req))
    throw ADC_exception("could not set dc level for channel 2");
#endif

  /* set input sensitivity for both channels */
  double sensitivity_req1=_sensitivity;
  SetSensitivity(1,&sensitivity_req1);
  double sensitivity_req2=_sensitivity;
  SetSensitivity(2,&sensitivity_req2);
  if (sensitivity_req1!=_sensitivity || sensitivity_req2!=_sensitivity)
    throw ADC_exception("requested sensitivity could not be set");
  sensitivity=_sensitivity;
  fprintf(stderr,"set sensitivity to %g\n",sensitivity);

  /* set input coupling to DC */
  if (0!=SetCoupling(Ch1, ctDC) || 0!=SetCoupling(Ch2, ctDC))
    throw ADC_exception("could not set coupling to dc");

  /* what to measure */
  if (E_NO_ERRORS!=SetMeasureMode(mmCh12))   /* Channel 1 and 2 */
    throw ADC_exception("could not set measurement mode");
  /* the trigger source */
  if (E_NO_ERRORS!=SetTriggerSource(tsExternal)) /* external trigger */
    throw ADC_exception("could not set trigger source");
  /* which slope to trigger */
  if (E_NO_ERRORS!=SetTriggerMode(tmRising))   /* 0=Rising slope */
    throw ADC_exception("could not set trigger source");
  /* set transfer mode */
  if (E_NO_ERRORS!=SetTransferMode(tmBlock))
    throw ADC_exception("could not set transfer mode");

  /* finally start the measurement */
  if (0!=ADC_Start())
    throw ADC_exception("could not start triggered adc measurement");
   fprintf(stderr,"started triggered adc measurement with %d samples, rate=%g, sensitivity=%g, resolution=%d\n",samples,rate,sensitivity,resolution);
}

void TiePieHS3::set_daq(state& exp) {
  size_t s=0;
  double f=0.0;
  size_t res=0;
  double sens=0.0;
  // todo: exception...
  state_iterator si(dynamic_cast<state_sequent&>(exp));
  state* a_state=(state*)si.get_state();
  while (NULL!=a_state) {
    // find a analogin section
    std::list<analogin*> inputs;
    for(state::iterator i=a_state->begin(); i!=a_state->end(); ++i) {
      analogin* input=dynamic_cast<analogin*>(*i);
      if (input!=NULL) {
	inputs.push_back(input);
	// replace this by a trigger pulse
	// to do only short one
	ttlout* trigger_pulse=new ttlout();
	trigger_pulse->ttls=trigger_line_mask;
	trigger_pulse->id=trigger_line_id;
	*i=trigger_pulse;
      }
    }
    if (!inputs.empty()) {
      if (s!=0 || si.get_count()>1)
	throw ADC_exception("Sorry, but TiePie Card does not support mutittriggering");
      s=inputs.front()->samples;
      f=inputs.front()->sample_frequency;
      res=inputs.front()->resolution;
      sens=inputs.front()->sensitivity;
      // to do only one channel
      while (!inputs.empty()) {delete inputs.front();inputs.pop_front();}
    }
    a_state=(state*)si.next_state();
  }
  //xml_state_writer().write_states(stderr,exp);
  fprintf(stderr,"f=%g s=%d sens=%f res=%d\n",f,s,sens,res);
  sample_after_external_trigger(f, s, sens, res);
}

result* TiePieHS3::get_samples(double timeout) {
  // return new error_result(1,"for some reasons disabled");
  if (samples==0 || rate<=0)
    return new adc_result(1);
  /* allocate necessary buffers */
  unsigned short int* data1=(unsigned short int*)calloc(samples,sizeof(unsigned short int));
  if (data1==NULL) throw ADC_exception("could not allocate memory for real data");
  unsigned short int* data2=(unsigned short int*)calloc(samples,sizeof(unsigned short int));
  if (data2==NULL) {
    free(data1);
    throw ADC_exception("could not allocate memory for imaginary data");
  }

  const double poll_timestep=5.0e-2;
  double poll_time=0.0;
#if 1
  fprintf(stderr,"expecting ADC result...");
#endif
  while (1!=ADC_Ready()) {
    usleep((unsigned int)floor(poll_timestep*1.0e6));
    poll_time+=poll_timestep;
    if (poll_time>=timeout) throw ADC_exception("ran into timeout!");
#if 1
    fprintf(stderr,"waiting...");
    fflush(stderr);
#endif
  }
#if 1
  fprintf(stderr,"done\n");
#endif
  if (ADC_GetData(data1, data2)!=0) {
    free(data1);
    free(data2);
    throw ADC_exception("error while fetching data\n");
  }

  short int* adc_data=(short int*)calloc(2*samples,sizeof(short int));
  if (adc_data==NULL) {
    free(data1);
    free(data2);
    throw ADC_exception("could not allocate memory for adc data");
  }

  for (size_t i=0;i<samples;i++) {
    int result=data1[i];
    // there is one more positive value than in two's complement, we have to map this to the hightest positive value
    if (result==65535) result-=1;
    result-=32767;
    adc_data[i*2]=result;
    result=data2[i];
    if (result==65535)  result-=1;
    result-=32767;
    adc_data[i*2+1]=result;
  }

  free(data1);
  free(data2);
  return new adc_result(1,samples,adc_data,rate);
}

TiePieHS3::~TiePieHS3() {
  ExitInstrument();
}
