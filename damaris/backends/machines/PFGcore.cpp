/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

 ****************************************************************************/
#include "machines/hardware.h"
#include "core/core.h"
#include "drivers/Tecmag-DAC20/DAC20.h"
#include "drivers/PTS-Synthesizer/PTS.h"
#include "drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h"
#include "drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h"

/**
 \defgroup pfgmachine PFG NMR Spectrometer
 \ingroup machines
 Uses Spincore Pulseblaster 24 Bit, Spectrum MI4021, one DAC20 and a synchronization board.
 \li line 0 for gate
 \li line 1 for pulse
 \li line 22 for trigger
 \li line 23 for synchronization

 \par Starting the hardware
 This procedure should assure the correct initialisation of the hardware:
 \li Switch off main switches of SpinCore Pulseblaster and Computer (the main switch of the computer is at the rear)
 \li Switch on Computer and start Windows or linux

 @{
 */

class PFG_hardware: public hardware
{

    SpinCorePulseBlaster24Bit* my_pulseblaster;
    SpectrumMI40xxSeries* my_adc;

public:
    PFG_hardware()
    {
        ttlout trigger;
        trigger.id = 0;
        /* trigger on line 22 */
        trigger.ttls = 1 << 22;
        my_adc = new SpectrumMI40xxSeries(trigger);
        /* device_id = 0, clock = 100 MHz, sync_mask = Bit 23 */
        my_pulseblaster = new SpinCorePulseBlaster24Bit(0, 1e8, 1 << 23);
        /* PTS has analog id = 0 */
        PTS* my_pts = new PTS_latched(0);
        the_fg = my_pts;
        the_pg = my_pulseblaster;
        the_adc = my_adc;
        /* DAC has analog id = 1 */
        DAC20* my_pfg = new DAC20(1);
        list_dacs.push_back(my_pfg);
    }

    result* experiment(const state& exp)
    {
        result* r = NULL;
        for (size_t tries = 0; r == NULL && core::term_signal == 0 && tries < 102; ++tries)
        {
            state* work_copy = exp.copy_flat();
            if (work_copy == NULL)
                return new error_result(1, "could create work copy of experiment sequence");
            try
            {
                if (the_fg != NULL)
                    the_fg->set_frequency(*work_copy);
                if (the_adc != NULL)
                    the_adc->set_daq(*work_copy);
                experiment_prepare_dacs(work_copy);
                // the pulse generator is necessary
                my_pulseblaster->run_pulse_program_w_sync(*work_copy, my_adc->get_sample_clock_frequency());
                // wait for pulse generator
                the_pg->wait_till_end();
                // after that, the result must be available
                if (the_adc != NULL)
                    r = the_adc->get_samples();
                else
                    r = new adc_result(1, 0, NULL);
            }
            catch (const RecoverableException &e)
            {
                r = new error_result(1, e.what());
            }
            delete work_copy;
            if (core::quit_signal != 0)
                break;
        }
        return r;
    }

    virtual ~PFG_hardware()
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
class PFG_core: public core
{
    std::string the_name;
public:
    PFG_core(const core_config& conf) :
        core(conf)
    {
        the_hardware = new PFG_hardware();
        the_name = "PFG core";
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
        PFG_core my_core(my_conf);
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
