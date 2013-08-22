/* **************************************************************************

 Author: Stefan Reutter
 Created: August 2013

****************************************************************************/

#include "tcore.h"
#include <iostream>

namespace DAMARIS
{
namespace test
{

bool testCoreSetup()
{
	details::TestCore* c = NULL;
	bool success = false;
	try
	{
		// initialize test core
		core_config testConf("./resources/spool0");
		c = new details::TestCore(testConf);
		success = true;
	}
	catch (const DamarisException& e)
	{
		if (c) delete c;
		std::cout << "<<<" << e.what() << ">>> ";
	}
	if (c) delete c;
	return success;
}



details::TestHardware::~TestHardware()
{
	if (the_adc!=NULL) delete the_adc;
	if (the_fg!=NULL) delete the_fg;
	if (the_pg!=NULL) delete the_pg;
	if (the_tc!=NULL) delete the_tc;
}


} // namespace test
} // namespace DAMARIS
