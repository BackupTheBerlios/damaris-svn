/* ***************************************************************************

 Author: Achim Gädke
 Created: October 2004

**************************************************************************** */

#ifndef PTS_H
#define PTS_H
#include "drivers/frequgen.h"
#include <vector>
#include "core/states.h"


/**
   \defgroup PTSSynthesizer PTS Frequency Synthesizer
   \ingroup drivers
   \brief driver for %PTS Frequency Synthesizer phase and frequency control
   @{
*/

/**
   \brief %PTS Synthesizer driver for phase and preselected frequency

   The driver can switch phase during experiment, the accuracy is determined by the number of connected phase selector lines.
   Frequency support will be soon available, one frequency can be selected, which is configured before the experiment.
   The Synthesizer is driven in transperent mode, i.e. no latch is used. Therefore phase information is added to all states.
   If no phase is defined by an analogout instruction, the default phase is taken.
 */

class PTS: public frequgen {

 public:
  /**
     \brief vector with the ttl output commands, that are added, to achieve the phase shift

     Tuples of 4 masks are for 22.5, 2.25 and 0.225 degree increments, each representing weights of 8, 4, 2 and 1.
     the most significant bit (180 degree) is the first member of the vector, significance is decreasing
   */
  std::vector<ttlout> ttl_masks;

  /**
     \brief the assigned id of this analogout section
   */
  int id;

  /**
     if negative logic should be used, this has to be set for noninverting cable drivers
   */
  int negative_logic;

  /**
     \brief default constructor
   */
  PTS(int myid=0);

  /**
     \brief transform phase to ttl values, return them in unsigned int lower bits: 0 - 11
   */
  unsigned int phase_ttl_values(double phase) const;
  
  /**
     \brief transform frequency to ttl values, return them in long unsigned int bits: 0 - 35
   */
  long long unsigned int frequency_ttl_values(double frequency) const;
  /**
     \brief add ttl instructions according to the phase
   */
  void phase_add_ttls(state& the_state, double phase) const;
 
  /**
     \brief sets the reference frequency for the experiment time
   */
 virtual void set_frequency(double f);

  /**
     \brief exchange analogout sections with phase TTLs and determines the experiments reference frequency
   */
  virtual void set_frequency_ttls(state& experiment);

  /**
     \brief exchange analogout sections in all substates
   */
  virtual void set_frequency(state& experiment);

  /**
     \brief destructor
     nothing to do now... will change with frequency control
   */
  virtual ~PTS();
};

/**
   @}
*/

#endif
