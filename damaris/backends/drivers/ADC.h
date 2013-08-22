/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

 ****************************************************************************/
#ifndef ADC_H
#define ADC_H

#include <cstdlib>
#include <string>
#include "core/result.h"
#include "core/states.h"
#include "core/core_exception.h"

/**
 \defgroup drivers Drivers
 */

/**
 \defgroup basedrivers Drivers' Base Classes
 \ingroup drivers
 */
//@{
/**
 * ADC exception
 */
class ADC_exception: public RecoverableException
{
public:
    explicit ADC_exception(const std::string& msg) throw (): RecoverableException(msg) {}
    explicit ADC_exception(const char* msg) throw (): RecoverableException(msg) {}
    virtual ~ADC_exception() throw () {}
protected:
    virtual const std::string prefix() const { return "ERROR (ADC_exception): "; }
};
/**
 base class for adc drivers
 */
class ADC
{
public:
    /**
     start sampling after trigger and return field of int
     */
    virtual void sample_after_external_trigger(double rate, size_t samples, double sensitivity = 5.0, size_t resolution = 14)=0;

    /**
     here the data aquisition unit is configured and adds the necessary pusle components to the program
     */
    virtual void set_daq(state& exp)=0;

    /**
     read the sample data
     */
    virtual result* get_samples(double timeout = 0.0)=0;

    virtual ~ADC()
    {
    }
};

//@}
#endif
