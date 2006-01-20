# -*- coding: iso-8859-1 -*-

#############################################################################
#                                                                           #
# Name: Class ResultReader                                                  #
#                                                                           #
#############################################################################

import os
import os.path
import glob
import time
import sys
import base64
import numarray
import xml.parsers.expat
from datetime import datetime

from ADC_Result import ADC_Result
from Error_Result import Error_Result
from Temp_Result import Temp_Result
from Config_Result import Config_Result

class ResultReader:
    """
    starts at some point and returns result objects until none are there
    """
    CONFIG_TYPE = 3
    TEMP_TYPE = 2
    ADCDATA_TYPE = 1
    ERROR_TYPE = 0

    def __init__(self, spool_dir=".", no=0, result_pattern="job.%09d.result", clear_jobs=False, clear_results=False):
        self.spool_dir = spool_dir
        self.start_no = no
        self.no = self.start_no
        self.result_pattern = result_pattern
        self.clear_jobs=clear_jobs
        self.clear_results=clear_results

    def __iter__(self):
        """
        get next job with iterator
        """
        expected_filename=os.path.join(self.spool_dir,self.result_pattern%(self.no))
        while os.access(expected_filename,os.R_OK):
            yield self.get_result_object(expected_filename)
            # purge result file
            if self.clear_results:
                if os.path.isfile(expected_filename): os.remove(expected_filename)
            if self.clear_jobs:
                if os.path.isfile(expected_filename[:-7]): os.remove(expected_filename[:-7])
            self.no+=1
            expected_filename=os.path.join(self.spool_dir,self.result_pattern%(self.no))
        return

    def get_result_object(self, in_filename):
        """
        get result object
        """
        # class-intern result-object currently being processed
        result_file = file(in_filename, "r")

        # get date of last modification 
        self.result_job_date = datetime.fromtimestamp(os.stat(in_filename)[8])
            
        self.__parseFile(result_file)
        
        result_file = None

        r=self.result
        self.result = None

        return r

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

        try:
            # Parsing all cdata as one block
            self.xml_parser.buffer_text = True
            buffersize=self.xml_parser.buffer_size*2
            databuffer=in_file.read(buffersize)
            while databuffer!="":
                self.xml_parser.Parse(databuffer,False)
                databuffer=in_file.read(buffersize)

            self.xml_parser.Parse("",True)
        except xml.parsers.expat.ExpatError:
            print "ToDo: proper parser error message"
            self.result = None
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
            self.try_base64 = True

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
            if self.try_base64:
                self.adc_result_trailing_chars+=in_cdata
            else:
                try:
                    values=(self.adc_result_trailing_chars+in_cdata).split()
                    if not in_cdata[-1].isspace():
                        self.adc_result_trailing_chars=values.pop()
                    else:
                        self.adc_result_trailing_chars=""

                        for i in values:
                            self.result.set_ydata(self.adc_result_current_channel, self.adc_result_sample_counter, int(i))
                            # print "added value " + str(i) + " at: " + str(self.current_channel) + ", " + str(self.current_pos)
                            self.adc_result_current_channel = (self.adc_result_current_channel + 1) % self.result.get_number_of_channels()
                            if self.adc_result_current_channel == 0:
                                self.result.set_xdata(self.adc_result_sample_counter, self.adc_result_sample_counter / self.result.get_sampling_rate())
                                self.adc_result_sample_counter += 1
                except ValueError:
                    self.try_base64=True
                    self.adc_result_trailing_chars=in_cdata

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
        if in_name == "adcdata":

            # ADC_Result
            if self.__filetype == ResultReader.ADCDATA_TYPE:
                if self.try_base64:
                    tmp_string=base64.standard_b64decode(self.adc_result_trailing_chars)
                    self.adc_result_trailing_chars=""
                    tmp=numarray.fromstring(tmp_string, numarray.Int16,(len(tmp_string)/2))
                    del tmp_string
                    sys.stdout.flush()
                    self.result.x[self.adc_result_sample_counter:]=(numarray.arange(tmp.size()/2)+self.adc_result_sample_counter)/self.result.get_sampling_rate()
                    self.result.y[0][self.adc_result_sample_counter:]=tmp[::2]
                    self.result.y[1][self.adc_result_sample_counter:]=tmp[1::2]
                    self.adc_result_sample_counter+=tmp.size()/2
                else:
                    if self.adc_result_trailing_chars!="":
                        self.__xmlCharacterDataFound(" ")
            return

        elif in_name == "result":
            pass

        # Error_Result
        elif self.__filetype == ResultReader.ERROR_TYPE:
            pass

        # Temp_Result
        elif self.__filetype == ResultReader.TEMP_TYPE:
            pass

        # Config_Result
        elif self.__filetype == ResultReader.CONFIG_TYPE:
            pass

class BlockingResultReader(ResultReader):
    """
    to follow an active result stream
    """

    def __init__(self, spool_dir=".", no=0, result_pattern="job.%09d.result", clear_jobs=False, clear_results=False):
        ResultReader.__init__(self, spool_dir, no, result_pattern, clear_jobs=clear_jobs, clear_results=clear_results)
        self.quit_flag=False # asychronous quit flag
        self.stop_no=None # end of job queue
        self.poll_time=0.1 # sleep interval for polling results, <0 means no polling and stop

    def __iter__(self):
        """
        get next job with iterator
        block until result is available
        """
        expected_filename=os.path.join(self.spool_dir,self.result_pattern%(self.no))
        while not self.quit_flag and (self.stop_no is None or self.stop_no>self.no):
            if not os.access(expected_filename,os.R_OK):
                # stop polling, if required
                if self.poll_time<0:
                    break
                time.sleep(self.poll_time)
                continue
            r=self.get_result_object(expected_filename)
            if self.quit_flag: break
            yield r
            if self.clear_results:
                if os.path.isfile(expected_filename): os.remove(expected_filename)
            if self.clear_jobs:
                if os.path.isfile(expected_filename[:-7]): os.remove(expected_filename[:-7])
            if self.quit_flag: break
            self.no+=1
            expected_filename=os.path.join(self.spool_dir,self.result_pattern%(self.no))
            
        return

    def quit(self):
        self.quit_flag=True
