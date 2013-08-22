/* **************************************************************************

 Author: Markus Rosenstihl
 Created: June 2010

 ****************************************************************************/
#include "machines/hardware.h"
#include "core/core.h"
#include "drivers/PTS-Synthesizer/PTS.h"
#include "drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h"
#include "drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h"

/**
 \defgroup bertamachine Berta NMR Spectrometer
 \ingroup machines
 Uses:
 \li Spincore Pulseblaster 24 Bit (SP 17) which has a reference clock with 50 MHz
 \li Spectrum MI4021 with gated sampling option (PB ref. clock is fed to Ext.Clock, impedance set to 1 MOhm)
 \li Programmed Test Sources PTS 310 frequency synthesizer with phase control (0.225 degrees step size)


 \par Starting the hardware
 This procedure should assure the correct initialisation of the hardware:
 \li Switch off main switches of SpinCore Pulseblaster and Computer (the main switch of the computer is at the rear)
 \li Switch on Computer and start Windows or linux

 @{
 */

class Berta_hardware: public hardware
{
    PTS* my_pts;
    SpinCorePulseBlaster24Bit* my_pulseblaster;
    SpectrumMI40xxSeries* my_adc;

public:
    Berta_hardware()
    {
        ttlout trigger;
        trigger.id = 0;
        /* trigger on line 22 */
        trigger.ttls = 1 << 22;
        int ext_reference_clock = (int) 50e6; // 50 MHz from PB24 SP17; defaults to 100MHz (PB24 SP 2)
        double impedance = 50; // Ohm ( or 50 Ohm)
        my_adc = new SpectrumMI40xxSeries(trigger, impedance, ext_reference_clock);
        // device_id=0, clock=100MHz, sync_mask: Bit 23
        my_pulseblaster = new SpinCorePulseBlaster24Bit(0, 1e8, 1 << 23);
        my_pts = new PTS_latched(0);
        // PTS 500 has 0.36 ;  PTS 310 has 0.225 degrees/step
        my_pts->phase_step = 0.225;

        // publish devices
        the_pg = my_pulseblaster;
        the_adc = my_adc;
        the_fg = my_pts;
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

    virtual ~Berta_hardware()
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
 \brief brings standard core together with the Berta NMR hardware
 */
class Berta_core: public core
{
    std::string the_name;
public:
    Berta_core(const core_config& conf) :
        core(conf)
    {
        the_hardware = new Berta_hardware();
        the_name = "berta core";
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
        Berta_core my_core(my_conf);
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
