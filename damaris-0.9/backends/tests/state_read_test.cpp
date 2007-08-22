/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include <string>
#include "core/xml_states.h"

int main() {
  std::string filename="states.xml";
  xml_state_reader reader;
  state_atom* states=reader.read_from_file(filename);
  if (states==NULL) {
    printf("wasn't successfull\n");
    return 1;
  }

  state_atom* copy_of_states=states->copy_new();

  delete states;
  delete copy_of_states;
  return 0;
}
