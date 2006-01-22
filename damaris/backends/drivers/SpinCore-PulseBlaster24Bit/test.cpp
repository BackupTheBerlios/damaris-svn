#include "SpinCore-PulseBlaster24Bit.h"
#include "core/xml_states.h"
#include "cstring"
#include "cerrno"

int lowlevel_test() {
  try {
    SpinCorePulseBlaster24Bit p;
    std::string program;
    p.append_command(program,1,SpinCorePulseBlaster::LOOP,5,97);
    p.append_command(program,3,SpinCorePulseBlaster::END_LOOP,0,97);
    p.append_command(program,4,SpinCorePulseBlaster::CONTINUE,0,97);
    p.append_command(program,0,SpinCorePulseBlaster::CONTINUE,0,100000000-3);
    p.append_command(program,1,SpinCorePulseBlaster::STOP,0,97);
    p.set_program(program);
    p.set_initialized();
    p.start();
  }
  catch(SpinCorePulseBlaster_error e) {
    fprintf(stderr, "%s\n", e.c_str());
  }

  return 0;
}

int simple_sequence() {
  try {
    SpinCorePulseBlaster24Bit sp24;
    PulseBlaster24BitProgram prog(sp24);
    // zero
    prog.push_back(prog.create_command());
    prog.back()->ttls=1;
    prog.back()->instruction=SpinCorePulseBlaster::LOOP;
    prog.back()->loop_count=10;
    prog.back()->length=20;
    // one
    prog.push_back(prog.create_command());
    prog.back()->ttls=0;
    prog.back()->instruction=SpinCorePulseBlaster::END_LOOP;
    prog.back()->jump=prog.begin();
    prog.back()->length=10;
    // two
    prog.push_back(prog.create_command());
    prog.back()->ttls=0;
    prog.back()->instruction=SpinCorePulseBlaster::STOP;
    sp24.run_pulse_program(prog);

    PulseBlaster24BitProgram prog2(prog);
    prog.write_to_file(stdout);
    sp24.run_pulse_program(prog2);
  }
  catch(SpinCorePulseBlaster_error e) {
    fprintf(stderr, "pulseblaster: %s\n", e.c_str());
    return 1;
  }
  catch(pulse_exception e) {
    fprintf(stderr, "pulse: %s\n", e.c_str());
    return 1;
  }
  return 0;
}

class core {
public:
  static int term_signal;
};


int state_sequence(const char* filename) {

  try {
    SpinCorePulseBlaster24Bit sp24;
    PulseBlaster24BitProgram prog;
    state_atom* sa=xml_state_reader().read_from_file(filename);
    state_sequent* ss=dynamic_cast<state_sequent*>(sa);
    if (ss==NULL) {
      delete sa;
      throw SpinCorePulseBlaster_error("found no states");
    }
    prog.push_front(prog.create_command());
    prog.append_sequence(*ss);
    prog.push_back(prog.create_command());
    prog.push_back(prog.create_command());
    prog.back()->instruction=SpinCorePulseBlaster::STOP;
    prog.write_to_file(stdout);
    state_iterator i(*ss);
    while (!i.is_last()) i.next_state();
    sp24.duration=i.get_time()+2*90e-9;
    sp24.run_pulse_program(prog);
    sp24.time_running.start();
    
    sp24.wait_till_end();

    fprintf(stdout, "%g seconds elapsed\n", sp24.time_running.elapsed());
  }
  catch(SpinCorePulseBlaster_error e) {
    fprintf(stderr, "pulseblaster: %s\n", e.c_str());
    return 1;
  }
  catch(pulse_exception e) {
    fprintf(stderr, "pulse: %s\n", e.c_str());
    return 1;
  }
  return 0;
}



int main(int argc, char** argv) {
  core::term_signal=0;
    size_t n=1;
    const char* filename=NULL;

    if (argc<2 || argc>3) {
	fprintf(stderr,"%s [repeat] pulsesequence\n",argv[0]);
	return 1;
    }
    
    if (argc==3) {
	char* endptr=NULL;
	n=strtoul(argv[1],&endptr,10);
	if (*endptr!=0 && n<1) {
	    fprintf(stderr,"%s [repeat] pulsesequence\n",argv[0]);
	    return 1;
	}
	filename=argv[2];
    }


    if (argc==2) {
	filename=argv[1];
    }

    if (0!=access(filename, R_OK)) {
	fprintf(stderr, "%s: could not open file %s, reason: %s\n",argv[0],filename, strerror(errno));
	return 1;
    }


    int value_returned=0;
    size_t i=0;
    while (i<n && value_returned==0) {
	value_returned=state_sequence(filename);
	i++;
    }
    if (n>1) {
	fprintf(stdout,"%s: executed %s %lu times\n",argv[0],filename,i);
    }
    return value_returned;
}
