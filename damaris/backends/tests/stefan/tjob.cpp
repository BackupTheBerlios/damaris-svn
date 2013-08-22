/* **************************************************************************

 Author: Stefan Reutter
 Created: August 2013

****************************************************************************/
#include "tjob.h"
#include<iostream>

namespace DAMARIS
{
namespace test
{
namespace details
{
bool quitJob(core* c)
{
	quit_job testJob(0);
	result* res = testJob.do_it(c);
	if (!res) return res;
	return true;
}
} // namespace internal

bool testJobs()
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

} // namespace test
} // namespace DAMARIS


