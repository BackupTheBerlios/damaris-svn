#include "SpinCore-PulseBlaster.h"
#include "PulseBlasterProgram.h"
#include "core/core.h"

#ifdef __linux__
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>

SpinCorePulseBlasterLowlevel::SpinCorePulseBlasterLowlevel() {
  device_file_descriptor=open("/dev/" PULSEBLASTER_DEVICE_FILE_NAME,O_NONBLOCK|O_RDWR);
  if (device_file_descriptor<0)
    throw SpinCorePulseBlaster_error("could not open the device /dev/" PULSEBLASTER_DEVICE_FILE_NAME "\n");  
}

SpinCorePulseBlasterLowlevel::~SpinCorePulseBlasterLowlevel() {
  close(device_file_descriptor);
}
#endif

#ifdef __CYGWIN__

SpinCorePulseBlasterLowlevel::SpinCorePulseBlasterLowlevel() {
  PBP_DLL = LoadLibrary("PBD03PC");
  if (PBP_DLL==NULL) {
    throw SpinCorePulseBlaster_error("could not open PBD03PC library\n");
  }
  sp_outp = (__attribute__((stdcall))int(*)(unsigned short, int))GetProcAddress(PBP_DLL,"pb_outp");
  if (sp_outp==NULL) {
    FreeLibrary(PBP_DLL);
    PBP_DLL=NULL;
    throw SpinCorePulseBlaster_error("could not access Pulseblaster PCI communication function PBD03PC_outp");
  }
  sp_inp = (__attribute__((stdcall))int(*)(unsigned short))GetProcAddress(PBP_DLL,"pb_inp");
  if (sp_inp==NULL) {
    FreeLibrary(PBP_DLL);
    PBP_DLL=NULL;
    throw SpinCorePulseBlaster_error("could not access Pulseblaster PCI communication function PBD03PC_inp");
  }
  sp_Init = (__attribute__((stdcall))int(*)())GetProcAddress(PBP_DLL,"InitPMster");
  if (sp_Init==NULL) {
    FreeLibrary(PBP_DLL);
    PBP_DLL=NULL;
    throw SpinCorePulseBlaster_error("could not access Pulseblaster Init function");
  }
  sp_Close = (__attribute__((stdcall))int(*)())GetProcAddress(PBP_DLL,"ClosePMster");
  if (sp_Close==NULL) {
    FreeLibrary(PBP_DLL);
    PBP_DLL=NULL;
    throw SpinCorePulseBlaster_error("could not access Pulseblaster Close function");
  }
  int result=sp_Init();
  if (result!=0) {
    FreeLibrary(PBP_DLL);
    PBP_DLL=NULL;
    throw SpinCorePulseBlaster_error("could not initialise Pulseblaster card");
  }
}

SpinCorePulseBlasterLowlevel::~SpinCorePulseBlasterLowlevel() {
  if (sp_Close!=NULL) sp_Close();
  sp_outp=NULL;
  sp_Close=NULL;
  sp_Init=NULL;
  if (PBP_DLL!=NULL) FreeLibrary(PBP_DLL);
}

#endif

void SpinCorePulseBlaster::reset_flags(unsigned int flags) {
  unsigned char data[40];
  data[0]=(flags&0xff000000)>>24;
  data[1]=(flags&0xff0000)>>16;
  data[2]=(flags&0xff00)>>8;
  data[3]=flags&0xff;
  write_register(0,0); // dev reset
  write_register(2,4); // bytes per word
  write_register(3,0xFF); // dev to program
  write_register(4,0); //reset address counter
  write_data(data,4);
  write_register(5,0); //strobe clock
  write_register(5,0); //strobe clock
}

void SpinCorePulseBlaster::set_program(const std::string& data) {
  if (command_length==0)
    throw SpinCorePulseBlaster_error("command length not set");      
  if (data.size()%command_length!=0)
    throw SpinCorePulseBlaster_error("command data length does not match");
  write_register(0,0); // dev reset
  write_register(2,command_length); // bytes per word
  write_register(3,0); // dev to program
  write_register(4,0); //reset address counter
  write_data(data);
}


void SpinCorePulseBlaster::run_pulse_program(state& exp) {
  // set duration
  state_sequent* seq=dynamic_cast<state_sequent*>(&exp);
  if (seq==NULL)
    throw pulse_exception("pulse program should be a sequence");
  state_iterator i(*seq);
  while (!i.is_last()) i.next_state();
  duration=i.get_time();
#if 0
  fprintf(stderr, "caluclated time of pulse program is %g\n",duration);
#endif
  PulseBlasterProgram* c=create_program(exp);
  if (c==NULL)
    throw pulse_exception("could not create PulseBlasterDDSIIIProgram");
  c->push_back(c->create_command());
  c->push_back(c->create_command());
  c->back()->instruction=STOP;
  run_pulse_program(*c);
  time_running.start();
  duration+=2.0*shortest_pulse/clock;
  delete c;
}

void SpinCorePulseBlaster::wait_till_end() {
    /* well.... a very bad implementation */
    double waittime=duration-time_running.elapsed();
#if 0
    fprintf(stderr,"waiting while pulseprogram running...");
    // Bit zero is stopped; bit one is reset; bit two is running; bit three is waiting.
    int status=get_status();
    fprintf(stderr,"read status: %d\n",status);
#endif
    while (waittime>0.0 && core::term_signal==0) {
      if (waittime<1e-3) waittime=1e-3;
#if 0
      fprintf(stderr,"sleeping for %g seconds...",duration);
      fflush(stderr);
#endif
      usleep((unsigned long)ceil(waittime*1e6));
      waittime=duration-time_running.elapsed();
    }
    if (core::term_signal!=0) {
      //reset pulseblaster
      stop();
      reset_flags(0);
    }
#if 0
    fprintf(stderr,"done\n");
#endif
}


