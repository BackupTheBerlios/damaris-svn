/* **************************************************************************

 Author: Stefan Reutter
 Created: August 2013

****************************************************************************/

#ifndef DAMARIS_TEST_JOB_H_
#define DAMARIS_TEST_JOB_H_

#include "tcore.h"
#include "core/core_config.h"


/**
 * Run a number of unit tests on job-related objects
 */
namespace DAMARIS
{
namespace test
{
bool testJobs();

namespace details
{
bool quitJob(core* c);
} // namespace details

} // namespace test
} // namespace DAMARIS


#endif // include guard
