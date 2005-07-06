/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

****************************************************************************/
#ifndef CORE_CONFIG_H
#define CORE_CONFIG_H

#include <cstdio>
#include <string>
#include <map>

class core;

/**
 */
typedef std::map<const std::string, std::string> xml_attrs;

/**
 */
class core_config {
 public:
    /**
       the directory for job and result data

       to be more precise: it is the current workdir of the core where state file goes in
     */
    std::string spool_directory;

    /**
       the pattern to create a result file, should contain %09lu printf token
     */
    std::string result_filename_pattern;

    /**
       name pattern to read jobs, should contain %09lu printf token
     */
    std::string job_filename_pattern;

    /*
      wait time in seconds until next read
    */
    double job_poll_wait;
    int xml_write_core_config_lines(FILE* f) const;
    int xml_read_core_config_startElement(const std::string& name, const xml_attrs& attrs);
    core_config():spool_directory("."), result_filename_pattern("job.%09lu.result"), job_filename_pattern("job.%09lu"), job_poll_wait(0.1) {}
    core_config(const char** argv, int argc);
    core_config(const core_config& c): spool_directory(c.spool_directory),
	result_filename_pattern(c.result_filename_pattern),
	job_filename_pattern(c.job_filename_pattern),
	job_poll_wait(c.job_poll_wait) {}
    core_config(const std::string& dir): spool_directory(dir), result_filename_pattern("job.%09lu.result"), job_filename_pattern("job.%09lu"), job_poll_wait(0.1) {}
};

class core_state: public core_config {
 public:
  pid_t pid;
  time_t start_time;
  std::string core_name;
  core_state(const core& the_core);
  core_state(const std::string& filename);
  int xml_read_core_state_startElement(const std::string& name, const std::map<const std::string, std::string>& attrs);
  int dump_state(const std::string& filename) const;
};


#endif
