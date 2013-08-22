/* **************************************************************************

 Author: Achim Gaedke
 Created: June 2004

 ****************************************************************************/
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <errno.h>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLException.hpp>

#include "core.h"
#include "job_receiver.h"
#include "xml_result.h"

#ifndef SIZETPRINTFLETTER
#  ifndef __LP64__
#    define SIZETPRINTFLETTER "u"
#  else
#    define SIZETPRINTFLETTER "lu"
#  endif
#endif

volatile int core::triggered_restart=0;
volatile int core::quit_signal=0;
volatile int core::term_signal=0;

void restart_signal_handler(int signal) {
    if (signal==SIGUSR1)
        core::triggered_restart=1;
    return;
}

void quit_signal_handler(int signal) {
    if (signal==SIGQUIT)
        core::quit_signal=1;
    return;
}

void term_signal_handler(int signal) {
    if (signal==SIGINT || signal==SIGTERM) {
        core::term_signal=1;
    }
    return;
}

core::core(const core_config& configuration) {
    // time of start
    if (-1==time(&start_time))
    {
        throw core_exception("could not determine start time");
    }

    // store away cwd for later reset
    _cwd = getcwd(NULL, 0);
    if (_cwd == NULL)
    {
        throw core_exception("Getting the current working directory failed");
    }

    // cd to spool directory if necessary
    if (!configuration.spool_directory.empty()) {
        // change to given directory
        if (0!=chdir(configuration.spool_directory.c_str())) {
            throw core_exception(std::string("can not change to \"")+configuration.spool_directory+"\"");
        }
    }
    char* spoolDir = getcwd(NULL, 0);

    the_configuration.spool_directory=spoolDir;
    free(spoolDir);

    // allocate job parser
    job_parser=new job_receiver("");

    // reset counter
    job_counter=0;

    if (configuration.job_filename_pattern.empty())
        the_configuration.job_filename_pattern="job.%09lu";
    else
        the_configuration.job_filename_pattern=configuration.job_filename_pattern;

    if (configuration.result_filename_pattern.empty())
        the_configuration.result_filename_pattern=the_configuration.job_filename_pattern+".result";
    else
        the_configuration.result_filename_pattern=configuration.result_filename_pattern;


    if (configuration.job_poll_wait<0.001)
        the_configuration.job_poll_wait=0.1;
    else
        the_configuration.job_poll_wait=configuration.job_poll_wait;

    // todo valid value?!
    the_configuration.result_encoding=configuration.result_encoding;
    // init hardware is done by derived class
    the_hardware=NULL;

    // install signal handler to reset the job counter
    signal(SIGUSR1,restart_signal_handler);
    signal(SIGQUIT,quit_signal_handler);
    signal(SIGTERM,term_signal_handler);
    signal(SIGINT,term_signal_handler);

    quit_mainloop=0;
}


int core::write_state() const {
    if (!core_name().empty())
        return core_state(*this).dump_state(core_name()+".state");
    else
        return core_state(*this).dump_state("core.state");
}

int core::remove_state() const {
    std::string state_filename;
    if (core_name().empty())
        state_filename="core.state";
    else
        state_filename=core_name()+".state";
    if (access(state_filename.c_str(),F_OK)!=0)
        throw core_exception("the core state file vanished...");
    if (0!=remove(state_filename.c_str()))
        throw core_exception(std::string("could not remove the core state file \"")+state_filename+"\"");
    return 0;
}


int core::run() {
    // writes the config and process id to file
    write_state();

    quit_mainloop=0;

    while (!quit_mainloop && term_signal==0 && quit_signal==0) {
        // wait for a job to run...
        job * this_job=NULL;
        result* this_result=NULL;
        try {
            this_job=wait_for_input();
        }
        catch (const job_exception& je) {
            this_result=new error_result(job_counter,je);
        }

        if (quit_mainloop!=0 || term_signal!=0) {
            if (this_job!=NULL) delete this_job;
            break;
        }
        // do the work ...
        if (this_job!=NULL) {
            this_result=do_the_job(*this_job);
            if (term_signal!=0) {
                delete this_job;
                if (this_result!=NULL) delete this_result;
                break;
            }
        }
        else
            this_job=new job(job_counter);
        if (this_result==NULL) {
            this_result=new error_result(job_counter,"unexpected: did not get any result");
        }
        // tell them...
        write_to_output(*this_job,*this_result);
        delete this_result;
        delete this_job;
        job_counter++;
    }

    // deletes config file
    remove_state();

    return 0;
}

