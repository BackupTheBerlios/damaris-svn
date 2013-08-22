/* **************************************************************************

 Author: Stefan Reutter
 Created: August 2013

****************************************************************************/

#include<iostream>
#include "tjob.h"
#include "tcore.h"
/**
 * \defgroup Tests
 * Stefan's DAMARIS test suite contains a number of unit tests meant to ensure that the program will
 * run correctly in case parts of the software are changed or added. They also provide a good
 * starting point for learning how the backend works.
 *
 * Each test case is a function that should return true if the test succeeded and false otherwise.
 * This means that the test functions must be strongly exception safe with respect to any DAMARIS
 * exceptions (i.e. the program should not be aborted and the program should be in the same state
 * after running the test as before) and ideally should be strongly exception safe in general.
 *
 * Tests should be designed to be run independently, so they must initialize any necessary data
 * for them to run. To avoid code duplication, use shared test classes or functions.
 *
 * Test functions should reside in the DAMARIS::test namespace, and that is the only thing which
 * should reside there. Implementation details (such as shared classes or non-member functions)
 * should go into the DAMARIS::test::details namespace.
 */
/*@{*/

namespace DAMARIS
{
namespace test
{

/**
 * Function wrapper for test functions that automatically creates formatted output with the result
 * value.
 *
 * \param func A function (pointer) to call
 * \param name A string identifying the test for output
 */
void runTest(bool (*func)(), const std::string& name)
{
	std::cout << "Running " << name << " test: ";
	std::cout << (func() ? "succeeded" : "failed") << "\n";
}

/**
 * Run all tests and print their result.
 */
void go()
{
	runTest(testCoreSetup, "Core Setup");
	runTest(testJobs, "Job");

}

} // namespace test
} // namespace DAMARIS
/*@}*/
int main(int argc, char* argv[])
{
	DAMARIS::test::go();
	return 0;
}




