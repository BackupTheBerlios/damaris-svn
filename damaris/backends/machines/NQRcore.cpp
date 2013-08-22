/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

 ****************************************************************************/
#include "machines/hardware.h"
#include "core/core.h"
#include "drivers/Datel-PCI416/Datel-PCI416.h"
#include "drivers/SpinCore-PulseBlasterDDSIII/SpinCore-PulseBlasterDDSIII.h"

/**
 \defgroup nqrmachine NQR machine
 \ingroup machines
 @{
 */
class NQR_hardware: public hardware
{

public:
    NQR_hardware()
    {
        ttlout trigger;
        trigger.id = 0;
        trigger.ttls = 1 << 2;
        the_adc = new DatelPCI416(trigger);
        SpinCorePulseBlasterDDSIII* PB = new SpinCorePulseBlasterDDSIII();
        the_pg = PB;
        the_fg = PB; /* the Pulseblaster Card is used for both*/
    }

    ~NQR_hardware()
    {
        if (the_adc != NULL)
            delete the_adc;
        if (the_pg != NULL)
            delete the_pg;
    }

};

class NQR_core: public core
{
    std::string my_name;

public:
    NQR_core(const core_config& conf) :
        core(conf),
        my_name("NQR machine")
    {
        the_hardware = new NQR_hardware();
    }

    virtual const std::string& core_name() const
    {
        return my_name;
    }
};

/**
 @}
 */

int main(int argc, const char** argv)
{
    int return_result = 0;
    try
    {
        core_config my_conf(argv, argc);
        // setup input and output
        NQR_core my_core(my_conf);
        // start core application
        my_core.run();
    }
    catch (const DamarisException& e)
    {
        fprintf(stderr, "%s\n", e.what());
        return_result = 1;
    }
    return return_result;
}
