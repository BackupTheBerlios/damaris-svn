/* **************************************************************************

 Author: Stefan Reutter
 Created: August 2013

****************************************************************************/
#include "pb_radio_processor_g_backend.h"
#include <iostream>

namespace DAMARIS
{
}

int main(int argc, char *argv[]) {
	
	DAMARIS::BackendConfigReader cfgReader("/damaris/backend.conf");
	
	std::cout << cfgReader.getInteger("ADC", "trigger_line") << "\n";

	return 0;
}
