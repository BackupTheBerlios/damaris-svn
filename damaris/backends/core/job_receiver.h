/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef JOB_RECEIVER_H
#define JOB_RECEIVER_H

#include <string>
#include "job.h"
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>


/**
   \ingroup jobs
   can create jobs from xml content
 */
class job_receiver {
  XERCES_CPP_NAMESPACE_QUALIFIER XercesDOMParser* parser;
  XERCES_CPP_NAMESPACE_QUALIFIER ErrorHandler* errHandler;

  std::string jobfilenamepattern;
  char* jobfilename;
  size_t jobfilenamesize;

 public:

  /**
     the receiver should know how to handle the job directory...
   */
  job_receiver(const std::string jobfilenamepattern);
  
  void setFilenamePattern(const std::string& filenamepattern);

  /**
     here, only the number should be given
   */
  job* receive(const size_t no);

  /**
     here, only the filename
   */
  job* receive(const std::string& filename);

  /**
     free everything
   */
  ~job_receiver();
};

#endif
