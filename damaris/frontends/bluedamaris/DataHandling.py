# -*- coding: iso-8859-1 -*-

#########################################################################
#                                                                       #
# Class: DataHandling                                                   #
#                                                                       #
# Purpose: Executes the script recieved from the GUI, also              #
#          provides functions for being used inside the script          #
#                                                                       #
#########################################################################

import threading
import compiler
import sys
import traceback
from ResultReader import *


class DataHandling(threading.Thread):
    def __init__(self, path = None, config_object = None):
        threading.Thread.__init__(self, name = "Thread-DataHandling")

        # Directory to check for result-files
        if path is None and config_object is None:
            raise ValueError("JobWriter: missing config-object")
        elif config_object is not None:
            self.config = config_object.get_my_config(self)
            self.path = self.config["path"]
            
        elif path is not None:
            self.path = path

        # Initiating ResultReader
        self.result_reader = ResultReader(self.path)

        # Event-Stuff
        self.event_lock = threading.Event()
        self.event = threading.Event()  # Timer

        # Quit
        self.quit_main_loop = False

        # Symbolizing the data handler is ready for running
        self.__error_occured = None   # None = Still parsing, True = Error occured, False = No Error occured

        # Symbolizing the data handler no errors in other threads occured
        self.__ok_to_start = None   # None = Still waiting, True = Startsignal, False = Error occured

        # Thread up and running?
        self.__busy = False


    # Private Methods ------------------------------------------------------------------------------

    def run(self):
        "Threads activity, trying to parse a data-handling script and idling again if it finished"

        while 1:
            # Idling...
            self.__busy = False
            self.event_lock.wait()

            # Quit thread?
            if self.quit_main_loop: break

            self.__busy = True

            # Telling other threads DataHandling is still parsing
            self.__error_occured = None

            # Import needed libraries
            from Result import Result
            from Accumulation import Accumulation

            # Remove leader/trailing whitespaces
            data_handling_string = self.gui.get_data_handling_script().strip()
            #print data_handling_string

            # Syntax ok?
            if self.check_syntax(data_handling_string):
                self.__error_occured = False
            else:
                self.__error_occured = True


            # Waiting for GUI and other Threads
            while self.__ok_to_start is None:
                self.event.wait(0.2)

            # Error occured -> __ok_to_start will be set to False (from the GUI)
            if not self.__ok_to_start:
                self.__ok_to_start = None
                self.event_lock.clear()
                continue
                                
            try:
                self.result_reader.start()
                exec data_handling_string in locals()
                # self.jobs_pending() doesnt work correctly if we start everything immedeatly,
                # todo: maybe resolved
                self.event.wait(1)
                data_handling(self)
            except Exception, e:
                tb_infos=traceback.extract_tb(sys.exc_info()[2])
                self.gui.show_syntax_error_dialog("Data Handling:\nerror during execution in line %d (function %s):\n"%tb_infos[-1][1:3]+str(e))

            # Cleanup
            self.result_reader.reset()
            self.__ok_to_start = None
            self.event_lock.clear()

            
    def check_syntax(self, cmd_string):
        "Checks syntax for syntax-errors"
        try:
            compiler.parse(cmd_string)
            return True
        except Exception, e:
            self.gui.show_syntax_error_dialog("Data Handling: " + str(e))
            return False
            

    # /Private Methods -----------------------------------------------------------------------------

    # Public Methods -------------------------------------------------------------------------------
    
    def get_variable(self, name, blocking = True):
        "Returns the value of the desired variable"
        if blocking:
            if self.__dict__.has_key(name):
                return self.__dict__[name]
            else:
                while self.__dict__.has_key(name) is False:
                    self.event.wait(0.1)

                return self.__dict__[name]

        else: # Not Blocking
            if self.__dict__.has_key(name):
                return self.__dict__[name]
            else:
                return None           


    def get_next_result(self):
        "Returns the next result in queue"
        tmp = self.result_reader.get_next_result()
        while tmp is None:
            self.event.wait(0.1)
            tmp = self.result_reader.get_next_result()

        if tmp.is_error():
            self.gui.show_error_result_dialog(tmp.get_description("error_msg"))

        return tmp


    def draw(self, result):
        "Displays a result on the GUI"
        self.gui.draw_result(result)


    def jobs_pending(self):
        "Returns true, if jobs are still processed"
        if self.result_reader.get_number_of_results_pending() == 0 and self.result_reader.get_number_of_results_read() == self.job_writer.jobs_written():
            return False
        else: return True

    
    # Public Methods (Internally used) -------------------------------------------------------------

    def connect_job_writer(self, job_writer):
        "Connects the Job-Writer to the data-handler (internally used)"
        self.job_writer = job_writer


    def connect_gui(self, gui):
        "Connects the GUI to the data-handler (internally used)"
        self.gui = gui


    def is_busy(self):
        "Returns true if DataHandling is currently not idling (internally used)"
        return self.__busy



    def start_handling(self, ready_for_start):
        "Sets an internal flag true/false if handling can start and no errors occured in this or other threads (only used internally)"
        self.__ok_to_start = ready_for_start


    def wake_up(self):
        "Waking this thread up(internally used)"
        self.event_lock.set()


    def error_occured(self):
        "Returns true, if an error occured while parsing (internally used)"
        return self.__error_occured


    def quit_data_handling(self):
        if self.result_reader.isAlive():
            self.result_reader.quit_result_reader()
            self.result_reader.join()

        self.quit_main_loop = True
        self.event_lock.set()


    # /Public Methods (Internally used) ------------------------------------------------------------
