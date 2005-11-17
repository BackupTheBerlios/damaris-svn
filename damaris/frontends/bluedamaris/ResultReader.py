# -*- coding: iso-8859-1 -*-

#############################################################################
#                                                                           #
# Name: Class ResultReader                                                  #
#                                                                           #
# Purpose: Checks periodically a given directory for Result-Files.          #
#          If some are found, they are parsed and stored internally, until  #
#          they are read (returned as results: See class Result             #
#                                                                           #
#############################################################################

import threading
import os
import xml.parsers.expat
from datetime import datetime

from ADC_Result import ADC_Result
from Error_Result import Error_Result
from Temp_Result import Temp_Result
from Config_Result import Config_Result

class ResultReader(threading.Thread):

    CONFIG_TYPE = 3
    TEMP_TYPE = 2
    ADCDATA_TYPE = 1
    ERROR_TYPE = 0

    # Private Methods ------------------------------------------------------------------------------

    def __init__(self, in_path, timeout = -1):
        threading.Thread.__init__(self, name = "Thread-ResultReader")

        # Directory to check for result-files
        self.path = in_path
        if not os.path.exists(self.path):
            raise IOError, "Error initialising ResultReader: Path does not exist"

        # The result-queue
        self.result_queue = []

        # Maximum Number of Results in Queue (if reached, ResultReader will sleep until results are read)
        self.max_result_queue_length = 100
        
        # Filename stuff
        self.filename = "job.%09d.result"

        # Kind of file (Error, ADC-Data...)
        self.__filetype = None

        # Quit main loop?
        self.quit_loop = False

        # Indicates thread being active (= reading, parsing, giving back results)
        self.is_idling = False

        # How many files have been read?
        self.files_read = 0

        # How long long shall the thread be put asleep, if no file is found?
        self.wait = 0.2

        # When shall the thread be quit, if not files are found?
        self.timeout = timeout
        self.timeout_counter = 0

        # class-intern result-object currently being processed
        self.tmp_result = None

        # Event-Objekt für Thread-Suspending
        self.event = threading.Event()          # Timeout
        self.event_lock = threading.Event()     # Thread locking

        # ResultReader reset?
        self.__reset = False

        # ResultReader started?
        self.__started = False



    def run(self):
        "Represents the threads activity: Checking the given directory for result-files"
        self.__started = True

        while not self.quit_loop:
            # Making ResultReader restartable and/or waiting for user to read pending results
            self.event_lock.wait()

            # Making sure it really quits, if ResultReader somehow was hold (like waiting for queue to empty)
            if self.quit_loop:
                break

            # Reset-quest from other Thread?
            if self.__reset:
                self.event_lock.clear()
                self.__do_reset()
                continue

            # Too many results pending?
            if self.get_number_of_results_pending() >= self.max_result_queue_length:
                self.event_lock.clear()
                #print "Waiting for results being read from Queue... %d >= %d" % (self.get_number_of_results_pending(), self.max_result_queue_length)
                continue                

            if os.access(os.path.join(self.path, self.filename % self.files_read), os.R_OK):
                #print "File found! " + os.path.join(self.path, self.filename % self.files_read)
                self.__readFile(os.path.join(self.path, self.filename % self.files_read))
                self.timeout_counter = 0
            else:
                #print "No File found, waiting... (" + str(self.wait) + ")"
                self.event.wait(self.wait)
                self.timeout_counter += self.wait
                #print self.timeout_counter
                
                if self.timeout_counter >= self.timeout and self.timeout is not -1:
                    print "Exception: ResultReader timeout!\n"
                    #print "Printing result-queue..."
                    #print self.result_queue
                    self.quit_loop = True




    def __readFile(self, in_filename):
        "Opens the found file and hands it to the parser"

        try:
            result_file = file(in_filename, "r")
            #print result_file

            # get date of last modification 
            self.result_job_date = datetime.fromtimestamp(os.stat(in_filename)[8])
            
            self.__parseFile(result_file)

            self.files_read += 1
            result_file.close()
            
        except IOError:
            self.__gui.new_log_message("ResultReader: File \"" + in_filename + "\" could not be opened.", "DH")

        except:
            result_file.close()
            raise



    def __parseFile(self, in_file):
        "Parses the given file, adding it to the result-queue"

        self.result = None
        self.result_description = { }

        self.result_job_number = None
        # Job Date is set in __read_file()
        self.__filetype = None

        # Expat XML-Parser & Binding handlers
        self.xml_parser = xml.parsers.expat.ParserCreate()
        self.xml_parser.StartElementHandler = self.__xmlStartTagFound
        self.xml_parser.CharacterDataHandler = self.__xmlCharacterDataFound
        self.xml_parser.EndElementHandler = self.__xmlEndTagFound

        # Parsing all cdata as one block
        self.xml_parser.buffer_text = True
        buffersize=self.xml_parser.buffer_size*2
        databuffer=in_file.read(buffersize)
        while databuffer!="":
            if self.__reset:
                self.event_lock.clear()
                self.__do_reset()
                return

            self.xml_parser.Parse(databuffer,False)
            databuffer=in_file.read(buffersize)

        self.xml_parser.Parse("",True)
        self.xml_parser = None

    # Callback when a xml start tag is found
    def __xmlStartTagFound(self, in_name, in_attribute):

        # General Result-Tag
        if in_name == "result":
            self.result_job_number = int(in_attribute["job"])
            # Job-Date is set in __read_file()

        # Description
        elif in_name == "description":
            self.result_description = in_attribute.copy()

        # ADC_Results
        elif in_name == "adcdata":
            self.__filetype = ResultReader.ADCDATA_TYPE

            self.adc_result_current_channel = 0
            self.adc_result_trailing_chars = ""

            if self.result is None:
                self.result = ADC_Result()

                # Change number of channels of your adc-card here
                channels = 2
                self.result.create_data_space(channels, int(in_attribute["samples"]))
                
                self.result.set_sampling_rate(float(in_attribute["rate"]))
                self.result.set_job_id(self.result_job_number)
                self.result.set_job_date(self.result_job_date)

                self.result.set_description_dictionary(self.result_description.copy())
                
                self.adc_result_sample_counter = 0
            else:
                self.result.add_sample_space(int(in_attribute["samples"]))
                
        # Error_Results
        elif in_name == "error":
            self.__filetype = ResultReader.ERROR_TYPE
            
            self.result = Error_Result()
            self.result.set_job_id(self.result_job_number)
            self.result.set_job_date(self.result_job_date)

            self.result.set_description_dictionary(self.result_description.copy())

        # Temp_Results
        elif in_name == "temp":
            self.__filetype = ResultReader.TEMP_TYPE

            self.result = Temp_Result()
            self.result.set_job_id(self.result_job_number)
            self.result.set_job_date(self.result_job_date)            

        # Config_Results
        elif in_name == "conf":
            self.__filetype = ResultReader.CONFIG_TYPE

            self.result = Config_Result()
            self.result.set_job_id(self.result_job_number)
            self.result.set_job_date(self.result_job_date)
    

    def __xmlCharacterDataFound(self, in_cdata):

        # ADC_Result
        if self.__filetype == ResultReader.ADCDATA_TYPE:
            values=(self.adc_result_trailing_chars+in_cdata).split()
            if not in_cdata[-1].isspace():
                self.adc_result_trailing_chars=values.pop()
            else:
                self.adc_result_trailing_chars=""

            for i in values:
                self.result.set_ydata(self.adc_result_current_channel, self.adc_result_sample_counter, int(i))
                #print "added value " + str(i) + " at: " + str(self.current_channel) + ", " + str(self.current_pos)
                self.adc_result_current_channel = (self.adc_result_current_channel + 1) % self.result.get_number_of_channels()
                if self.adc_result_current_channel == 0:
                    self.result.set_xdata(self.adc_result_sample_counter, self.adc_result_sample_counter / self.result.get_sampling_rate())
                    self.adc_result_sample_counter += 1

        # Error_Result
        elif self.__filetype == ResultReader.ERROR_TYPE:
            tmp_string = self.result.get_error_message()
            if tmp_string is None: tmp_string = ""

            tmp_string += in_cdata
            self.result.set_error_message(tmp_string)

        # Temp_Results
        elif self.__filetype == ResultReader.TEMP_TYPE:
            pass

        # Config_Results
        elif self.__filetype == ResultReader.CONFIG_TYPE:
            pass



    def __xmlEndTagFound(self, in_name):
        if in_name == "result":

            # ADC_Result
            if self.__filetype == ResultReader.ADCDATA_TYPE:
                if self.adc_result_trailing_chars!="":
                    self.__xmlCharacterDataFound(" ")
                self.__gui.new_log_message("Result Reader: Successfully parsed and saved %s" % os.path.join(self.path, self.filename % self.files_read), "DH")

            # Error_Result
            elif self.__filetype == ResultReader.ERROR_TYPE:
                self.__gui.new_log_message("Result Reader: Error Result parsed! (%s)" % os.path.join(self.path, self.filename % self.files_read), "DH")

            # Temp_Result
            elif self.__filetype == ResultReader.TEMP_TYPE:
                self.__gui.new_log_message("ResultReader: Temperature Result parsed!", "DH")

            # Config_Result
            elif self.__filetype == ResultReader.CONFIG_TYPE:
                self.__gui.new_log_message("ResultReader: Config Result parsed!", "DH")

            self.result_queue.append(self.result)   



    def __do_reset(self):

        if len(self.result_queue) != 0:
            self.__gui.new_log_message("ResultReader Warning: Deleting %d results due reset-request!" % len(self.result_queue), "DH")
        
        # The result-queue
        self.result_queue = []
        
        # Quit main loop?
        self.quit_loop = False

        # Indicates thread being active (= reading, parsing, giving back results)
        self.is_idling = False

        # How many files have been read?
        self.files_read = 0

        # When shall the thread be quit, if not files are found?
        self.timeout_counter = 0

        # class-intern result-object currently being processed
        self.tmp_result = None

        # Type of result-file - Error or Result?
        self.result_type = -1 # Nothing so far

        # Setting back reset
        self.__reset = False

    # /Private Methods -----------------------------------------------------------------------------

    # Public Methods -------------------------------------------------------------------------------

    def start(self):
        "Overwritten start-method - allowing a reset ResultReader to be started again"
        if self.__started:
            self.event_lock.set()
        else:
            self.event_lock.set()
            threading.Thread.start(self)



    def get_next_result(self):
        "Returns the next result in queue"

        if len(self.result_queue) == 0: return None
        else:
            out_result = self.result_queue[0]
            del self.result_queue[0]
            
            if self.get_number_of_results_pending() < self.max_result_queue_length and not self.event_lock.isSet():
                self.event_lock.set()
            
            return out_result



    def get_number_of_results_pending(self):
        "Returns the number of results waiting to be returned"
        return len(self.result_queue)

    

    def quit_result_reader(self):
        self.quit_loop = True
        self.event.set()
        self.event_lock.set()


    
    def is_running(self):
        "Returns true, if the ResultReader is still running (no matter if idling or parsing etc...)"
        return not self.quit_loop



    def get_number_of_results_read(self):
        "Returns the number of result-files read"
        return self.files_read + 0



    def reset(self):
        "Resets the result reader, so it can be started again"
        # Waiting for ResultReader to finish parsing the current file (look into run() method)
        self.__reset = True


    def connect_gui(self, gui):
        "Connects the gui"
        self.__gui = gui

    # /Public Methods ------------------------------------------------------------------------------
