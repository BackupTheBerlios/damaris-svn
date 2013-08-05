# -*- coding: iso-8859-1 -*-

from Resultable import Resultable
from Drawable import Drawable
from Signalpath import Signalpath
from DamarisFFT import DamarisFFT
import threading
import numpy
import sys
import types
import datetime
import tables
#############################################################################
#                                                                           #
# Name: Class ADC_Result                                                    #
#                                                                           #
# Purpose: Specialised class of Resultable and Drawable                     #
#          Contains recorded ADC Data                                       #
#                                                                           #
#############################################################################

class ADC_Result(Resultable, Drawable, DamarisFFT, Signalpath):
    def __init__(self, x = None, y = None, index = None, sampl_freq = None, desc = None, job_id = None, job_date = None):
        Resultable.__init__(self)
        Drawable.__init__(self)
     
        # Title of this accumulation: set Values: Job-ID and Description (plotted in GUI -> look Drawable)
        # Is set in ResultReader.py (or in copy-construktor)
        self.__title_pattern = "ADC-Result: job_id = %s, desc = %s"
        
        # Axis-Labels (inherited from Drawable)
        self.xlabel = "Time (s)"
        self.ylabel = "Samples [Digits]"
        self.lock=threading.RLock()
        self.nChannels = 0

        if (x is None) and (y is None) and (index is None) and (sampl_freq is None) and (desc is None) and (job_id is None) and (job_date is None):
            self.cont_data = False
            self.sampling_rate = 0
            self.index = []
            self.x = []
            self.y = []

        elif (x is not None) and (y is not None) and (index is not None) and (sampl_freq is not None) and (desc is not None) and (job_id is not None) and (job_date is not None):
            self.x = x
            self.y = y
            self.index = index
            self.sampling_rate = sampl_freq
            self.cont_data = True
            self.description = desc
            self.job_id = job_id
            self.job_date = job_date
            title="ADC-Result: job-id=%d"%int(self.job_id)
            if len(self.description)>0:
                for k,v in self.description.iteritems():
                    title+=", %s=%s"%(k,v)
            self.set_title(title)

        else:
            raise ValueError("Wrong usage of __init__!")


    def create_data_space(self, channels, samples):
        "Initialises the internal data-structures"

        if self.contains_data():
            print "Warning ADC-Result: Tried to run \"create_data_space()\" more than once."
            return
        
        if channels <= 0: raise ValueError("ValueError: You cant create an ADC-Result with less than 1 channel!")
        if samples <= 0: raise ValueError("ValueError: You cant create an ADC-Result with less than 1 sample!")
        
        for i in range(channels):
            self.y.append(numpy.zeros((samples,), dtype="Int16"))

        self.x = numpy.zeros((samples,), dtype="Float64")

        self.index.append((0, samples-1))
        self.cont_data = True


    def contains_data(self):
        "Returns true if ADC_Result contains data. (-> create_data_space() was called)"
        return self.cont_data


    def add_sample_space(self, samples):
        "Adds space for n samples, where n can also be negative (deletes space). New space is filled up with \"0\""

        self.lock.acquire()

        if not self.cont_data:
            print "Warning ADC-Result: Tried to resize empty array!"
            return

        length = len(self.y[0])

        self.x = numpy.resize(self.x, (length+samples))

        for i in range(self.get_number_of_channels()):
            self.y[i] = numpy.resize(self.y[i], (length+samples))

        self.index.append((length, len(self.y[0])-1))
        self.lock.release()


    def get_result_by_index(self, index):

        self.lock.acquire()
        try:
            start = self.index[index][0]
            end = self.index[index][1]
        except:
            self.lock.release()
            raise

        tmp_x = self.x[start:end+1].copy()
        tmp_y = []

        for i in range(self.get_number_of_channels()):
            tmp_y.append(self.y[i][start:end+1].copy())

        r = ADC_Result(x = tmp_x, y = tmp_y,  index = [(0,len(tmp_y[0])-1)], sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
        self.lock.release()
        return r


    def get_sampling_rate(self):
        "Returns the samplingfrequency"
        return self.sampling_rate + 0


    def set_sampling_rate(self, hz):
        "Sets the samplingfrequency in hz"
        self.sampling_rate = float(hz)    
        
        
    def get_nChannels(self):
        "Gets the number of channels"
        return self.nChannels + 0   
        
    def set_nChannels(self, channels):
        "Sets the number of channels"
        self.nChannels = int(channels)


    def get_index_bounds(self, index):
        "Returns a tuple with (start, end) of the wanted result"
        return self.index[index]

    def uses_statistics(self):
        return False

    def write_to_csv(self, destination=sys.stdout, delimiter=" "):
        """
        writes the data to a file or to sys.stdout
        destination can be a file or a filename
        suitable for further processing
        """
        # write sorted
        the_destination=destination
        if type(destination) in types.StringTypes:
            the_destination=file(destination, "w")

        the_destination.write("# adc_result\n")
        the_destination.write("# t y0 y1 ...\n")
        self.lock.acquire()
        try:
            xdata=self.get_xdata()
            ch_no=self.get_number_of_channels()
            ydata=map(self.get_ydata, xrange(ch_no))
            #yerr=map(self.get_yerr, xrange(ch_no))
            for i in xrange(len(xdata)):
                the_destination.write("%e"%xdata[i])
                for j in xrange(ch_no):
                    the_destination.write("%s%e"%(delimiter, ydata[j][i]))
                the_destination.write("\n")
            the_destination=None
            xdata=ydata=None
        finally:
            self.lock.release()

    def write_to_simpson(self, destination=sys.stdout, delimiter=" "):
        """
        writes the data to a text file or sys.stdout in Simpson format,
        for further processing with the NMRnotebook software;
        destination can be a file or a filename
        """
        # write sorted
        the_destination=destination
        if type(destination) in types.StringTypes:
            the_destination=file(destination, "w")
  
        self.lock.acquire()
        try:
            xdata=self.get_xdata()
            the_destination.write("SIMP\n")
            the_destination.write("%s%i%s"%("NP=", len(xdata), "\n"))
            the_destination.write("%s%i%s"%("SW=", self.get_sampling_rate(), "\n"))
            the_destination.write("TYPE=FID\n")
            the_destination.write("DATA\n")
            ch_no=self.get_number_of_channels()
            ydata=map(self.get_ydata, xrange(ch_no))
            for i in xrange(len(xdata)):
                for j in xrange(ch_no):
                    the_destination.write("%g%s"%(ydata[j][i], delimiter))
                the_destination.write("\n")
            the_destination.write("END\n")
            the_destination=None
            xdata=ydata=None
        finally:
            self.lock.release()        

    def write_to_hdf(self, hdffile, where, name, title, complib=None, complevel=None):
        accu_group=hdffile.createGroup(where=where,name=name,title=title)
        accu_group._v_attrs.damaris_type="ADC_Result"
        if self.contains_data():
            self.lock.acquire()
            try:
                # save time stamps
                if "job_date" in dir(self) and self.job_date is not None:
                    accu_group._v_attrs.time="%04d%02d%02d %02d:%02d:%02d.%03d"%(self.job_date.year,
                                                                                 self.job_date.month,
                                                                                 self.job_date.day,
                                                                                 self.job_date.hour,
                                                                                 self.job_date.minute,
                                                                                 self.job_date.second,
                                                                                 self.job_date.microsecond/1000)

                if self.description is not None:
                    for (key,value) in self.description.iteritems():
                        accu_group._v_attrs.__setattr__("description_"+key,str(value))

                # save interval information
                filter=None
                if complib is not None:
                    if complevel is None:
                        complevel=9
                    filter=tables.Filters(complevel=complevel,complib=complib,shuffle=1)

                index_table=hdffile.createTable(where=accu_group,
                                                name="indices",
                                                description={"start": tables.UInt64Col(),
                                                             "length": tables.UInt64Col(),
                                                             "start_time": tables.Float64Col(),
                                                             "dwelltime": tables.Float64Col()},
                                                title="indices of adc data intervals",
                                                filters=filter,
                                                expectedrows=len(self.index))
                index_table.flavor="numpy"
                # save channel data
                new_row=index_table.row
                for i in xrange(len(self.index)):
                    new_row["start"]=self.index[i][0]
                    new_row["dwelltime"]=1.0/self.sampling_rate
                    new_row["start_time"]=1.0/self.sampling_rate*self.index[i][0]
                    new_row["length"]=self.index[i][1]-self.index[i][0]+1
                    new_row.append()

                index_table.flush()
                new_row=None
                index_table=None

                # prepare saving data
                channel_no=len(self.y)
                timedata=numpy.empty((len(self.y[0]),channel_no),
                                     dtype = "Int32")
                for ch in xrange(channel_no):
                    timedata[:,ch]=self.get_ydata(ch)
                
                # save data
                time_slice_data=None
                if filter is not None:
                    chunkshape = numpy.shape(timedata)
                    if len(chunkshape) <= 1:
                        chunkshape = (min(chunkshape[0],1024*8),)
                    else:
                        chunkshape = (min(chunkshape[0],1024*8), chunkshape[1])
                    if tables.__version__[0]=="1":
                        time_slice_data=hdffile.createCArray(accu_group,
                                                             name="adc_data",
                                                             shape=timedata.shape,
                                                             atom=tables.Int32Atom(shape=chunkshape,
                                                                                   flavor="numpy"),
                                                             filters=filter,
                                                             title="adc data")
                    else:
                        time_slice_data=hdffile.createCArray(accu_group,
                                                             name="adc_data",
                                                             shape=timedata.shape,
                                                             chunkshape=chunkshape,
                                                             atom=tables.Int32Atom(),
                                                             filters=filter,
                                                             title="adc data")
                    time_slice_data[:]=timedata
                else:
                    time_slice_data=hdffile.createArray(accu_group,
                                                        name="adc_data",
                                                        object=timedata,
                                                        title="adc data")

            finally:
                timedata=None
                time_slice_data=None
                accu_group=None
                self.lock.release()

    # Überladen von Operatoren und Built-Ins -------------------------------------------------------

    def __len__(self):
        "Redefining len(ADC_Result obj), returns the number of samples in one channel and 0 without data"
        if len(self.y)>0:
            return len(self.y[0])
        return 0

    def __repr__(self):
        """
        writes job meta data and data to string returned
        """
        tmp_string  = "Job ID:              " + str(self.job_id) + "\n"
        tmp_string += "Job Date:            " + str(self.job_date) + "\n"
        tmp_string += "Description:         " + str(self.description) + "\n"
        if len(self.y)>0:
            tmp_string += "Indexes:             " + str(self.index) + "\n"
            tmp_string += "Samples per Channel: " + str(len(self.y[0])) + "\n"
            tmp_string += "Samplingfrequency:   " + str(self.sampling_rate) + "\n"
            tmp_string += "X:                   " + repr(self.x) + "\n"
            for i in range(self.get_number_of_channels()):
                tmp_string += ("Y(%d):                " % i) + repr(self.y[i]) + "\n"

        return tmp_string

    def __add__(self, other):
        "Redefining self + other (scalar)"
        if isinstance(other, types.IntType) or isinstance(other, types.FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numpy.array(self.y[i], dtype="Float64") + other)

            r = ADC_Result(x = self.x[:], y = tmp_y, index = self.index[:], sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
            return r
        else:
            raise ValueError("ValueError: Cannot add \"%s\" to ADC-Result!" % str(other.__class__))


    def __radd__(self, other):
        "Redefining other (scalar) + self"
        return self.__add__(other)


    def __sub__(self, other):
        "Redefining self - other (scalar)"
        if isinstance(other, types.IntType) or isinstance(other, types.FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numpy.array(self.y[i], dtype="Float64") - other)

            r = ADC_Result(x = self.x[:], y = tmp_y, index = self.index[:], sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
            return r
        else:
            raise ValueError("ValueError: Cannot subtract \"%s\" to ADC-Result!") % str(other.__class__)


    def __rsub__(self, other):
        "Redefining other (scalar) - self"
        if isinstance(other, types.IntType) or isinstance(other, types.FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(other - numpy.array(self.y[i], dtype="Float64"))

            r = ADC_Result(x = self.x[:], y = tmp_y, index = self.index[:], sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
            return r

        else:
            raise ValueError("ValueError: Cannot subtract \"%s\" to ADC-Result!") % str(other.__class__)


    def __mul__(self, other):
        "Redefining self * other (scalar)"
        if isinstance(other, types.IntType) or isinstance(other, types.FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numpy.array(self.y[i], dtype="Float64") * other)

            r = ADC_Result(x = self.x[:], y = tmp_y, index = self.index[:], sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
        else:
            raise ValueError("ValueError: Cannot multiply \"%s\" to ADC-Result!") % str(other.__class__)    


    def __rmul__(self, other):
        "Redefining other (scalar) * self"
        return self.__mul__(other)


    def __pow__(self, other):
        "Redefining self ** other (scalar)"
        if isinstance(other, types.IntType) or isinstance(other, types.FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numpy.array(self.y[i], dtype="Float64") ** other)

            r = ADC_Result(x = self.x[:], y = tmp_y, index = self.index[:], sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
            return r
        else:
            raise ValueError("ValueError: Cannot multiply \"%s\" to ADC-Result!") % str(other.__class__)

        
    def __div__(self, other):
        "Redefining self / other (scalar)"
        if isinstance(other, types.IntType) or isinstance(other, types.FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numpy.array(self.y[i], dtype="Float64") / other)

            r = ADC_Result(x = self.x[:], y = tmp_y, index = self.index[:], sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
            return r
        else:
            raise ValueError("ValueError: Cannot multiply \"%s\" to ADC-Result!") % str(other.__class__)


    def __rdiv__(self, other):
        "Redefining other (scalar) / self"
        if isinstance(other, types.IntType) or isinstance(other, types.FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(other / numpy.array(self.y[i], dtype="Float64"))

            r = ADC_Result(x = self.x[:], y = tmp_y, index = self.index[:], sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
        else:
            raise ValueError("ValueError: Cannot multiply \"%s\" to ADC-Result!") % str(other.__class__)


    def __neg__(self):
        "Redefining -self"
        self.lock.acquire()
        tmp_y = []

        for i in range(self.get_number_of_channels()):
            tmp_y.append(numpy.array(-self.y[i]))

        r = ADC_Result(x = self.x[:], y = tmp_y, index = self.index[:], sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
        self.lock.release()
        return r


def read_from_hdf(hdf_node):
    """
    read accumulation data from HDF node and return it.
    """

    # formal checks first
    if not isinstance(hdf_node, tables.Group):
        return None

    if hdf_node._v_attrs.damaris_type!="ADC_Result":
        return None

    if not (hdf_node.__contains__("indices") and hdf_node.__contains__("adc_data")):
        return None

    # job id and x,y titles are missing
    adc=ADC_Result()
    # populate description dictionary
    adc.description={}
    for attrname in hdf_node._v_attrs._v_attrnamesuser:
        if attrname.startswith("description_"):
            adc.description[attrname[12:]]=hdf_node._v_attrs.__getattr__(attrname)

    if "time" in dir(hdf_node._v_attrs):
        timestring=hdf_node._v_attrs.__getattr__("time")
        adc.job_date=datetime.datetime(int(timestring[:4]),  # year
                                       int(timestring[4:6]), # month
                                       int(timestring[6:8]), # day
                                       int(timestring[9:11]), # hour
                                       int(timestring[12:14]), # minute
                                       int(timestring[15:17]), # second
                                       int(timestring[18:21])*1000 # microsecond
                                       )
    

    # start with indices
    for r in hdf_node.indices.iterrows():
        adc.index.append((r["start"],r["start"]+r["length"]-1))
        adc.sampling_rate=1.0/r["dwelltime"]

    # now really belief there are no data
    if len(adc.index)==0:
        adc.cont_data=False
        return adc

    adc.cont_data=True
    # now do the real data
    adc_data=hdf_node.adc_data.read()
    
    adc.x=numpy.arange(adc_data.shape[0], dtype="Float64")/adc.sampling_rate

    for ch in xrange(adc_data.shape[1]):
        adc.y.append(adc_data[:,ch])

    return adc
