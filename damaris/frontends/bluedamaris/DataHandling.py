#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-

import threading
import compiler
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



    def run(self):

        while 1:
            # Idling...
            self.__busy = False
            self.event_lock.wait()
            self.__busy = True

            # Telling other threads DataHandling is still parsing
            self.__error_occured = None

            # Quit thread?
            if self.quit_main_loop: break

            # Import needed libraries
            from Result import Result
            from Accumulation import Accumulation

            # Remove leader/trailing whitespaces
            data_handling_string = self.gui.get_data_handling_script().strip()
            #print data_handling_string

            # Syntax ok?
            if self.check_syntax(data_handling_string):
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
                self.__ok_to_start = None
                self.__error_occured = True
                self.event_lock.clear()
                continue
                
            try:
                print "1"
                self.result_reader.start()
                print "2"
                exec data_handling_string in locals()
                print "3"
                data_handling(self)
                print "4"
            except Exception, e:
                self.gui.show_syntax_error_dialog("Data Handling: Unexpected error during execution of data handling script.\n" + str(e))

            # Cleanup
            self.result_reader.reset()
            self.__ok_to_start = None
            self.event_lock.clear()

            
    def check_syntax(self, cmd_string):
        try:
            compiler.parse(cmd_string)
            return True
        except Exception, e:
            self.gui.show_syntax_error_dialog("Data Handling: " + str(e))
            return False
            
        
    def get_variable(self, name, blocking = True):
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


    def quit_data_handling(self):
        self.result_reader.quit_result_reader()
        self.result_reader.join()

        self.quit_main_loop = True


    def join(self):
        self.result_reader.join()


    def get_next_result(self):
        tmp = self.result_reader.get_next_result()
        while tmp is None:
            self.event.wait(0.1)
            tmp = self.result_reader.get_next_result()

        return tmp
 

    def connect_job_writer(self, job_writer):
        self.job_writer = job_writer



    def connect_gui(self, gui):
        self.gui = gui


    def draw(self, result):
        self.gui.draw_result(result)
        #self.gui.flush()


    def jobs_pending(self):
        if self.result_reader.get_number_of_results_pending() == 0 and self.result_reader.get_number_of_results_read() == self.job_writer.jobs_written():
            return False
        else: return True

    
    # Schnittstellen nach Außen --------------------------------------------------------------------

    def is_busy(self):
        return self.__busy


    def quit_data_handling(self):
        self.quit_main_loop = True
        self.event_lock.set()


    def start_handling(self, ready_for_start):
        self.__ok_to_start = ready_for_start


    def wake_up(self):
        self.event_lock.set()


    def error_occured(self):
        return self.__error_occured

    # / Schnittstellen nach Außen ------------------------------------------------------------------
