# -*- coding: iso-8859-1 -*-

#########################################################################
#                                                                       #
# Class: JobWriter                                                      #
#                                                                       #
# Purpose: Creates Job-Files out of the experiment-script.              #
#                                                                       #
#########################################################################

import threading
import os
import compiler
import traceback
import sys

from Experiment import *

import time

class JobWriter(threading.Thread):
    def __init__(self, path = None, config_object = None):
        threading.Thread.__init__(self, name = "Thread-JobWriter")

        # Directory to check for result-files
        if path is None and config_object is None:
            raise ValueError("JobWriter: missing config-object")
        elif config_object is not None:

            try:
                self.config = config_object.get_my_config(self)
                self.path = self.config["path"]
            except KeyError, e:
                print "JobWriter:  Required config attribute not found: %s" % str(e)
                print "Excpecting: 'path' -> directory where the job-files will be written into"
                raise
            except:
                raise

            
        elif path is not None:
            self.path = path


        if not os.path.exists(self.path):
            raise IOError("Error initialising JobWriter: Path does not exist")


        # Filenamestuff
        self.filename = "job.%09d"

        # Suspending / Waking up thread
        self.event_lock = threading.Event()
        self.event = threading.Event()

        # Flowcontrol
        self.quit_main_loop = False
        self.stop_job_writing = False

        # Symbolizing the job writer is ready to write the jobs
        self.__is_ready = False

        # Telling other threads that an error occured
        self.__error_occured = None   # None = Parsing, True = Error occured, False = Parsing ok

        # Symbolizing the data handler no errors in other threads occured
        self.__ok_to_start = None   # None = Still waiting, True = Startsignal, False = Error occured

        # Thread up and running?
        self.__busy = False

        # How many jobs have been written? (updated when all jobs have been written)
        self.__jobs_written = None

        # Used to stop an experiment
        self.__stop_experiment = False

        # Used to determine, if a synchronous experiment should be run
        self.__write_jobs_synchronously = False
        self.__write_another_job = 0 # Command for writing another job (or X jobs)
        self.__job_id_written_last = -1 # Last job-id written to disc



    # Private Methods ------------------------------------------------------------------------------

    def run(self):
        "Represents threads acitivity: working through the experiment script and writing job-files"

        while 1:
            # Idling...
            self.__busy = False
            self.event_lock.wait()
            self.__busy = True

            # Telling other threads we are still parsing
            self.__error_occured = None
            self.__stop_experiment = False

            # Quit thread?
            if self.quit_main_loop: break

            self.__jobs_written = None

            # Import needed libraries
            from Experiment import Experiment
            reset()

            # Remove leading/trailing whitespaces and substituting print with self.gui.new_log_message(text, "ES")
            experiment_script_string = self.gui.get_experiment_script().strip()

            # Running Preprocessor

            # Syntax ok? If not, tell other threads an error occured
            if self.check_syntax(experiment_script_string):
                self.__error_occured = False
            else:
                self.__error_occured = True


            # Waiting for GUI & other Threads
            while self.__ok_to_start is None:
                if self.quit_main_loop: return
                if self.__stop_experiment: break
                self.event.wait(0.2)

            if self.__stop_experiment: break

            # Error occured -> __ok_to_start will be set False (from the GUI)
            if not self.__ok_to_start:
                self.__ok_to_start = None
                self.event_lock.clear()
                continue

            # Bind script to self
            try:
                exec experiment_script_string in locals()
                self.experiment_script = experiment_script

            except Exception, e:
                tb_infos=traceback.extract_tb(sys.exc_info()[2])
                self.gui.show_error_dialog("Experiment Script Execution Error", "Experiment Script:\nerror during execution in line %d (function %s):\n"%tb_infos[-1][1:3]+str(e))
                self.gui.new_log_message("Execution Error: Error during execution in line %d (function %s):\n"%tb_infos[-1][1:3]+str(e), "ES")
                self.event_lock.clear()
                self.__ok_to_start = None
                continue

            # Run script
            try:
                for exp in self.experiment_script(self):

                    job_file = file(os.path.join(self.path, "job.tmp"), "w")
                    job_file.write(exp.write_xml_string())
                    job_file.close()
                    os.rename(os.path.join(self.path, "job.tmp"), os.path.join(self.path, self.filename % exp.get_job_id()))

                    self.__job_id_written_last = exp.get_job_id()

                    # Experiment started with sync-flag?
                    if self.__write_jobs_synchronously:
                        while not bool(self.__write_another_job):
                            self.event.wait(0.1)
                            if self.__stop_experiment: break

                        self.__write_another_job -= 1
                        

                    # Stop Experiment?
                    if self.__stop_experiment: break

                end_job = Experiment()

                end_job_file = file(os.path.join(self.path, "job.tmp"), "w")
                end_job_file.write(end_job.write_quit_job())
                end_job_file.close()
                os.rename(os.path.join(self.path, "job.tmp"), os.path.join(self.path, self.filename % end_job.get_job_id()))

            except Exception, e:
                tb_infos=traceback.extract_tb(sys.exc_info()[2])
                self.gui.show_error_dialog("Execution Error In Experiment Script", "Experiment Script:\nerror during execution in line %d (function %s):\n"%tb_infos[-1][1:3]+str(e))
                self.gui.new_log_message("Execution Error: Error during execution in line %d (function %s):\n"%tb_infos[-1][1:3]+str(e), "ES")
                self.gui.stop_experiment(None)

            # Cleanup
            self.event_lock.clear()
            self.__jobs_written = Experiment.job_id + 0
            self.__ok_to_start = None


    def check_syntax(self, cmd_string):
        "Checks syntax for syntax-errors"
        try:
            compiler.parse(cmd_string)
            return True
        except Exception, e:
            self.gui.show_error_dialog("Syntax Error In Experiment Script", "Experiment Script: " + str(e))
            self.gui.new_log_message("Syntax Error: " + str(e), "ES")
            return False


    # /Private Methods -----------------------------------------------------------------------------

    # Public Methods (internally used) -------------------------------------------------------------

    def connect_data_handler(self, data_handler):
        "Connects the data handler (internally used)"
        self.data_handling = data_handler


    def connect_gui(self, gui):
        "Connects the GUI (internally used)"
        self.gui = gui


    def start_writing(self, ready_to_start):
        "Sets internal flag to true/false, depending if errors occured in other threads (internally used)"

        if self.__busy:
            self.__ok_to_start = ready_to_start
        else: return


    def quit_job_writer(self):
        self.quit_main_loop = True
        self.event_lock.set()


    def wake_up(self):
        "Wakes the thread up, used by DamarisGUI to start experiment script"
        self.event_lock.set()


    def error_occured(self):
        "Returns true if an error occured while parsing (internally used)"
        return self.__error_occured


    def is_busy(self):
        "Returns true if the thread is not idling(internally used)"
        return self.__busy


    def stop_experiment(self):
        self.__stop_experiment = True

        
    def write_next_job(self):
        "Opens the lock to write next job"        
        self.__write_another_job += 1


    def write_jobs_synchronous(self, value):
        self.__write_jobs_synchronously = value
        
    # /Public Methods (internally used) ------------------------------------------------------------


    # Public Methods -------------------------------------------------------------------------------
    
    def jobs_written(self):
        "Returns the total number of jobs written or None if still writing"
        return self.__jobs_written


    def get_job_writer_path(self):
        return self.path


    def get_data_handler_variable(self, name):
        "Gets a variable from the data handler"

        tmp = self.data_handling.get_next_variable(name)

        while tmp is None:
            self.event.wait(0.1)
            if self.__stop_experiment: break
            if self.quit_main_loop: break
            
            tmp = self.data_handling.get_next_variable(name)

        return tmp


    def job_id_written_last(self):
        return self.__job_id_written_last


    # /Public Methods ------------------------------------------------------------------------------
