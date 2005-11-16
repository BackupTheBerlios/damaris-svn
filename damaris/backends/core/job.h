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
#include "machines/hardware.h"


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

#include <xercesc/dom/DOMElement.hpp>

class job {
 public:

  size_t job_no;

  job(const size_t n): job_no(n) {
  }

  size_t no() const {
    return job_no;
  }

  virtual int print() const{
    printf("job no %d\n",job_no);
    return 0;
  }

  virtual result* do_it() {
    throw job_exception("insuficient implementation of a job class");
    return (result*)NULL;
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
  result* do_it(core* c);
};

/**
   just do nothing...
 */
class do_nothing_job: public control {

 public:
  do_nothing_job(const size_t n): control(n) {}

  result* do_it(core* c) {
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

  result* do_it(core* c);

};


/**
   infinite pause until a signal comes
 */
class pause_job: public control {

 public:

  pause_job(const size_t n): control(n) {}
  result* do_it(core* c);
};

/**
   restarts the cores job queue
 */
class restart_job: public control {

 public:

  restart_job(const size_t n): control(n) {}
  result* do_it(core* c);
};


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

/**
   do a simple one-pulse experiment and record transient
   ToDo: by now only few parameters are available, this must change
  */
class single_pulse_experiment: public experiment {
 public:
  /** the pulse length */
  double pulse_length;
  /** time to wait and switch on frequency generator */
  double t_before;
  /** frequency in Hz */
  double frequency;
  /** frequency for ADC recording */
  double sample_frequency;
  /** samples for each channel */
  size_t samples;

  /**
     quick instantiation of single pulse experiment
   */
  single_pulse_experiment(size_t n, double l=0): experiment(n) {
    pulse_length=l;
    t_before=0;
    frequency=1e4;
    sample_frequency=5e6;
    samples=1<<12;
  }

  single_pulse_experiment(size_t n,const XERCES_CPP_NAMESPACE_QUALIFIER DOMNamedNodeMap* attrs);

  /**
     does the single pulse experiment
     \return in case of success ADC_result object is returned, otherwise an error_result object
   */
  result* do_it(hardware* hw) {
    try {
      result* data=hw->single_pulse_experiment(frequency,t_before,pulse_length,sample_frequency,samples);
      data->job_no=job_no;
      return data;
    }
    catch (ADC_exception e) {
      return new error_result(job_no,e);
    }
    catch (pulse_exception& e) {
      return new error_result(job_no,e);
    }
  }


};

//@}

#endif
