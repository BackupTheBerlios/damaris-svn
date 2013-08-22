/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

 ****************************************************************************/
#include "hardware.h"
#include "core/core.h"
#include "drivers/dummy/dummy.h"

/**
   \defgroup dummymachine Test stub
   \brief tests for generic methods of core and drivers
   \ingroup machines
   should compile under linux and windows and run without any hardware
   @{
 */


/**
   sets the dummy driver to all devices
 */
class dummy_hardware: public hardware {

public:
    dummy_hardware(){
        /* the dummy driver does everything itself */
        dummy* d=new dummy;
        the_adc=d;
        the_pg=d;
        the_fg=d;
        the_tc=d;
        configurable_devices["dummy"]=d;
    }

    ~dummy_hardware() {
        if (the_adc!=NULL) delete the_adc;
    }

};


/**
   a boring core with dummy hardware
 */
class dummycore: public core {
    /** the dummy core name */
    std::string dummycore_name;

public:
    dummycore(const core_config& conf): core(conf) {
        dummycore_name="dummycore";
        the_hardware=new dummy_hardware();
    }

    /** return the name */
    virtual const std::string& core_name() const {
        return dummycore_name;
    }

};


/**
   @}
 */

int main( int argc,const char** argv ) {
    fprintf(stderr,"!!!CAUTION: you are using a test case!!!\n");
    int return_result=0;
    try {
        core_config my_config(argv, argc);
        // setup input and output
        dummycore my_core(my_config);
        // start core application
        my_core.run();
    }
    catch(const DamarisException& e)
    {
        fprintf(stderr,"%s\n",e.what());
        return_result=1;
    }
    return return_result;
}
