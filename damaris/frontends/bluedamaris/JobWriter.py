#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-

import threading
import os
import compiler
from Experiment import *

class JobWriter(threading.Thread):
    def __init__(self, path = None, config_object = None):
        threading.Thread.__init__(self)

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

        # Flowcontrol
        self.quit_main_loop = False
        self.stop_job_writing = False

        # Symbolizing the job writer is ready to write the jobs
        self.__is_ready = False



    def run(self):

        while 1:
            # Idling...
            self.event_lock.wait()

            # Quit thread?
            if self.quit_main_loop: break

            # Import needed libraries
            from Experiment import Experiment

            # Remove leader/trailing whitespaces
            experiment_script_string = self.gui.get_experiment_script().strip()

            # Syntax ok?
            if self.check_syntax(experiment_script_string):
                print "Syntax ok!"
            else:
                self.event_lock.clear()
                continue
                # send Error Signal
##        while 1:
##
##            # Waiting for start from gui
##            self.event_lock.wait()
##
##            # Quit thread?
##            if self.quit_main_loop: break
##
##            from Experiment import Experiment
##
##            experiment_script_string = self.gui.get_experiment_script()
##            experiment_script_string = experiment_script_string.strip()
##
##            # Syntax checking
##            if experiment_script_string is not ok:
##                self.gui.show_syntax_error_dialog("Experiment Script: " + str(e))
##                self.event_lock.clear()
##                continue
##            else:
##                try:
##                    exec experiment_script_string in locals()
##                    self.experiment_script = experiment_script
##                except:
##                    Fehlerbehandlung
##
##                self.__is_ready = True
##                self.event_lock.wait()   # waiting for data_handling ok
##                self.event_lock.clear()  # Sofort wieder set() l�schen
##                
##        
##                try:
##                    for exp in self.experiment_script(self):
##                        job_file = file(os.path.join(self.path, "job.tmp"), "w")
##                        job_file.write(exp.write_xml_string())
##                        job_file.close()
##                        os.rename(os.path.join(self.path, "job.tmp"), os.path.join(self.path, self.filename % exp.get_job_id()))
##
##                    end_job = Experiment()
##
##                    end_job_file = file(os.path.join(self.path, "job.tmp"), "w")
##                    end_job_file.write(end_job.write_quit_job())
##                    end_job_file.close()
##                    os.rename(os.path.join(self.path, "job.tmp"), os.path.join(self.path, self.filename % end_job.get_job_id()))
##
##                except:
##                    raise
##
##            self.event_lock.clear()
##            self.is_ready = False
##            Experiment.reset()



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


    def start_job_writing(self):
        print "Start writing jobs..."
        self.event_lock.set()


    def stop_job_writing(self):
        pass


    def quit_job_writer(self):
        self.quit_main_loop = True
        self.event_lock.set()
        

    def wake_up(self):
        self.event_lock.set()


    def is_ready(self):
        return self.__is_ready

    # / Schnittstellen nach Au�en ------------------------------------------------------------------