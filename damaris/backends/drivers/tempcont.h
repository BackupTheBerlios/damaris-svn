/* **************************************************************************

 Author: Achim Gaedke
 Created: January 2005

****************************************************************************/

#ifndef TEMPCONT_H
#define TEMPCONT_H

#include <vector>
#include <string>
#include <pthread.h>
#include "device.h"

/**
   \addtogroup basedrivers
   @{
*/

/**
   exception for errors of temperature control hardware
*/
class tempcont_error: public device_error {
 public:
  tempcont_error(std::string message): device_error(message) {}
  tempcont_error(const tempcont_error& orig): device_error((const device_error&)orig) {}
};

/**
   history of temperatures
*/
class temp_history: public std::vector<double> {
 public:
  /** state without values */
  temp_history() {latest=0; step=0;}
  /** copy constructor */
  temp_history(const temp_history& orig): std::vector<double>(orig) {latest=orig.latest;step=orig.step;}
  /**
     print xml like data to a file
   */
  void print_xml(FILE* f) const;
  /**
     get back a configuration result object
   */
  configuration_result* as_result() const;
  /**
     time of last element
  */
  time_t latest;
  /**
     time difference between two elemnts
  */
  size_t step;
};


/**
   abstract base class for temperature sensor with temperature history and controler
 */
class tempcont: public virtual device {
 protected:

  /**
     time interval between two history entries
   */
  size_t history_step;
  /**
     time of latest entry
   */
  time_t history_latest_time;
  /**
     index of latest value
  */
  size_t history_latest_index;
  /**
     number of used entries
  */
  size_t history_used;
  /**
     full length of history list
  */
  size_t history_length;
  /**
     ring buffer of temperature history whith hist_length entries
  */
  double* history_buffer;
  /**
     for history maintainance
   */
  pthread_t history_thread;
  /**
     a mutex for device communication
   */
  pthread_mutex_t device_lock;
  /**
     a mutex for history reading and updates
   */
  pthread_mutex_t history_lock;

  /**
     a value different from zero indicates a device failure due to communication errors or broken sensor
     it is set by the history maintainance thread
   */
  int device_failed;

 public:
  /**
     sets up the temperature controler to useful values
     starts the temperature history thread (in deactivated state), the mutexes are setup
     the function set_history_stepsize must be called after the communication is set up
   */
  tempcont();

  /**
     
   */
  virtual int device_failure() const {return device_failed;}
  
  /**
     get a temperature history, that lasts the given number of seconds
     \return an extract of the temperature history
   */
  virtual temp_history* get_history(size_t duration) const;

  /**
     wait for stable temperature
     \return remaining temperature difference
   */
  virtual double wait_setpoint_reached(double delta, size_t timeout) const;

  /**
     get the actual temperature
     function must be reentrant
   */
  virtual double get_temperature() const=0;

  /**
     the history stepsize is changed by this value.
     if the step size is zero, history is disabled
     changing the value clears the history, the first new history entry is gained immediately
   */
  virtual void set_history_stepsize(size_t step);

  /**
     get the length of the time interval between two history entries
     \return the length, 0 if history updating is diabled
   */
  virtual size_t get_history_stepsize() const {return history_step;};

  /**
     set temperature control value
     \return temperature, that is set
  */
  virtual double set_setpoint(double temperature)=0;

  /**
     get temperature control value
     \return temperature, that is set
  */
  virtual double get_setpoint() const=0;

  /**
     reads the temperature in time intervals and maintains the history
     this function is only used by the history thread
   */
  void maintain_history();

  /**
     terminates threads and exit
   */
  virtual ~tempcont();

  /**
     set and read configuration
   */
  virtual configuration_result* configure(const configuration_device_section& conf, int run);

};
/**
   @}
*/
#endif /* TEMPCONT_H */
