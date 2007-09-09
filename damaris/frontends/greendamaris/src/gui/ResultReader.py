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
import numpy
import xml.parsers.expat
import threading
from datetime import datetime

from damaris.data import ADC_Result
from damaris.data import Error_Result
from damaris.data import Temp_Result
from damaris.data import Config_Result

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
        self.quit_flag=threading.Event() # asychronous quit flag

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
        retries=0
        result_file=None
        while result_file is None:
            try:
                result_file = file(in_filename, "r")
            except IOError, e:
                if retries>10:
                    raise e
                print e, "retry", retries
                time.sleep(0.05)
                retries+=1

        # get date of last modification 
        self.result_job_date = datetime.fromtimestamp(os.stat(in_filename)[8])
            
        self.__parseFile(result_file)

        result_file.close()
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
        self.element_stack=[]

        try:
            # short version, but pyexpat buffers are awfully small
            # self.xml_parser.ParseFile(in_file)
            # read all, at least try
            databuffer=in_file.read(-1)
            # test wether really everything was read...
            databuffer2=in_file.read(self.xml_parser.buffer_size)
            if databuffer2=="":
                # parse everything at once
                self.xml_parser.Parse(databuffer,True)
            else:
                # do the first part ...
                self.xml_parser.Parse(databuffer,False)
                databuffer=databuffer2
                # ... and again and again
                while databuffer!="":
                    self.xml_parser.Parse(databuffer,False)
                    databuffer=in_file.read(-1)
                self.xml_parser.Parse("",True)
        except xml.parsers.expat.ExpatError, e:
            print "result file %d: xml parser '%s' error at line %d, offset %d"%(self.no,
                                                                                 xml.parsers.expat.ErrorString(e.code),
                                                                                 e.lineno,
                                                                                 e.offset)
            self.result = None

        del databuffer
        self.xml_parser.StartElementHandler=None
        self.xml_parser.EndElementHandler=None
        self.xml_parser.CharacterDataHandler=None
        del self.xml_parser

        # prepare result data
        if self.result is not None and \
               self.__filetype == ResultReader.ADCDATA_TYPE and \
               self.adc_result_sample_counter>0:
            # fill the ADC_Result with collected data
            self.result.x=numpy.arange(self.adc_result_sample_counter, dtype="Float64")/\
                                                        self.result.get_sampling_rate()
            self.result.y=[]
            self.result.index=[]
            for i in xrange(2):
                self.result.y.append(numpy.empty((self.adc_result_sample_counter,), dtype="Int16"))
            tmp_sample_counter=0
            while self.adc_result_parts:
                tmp_part=self.adc_result_parts.pop(0)
                tmp_size=tmp_part.size/2
                self.result.y[0][tmp_sample_counter:tmp_sample_counter+tmp_size]=tmp_part[::2]
                self.result.y[1][tmp_sample_counter:tmp_sample_counter+tmp_size]=tmp_part[1::2]
                self.result.index.append((tmp_sample_counter,tmp_sample_counter+tmp_size-1))
                tmp_sample_counter+=tmp_size
            self.result.cont_data=True

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

            self.adc_result_trailing_chars = ""

            if self.result is None:
                self.result = ADC_Result()
                # None: new guess for adc data encoding
                # "a": ascii
                # "b": base64
                self.adc_data_encoding = None

                self.result.set_sampling_rate(float(in_attribute["rate"]))
                self.result.set_job_id(self.result_job_number)
                self.result.set_job_date(self.result_job_date)

                self.result.set_description_dictionary(self.result_description.copy())
                title="ADC-Result: job-id=%d"%int(self.result_job_number)
                if len(self.result_description)>0:
                    for k,v in self.result_description.iteritems():
                        title+=", %s=%s"%(k,v)
                self.result.set_title(title)
                self.result_description=None
                self.adc_result_sample_counter = 0
                self.adc_result_parts=[] # will contain arrays of sampled intervals, assumes same sample rate
            else:
                if float(in_attribute["rate"])!=self.result.get_sampling_rate():
                    print "sample rate different in ADC_Result, found %f, former value %f"%\
                          (float(in_attribute["rate"]),self.result.get_sampling_rate())
            new_samples=int(in_attribute["samples"])
            self.adc_result_sample_counter += new_samples

            # version depends on the inclusion of http://bugs.python.org/issue1137
            if sys.hexversion>=0x020501f0:
                # extend buffer to expected base64 size (2 channels, 2 byte)
                required_buffer=int(new_samples*4/45+1)*62
                if self.xml_parser.buffer_size < required_buffer:
                    try:
                        self.xml_parser.buffer_size=required_buffer
                    except AttributeError:
                        pass

            # pass all chardata as one block
            self.xml_parser.buffer_text = True
            # do not change the contents
            self.xml_parser.returns_unicode=False

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
            
        # maintain the stack
        self.element_stack.append(in_name)
    
    def __xmlCharacterDataFound(self, in_cdata):

        # ADC_Result
        if self.__filetype == ResultReader.ADCDATA_TYPE and self.element_stack[-1]=="adcdata":
            self.adc_result_trailing_chars+=in_cdata

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

        # maintain the stack
        self.element_stack.pop()

        if in_name == "adcdata":

            # ADC_Result
            if self.__filetype == ResultReader.ADCDATA_TYPE:
                # detect type of data encoding from first line
                if self.adc_data_encoding is None:
                    self.adc_result_trailing_chars=self.adc_result_trailing_chars.strip()
                    first_line_end=self.adc_result_trailing_chars.find("\n")
                    first_line=""
                    if first_line_end!=-1:
                        first_line=self.adc_result_trailing_chars[:first_line_end]
                    else:
                        first_line=self.adc_result_trailing_chars
                    if len(first_line.lstrip("-0123456789 \t\n\r"))==0:
                        try:
                            map(int,filter(len,first_line.split()))
                        except ValueError,e:
                            pass
                        else:
                            self.adc_data_encoding="a"
                    if self.adc_data_encoding is None and len(first_line)%4==0:
                        try:
                            base64.standard_b64decode(first_line)
                        except TypeError:
                            pass
                        else:
                            self.adc_data_encoding="b"
                    if self.adc_data_encoding is None:
                        print "unknown ADC data format \"%s\""%first_line
                
                tmp=None
                if self.adc_data_encoding=="a":
                    values=map(int,self.adc_result_trailing_chars.split())
                    tmp=numpy.array(values, dtype="Int16")
                elif self.adc_data_encoding=="b":
                    tmp_string=base64.standard_b64decode(self.adc_result_trailing_chars)
                    tmp=numpy.fromstring(tmp_string, dtype="Int16")
                    del tmp_string
                else:
                    print "unknown ADC data format"

                self.adc_result_trailing_chars=""
                self.adc_result_parts.append(tmp)
                del tmp
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
        self.stop_no=None # end of job queue
        self.poll_time=0.1 # sleep interval for polling results, <0 means no polling and stop
        self.in_advance=0

    def __iter__(self):
        """
        get next job with iterator
        block until result is available
        """
        expected_filename=os.path.join(self.spool_dir,self.result_pattern%(self.no))
        while (not self.quit_flag.isSet()) and (self.stop_no is None or self.stop_no>self.no):
            if not os.access(expected_filename,os.R_OK):
                # stop polling, if required
                if self.poll_time<0: break
                self.quit_flag.wait(self.poll_time)
                continue

            # find pending results
            self.in_advance=max(self.no,self.in_advance)
            in_advance_filename=os.path.join(self.spool_dir,self.result_pattern%(self.in_advance+1))
            while os.access(in_advance_filename, os.R_OK) and (self.stop_no is None or self.stop_no>self.in_advance+1):
                # do not more than 100 results in advance at one glance
                if self.in_advance>self.no+100: break
                self.in_advance+=1
                in_advance_filename=os.path.join(self.spool_dir,self.result_pattern%(self.in_advance+1))

            if self.quit_flag.isSet(): break
            r=self.get_result_object(expected_filename)
            if self.quit_flag.isSet(): break
            
            if self.quit_flag.isSet(): break
            yield r
            
            if self.clear_results:
                if os.path.isfile(expected_filename): os.remove(expected_filename)
            if self.clear_jobs:
                if os.path.isfile(expected_filename[:-7]): os.remove(expected_filename[:-7])

            self.no+=1
            expected_filename=os.path.join(self.spool_dir,self.result_pattern%(self.no))

        return

    def quit(self):
        self.quit_flag.set()
