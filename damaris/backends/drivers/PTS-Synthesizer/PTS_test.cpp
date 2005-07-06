/* ***************************************************************************

 Author: Achim Gädke
 Created: October 2004

**************************************************************************** */

#include "PTS.h"
#include <cstdio>
#include "core/xml_states.h"

class PTS_test {
public:
  PTS p;

  PTS_test() {
    p.id=0;
    p.set_frequency(0);
    ttlout t;
    t.ttls=std::bitset<32>(1<<10);
    p.ttl_masks.push_back(t);
    t.ttls=std::bitset<32>(1<<9);
    p.ttl_masks.push_back(t);
    if (0) {
      t.ttls=std::bitset<32>(1<<8);
      p.ttl_masks.push_back(t);
      t.ttls=std::bitset<32>(1<<7);
      p.ttl_masks.push_back(t);
    }
  }

  void phase_digit_translation_test() {
    printf("100: %x\n",p.phase_ttl_values(100));
    printf("-260: %x\n",p.phase_ttl_values(-360+100));
    printf("0: %x\n",p.phase_ttl_values(0));
    printf("90: %x\n",p.phase_ttl_values(90));
    printf("180: %x\n",p.phase_ttl_values(180));
    printf("270: %x\n",p.phase_ttl_values(270));
  }


  void phase_add_test() {
    state s(1,NULL);
    p.phase_add_ttls(s,270.5);
    xml_state_writer().write_states(stdout,s);
  }

  void state_modify_test() {
    state s(1);
    analogout aout;
    aout.phase=90;
    aout.frequency=2e7;
    s.push_back(aout.copy_new());
    p.set_frequency(s);
    xml_state_writer().write_states(stdout,s);
  }

  void sequent_iterate_test() {
    state_sequent ss;
    state s(1e-3,&ss);
    ss.push_back(s.copy_new());
    s.length=1;
    analogout aout;
    aout.phase=90;
    aout.frequency=2e7;
    s.push_back(aout.copy_new());
    ss.push_back(s.copy_new());
    delete s.front();
    aout.frequency=2e6;
    s.front()=aout.copy_new();
    ss.push_back(s.copy_new());

    p.set_frequency(ss);
    xml_state_writer().write_states(stdout,ss);
  }

  void freq_digit_translation_test() {
    printf("270e6: %llx\n",p.frequency_ttl_values(270e6));    
  }

};

int main() {

  //PTS_test().sequent_iterate_test();
  PTS_test().freq_digit_translation_test();
  return 0;
}

