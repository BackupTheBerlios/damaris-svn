/* **************************************************************************

 Author: Achim Gaedke
 Created: March 2005

****************************************************************************/
#ifndef SPINCOREPULSEBLASTER24BIT_H
#define SPINCOREPULSEBLASTER24BIT_H

#include "drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.h"
#include "drivers/SpinCore-PulseBlaster/PulseBlasterProgram.h"
#include "core/stopwatch.h"
#include <cstdio>
#include <string>
#include <vector>
#include <list>
#include <cmath>
#include <unistd.h>

/**
   \defgroup SpinCorePulseBlaster24Bit SpinCore PulseBlaster 24Bit
   \ingroup drivers
   \brief classes concerning SpinCore PulseBlaster 24Bit version
   @{
*/

class SpinCorePulseBlaster24Bit;
class PulseBlaster24BitCommand;

/**
   \brief holds the complete and detailed initialisation of the device
 */
class PulseBlaster24BitProgram: public PulseBlasterProgram {
public:
  /// ttl device id
  int ttl_device_id;

  /// standard allocator
  PulseBlaster24BitProgram();

  /// copy constructor has to correct the pointers and references in the command list
  PulseBlaster24BitProgram(const PulseBlaster24BitProgram& orig);

  /// create suitable empty program
  PulseBlaster24BitProgram(const SpinCorePulseBlaster24Bit& pb24);

  /// write all configuration to a file to xml
  int write_to_file(FILE* out, size_t indent=0) const;

  /// create a suitable state for this kind of program
  virtual PulseBlasterCommand* create_command(const state& state);

  /// create a copy or a minimal empty state
  virtual PulseBlasterCommand* create_command(const PulseBlasterCommand* orig=NULL);
};

/**
   \brief parameters for each PulseBlaster command
*/
class PulseBlaster24BitCommand: public PulseBlasterCommand {
public:
  /// the simple constructor
  PulseBlaster24BitCommand();

  /// constructs a minimal length state
  PulseBlaster24BitCommand(PulseBlaster24BitProgram&);

  /// the copy constructor
  PulseBlaster24BitCommand(const PulseBlaster24BitCommand&);

  // a useful constructor for a simple CONTINUE state
  PulseBlaster24BitCommand(PulseBlaster24BitProgram& p, int _ttls, double _length);

  /// write the command to a given file as xml tag
  int write_to_file(FILE* out, size_t indent=0) const;
};

/**
   \brief the driver implementation for PulseBlaster24Bit
   uses the classes PulseBlaster24BitProgram and PulseBlaster24BitCommand
*/
class SpinCorePulseBlaster24Bit: public SpinCorePulseBlaster {
 public:

    /**
       initialises the PulseBlaster24Bit of Spincore
    */
    SpinCorePulseBlaster24Bit(int the_id=0, double the_clock=1e8, unsigned int _sync_mask=0):  SpinCorePulseBlaster(10,the_clock) {
    ttl_device_id=the_id;
    sync_mask=_sync_mask;
    fprintf(stderr, "SpinCore PulseBlaster 24Bit: clock=%f Hz, device id=%d", the_clock, the_id);
    if (sync_mask!=0)
      fprintf(stderr, ", sync_mask=0x%06x", sync_mask);
    fprintf(stderr, "\n");
  }

    SpinCorePulseBlaster24Bit(const SpinCorePulseBlaster24Bit& orig);
    /**
     needed by run_pulse_program method
   */
  virtual PulseBlasterProgram* create_program(state& exp);

  /**
     first test routine for spectrometer
   */
  virtual void single_pulse_program(double before, double length, double after);

  /**
     run a PulseBlaster Program
  */
  virtual void run_pulse_program(const PulseBlasterProgram& p);

  /// write command as a 10 byte machine word
  virtual void write_command(unsigned char* data, const PulseBlasterCommand& command);

  /// write raw parameters as a 10 byte machine word
  virtual void write_command(unsigned char* data, int flags, opcode inst, unsigned int inst_data, size_t delay);

  /// append raw parameters as a 10 byte machine word
  virtual void append_command(std::string& data, int flags, opcode inst, int inst_data, size_t delay) {
      unsigned char buffer[10];
      write_command(buffer,flags,inst,inst_data,delay);
      data.append((char*)buffer,(size_t)10);
  }
  
  /// append command as a 10 byte machine word
  virtual void append_command(std::string& data, const PulseBlasterCommand& command) {
      unsigned char buffer[10];
      write_command(buffer,command);
      data.append((char*)buffer,(size_t)10);
  }

  virtual void write_to_device(const PulseBlaster24BitProgram& p);

  /**
     destructor (close device)
  */
  virtual ~SpinCorePulseBlaster24Bit() {}
};

/**
   @}
*/
#endif
