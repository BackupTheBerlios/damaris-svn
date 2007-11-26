/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef DATELPCI416
#define DATELPCI416

#include "drivers/ADC.h"
#include "core/states.h"
#include <cstdlib>
#include <windows.h>
#include "416inc/pcilib32.h"
#include "416inc/pci416df.h"

/**
   \defgroup datelpci416 Datel PCI416 adc card
   \ingroup drivers
   \brief supports Datel PCI416 pci cards with dma data transfer

   @{
 */
class DatelPCI416: public ADC {
  HINSTANCE PCI_DLL;
  // make definitions private
# include "416inc/41632DLL.H"
  /** the board this driver works on */
  long unsigned int board;
  /** number of available boards */
  long unsigned int brdcount;


  /**
     \brief the capabilities of the PCI slot used by this card

     the members of this structure are:
     0 sizeFIFO;
     1 bufsizeDMA;
     2 indexADM;
     3 acqmode;
  */

  DWORD caps[4];
  /**
     the sample count per trigger pulse and per channel
   */
  size_t sample_count;
  /**
     the count of expected triggers
   */
  size_t trigger_count;

  /**
     sampling frequency of initialised card
   */
  double sample_frequency;

  /**
     when initialised, this handle gives access to data
   */
  DWORD dma_buffer_handle;
  
  /**
     the ttl signal for pulse programmer
   */
  ttlout trigger_line;

public:
  DatelPCI416(const ttlout& t_line);

  ~DatelPCI416();

  /**
     start sampling after trigger and return field of int
  */
  virtual void sample_after_external_trigger(const double rate, const size_t samples, double sensitivity=5.0, size_t resolution=12);

  /**
     here the data aquisition unit is configured and adds the necessary pusle components to the program

     \exception ADC_exception if data aquisition can not be done by this driver, reason is given
  */
  virtual void set_daq(state& exp);

  /**
     read the sample data
   */
  virtual adc_result* get_samples(double timeout=0.0);

};

/**
  @}
*/

#endif
