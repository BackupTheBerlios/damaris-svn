#include <list>
#include "core/states.h"
#include "core/stopwatch.h"
#include "drivers/pulsegen.h"

class PulseBlasterCommand;

/**
   \brief holds the complete and detailed initialisation of the device
*/
class PulseBlasterProgram: public std::list<PulseBlasterCommand*> {
public:
  double internal_clock_freq;
  size_t minimum_interval;
  
  /**
     supports recursive traverse of states, in order insert workarounds and assemble PulseBlasterProgram
   */
  struct stackentry {
    state_sequent* subsequence;
    state_sequent::iterator subsequence_position;
    PulseBlasterProgram::iterator command;
  };


  PulseBlasterProgram(size_t mi=9, double cf=1e8) {
      internal_clock_freq=cf;
      minimum_interval=mi;
  }

  /// do not copy the list. This must be done by the derived class...
  PulseBlasterProgram(const PulseBlasterProgram& orig);

  /// insert some commands and get references right
  int insert_ref(PulseBlasterProgram::iterator target_pos,
		 PulseBlasterProgram::const_iterator start_copy,
		 PulseBlasterProgram::const_iterator end_copy);
  
  /// appends a sequence of states to existing program
  void append_sequence(const state& seq);

  /// write all configuration to a file to xml
  virtual int write_to_file(FILE* out, size_t indent=0) const=0;

  /// create a suitable state for this kind of program
  virtual PulseBlasterCommand* create_command(const state& the_state)=0;

  /// create a copy or a minimal empty state
  virtual PulseBlasterCommand* create_command(const PulseBlasterCommand* orig=NULL)=0;

  virtual ~PulseBlasterProgram();
};

/**
   \brief parameters for each pulseblaster command
*/
class PulseBlasterCommand {
public:
  int ttls;
  /// which instruction is used...
  SpinCorePulseBlaster::opcode instruction;
  /// data of instruction for loop counts
  int loop_count;
  /// length in units of clock cycle
  size_t length;

  /// the pulse program start iterator
  const PulseBlasterProgram* program;

  /// data of instruction for jumps
  PulseBlasterProgram::const_iterator jump;

  PulseBlasterCommand();

  /// one for minimal length
  PulseBlasterCommand(PulseBlasterProgram& p);

  // a useful constructor for a simple CONTINUE state
  PulseBlasterCommand(PulseBlasterProgram& p, int _ttls, double _length);

  /// copy constructor
  PulseBlasterCommand(const PulseBlasterCommand& orig);

  /// write the command to a give file as xml tag
  virtual int write_to_file(FILE* out, size_t indent=0) const=0;

  virtual ~PulseBlasterCommand() {}
};
