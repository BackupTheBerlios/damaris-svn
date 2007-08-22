/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include "core/states.h"
#include "core/xml_states.h"
#include <cstdio>

int main() {
  std::string filename="experiment_job.xml";
  xml_state_reader reader;
  state_atom* states=reader.read_from_file(filename);
  if (states==NULL) {
    printf("wasn't successfull\n");
    return 1;
  }
  state* pulse_program=dynamic_cast<state*>(states);
  if (pulse_program!=NULL) {
    state* flat_one=pulse_program->copy_flat();
    xml_state_writer().write_states(stdout,*flat_one,1);
  }
  else {
    printf("this was no pulse program!\n");
  }
  delete states;
  return 0;
}
