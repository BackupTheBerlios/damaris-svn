/* **************************************************************************

 Author: Stefan Reutter
 Created: August 2013

****************************************************************************/

#ifndef DAMARIS_TEST_CORE_H_
#define DAMARIS_TEST_CORE_H_

#include "core/core.h"
#include "machines/hardware.h"

namespace DAMARIS
{
namespace test
{

/**
 * Test whether a core can be successfully set up (constructor test)
 */
bool testCoreSetup();

namespace details
{

/**
 *
 */
class TestHardware: public hardware
{
public:
	virtual ~TestHardware();
};

/**
 * Since core is abstract, we have to derive it.
 */
class TestCore: public core {
public:
	TestCore(const core_config& conf):
    	core(conf),
    	_name("Test Core")
    {
        the_hardware=new hardware();
    }
    virtual const std::string& core_name() const {
        return _name;
    }

    virtual ~TestCore(){
    };

private:
    std::string _name;
};

} // namespace details

} // namespace test
} // namespace DAMARIS

#endif // include guard
