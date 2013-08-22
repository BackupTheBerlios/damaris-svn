/* **************************************************************************

 Author: Achim Gaedke
 Created: October 2006

 ****************************************************************************/
#include "machines/hardware.h"
#include "core/core.h"
#include "drivers/PTS-Synthesizer/PTS.h"
#include "drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h"
#include "drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h"

/**
   \defgroup mobilemachine Mobile NMR Spectrometer
   \ingroup machines
   Uses Spincore Pulseblaster 24 Bit and Spectrum MI4021, but there is no cable driver with phase control

   \par Starting the hardware
   This procedure should assure the correct initialisation of the hardware:
   \li Switch off main switches of SpinCore Pulseblaster and Computer (the main switch of the computer is at the rear)
   \li Switch on Computer and start Windows or linux

   @{
 */



/**
   line 0 for gate
   line 1 for pulse
   line 2 for trigger
   line 3 free
 */
class bg_hardware: public hardware {

public:
    bg_hardware(){
        ttlout trigger;
        trigger.id=0;
        trigger.ttls=4; /* line 2 */
        int refclock=50e6; /* 50 MHz for Pulseblaster SP-17; 100 MHz for SP-2 */
        double impedance=1e6; /* 1MOhm or 50 Ohm impedance */
        the_adc=new SpectrumMI40xxSeries(trigger, impedance, refclock);
        the_pg=new SpinCorePulseBlaster24Bit();
        PTS* my_pts=new PTS(0);

        ttlout t;
        for (int i=23; i>15; --i) {
            t.ttls=std::bitset<32>(1<<i);
            my_pts->ttl_masks.push_back(t);
        }
        my_pts->negative_logic=0;
        the_fg=my_pts;
    }

    virtual ~bg_hardware() {
        if (the_adc!=NULL) delete the_adc;
        if (the_fg!=NULL) delete the_fg;
        if (the_pg!=NULL) delete the_pg;
    }

};

/**
   \brief brings standard core together with the Mobile NMR hardware
 */
class bg_core: public core {
    std::string the_name;
public:
    bg_core(const core_config& conf): core(conf) {
        the_hardware=new bg_hardware();
        the_name="Burkhard's core";
    }
    virtual const std::string& core_name() const {
        return the_name;
    }
};

/**
   @}
 */

int main(int argc, const char** argv) {
    int return_result=0;
    try {
        core_config my_conf(argv, argc);
        // setup input and output
        bg_core my_core(my_conf);
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
