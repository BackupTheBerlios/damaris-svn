#ifndef SPINCORE_PULSEBLASTER_H
#define SPINCORE_PULSEBLASTER_H

#include <cstdio>
#include <cstring>
#include <vector>
#include <cmath>
#include <cerrno>
#include <unistd.h>
#include <core/stopwatch.h>
#include <drivers/pulsegen.h>

#ifndef SP_DEBUG
# define SP_DEBUG 1
#endif

class SpinCorePulseBlaster_error: public pulse_exception
{
public:
    explicit SpinCorePulseBlaster_error(const std::string& msg) throw (): pulse_exception(msg) {}
    explicit SpinCorePulseBlaster_error(const char* msg) throw (): pulse_exception(msg) {}
    virtual ~SpinCorePulseBlaster_error() throw () {}
protected:
    virtual const std::string prefix() const { return "ERROR (SpinCorePulseBlaster_error): "; }
};

#if !( defined(__linux__)||defined(__CYGWIN__))
# error "Sorry, we are not prepared for this target!"
#endif

#ifdef __linux__
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/ioctl.h>
# include <unistd.h>
# include "pulseblaster.h"
#endif

#ifdef __CYGWIN__
# include <windows.h>
#endif

class SpinCorePulseBlasterLowlevel {
protected:
# ifdef __linux__
  /**
     device file handle
   */
  int device_file_descriptor;
# endif

# ifdef __CYGWIN__
  /**
     the loaded dll
   */
  HINSTANCE PBP_DLL;
  /**
     the pulseblaster communication function sp_outp
     \param address address register in board's internal memory
     \param value value to store
  */
  __attribute__((stdcall)) int (*sp_outp)(unsigned short address, int value);
  /**
     the pulseblaster communication function sp_inp
     \param address addres register in board's internal memory
  */
  __attribute__((stdcall)) int (*sp_inp)(unsigned short address);
  /**
     initialisation of the Pulseblaster board
   */
  __attribute__((stdcall)) int (*sp_Init)();
  /**
     close access to Pulseblaster board
   */
  __attribute__((stdcall)) int (*sp_Close)();
#endif

  SpinCorePulseBlasterLowlevel();

#ifdef __linux__
  inline int write_register(int reg, int data) {
    int result=ioctl(device_file_descriptor,IOCTL_OUTB,(reg&0xff)<<8|(data&0xff));
    if (result==-1) throw SpinCorePulseBlaster_error(std::string("write_register: ioctl error \"")+strerror(errno)+"\"");
    if (result!=0) {
      char errorno[256];
      snprintf(errorno, 256, "%d",result);
      throw SpinCorePulseBlaster_error(std::string("write_register: ioctl returned nonzero value = ")+errorno);
    }
    return result;
  }

  /**
     writes data to pulseblaster device

     care about buffer length and write in chunks to avoid freeze of user space
   */
  int write_data(const unsigned char* data, size_t size);

  inline int read_register(int reg) {
    int result=ioctl(device_file_descriptor,IOCTL_INB, reg&0xff);
    if (result==-1) throw SpinCorePulseBlaster_error(std::string("write_register: ioctl error \"")+strerror(errno)+"\"");
    if (result<0) throw SpinCorePulseBlaster_error("read_register: ioctl returned nonzero value");
    return result;
  }

#endif

#ifdef __CYGWIN__
  inline int write_register(unsigned int reg, unsigned int data) {
    int result=sp_outp(reg,data);
#if SP_DEBUG
    fprintf(stderr, "sp_outp(0x%02x,0x%02x)\n",reg,data);
#endif
    if (result!=0) throw SpinCorePulseBlaster_error("write_register returned nonzero value");
    return result;
  }

  inline int write_data(const unsigned char* data, size_t size) {
    int result=0;
    size_t i;
    for (i=0; i<size && result==0; ++i) {
      result=write_register(6,data[i]);
    }
    if (result!=0) throw SpinCorePulseBlaster_error("write_data: error while writing");
    return i;
  }

  inline int read_register(int reg) {
      int result=sp_inp(reg);
      if (result<0) throw SpinCorePulseBlaster_error("read_register: error while reading");
      return result;
  }

#endif
  inline int write_data(const std::string& data) {
    return write_data((const unsigned char*)data.c_str(),data.size());
  }

  ~SpinCorePulseBlasterLowlevel();

};


class PulseBlasterProgram;

class SpinCorePulseBlaster: protected SpinCorePulseBlasterLowlevel, public pulsegen {

 public:
    /**
       clock in Hz
    */
    double clock;

    /**
       shortest pulse in clock cycles
    */
    unsigned int shortest_pulse;

    /**
       maximum number of commands
     */
    unsigned int max_commands;

    /**
       command/flags wordlength
    */
    unsigned int command_length;

    /**
       is started with pulse program
    */
    stopwatch time_running;

    /**
       expected duration in seconds, nonzero if pulseprogram is running
    */
    double duration;
    /**
       id of ttlout sections affecting this device
     */
    int ttl_device_id;

    /**
       bit mask to set while WAIT command for synchronization is executed

       no synchronization when zero.
     */
    unsigned int sync_mask;

public:

  enum opcode {CONTINUE=0, STOP=1, LOOP=2, END_LOOP=3, JSR=4, RTS=5, BRANCH=6, LONG_DELAY=7, WAIT=8};

  /**
   status codes retured by get_status function

   they are combinable like bit masks
   */
  enum statuscode { STOPPED=1<<0,  /// Bit 0 - Stopped
		    RESET=1<<1,    /// Bit 1 - Reset
		    RUNNING=1<<2,  /// Bit 2 - Running
		    WAITING=1<<3,  /// Bit 3 - Waiting
		    SCANNING=1<<4  /// Bit 4 - Scanning (RadioProcessor boards only)
		  };

  SpinCorePulseBlaster(int c_length, double the_clock) {
    clock=the_clock;
    shortest_pulse=9;
    command_length=c_length;
    max_commands=1<<15;
  }

  void reset_flags(unsigned int flags=0);

  inline void set_initialized() {
    write_register(7,0);
  }

  /**
     starts execution of pulseprogram, needs an initialized pulseblaster
   */
  inline void start() {
    write_register(1,0);
  }

  /**
     stops execution of pulseprogram
   */
  inline void stop() {
    write_register(0,0);
  }

  /**
     returns status information of pulseblaster board
     \return status 1: stopped, 2: reset, 4: running, 8: waiting
   */
  inline int get_status() {
    return read_register(0);
  }

  void set_program(const std::string& data);

  /**
     run a experiment given as state sequence
   */
  virtual void run_pulse_program(state& exp) {
    run_pulse_program_w_sync(exp, 0);
  }

  /**
     run a experiment given as state sequence, but optionally synchronize sequence start with ADC sampling clock
   */
  virtual void run_pulse_program_w_sync(state& exp, double sync_freq=0);

  /**
     needed by run_pulse_program method
   */
  virtual PulseBlasterProgram* create_program(state& exp)=0;

  /**
     needed by run_pulse_program method
   */
  virtual void run_pulse_program(const PulseBlasterProgram& p)=0;

  /**
     wait till end of pulseprogram
   */
  virtual void wait_till_end();

};

#endif /* SPINCORE_PULSEBLASTER_H */
