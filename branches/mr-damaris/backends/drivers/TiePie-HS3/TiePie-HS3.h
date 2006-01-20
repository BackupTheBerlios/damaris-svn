/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef TIEPIEHS3_H
#define TIEPIEHS3_H

#include "drivers/ADC.h"

/**
   \defgroup tiepiehs3 TiePie Handyscope3
   \ingroup drivers
   driver for TiePie HS3 oscilloscope card
   @{
 */
class TiePieHS3: public ADC {
  /** id of ttl device */
  int trigger_line_id;
  /** mask for ttl state, that triggers the adc */
  unsigned long trigger_line_mask;
  /**
     keep track of required samples per channel
   */
  size_t samples;
  /**
     keep track of required resolution
   */
  size_t resolution;
  /**
     keep track of required sensitivity
   */
  double sensitivity;
  /**
     keep track of required sampling rate
   */
  double rate;
  /**
     keep track of requested channels
   */
  unsigned long channels;

 public:
  TiePieHS3();
  /**
     simple program
   */
  virtual void sample_after_external_trigger(const double rate, const size_t samples, double sensitivity=5.0, size_t resolution=12);

  /**
     find the analogin sections, count them and select trigger mode
   */
  virtual void set_daq(state&);

  /**
     get samples after a while of waiting
   */
  virtual result* get_samples(double timeout=0.0);

  /**
     free all resources
   */
  virtual ~TiePieHS3();
};
/**
   @}
 */

#endif
