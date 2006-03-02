/* ***************************************************************************

 Author: Achim Gädke
 Created: October 2004

**************************************************************************** */

#include "DAC20.h"
#include <cstdio>
#include "core/xml_states.h"

class PFG_test {

public:
  PFG p;

  PFG_test() {
    p.id=0;
  }

  void test_simple() {
    state_sequent ss;
    state s(1e-3,&ss);
    ss.push_back(s.copy_new());
    s.length=4e-6;
    analogout aout;
    aout.phase=90.225;
    aout.dac_value=3;
    s.push_back(aout.copy_new());
    ss.push_back(s.copy_new());
    delete s.front();
    aout.phase=190;
    aout.frequency=2e6;
	aout.dac_value=16;
    s.front()=aout.copy_new();
    ss.push_back(s.copy_new());

    xml_state_writer().write_states(stdout,ss);    
    p.set_dac(ss);
    xml_state_writer().write_states(stdout,ss);    
  }
  

};

int main() {

  PFG_test().test_simple();
  //PTS_test().freq_digit_translation_test();
  return 0;
}

