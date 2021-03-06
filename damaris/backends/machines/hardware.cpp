/* **************************************************************************

 Author: Achim Gaedke
 Created: August 2004

 ****************************************************************************/

#include "core/core.h"
#include "core/result.h"
#include "hardware.h"

void hardware::experiment_prepare_dacs(state* work_copy)
{
    if (!list_dacs.empty())
    {
        std::list<GenericDAC*>::iterator i;
        for (i = list_dacs.begin(); i != list_dacs.end(); i++)
        {
            (*i)->set_dac(*work_copy);
        }
    }
}

hardware::~hardware()
{
    while (!list_dacs.empty())
    {
        if (list_dacs.back())
            delete list_dacs.back();
        list_dacs.pop_back();
    }
}

void hardware::experiment_run_pulse_program(state* work_copy)
{
    the_pg->run_pulse_program(*work_copy);
}

result* hardware::experiment(const state& exp)
{
    result* r = NULL;
    for (size_t tries = 0; r == NULL && core::term_signal == 0 && tries < 102; ++tries)
    {
        state* work_copy = exp.copy_flat();
        if (work_copy == NULL)
            return new error_result(1, "couldn't create work copy of experiment sequence");
        try
        {
            if (the_fg != NULL)
                the_fg->set_frequency(*work_copy);
            if (the_adc != NULL)
                the_adc->set_daq(*work_copy);
            experiment_prepare_dacs(work_copy);
            // the pulse generator is necessary
            experiment_run_pulse_program(work_copy);
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

configuration_results* hardware::configure(const std::list<configuration_device_section>& d)
{
    configuration_results* r = new configuration_results(0);

    // create a work list
    std::list<const configuration_device_section*> to_configure;
    for (std::list<configuration_device_section>::const_iterator i = d.begin(); i != d.end(); ++i)
    {
        // if the device name is available, we will add a pointer, otherwise a remark about missing device
        if (configurable_devices.count(i->name) != 0)
            to_configure.push_back(&(*i));
        else
        {
            // todo generate a remark
            r->push_back(new configuration_result(0));
        }
    }

    // go through this list again and again, until all devices returned at least something...
    for (int run = 0; !to_configure.empty(); ++run)
    {

        std::list<const configuration_device_section*>::iterator i = to_configure.begin();
        while (i != to_configure.end())
        {
            configuration_result* config_result = NULL;
            std::map<const std::string, device*>::iterator dev_iterator = configurable_devices.find((*i)->name);
            device* dev = dev_iterator->second;
            if (dev != NULL)
            {
                try
                {
                    config_result = dev->configure(**i, run);
                }
                catch (const device_error& e)
                {
                    // error to result...
                    config_result = new configuration_result(0);
                }
            }
            else
            {
                // error to result...
                config_result = new configuration_result(0);
            }
            if (config_result != NULL)
            {
                // do not configure this device again...
                r->push_back(config_result);
                i = to_configure.erase(i);
            }
            else
            {
                // once more
                ++i;
            }
        }

    }

    return r;
}
