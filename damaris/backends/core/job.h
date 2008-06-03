/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef JOB_H
#define JOB_H

#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <unistd.h>
#include <string>
#include <map>
#include "core/result.h"
#include "core/states.h"

#ifndef SIZETPRINTFLETTER
#  ifndef __LP64__
#    define SIZETPRINTFLETTER "u"
#  else
#    define SIZETPRINTFLETTER "lu"
#  endif
#endif

class core;
/**
   \defgroup jobs Jobs
   \brief jobs to control core and make experiments
   the file format for a job file is simple xml: the element name of the root section is also the name of the job.

   \verbinclude experiment_job.xml

*/
//@{

/**
   \brief exception for job related things
 */
class job_exception: public std::string {
public:
  job_exception(const std::string& s): std::string(s){}
};

/**
   \brief base class for a job that comes from input

   two groups of classe are available:
   - core control jobs
   - experiment jobs
*/

class job {
 public:

  size_t job_no;

  job(const size_t n): job_no(n) {
  }

  size_t no() const {
    return job_no;
  }

  virtual int print() const{
    printf("job no %" SIZETPRINTFLETTER "\n", job_no);
    return 0;
  }


  virtual ~job() {}
};

/**
   base class for core control
 */
class control: public job {
 public:
  control(size_t n):job(n) {}
  virtual result* do_it(core* c)=0;
};

/**
   end main loop
 */
class quit_job: public control {
 public:
  quit_job(const size_t n): control(n) {}
  virtual result* do_it(core* c);
};

/**
   just do nothing...
 */
class do_nothing_job: public control {

 public:
  do_nothing_job(const size_t n): control(n) {}

  virtual result* do_it(core* c) {
    /* of course nothing to do...*/
    return new result(job_no);
  }

};

/**
   wait a specified time period
 */
class wait_job: public control {

 public:
  double sec;

  wait_job(const size_t n): control(n) {
    sec=0;
  }
  wait_job(const size_t n, const XERCES_CPP_NAMESPACE_QUALIFIER DOMNamedNodeMap* attrs);
  
  wait_job(const size_t n, double sec_to_wait): control(n) {
    sec=sec_to_wait;
  }

  virtual result* do_it(core* c);

};


/**
   infinite pause until a signal comes
 */
class pause_job: public control {

 public:

  pause_job(const size_t n): control(n) {}
  virtual result* do_it(core* c);
};

/**
   restarts the cores job queue
 */
class restart_job: public control {

 public:

  restart_job(const size_t n): control(n) {}

  virtual result* do_it(core* c);
};

class hardware;

/**
   base class for experiments
 */
class experiment: public job {
 private:
  state_atom* state_factory(const XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* element);
  char* get_parameter(const XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* element, ...);
 public:
  /// holds the experiments states
  state* experiment_states;
  /// contains teh description of this experiment
  std::string description;

  /// initialise the experiment data
  experiment(size_t n, XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* exp_data=NULL );

  virtual result* do_it(hardware* hw);

  virtual ~experiment() {
    if (experiment_states!=NULL) delete experiment_states;
  }
};


class configuration_device_section {
 public:
  std::string name;
  std::map<std::string,std::string> attributes;
  std::string data;
  void print(FILE* f=stdout) const;
};

/**
   a configuration job changes the instrument to another state, something like:
   * temperature,
   * sample position (lateral, axial),
   * shim, tuning
   these configuration changes generally do not occurr during a pulse sequence and are not very frequent.
 */
class configuration: public job {
 public:
  /*
     suitable data model for each device
  */

  std::list<configuration_device_section> configuration_changes;

 public:
  configuration(size_t n, XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* conf_data=NULL );
  void print(FILE* f=stdout) const;

  result* do_it(hardware* hw);
  
};


//@}

#endif
