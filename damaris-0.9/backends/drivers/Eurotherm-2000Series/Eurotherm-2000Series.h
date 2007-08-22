/* **************************************************************************

 Author: Holger Stork, Achim Gaedke
 Created: January 2005

****************************************************************************/

#ifndef EUROTHERM2000SERIES_H
#define EUROTHERM2000SERIES_H

#include <string>
#include <map>
#include "drivers/tempcont.h"

/**
   \defgroup eurotherm2000series Eurotherm 2000 Series
   \ingroup drivers
   \brief remote control over serial interface

   Wiring the serial interface is explained in Series 2000 Communications Handbook, Chapter 2.7.
   This handbook is online available at http://www.eurotherm.com/comms/instcom.htm
   The implemented EI-Bisync Protocol is described in caphter 4, the used parameter names are taken from Chapter 5

   @{
 */

/**
   special exception for Eurotherm 2000 drivers
 */
class Eurotherm2000Series_error: public tempcont_error {
public:
  Eurotherm2000Series_error(std::string message): tempcont_error(message) {};
};

/**
   this class communicates with Eurotherm 2200 Series temperature controlers
   the contolers must be set to a known address, the serial line uses 19200baud, 7bit characters and even parity
   there are various timeouts, that are hardcoded to detect transmition failures
 */

class Eurotherm2000Series: public tempcont {
 private:
  /**
     file descriptor for serial device with Eurotherm controler
  */
  int serial_dev;
  /**
     device address, which must be set in Eurotherm device
  */
  int address;
  /**
     serial device name
   */
  std::string device_name;
  /**
     mask defining severe failures (abort of program could be triggered)
   */
  int failure_mask;
  /**
     all these strange data formats of Bisynch: float values
  */
  std::string fp_format;
  /**
     all these strange data formats of Bisynch: integer values with dot!
  */
  std::string int_format;
  /**
     all these strange data formats of Bisynch: hexadecimal values
  */
  std::string hex_format;

  /**
     reads a value from Eurotherm
  */
  void read_value(const std::string& param_name, std::string& return_value) const;

  /**
     sends a value to Eurotherm
  */
  int set_value(const std::string& param_name, const std::string& value);

  /**
     send a list of configuration commands to eurotherm, make sure, that nobody uses eurotherm when calling this function
     the configuration time is quite long (5 to 10 seconds) because of full device reset
   */
  void configure(const std::map<std::string,std::string>& config);

 public:
  /**
     \brief initialise serial interface, test device communication, configure eurotherm
     \param dev_name absolute path name of serial device used for communication
     \param dev_address of Eurotherm device (to be set manualy in Eurotherm configuration)
     \param failure_status_mask mask for status sumary to detect failures, see get_summary_status method

     sets up the communication, configures Eurotherm2000 to °K and nnn.n display format, tests the sensor and starts temperature history
   */
  Eurotherm2000Series(const std::string& dev_name=std::string("/dev/ttyS0"), int dev_address=1, int failure_status_mask=32);

  /**
   */
  virtual ~Eurotherm2000Series();

  /**
     get the actual temperature
  */
  virtual double get_temperature() const;

  /**
     set temperature setpoint value
     \return temperature, that is set
  */
  virtual double set_setpoint(double temperature);

  /**
     get temperature setpoint value
     \return temperature, that is set
  */
  virtual double get_setpoint() const;

  /**
     returns summary for eurotherm as hexadecimal value
     
     the bits meanings are:
     0: Alarm 1 State
     1: Alarm 2 State
     2: Alarm 3 State
     3: Alarm 4 State
     
     4: Manual Mode
     5: Sensor Break
     6: Loop Break
     7: Heater Fail
     
     8: Load Fail
     9: Ramp/Program Complete
     10: Process Variable out of Range
     11: SSR Fail

     12: New Alarm
     13: Remote Input Sensor Break
   */
  virtual int get_summary_status() const;

  /**
     get the low and high limits for setpoints
   */
  virtual void get_setpoint_limits(double& min, double& max) const;
};

/**
   @}
*/
#endif /* EUROTHERM2000SERIES_H */
