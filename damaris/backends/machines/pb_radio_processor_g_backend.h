/* **************************************************************************

 Author: Stefan Reutter
 Created: August 2013

****************************************************************************/
#ifndef DAMARIS_PB_RADIO_PROCESSOR_G_BACKEND_H
#define DAMARIS_PB_RADIO_PROCESSOR_G_BACKEND_H

#include "core/backend_config_reader.h"
#include "machines/hardware.h"
#include "core/core.h"

namespace DAMARIS
{


class pbRadioProcessorGHardware: public hardware {
	
};

class pbRadioProcessorGCore: public core {

public:
    /* -------------------------------------------------------------------- */
    pbRadioProcessorGCore(const core_config& conf):
    	core(conf),
    	the_name("PB RadioProcessorG Core")
    {
        the_hardware=new pbRadioProcessorGHardware();
    }
    
    /* -------------------------------------------------------------------- */
    virtual const std::string& core_name() const {
        return the_name;
    }
private:
    std::string the_name;
};

} //namespace DAMARIS

#endif // include guard
