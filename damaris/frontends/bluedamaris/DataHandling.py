#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-

import threading
import compiler
from ResultReader import *


class DataHandling(threading.Thread):
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

        # Initiating ResultReader
        self.result_reader = ResultReader(self.path)

        # Event-Stuff
        self.event_lock = threading.Event()

        # Quit
        self.quit_main_loop = False

        # Symbolizing the data handler is ready for running
        self.__is_ready = False



    def run(self):

        while 1:
            # Idling...
            self.event_lock.wait()

            # Quit thread?
            if self.quit_main_loop: break

            # Import needed libraries
            from Result import Result
            from Accumulation import Accumulation

            # Remove leader/trailing whitespaces
            data_handling_string = self.gui.get_data_handling().strip()

            # Syntax ok?
            if self.check_syntax(data_handling_string):
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
##            from Result import Result
##            from Accumulation import Accumulation
##
##            data_handling_script_string = self.gui.get_data_handling_script()
##            data_handling_script_string = data_handling_script_string.strip()
##
##            # Syntax checking
##            if data_handling_script_string is not ok:
##                self.gui.show_syntax_error_dialog("Data Handling: " + str(e))
##                self.event_lock.clear()
##                continue
##            else:
##                try:
##
##                    self.is_ready = True
##                    self.event_lock.wait()   # waiting for job_writer ok (von außen betätigt)
##                    self.event_lock.clear()  # Sofort wieder set() löschen
##                    
##                    self.result_reader.start()                    
##                    exec data_handling_script_string in locals()
##                except:
##                    Fehlerbehandlung
##
##            self.event_lock.clear()
##            self.is_ready = False


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


    def get_next_result(self, blocking = True):
        if blocking:
            while self.result_reader.get_number_of_results_pending() is 0:
                self.event.wait(0.1)

            if self.result_reader.is_running():
                return self.result_reader.get_next_result()
            else:
                return 0
        else:
            if self.result_reader.is_running():
                return self.result_reader.get_next_result()
            else:
                return 0


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

    def quit_data_handling(self):
        print "Quitting data handling..."
        self.quit_main_loop = True
        self.event_lock.set()


    def start_data_handling(self):
        print "Starting data handling..."
        self.event_lock.set()


    def wake_up(self):
        self.event_lock.set()


    def is_ready(self):
        return __is_ready

    # / Schnittstellen nach Außen ------------------------------------------------------------------