job* core::wait_for_input() {
    if (triggered_restart!=0) {
        job_counter=0;
        triggered_restart=0;
    }
    if (quit_signal!=0 || term_signal!=0) {
        quit_mainloop=1;
        return (job*)NULL;
    }
    std::string job_filename;
    struct stat job_stat;
    // polling loop
    while(1) {
        if (job_filename.empty()) {
            char* job_filename_buffer=(char*)malloc(the_configuration.job_filename_pattern.size()+100);
            if (job_filename_buffer==NULL)
                throw core_exception("could not allocate memory for job filename");
            snprintf(job_filename_buffer,
            the_configuration.job_filename_pattern.size()+100,
            the_configuration.job_filename_pattern.c_str(),
            job_counter);
            job_filename=job_filename_buffer;
            free(job_filename_buffer);
        }
        int stat_success=stat(job_filename.c_str(), &job_stat);
        if (0==stat_success && job_stat.st_size>0) {
            int file_access=access(job_filename.c_str(),F_OK);
            if (0==file_access) {
                // job_reciever creates a job
                job* new_job=job_parser->receive(job_filename);
                if (new_job->no()!=job_counter) {
                    new_job->job_no=job_counter;
                    fprintf(stderr,"%s : corrected job number\n", job_filename.c_str());
                }
                return new_job;
            }
        }
        usleep((size_t)floor(the_configuration.job_poll_wait*1.0e6));
        // respect triggered restart of program
        if (quit_signal!=0 || term_signal!=0) {
            quit_mainloop=1;
            break;
        }
        if (triggered_restart!=0) {
            job_counter=0;
            job_filename="";
            triggered_restart=0;
        }
    }
    return (job*)NULL;
}

result* core::do_the_job(job& the_job) {
    // look for main loop commands, like quit, wait or nop
    control* cjob=dynamic_cast<control*>(&the_job);
    if (cjob!=NULL) {
        return cjob->do_it(this);

    }

    // experiments
    experiment* ejob=dynamic_cast<experiment*>(&the_job);
    if (ejob!=NULL) {
        result* r=ejob->do_it(the_hardware);
        return r;
    }

    // experiments
    configuration* confjob=dynamic_cast<configuration*>(&the_job);
    if (confjob!=NULL) {
        result* r=confjob->do_it(the_hardware);
        return r;
    }


    // execute everything else without parameter ...
    return NULL; //the_job.do_it();
}

int core::write_to_output(const job& job_done, const result& result_gained){
    if (job_done.no()!=result_gained.no()) throw core_exception(std::string(__FUNCTION__)+": job and result number do not match");
    // create job file name
    char* expansion_buffer=(char*)malloc(the_configuration.result_filename_pattern.size()+100);
    if (expansion_buffer==NULL)
        throw core_exception(std::string(__FUNCTION__)+": could not allocate expansion_buffer");
    snprintf(expansion_buffer,the_configuration.result_filename_pattern.size()+100,the_configuration.result_filename_pattern.c_str(),job_done.no());
    std::string resultfile=expansion_buffer;
    free(expansion_buffer);
    // remove old result file
    if (unlink(resultfile.c_str())==-1 && errno!=ENOENT) {
        throw core_exception(std::string(__FUNCTION__)+": could not remove existing old result file");
    }
    if (term_signal!=0) return 0;
    // write to temporary file
    std::string tempfile=resultfile+".temp";
    if (unlink(tempfile.c_str())==-1 && errno!=ENOENT) {
        throw core_exception(std::string(__FUNCTION__)+": could not remove existing temporary result file");
    }
    if (term_signal!=0) return 0;
    int write_result=xml_result_writer(the_configuration.result_encoding).write_to_file(tempfile,&result_gained);
    if (term_signal!=0) {
        unlink(tempfile.c_str());
        return 0;
    }
    // finally move temporary file to result file
    if (0!=rename(tempfile.c_str(),resultfile.c_str()))
        throw core_exception(std::string(__FUNCTION__)+": could not rename result-tempfile");
    return write_result;
}

core::~core() {
    // todo: send termination message
    // release hardware
    // reset cwd
    chdir(_cwd);
    if (_cwd != NULL) free(_cwd);
    if (the_hardware!=NULL) delete the_hardware;
    if (job_parser!=NULL) delete job_parser;
}
