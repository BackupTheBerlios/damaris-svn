/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include "job_receiver.h"
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMException.hpp>

job_receiver::job_receiver(std::string the_jobfilenamepattern) {
  try {
    XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::Initialize();
  }
  catch (const XERCES_CPP_NAMESPACE_QUALIFIER XMLException& toCatch) {
    char* ini_error=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(toCatch.getMessage());
    job_exception new_exception(std::string("xerces initialisation error: ")+ini_error);
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&ini_error);
    throw new_exception;
  }

  jobfilename=NULL;
  setFilenamePattern(the_jobfilenamepattern);
  parser=new XERCES_CPP_NAMESPACE_QUALIFIER XercesDOMParser();
  if (parser==NULL) {
    delete jobfilename;
    throw job_exception("could not allocate parser");
  }
  parser->setValidationScheme(XERCES_CPP_NAMESPACE_QUALIFIER XercesDOMParser::Val_Always);
  parser->setDoNamespaces(true);
  errHandler = (XERCES_CPP_NAMESPACE_QUALIFIER ErrorHandler*) new XERCES_CPP_NAMESPACE_QUALIFIER HandlerBase();
  parser->setErrorHandler(errHandler);
}

void job_receiver::setFilenamePattern(const std::string& filenamepattern) {
  if (jobfilename!=NULL) delete jobfilename;
  jobfilenamepattern=filenamepattern;
  jobfilenamesize=filenamepattern.size()+20;
  jobfilename=new char[jobfilenamesize];
  if (jobfilename==NULL) throw job_exception("could not allocate memory for filename");
}

job* job_receiver::receive(size_t no) {
  snprintf(jobfilename,jobfilenamesize+20,jobfilenamepattern.c_str(),no);
  job* new_job=receive(std::string(jobfilename));
  if (new_job->no()!=no) fprintf(stderr, "expected job number %" SIZETPRINTFLETTER " and specified number %" SIZETPRINTFLETTER " are different\n", no, new_job->no() );
  new_job->job_no=no;
  return new_job;
}

job* job_receiver::receive(const std::string& filename) {

  try {
    parser->parse(filename.c_str());
  }
  catch(const XERCES_CPP_NAMESPACE_QUALIFIER XMLException& toCatch) {
    char* message = XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(toCatch.getMessage());
    job_exception je(std::string("XML error: ")+message);
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&message);
    throw je;
  }
  catch(const XERCES_CPP_NAMESPACE_QUALIFIER DOMException& toCatch) {
    char* message = XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(toCatch.msg);
    job_exception je(std::string("XML DOM error: ")+message); 
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&message);
    throw je;
  }
  catch(const XERCES_CPP_NAMESPACE_QUALIFIER SAXParseException& toCatch) {
    // more verbose for parser errors
    char* message = XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(toCatch.getMessage());
    job_exception je(std::string("XML SAX Parser error: ")+message);
    char location[100];
    snprintf(location,sizeof(location),", line %ld column %ld",toCatch.getLineNumber(),toCatch.getColumnNumber());
    je.append(location);
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&message);
    throw je;
  }
  catch(const XERCES_CPP_NAMESPACE_QUALIFIER SAXException& toCatch) {
    char* message = XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(toCatch.getMessage());
    job_exception je(std::string("XML SAX error: ")+message); 
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&message);
    throw je;
  }

  // extract root element, root attributes and root name
  XERCES_CPP_NAMESPACE_QUALIFIER DOMDocument* doc=parser->getDocument();
  if (doc==NULL) throw job_exception("xml job document not found");
  XERCES_CPP_NAMESPACE_QUALIFIER DOMElement* rootelement=doc->getDocumentElement();
  if (rootelement==NULL) throw job_exception("xml job root document not found");
  char* docname=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(rootelement->getNodeName());
  XERCES_CPP_NAMESPACE_QUALIFIER DOMNamedNodeMap* rootattrs=rootelement->getAttributes();

  // check the job number
  XMLCh* docnoname=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode("no");
  XERCES_CPP_NAMESPACE_QUALIFIER DOMNode* jobno_attr=rootattrs->getNamedItem(docnoname);
  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&docnoname);
  if (jobno_attr==NULL) {
    docnoname=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode("no");
    jobno_attr=rootattrs->getNamedItem(docnoname);
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&docnoname);
  }
  size_t no=0;
  if (jobno_attr==NULL) fprintf(stderr,"Warning: job %" SIZETPRINTFLETTER ": root element has no job number\n",no);
  else {
    char* docno=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(jobno_attr->getNodeValue());
    no=strtoul(docno,NULL,0);
    XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&docno);
  }

  job* this_job=NULL;
  // determine the job type by name
  if (strcasecmp(docname,"quit")==0) {
    this_job=new quit_job(no);
  }
  else if (strcasecmp(docname,"nop")==0) {
    this_job=new do_nothing_job(no);
  }
  else if (strcasecmp(docname,"pause")==0) {
    this_job=new pause_job(no);
  } /* pause */
  else if (strcasecmp(docname,"restart")==0) {
    this_job=new restart_job(no);
  } /* restart */
  else if (strcasecmp(docname,"wait")==0) {
    this_job=new wait_job(no,rootattrs);
  } /* wait */
  else if (strcasecmp(docname,"experiment")==0) {
    try {
      this_job=new experiment(no, rootelement);
    }
    catch (const XERCES_CPP_NAMESPACE_QUALIFIER DOMException& de) {
      char* domerrmsg=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(de.msg);
      char domerrno[5];
      snprintf(domerrno,5,"%d",de.code);
      job_exception je("sorry, something happend while parsing experiment job: ");
      je.append(domerrmsg);
      je.append(", code ");
      je.append(domerrno);
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&domerrmsg);
      // cleanup missing
      throw je;
    }
  }
  else if (strcasecmp(docname,"configuration")==0) {
    try {
      this_job=new configuration(no, rootelement);
    }
    catch (const XERCES_CPP_NAMESPACE_QUALIFIER DOMException& de) {
      char* domerrmsg=XERCES_CPP_NAMESPACE_QUALIFIER XMLString::transcode(de.msg);
      char domerrno[5];
      snprintf(domerrno,5,"%d",de.code);
      job_exception je("sorry, something happend while parsing configuration job: ");
      je.append(domerrmsg);
      je.append(", code ");
      je.append(domerrno);
      XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&domerrmsg);
      // cleanup missing
      throw je;
    }
  }
  else if (strcasecmp(docname,"singlepulse")==0) {
    this_job=new single_pulse_experiment(no,rootattrs);
  }

  XERCES_CPP_NAMESPACE_QUALIFIER XMLString::release(&docname);
  parser->reset();
  parser->resetDocument();
  parser->resetDocumentPool();

  return this_job;
}


job_receiver::~job_receiver() {
  delete jobfilename;
  delete errHandler;
  delete parser;
  XERCES_CPP_NAMESPACE_QUALIFIER XMLPlatformUtils::Terminate();
}
