/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

 ****************************************************************************/
#include "machines/hardware.h"
#include "core/core.h"
#include "drivers/PTS-Synthesizer/PTS.h"
#include "drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h"
#include "drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h"

/**
 \defgroup mobilemachine Mobile NMR Spectrometer
 \ingroup machines
 Uses Spincore Pulseblaster 24 Bit and Spectrum MI4021, PTS310 with cable driver with phase control and no synchronization board
 \li line 0 for gate
 \li line 1 for pulse
 \li llne 17 for trigger
 \li line 16 free (not using synchronization board)

 \par Starting the hardware
 This procedure should assure the correct initialisation of the hardware:
 \li Switch off main switches of SpinCore Pulseblaster and Computer (the main switch of the computer is at the rear)
 \li Switch on Computer and start Windows or linux

 @{
 */

/**
 */
class Mobile_hardware: public hardware
{

    PTS* my_pts;
    SpinCorePulseBlaster24Bit* my_pulseblaster;
    SpectrumMI40xxSeries* my_adc;

public:
    Mobile_hardware()
    {
        ttlout trigger;
        trigger.id = 0;
        /* trigger on line 17 */
        trigger.ttls = 1 << 17;
        my_adc = new SpectrumMI40xxSeries(trigger);
        /* device_id=0, clock=100MHz, sync_mask: Bit 16 */
        my_pulseblaster = new SpinCorePulseBlaster24Bit(0, 1e8, 0 << 16);
        my_pts = new PTS_latched(0);

        // publish devices
        the_pg = my_pulseblaster;
        the_adc = my_adc;
        the_fg = my_pts;
    }

    virtual ~Mobile_hardware()
    {
        if (the_adc != NULL)
            delete the_adc;
        if (the_fg != NULL)
            delete the_fg;
        if (the_pg != NULL)
            delete the_pg;
    }

};

/**
 \brief brings standard core together with the Mobile NMR hardware
 */
class Mobile_core: public core
{
    std::string the_name;
public:
    Mobile_core(const core_config& conf) :
        core(conf)
    {
        the_hardware = new Mobile_hardware();
        the_name = "Mobile backend without synchronisation card";
    }
    virtual const std::string& core_name() const
    {
        return the_name;
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
        Mobile_core my_core(my_conf);
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
