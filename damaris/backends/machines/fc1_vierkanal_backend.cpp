/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

 ****************************************************************************/
#include "machines/hardware.h"
#include "core/core.h"
#include "drivers/Tecmag-DAC20/DAC20.h"
#include "drivers/PTS-Synthesizer/PTS.h"
#include "drivers/Spectrum-M2i40xxSeries/Spectrum-M2i40xxSeries.h"
#include "drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h"

/**
 \defgroup fc1neu_machine FC1neu NMR Spectrometer
 \ingroup machines
 Uses Spincore Pulseblaster 24 Bit and Spectrum M2i4021, and some DACs

 \par Starting the hardware
 This procedure should assure the correct initialisation of the hardware:
 \li Switch off main switches of SpinCore Pulseblaster and Computer (the main switch of the computer is at the rear)
 \li Switch on Computer and start Windows or linux

 @{
 */

/**
 line 0 for gate
 line 1 for pulse
 line 22 for trigger
 line 3 free
 */
class FC1_hardware: public hardware
{
    SpinCorePulseBlaster24Bit* my_pulseblaster;
    SpectrumM2i40xxSeries* my_adc;

public:
    FC1_hardware()
    {
        ttlout trigger;
        trigger.id = 0;
        trigger.ttls = 0x400000; /* line 22 */ //
        my_adc = new SpectrumM2i40xxSeries(trigger, 1e8);
        my_pulseblaster = new SpinCorePulseBlaster24Bit(0, 1e8, 0);
        PTS* my_pts = new PTS_latched(0); // ID of PTS_analogout 0
        the_fg = my_pts;
        the_pg = my_pulseblaster;
        the_adc = my_adc;

        DAC20* dac;

        dac = new DAC20(1);
        list_dacs.push_back(dac);

        dac = new DAC20(2);
        dac->set_latch_bit(19);
        list_dacs.push_back(dac);

        dac = new DAC20(3);
        dac->set_latch_bit(20);
        list_dacs.push_back(dac);

        dac = new DAC20(4);
        dac->set_latch_bit(21);
        list_dacs.push_back(dac);
    }

    /* virtual */ //void experiment_run_pulse_program(state* work_copy) {
    //my_pulseblaster->run_pulse_program_w_sync(*work_copy, my_adc->get_sample_clock_frequency());
    //}

    virtual ~FC1_hardware()
    {
        if (the_adc != NULL)
            delete the_adc;
        if (the_pg != NULL)
            delete the_pg;
        if (the_fg != NULL)
            delete the_fg;
    }
};

/**
 \brief brings standard core together with the Mobile NMR hardware
 */
class FC1_core: public core
{
    std::string the_name;
public:
    FC1_core(const core_config& conf) :
        core(conf),
        the_name("FC1 core")
    {
        the_hardware = new FC1_hardware();
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
        FC1_core my_core(my_conf);
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
