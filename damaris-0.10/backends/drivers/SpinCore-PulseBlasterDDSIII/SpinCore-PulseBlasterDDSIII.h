/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef SPINCOREPULSEBLASTERDDSIII_H
#define SPINCOREPULSEBLASTERDDSIII_H

#include "drivers/frequgen.h"
#include "drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.h"
#include "drivers/SpinCore-PulseBlaster/PulseBlasterProgram.h"
#include "core/core.h"
#include "core/states.h"
#include "core/stopwatch.h"
#include <cstdio>
#include <string>
#include <vector>
#include <list>
#include <cmath>
#include <unistd.h>

/**
   \defgroup SpinCorePulseblasterDDSIII SpinCore PulseblasterDDSIII
   \ingroup drivers
   \brief classes concerning SpinCore PulseBlasterDDSIII
   @{
*/

class PulseBlasterDDSIIIProgram;
class PulseBlasterDDSIIICommand;

class SpinCorePulseBlasterDDSIII: public frequgen, public SpinCorePulseBlaster {

  /**
     number of frequency registers
   */
  unsigned int freq_regno;
  /**
     number of phase registers
   */
  unsigned int phase_regno;

  
  void set_registers(int device, unsigned int register_size, double divider, const std::vector<double>& values);

  void set_phase_registers(std::vector<double> rx_phases, std::vector<double> tx_phases);

  void set_frequency_registers(const std::vector<double>& values);

  /**
     makes a machine command word from PulseBlasterDDSIIICommand object
   */
  void write_command(unsigned char* data, const PulseBlasterCommand& command);

  /**
    send the entire program to the device
  */
  int write_to_device(const PulseBlasterDDSIIIProgram& program);

 public:

  typedef enum {ANALOG_OFF=0, ANALOG_ON=1} analog_state;

  analog_state rx_analog;

  analog_state tx_analog;

  /**
     initialises the PulseblasterDDSIII of Spincore
   */
  SpinCorePulseBlasterDDSIII(int the_id=0, double the_clock=1e8, unsigned int _sync_mask=0);

#if 1
  /**
     a simple pulse program for one pulse experiments
   */
  virtual void single_pulse_program(double before, double length, double after);
#endif
  /**
     sets the frequency for simple frequency generator use
     if set, by default the analog output is set to rx and tx channel with 90 degree shift
   */
  virtual void set_frequency(state& s) {frequency=10e6;}

  /**
     sets the frequency for simple frequency generator use
     if set, by default the analog output is set to rx and tx channel with 90 degree shift
   */
  virtual void set_frequency(double f) {frequency=f;}

  /**
     \brief programm the pulseblaster with the subset of states known to him

     the pulse program is ended with a wait command, that resets all channels and a stop command that stops the pulseblaster in this state
   */
  virtual PulseBlasterProgram* create_program(state& the_states);

  /**
     run a PulseBlaster Program
   */
  virtual void run_pulse_program(const PulseBlasterProgram& p);

  /**
     provide fake substitution for get_status
   */
  inline int get_status() {
    fprintf(stderr,"PulseBlaster DDSIII does not support status readback! (assuming running state)\n");
    return 4;
  }

  /**
     status queries not working, workaround
   */
  void wait_till_end();

  /**
     destructor (close device)
  */
  virtual ~SpinCorePulseBlasterDDSIII() {};
};

/**
   \brief holds the complete and detailed initialisation of the device
 */
class PulseBlasterDDSIIIProgram: public PulseBlasterProgram {
public:
  double freq_accuracy;
  double phase_accuracy;

  /// here keep track of the programed frequencies(Hz)
  std::vector<double> frequency_registers;
  /// here keep track of the programed RX phases (degree)
  std::vector<double> rx_phase_registers;
  /// here keep track of the programed TX phases (degree)
  std::vector<double> tx_phase_registers;

  /// finds or allocates frequency given in Hz
  int get_frequency_regno(double f);
  /// finds or allocates rx phase given in degree
  int get_rx_phase_regno(double p);
  /// finds or allocates tx phase given in degree
  int get_tx_phase_regno(double p);

  /// standard allocator sets only accuracy
  PulseBlasterDDSIIIProgram();

  /// copy constructor has to correct the pointers and references in the command list
  PulseBlasterDDSIIIProgram(const PulseBlasterDDSIIIProgram& orig);

  PulseBlasterDDSIIIProgram(const SpinCorePulseBlasterDDSIII& pb);
  /**
     create a new continue command, that merges all state_atom features
  */
  virtual PulseBlasterCommand* create_command(const state& the_state);
  /**
     create a new continue command by copying or a default one
  */
  virtual PulseBlasterCommand* create_command(const PulseBlasterCommand* c=NULL);

  /// write all configuration to a file to xml
  int write_to_file(FILE* out, size_t indent=0) const;
};

/**
   \brief parameters for each pulseblaster command
*/
class PulseBlasterDDSIIICommand: public PulseBlasterCommand {
public:
  /// register for frequency
  int freq_reg;
  /// register for rx phase
  int rx_phase_reg;
  /// enable rx analog output
  SpinCorePulseBlasterDDSIII::analog_state rx_enable;
  /// register for tx phase
  int tx_phase_reg;
  /// enable tx analog output
  SpinCorePulseBlasterDDSIII::analog_state tx_enable;
  /// full ttl state

  /// the simple constructor
  PulseBlasterDDSIIICommand();

  /// the copy constructor
  PulseBlasterDDSIIICommand(const PulseBlasterDDSIIICommand&);

  /// produces a simple state
  PulseBlasterDDSIIICommand(PulseBlasterDDSIIIProgram& p,
			  double frequency,
			  double rx_phase, SpinCorePulseBlasterDDSIII::analog_state rx_enable,
			  double tx_phase, SpinCorePulseBlasterDDSIII::analog_state tx_enable,
			  int ttls, double length);

  /// write the command to a give file as xml tag
  int write_to_file(FILE* out, size_t indent=0) const;
};

/**
   \brief the driver implementation for PulseBlasterDDSIII!
   uses the classes PulseBlasterDDSIIIProgram and PulseBlasterDDSIIICommand
*/

/**
   @}
*/
#endif
