/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include "SpinCore-PulseBlasterDDSIII.h"
#include "core/xml_states.h"
#include <cstdlib>
#include <cstdio>
#include <cmath>
/*
 run it once
*/

#if 0
int main(int argc, char** argv) {

	if (argc<4) {
	  fprintf(stderr,"%s frequency_MHz pulselength_microsec pulselength_180deg_microsec [startphase_degree]\n",argv[0]);
	  return 1;
	}

	double freq_MHz=atof(argv[1]);
	double pulselength_us=atof(argv[2]);
	double pulselength_180deg_microsec=atof(argv[3]);
	double phase_deg=0;
	if (argc>4)
		phase_deg=atof(argv[4]);
	if (pulselength_us>100) {
	    fprintf(stderr,"pulselength resticted to 100 microseconds\n");
		return 1;
	}

	SpinCorePulseBlasterPlus pb;
	pb.set_frequency(freq_MHz);
	pb.single_pulse_and_180deg_program((50.0+M_PI*phase_deg/180.0)/(1.0e6*freq_MHz)-1.0e-5,
										pulselength_us*1e-6,
										pulselength_180deg_microsec*1e-6,
										1e-3);
	return 0;
}
#endif

#if 0
int main () {

  PulseBlasterPlusProgram p;
  PulseBlasterPlusCommand loop(p,1e6,90,0,0,0,0x0,1e-6);
  loop.instruction=2;
  loop.loop_count=2;
  p.push_back(loop);
  p.push_back(PulseBlasterPlusCommand(p,1e6,90,0,0,0,255,1e-6));
  PulseBlasterPlusCommand c_loop_end(p,1e6,90,0,0,0,0x0,1e-6);
  c_loop_end.instruction=3;
  c_loop_end.jump=p.begin();
  p.push_back(c_loop_end);

  PulseBlasterPlusCommand c_stop(p,1e6,90,1,0,1,0x0,1e-6);
  c_stop.instruction=1;
  p.push_back(c_stop);

  p.write_to_file(stdout);
  try {
    SpinCorePulseBlasterPlus().run_program(p);
  }
  catch(pulse_exception e) {
    fprintf(stderr,e.c_str());
  }
  return 0;
}
#endif

#if 0
int main() {

  state s;
  s.length=1e-3;
  ttlout* t=new ttlout;
  t->ttls=2;
  s.push_back(t);
  t=new ttlout;
  t->ttls=5;
  s.push_back(t);
  analogout* a=new analogout;
  a->id=0;
  a->frequency=1e6;
  a->phase=90;
  s.push_back(a);
  a=new analogout;
  a->id=1;
  a->frequency=1e6;
  a->phase=0;
  s.push_back(a);

  try {
    PulseBlasterPlusProgram* p=SpinCorePulseBlasterPlus().create_program(s);
	if (p==NULL) {
    	fprintf(stderr,"got a NULL pointer from SpinCorePulseBlasterPlus::create_program");
		return 1;
  	}
  	p->write_to_file(stdout);
  	delete p;
  }
  catch (pulse_exception e) {
    fprintf(stderr,"%s\n",e.c_str());
    return 1;
  }
  return 0;
}
#endif


#if 1

int main(int argc, char** argv) {
  if (argc!=2) {
  	fprintf(stderr,"%s filename",argv[0]);
  	return 1;
  }
  std::string filename(argv[1]);
  xml_state_reader reader;
  state_atom* state_file=reader.read_from_file(filename);
  if (state_file==NULL) {
    fprintf(stderr,"reading was not successfull\n");
    return 1;
  }
  state* s=dynamic_cast<state*>(state_file);
  if (s==NULL) {
    fprintf(stderr,"File does not contain a state or state sequence!");
    delete state_file;
    return 1;
  }

  try {
    SpinCorePulseBlasterDDSIII pb;
    PulseBlasterProgram* p=pb.create_program(*s);
    if (p==NULL) {
      fprintf(stderr,"got a NULL pointer from SpinCorePulseBlasterPlus::create_program");
      return 1;
    }
    delete p;
    //p->write_to_file(stdout);
    for (int i=0; i<1;++i) {
      pb.SpinCorePulseBlaster::run_pulse_program(*s);
      pb.wait_till_end();
    }
  }
  catch (pulse_exception e) {
    delete s;
    fprintf(stderr,"%s\n",e.c_str());
    return 1;
  }
  catch (SpinCorePulseBlaster_error e) {
    delete s;
    fprintf(stderr,"%s\n",e.c_str());
    return 1;
  }
  delete s;
  return 0;
}
#endif
