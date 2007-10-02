#include "SpinCore-PulseBlaster.h"
#include "PulseBlasterProgram.h"
#include "core/core.h"

#ifndef SP_DEBUG
# define SP_DEBUG 0
#endif

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

  const char spincore_dll[]="spinapi";
  const char sp_outp_func_name[]="pb_outp";
  const char sp_inp_func_name[]="pb_inp";
  const char sp_Close_func_name[]="pb_close";
  const char sp_Init_func_name[]="pb_init";

  PBP_DLL = LoadLibrary(spincore_dll);
  if (PBP_DLL==NULL) {
    throw SpinCorePulseBlaster_error(std::string("could not open ")+spincore_dll+" library\n");
  }
  sp_outp = (__attribute__((stdcall))int(*)(unsigned short, int))GetProcAddress(PBP_DLL,sp_outp_func_name);
  if (sp_outp==NULL) {
    FreeLibrary(PBP_DLL);
    PBP_DLL=NULL;
    throw SpinCorePulseBlaster_error("could not access Pulseblaster PCI communication function sp_outp");
  }
  sp_inp = (__attribute__((stdcall))int(*)(unsigned short))GetProcAddress(PBP_DLL, sp_inp_func_name);
  if (sp_inp==NULL) {
    FreeLibrary(PBP_DLL);
    PBP_DLL=NULL;
    throw SpinCorePulseBlaster_error("could not access Pulseblaster PCI communication function sp_inp");
  }
  sp_Init = (__attribute__((stdcall))int(*)())GetProcAddress(PBP_DLL,sp_Init_func_name);
  if (sp_Init==NULL) {
    FreeLibrary(PBP_DLL);
    PBP_DLL=NULL;
    throw SpinCorePulseBlaster_error("could not access Pulseblaster Init function");
  }
  sp_Close = (__attribute__((stdcall))int(*)())GetProcAddress(PBP_DLL, sp_Close_func_name);
  if (sp_Close==NULL) {
    FreeLibrary(PBP_DLL);
    PBP_DLL=NULL;
    throw SpinCorePulseBlaster_error("could not access Pulseblaster Close function");
  }
#if SP_DEBUG
  __attribute__((stdcall))void (*sp_set_debug)(int flag);
  sp_set_debug=(__attribute__((stdcall))void(*)(int))GetProcAddress(PBP_DLL,"pb_set_debug");
  if (sp_set_debug!=NULL) sp_set_debug(1);
  else fprintf(stderr, "could not load debug function from %s DLL\n",spincore_dll);
#endif
  int result=sp_Init();
  if (result!=0) {
    fprintf(stderr, "sp_Init returned %d\n", result);
    FreeLibrary(PBP_DLL);
    PBP_DLL=NULL;
    throw SpinCorePulseBlaster_error("could not initialise Pulseblaster card");
  }
}

SpinCorePulseBlasterLowlevel::~SpinCorePulseBlasterLowlevel() {
  if (sp_Close!=NULL) sp_Close();
  sp_inp=NULL;
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
    throw SpinCorePulseBlaster_error("program data length does not match command length");
  if (data.size()/command_length>max_commands) {
    throw SpinCorePulseBlaster_error("program length exceeds maximum command number");
  }
  write_register(0,0); // dev reset
  write_register(2,command_length); // bytes per word
  write_register(3,0); // dev to program
  write_register(4,0); //reset address counter
  write_data(data);
}

void SpinCorePulseBlaster::run_pulse_program_w_sync(state& exp, double sync_freq) {
  // set duration
  state_sequent* seq=dynamic_cast<state_sequent*>(&exp);
  if (seq==NULL)
    throw pulse_exception("pulse program should be a sequence");
  state_iterator i(*seq);
  while (!i.is_last()) i.next_state();
  duration=i.get_time();
#if SP_DEBUG
  fprintf(stderr, "caluclated time of pulse program is %g\n",duration);
#endif
  PulseBlasterProgram* prog=create_program(exp);
  if (prog==NULL)
    throw pulse_exception("could not create PulseBlasterProgram");
  // some initialisiation...
  // one short "do nothing" state at beginning, because Pulseblaster has some problems with first command
  PulseBlasterCommand* c;
  c=prog->create_command();
  c->ttls=0x0;
  c->instruction=SpinCorePulseBlaster::CONTINUE;
  c->length=9;
  prog->push_front(c);
  if (sync_freq>0 && sync_mask!=0) {
    // syncronization
    c=prog->create_command();
    c->ttls=sync_mask;
    c->instruction=SpinCorePulseBlaster::CONTINUE;
    c->length=9;
    prog->push_front(c);
    c=prog->create_command();
    c->ttls=sync_mask;
    c->instruction=SpinCorePulseBlaster::WAIT;
    c->length=9;
    prog->push_front(c);
    duration+=2.0*shortest_pulse/clock+1.0/sync_freq;
  }
  // end: clear flags and stop pulseblaster
  prog->push_back(prog->create_command());
  prog->push_back(prog->create_command());
  prog->back()->instruction=STOP;
#if SP_DEBUG
  prog->write_to_file(stderr);
#endif
  run_pulse_program(*prog);
  time_running.start();
  duration+=3.0*shortest_pulse/clock;
  delete prog;
}


void SpinCorePulseBlaster::wait_till_end() {

    double waittime=duration-time_running.elapsed();
    double timeout=(waittime>10)?(waittime*0.01):0.1;
#if SP_DEBUG
    fprintf(stderr,"waiting while pulseprogram running (%f)...",waittime);
#endif
    // Bit zero is stopped; bit one is reset; bit two is running; bit three is waiting.
    int status=get_status();
#if SP_DEBUG
    fprintf(stderr,"status: 0x%0x ",status);
#endif
    while (waittime>-timeout && core::term_signal==0 && (status&0x4)) {
      if (waittime<1e-2)
	waittime=1e-2;
      else
	waittime*=0.9;
#if SP_DEBUG
      fprintf(stderr,"sleeping for %g seconds...",waittime);
      fflush(stderr);
#endif
      timespec nanosleep_time;
      nanosleep_time.tv_sec=(time_t)floor(waittime);
      nanosleep_time.tv_nsec=(long)ceil((waittime-nanosleep_time.tv_sec)*1e9);
      nanosleep(&nanosleep_time,NULL);
      waittime=duration-time_running.elapsed();
      status=get_status();
#if SP_DEBUG
      fprintf(stderr,"status: 0x%0x ",status);
      fflush(stderr);
#endif
    }
    if (core::term_signal!=0) {
      //reset pulseblaster
      stop();
      reset_flags(0);
    }
    if (waittime<=-timeout) {
      fprintf(stderr, "ran into timeout, aborting...");
      stop();
      reset_flags(0);
      status=get_status();
      fprintf(stderr,"status: 0x%0x\n",status);
    }
#if SP_DEBUG
    fprintf(stderr,"done\n");
#endif
}
