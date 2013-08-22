/* ******************************************************************************

author: Achim Gaedke


********************************************************************************/

#ifndef XML_RESULT_H
#define XML_RESULT_H

#include "expat.h"
#include "result.h"
#include <cstdio>
#include <string>

/**
   can read result files
*/
class xml_result_reader {
  std::string jobfilenamepattern;

 public:
  /**
     instantiates parser
   */
  xml_result_reader();
  /**
     instantiates parser
   */
  xml_result_reader(const std::string& jobpattern);

#if 1
  /**
     returns the number of obtained adc results, and 0 if no result available
   */
  result* read(size_t jobno);

  /**
     returns the number of obtained adc results, and 0 if no result available
   */
  result* read(const std::string& filename);

#else
  /**
     returns the number of obtained adc results, and 0 if no result available
   */
  size_t read(size_t jobno);

  /**
     returns NULL, if no result available, or file contains no result
   */
  size_t read(const std::string& filename);
#endif

  /**
     returns error message or empty string, if none
   */
  std::string get_error_message() const;

  /**
     return next adc result data, the result data are no longer available from the handler
     if there is no result left, NULL is returned
   */
  adc_result* get_result();

  /**
     frees the parser
   */
  ~xml_result_reader();

};

/**
   writes results in xml formated files
*/
class xml_result_writer {

 public:
  /**
     \brief the adc data store method to choose
  */
  typedef enum {defaultmode, separate_file, ascii, csv, base64, hex} data_save_mode_type;

 private:
  /**
     the chosen save method for adc data
   */
  data_save_mode_type data_save_mode;

  /**
     the pattern for filename creation from job number by snprintf
   */
  std::string resultfilename_pattern;

 public:
  /**
     also default constructor, no resultfile name pattern
   */
  xml_result_writer(data_save_mode_type how=defaultmode);

  /**
     also default constructor, resultfile name pattern can be specified
   */
  xml_result_writer(const std::string& pattern, data_save_mode_type how=defaultmode);

  /**
     the result is written according to the result number and result file pattern
   */
  void write(result* res);

  /**
     the result is written to the specified file
   */
  int write_to_file(const std::string& filename, const result* res) const;

  /**
     write an adc result, choose the right method
   */
  int write_adc_to_file(const std::string& filename, const adc_result* res) const;

  /**
     write an adc result, choose the right method
   */
  int write_adcs_to_file(const std::string& filename, const adc_results* res) const;

  /**
     write data to a seperate file
   */
  int write_adcdata_separate(FILE* out, const std::string& datafilename, const adc_result* res) const;

  /**
     write the adcdata formated, pairs of samples each line
   */
  int write_adcdata_formated(FILE* out, const std::string& format, const adc_result* res) const;

  /**
     write the data in base64 code to file
   */
  int write_adcdata_base64(FILE* out, const adc_result* res) const;

  /**
     write configuration tags to one file
   */
  int write_configuration_results_to_file(const std::string& filename, const configuration_results& ress) const;

  /**
     write the error message
   */
  int write_error_to_file(const std::string& filename, const error_result* res) const;

  /**
     if nothing is konwn, use this one
   */
  int write_unknown_to_file(const std::string& filename, const result* res) const;

};

#endif /* XML_RESULT_H */
