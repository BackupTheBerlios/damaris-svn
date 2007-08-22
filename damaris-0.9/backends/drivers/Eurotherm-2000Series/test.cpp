/* **************************************************************************

 Author: Holger Stork, Achim Gaedke
 Created: January 2005

****************************************************************************/

#include "Eurotherm-2000Series.h"

int main() {
  try {
    Eurotherm2000Series eurotherm("/dev/ttyS0",2);
  }
  catch (Eurotherm2000Series_error e) {
    fprintf(stderr,"error: %s\n",e.c_str());
  }
  return 0;
}
