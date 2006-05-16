/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef RESULT_H
#define RESULT_H

#include <xercesc/dom/DOM.hpp>
#include <list>
#include <string>

/**
   \defgroup results Results of jobs
   \brief results are answers to jobs, success, data or errors
   Since now only errors, success and adc results are implemented. Also the results are saved in xml.

   ToDo: externalise the xml read/write interface
 */
//@{

/** a result that goes to output */
class result {
 public:
  /** the coresponding job number */
  size_t job_no;

  /** description, cited from job file */
  std::string description;

  /** jobs can only be instantiated with the number */
  result(size_t _no): job_no(_no) {
  }

  size_t no() const {
    return job_no;
  }

  virtual ~result(){}
};


/**
   simple error message
 */
class error_result: public result {
 public:
  /** the error message should be human understandable */
  std::string error_message;

  /**
     \brief save error message inside
     
     the errormessage should give a hint, what happened, not only which function found an error
   */
  error_result(size_t _no, const std::string& s):result(_no) {
    error_message=s;
  }
};


/**
   \brief stores the data of ADC
 */
class adc_result: public result {

 public:
  /**
     the data: signed 16 bit values of channel 1 and 2 alternating
   */
  short int* data;

  /**
     samples per channel
   */
  size_t samples;

  /**
     sampling frequency for each channel
   */
  double sampling_frequency;
  
  /**
     instantiation without data is possible
   */
  adc_result(size_t _no, size_t s=0, short int* d=NULL, double freq=0): result(_no) {
    data=d;
    samples=s;
    sampling_frequency=freq;
  }

  /**
     free the data arrays
   */
  virtual ~adc_result() {
    if (data!=NULL) free(data);
  }
};

/**
   
 */
class configuration_result: public result {
 public:
  XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* tag;
  configuration_result(size_t _no);

  ~configuration_result();
};


class configuration_results: public std::list<configuration_result*>, public result {
 public:
  configuration_results(size_t _no): result(_no) {}
  virtual ~configuration_results() {
    while (!empty()) {
      delete back();
      pop_back();
    }
  }

};


/**
 */
class adc_results: public std::list<adc_result*>, public result {
 public:
  adc_results(size_t _no): result(_no) {}
  virtual ~adc_results() {
    while (!empty()) {
      delete back();
      pop_back();
    }
  }
};

//@}

#endif
