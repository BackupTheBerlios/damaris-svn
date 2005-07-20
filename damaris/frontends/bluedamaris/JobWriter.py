#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-

import threading
import os
import compiler
import glob
from Experiment import *

class JobWriter(threading.Thread):
    def __init__(self, path = None, config_object = None):
        threading.Thread.__init__(self, name = "Thread-JobWriter")

        # Directory to check for result-files
        if path is None and config_object is None:
            raise ValueError("JobWriter: missing config-object")
        elif config_object is not None:
            self.config = config_object.get_my_config(self)
            self.path = self.config["path"]
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


    def run(self):

        while 1:
            # Idling...
            self.__busy = False
            self.event_lock.wait()
            self.__busy = True

            # Delete existing job-files
            file_list = glob.glob(os.path.join(self.path, "job*"))
            try:
                for job_file in file_list:
                    if job_file.find("job.state") != -1:
                        continue
                    os.remove(job_file)
            except IOError, e:
                print "IOError: Cannot delete Job-Files" + str(e)
                raise

            # Telling other threads we are still parsing
            self.__error_occured = None

            # Quit thread?
            if self.quit_main_loop: break

            # Import needed libraries
            from Experiment import Experiment
            reset()

            # Remove leader/trailing whitespaces
            experiment_script_string = self.gui.get_experiment_script().strip()

            # Syntax ok?
            if self.check_syntax(experiment_script_string):
                self.__error_occured = False

                # Wait for ok from GUI (no errors occured in other threads)
                while self.__ok_to_start is None:
                    self.event.wait(0.2)

                # Error occured somewhere else
                if not self.__ok_to_start:
                    self.__ok_to_start = None
                    self.event_lock.clear()
                    continue
                
            else:
                self.__error_occured = True
                self.__ok_to_start = None
                self.event_lock.clear()
                continue

            # Bind script to self
            try:
                exec experiment_script_string in locals()
                self.experiment_script = experiment_script
            except Exception, e:
                self.gui.show_syntax_error_dialog("Experiment Script: Unexpected error during execution!\n" + str(e))
                self.event_lock.clear()
                self.__error_occured = None
                self.__ok_to_start = None
                continue

            # Run script
            try:
                for exp in self.experiment_script(self):
                    job_file = file(os.path.join(self.path, "job.tmp"), "w")
                    job_file.write(exp.write_xml_string())
                    job_file.close()
                    os.rename(os.path.join(self.path, "job.tmp"), os.path.join(self.path, self.filename % exp.get_job_id()))

                end_job = Experiment()

                end_job_file = file(os.path.join(self.path, "job.tmp"), "w")
                end_job_file.write(end_job.write_quit_job())
                end_job_file.close()
                os.rename(os.path.join(self.path, "job.tmp"), os.path.join(self.path, self.filename % end_job.get_job_id()))

            except Exception, e:
                self.gui.show_syntax_error_dialog("Experiment Script: Unexpected error during execution!\n" + str(e))

            # Cleanup
            self.event_lock.clear()
            self.__error_occured = None
            self.__ok_to_start = None


    def check_syntax(self, cmd_string):
        try:
            compiler.parse(cmd_string)
            return True
        except Exception, e:
            self.gui.show_syntax_error_dialog("Experiment Script: " + str(e))
            return False

    # Schnittstellen nach Au�en --------------------------------------------------------------------

    def connect_data_handler(self, data_handler):
        self.data_handling = data_handler


    def connect_gui(self, gui):
        self.gui = gui


    def jobs_written(self):
        return Experiment.job_id + 0


    def start_writing(self, ready_to_start):
        self.__ok_to_start = ready_to_start


    def quit_job_writer(self):
        self.quit_main_loop = True
        self.event_lock.set()
        

    def wake_up(self):
        self.event_lock.set()


    def error_occured(self):
        return self.__error_occured


    def is_busy(self):
        return self.__busy

    # / Schnittstellen nach Au�en ------------------------------------------------------------------
