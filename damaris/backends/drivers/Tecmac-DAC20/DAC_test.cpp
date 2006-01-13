#include <cstdio>
#include "core/xml_states.h"
#include "drivers/Tecmac-DAC20/DAC20.h"


int main(int argc, char** argv) {
  state_atom* a=xml_state_reader().read_from_file("/dev/stdin");
  if (a==NULL) {
    fprintf(STDERR, "%s: could not read a state tree from stdin\n", argv[0] );
    return 1;
  }
  PFG().set_dac(*a);
  xml_state_writer().write_states(STDOUT,*a,1);
  del a;  
  return 0;
}
