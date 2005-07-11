# -*- coding: iso-8859-1 -*-

import threading
import os
import xml.parsers.expat
from Result import *

class ResultReader(threading.Thread):

    RESULT_TYPE = 1
    ERROR_TYPE = 0

    def __init__(self, in_path, timeout = -1):
        threading.Thread.__init__(self, name = "Thread-ResultReader")

        # Directory to check for result-files
        self.path = in_path
        if not os.path.exists(self.path):
            raise IOError, "Error initialising ResultReader: Path does not exist"

        # The result-queue
        self.result_queue = []

        # Maximum Number of Results in Queue (if reached, ResultReader will sleep until results are read)
        self.max_result_queue_length = 1000
        
        # Filename stuff
        self.filename = "job.%09d.result"

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

        # Type of result-file - Error or Result?
        self.result_type = -1 # Nothing so far

        # ResultReader reset?
        self.__reset = False

        # ResultReader started?
        self.__started = False


 
    # Thread activity - checks for result files in the given directory
    def run(self):
        self.__started = True

        while not self.quit_loop:
            # Making ResultReader restartable and/or waiting for user to read pending results
            self.event_lock.wait()

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
                self.readFile(os.path.join(self.path, self.filename % self.files_read))
                self.timeout_counter = 0
            else:
                #print "No File found, waiting... (" + str(self.wait) + ")"
                self.is_idling = True
                self.event.wait(self.wait)
                self.timeout_counter += self.wait
                #print self.timeout_counter
                
                if self.timeout_counter >= self.timeout and self.timeout is not -1:
                    print "ResultReader: Timeout!\n"
                    #print "Printing result-queue..."
                    #print self.result_queue
                    self.quit_loop = True

                self.is_idling = False



    # Overwritten start-method (asuring "run()" is called only once)
    def start(self):
        if self.__started:
            self.event_lock.set()
        else:
            self.event_lock.set()
            threading.Thread.start(self)



    # Opens the file and hands it to the parser
    def readFile(self, in_filename):

        try:
            result_file = file(in_filename, "r")
            #print result_file
            
            self.parseFile(result_file)

            self.files_read += 1
            result_file.close()
            
        except IOError:
            print "File \"" + in_filename + "\" could not be opened"

        except:
            result_file.close()
            raise



    # Parses the file and adds it relevant data to the result-object
    def parseFile(self, in_file):

        self.result_description = {}
        self.current_channel = 0
        self.current_pos = 0

        # Expat XML-Parser & Binding handlers
        self.xml_parser = xml.parsers.expat.ParserCreate()
        self.xml_parser.StartElementHandler = self.xmlStartTagFound
        self.xml_parser.CharacterDataHandler = self.xmlCharacterDataFound
        self.xml_parser.EndElementHandler = self.xmlEndTagFound

        # Parsing all cdata as one block
        self.xml_parser.buffer_text = True
        self.xml_parser.ParseFile(in_file)


    # Callback when a xml start tag is found
    def xmlStartTagFound(self, in_name, in_attribute):

        # Beginning for the adcdata-section
        if in_name == "adcdata":
            if in_attribute.has_key("samples"):
                self.adcdata_samples = int(in_attribute["samples"])
                self.result_type = ResultReader.RESULT_TYPE
            else:
                print "Error parsing result-file: Missing number of recorded samples"
                return 
                
            if in_attribute.has_key("rate"):
                self.adcdata_rate = float(in_attribute["rate"])
            else:
                print "Error parsing result-file: Missing samplingrate"
                return

            # tmp-Result Objekt erstellen mit (Kanäle, Samples)
            self.tmp_result = Result(2, self.adcdata_samples)
            self.tmp_result.set_job_number(self.result_job_number)
            self.tmp_result.set_sampling_rate(self.adcdata_rate)

            key_list = self.result_description.keys()
            for key in key_list:
                self.tmp_result.add_description(key, self.result_description[key])


        elif in_name == "error":
            self.tmp_result = Result(1, 1)
            self.tmp_result.set_description("error", "no message")
        
        elif in_name == "result":
            if in_attribute.has_key("job"):
                self.result_job_number = int(in_attribute["job"])
            else:
                print "Error parsing result-file: Missing job-id"
                return


        elif in_name == "description":
                self.result_description = in_attribute.copy()

     

    def xmlCharacterDataFound(self, in_cdata):
        if not in_cdata.isspace():
            values = map(int, in_cdata.split())

            for i in values:
                self.tmp_result.set_value(self.current_channel, self.current_pos, i)
                #print "added value " + str(i) + " at: " + str(self.current_channel) + ", " + str(self.current_pos)
                self.current_channel = (self.current_channel + 1) % self.tmp_result.get_number_of_channels()
                if self.current_channel == 0:
                    self.tmp_result.set_xvalue(self.current_pos, self.current_pos / float(self.tmp_result.get_sampling_rate()))
                    self.current_pos += 1



    def xmlEndTagFound(self, in_name):
        if in_name == "adcdata":
            self.result_queue.append(self.tmp_result)
            print "Result Reader: Successfully parsed and saved %s" % os.path.join(self.path, self.filename % self.files_read)
            self.current_channel = 0
            self.current_pos = 0
            self.result_type = -1

           

    # Returns the next result from the queue
    def get_next_result(self):

        if len(self.result_queue) == 0: return None
        else:
            out_result = self.result_queue[0]
            del self.result_queue[0]
            
            if self.get_number_of_results_pending() < self.max_result_queue_length and not self.event_lock.isSet():
                print "True"
                self.event_lock.set()                
            
            return out_result


    # Returns the number of waiting elements
    def get_number_of_results_pending(self):
        return len(self.result_queue)

    

    def quit_result_reader(self):
        self.quit_loop = True


    
    def is_running(self):
        return not self.quit_loop


    # Ausbauwürdig und fraglich, ob nötig.
    def isIdle(self):
        return self.is_idling


    def get_number_of_results_read(self):
        return self.files_read + 0


    def reset(self):

        # Waiting for ResultReader to parse current file
        self.__reset = True
        

    def __do_reset(self):

        if len(self.result_queue) != 0:
            print "ResultReader Warning: Deleting %d results due reset-request!" % len(self.result_queue)
        
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
