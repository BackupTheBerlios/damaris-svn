/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#include "core_config.h"
#include "core_exception.h"
#include "core.h"
#include <cerrno>
#include <unistd.h>
#include <expat.h>
#include <math.h>


core_config::core_config(const char** argv, int argc) {
    spool_directory=".";
    result_filename_pattern="job.%09lu.result";
    job_filename_pattern="job.%09lu";
    job_poll_wait=0.1;
    // find spool directory argument
    for (int i=1; i<argc; ++i) {
	if (strncmp(argv[i],"--spool",7)==0) {
	    if (argv[i][7]==0) {
		if (i>=argc || argv[i+1]==NULL) throw core_exception("spool directory not specified after --spool argument");
		++i;
		spool_directory=argv[i];
	    } else if (argv[i][7]=='=') {
		spool_directory=(argv[i]+8);
	    }
	    else
		throw core_exception("spool directory not specified after --spool argument");
	}
	else {
	    fprintf(stderr, "ignoring argument %s\n", argv[i]);
	}
    }
}

int core_config::xml_write_core_config_lines(FILE* f) const {
  fprintf(f,"<jobs spool=\"%s\" pattern=\"%s\" polltime=\"%g\"/>\n", spool_directory.c_str(), job_filename_pattern.c_str(), job_poll_wait);
  fprintf(f,"<results spool=\"%s\" pattern=\"%s\"/>\n", spool_directory.c_str(), result_filename_pattern.c_str());
  return 0;
}

int core_config::xml_read_core_config_startElement(const std::string& name, const xml_attrs& attrs) {
  if (name=="jobs") {
    xml_attrs::const_iterator spool_attr=attrs.find("spool");
    xml_attrs::const_iterator pattern_attr=attrs.find("pattern");
    xml_attrs::const_iterator polltime_attr=attrs.find("polltime");
    if (spool_attr!=attrs.end()) spool_directory=spool_attr->second;
    if (pattern_attr!=attrs.end()) job_filename_pattern=pattern_attr->second;
    char* polltime_char;
    job_poll_wait=(size_t)floor(1e6*strtod(polltime_attr->second.c_str(),&polltime_char));
    if (polltime_char==polltime_attr->second.c_str())
	throw core_exception("polltime is no valid number");
  }
  else if (name=="results") {
    xml_attrs::const_iterator spool_attr=attrs.find("spool");
    xml_attrs::const_iterator pattern_attr=attrs.find("pattern");
    if (spool_attr!=attrs.end()) spool_directory=spool_attr->second;
    if (pattern_attr!=attrs.end()) result_filename_pattern=pattern_attr->second;
  }
  return 0;
}


void xml_core_status_startelement_handler(core_state* cs, const char* name, char* const* attrs) {
  size_t i;
  xml_attrs the_attrs;
  for (i=0;attrs[i]!=0;i+=2) {
    the_attrs[attrs[i]]=attrs[i+1];
  }
  cs->xml_read_core_state_startElement(std::string(name),the_attrs);
}

int core_state::xml_read_core_state_startElement(const std::string& name, const xml_attrs&  attrs) {
  if (name=="state") {
    xml_attrs::const_iterator pid_attr=attrs.find("pid");
    if (pid_attr==attrs.end()) throw core_exception("no pid attribute found");
    char* pid_char;
    pid=strtol(pid_attr->second.c_str(),&pid_char,10);
    if (pid_char==pid_attr->second.c_str()) throw core_exception("pid attribute did not contain a number");
    xml_attrs::const_iterator name_attr=attrs.find("name");
    if (name_attr!=attrs.end()) core_name=name_attr->second;
  }
  else
    xml_read_core_config_startElement(name,attrs);
  return 0;
}

int core_state::dump_state(const std::string& statefilename) const {
    // first delete old state file
    int unlink_result=unlink(statefilename.c_str());
    if (unlink_result!=0 && errno!=ENOENT) {
	throw core_exception("could not remove state file "+statefilename+"\"");
    }

    //then try to write new state file
   FILE* configfile=fopen(statefilename.c_str(),"w");
   if (configfile==0) throw core_exception("could not write core configuration file \""+statefilename+"\"");
   fprintf(configfile,"<?xml version=\"1.0\"?>\n");
   fprintf(configfile,"<state name=\"%s\" pid=\"%d\" starttime=\"%ld\">\n", core_name.c_str(), pid, start_time);
   xml_write_core_config_lines(configfile);
   fprintf(configfile,"</state>\n");
   fclose(configfile);
   return 0;
}

core_state::core_state(const core& the_core): core_config(the_core.the_configuration) {
  pid=getpid();
  core_name=the_core.core_name();
  start_time=the_core.start_time;
}

core_state::core_state(const std::string& filename) {
  
  if (access(filename.c_str(),R_OK)!=0)
    throw core_exception("could not open status file "+filename);

  FILE* configfile=fopen(filename.c_str(),"r");

  /* parse document */
  XML_Parser p=XML_ParserCreate(NULL);

  /* set handler and user data*/
  XML_SetStartElementHandler(p,(XML_StartElementHandler)&xml_core_status_startelement_handler);
  XML_SetUserData(p,(void*)this);

  /* read buffer */
  for (;;) {
    const size_t BUFF_SIZE=4000;
    int bytes_read;
    void *buff = XML_GetBuffer(p,BUFF_SIZE);
    if (buff == NULL) {
      /* handle error */
    }

    bytes_read = fread(buff, 1, BUFF_SIZE, configfile);
    if (bytes_read < 0) {
      fclose(configfile);
      XML_ParserFree(p);
      throw core_exception("error while reading file "+filename);
    }

    if (! XML_ParseBuffer(p, bytes_read, bytes_read == 0)) {
      /* handle parse error */
      fclose(configfile);
      XML_ParserFree(p);
      throw core_exception("error while parsing file "+filename);
    }

    if (bytes_read == 0)
      break;
  }
  XML_ParserFree(p);
  fclose(configfile);
}
