# -*- coding: iso-8859-1 -*-

from Resultable import Resultable
from Drawable import Drawable
from Accumulation import Accumulation

import threading
import numarray
import sys
from types import *
import tables
#############################################################################
#                                                                           #
# Name: Class ADC_Result                                                    #
#                                                                           #
# Purpose: Specialised class of Resultable and Drawable                     #
#          Contains recorded ADC Data                                       #
#                                                                           #
#############################################################################

class ADC_Result(Resultable, Drawable):
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
            self.y.append(numarray.zeros((samples,), type="Int16"))

        self.x = numarray.zeros((samples,), type="Float64")

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

        self.x = numarray.resize(self.x, (length+samples))

        for i in range(self.get_number_of_channels()):
            self.y[i] = numarray.resize(self.y[i], (length+samples))

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


    def get_index_bounds(self, index):
        "Returns a tuple with (start, end) of the wanted result"
        return self.index[index]

    def uses_statistics(self):
        return False

    def write_as_csv(self, destination=sys.stdout):
        """
        writes the data to a file or to sys.stdout
        destination can be a file or a filename
        suitable for further processing
        """
        # write sorted
        the_destination=None
        if isinstance(destination,FileType):
            the_destination=destination
        elif isinstance(destination,StringTypes):
            the_destination=file(destination,"w")
        else:
            raise Exception("sorry destination %s is not valid"%(repr(destination)))

        the_destination.write("# adc_result\n")
        the_destination.write("# t y0 y1 ...\n")
        self.lock.acquire()
        try:
            xdata=self.get_xdata()
            ch_no=self.get_number_of_channels()
            ydata=map(self.get_ydata, xrange(ch_no))
            #yerr=map(self.get_yerr, xrange(ch_no))
            for i in xrange(len(xdata)):
                the_destination.write("%g"%xdata[i])
                for j in xrange(ch_no):
                    the_destination.write(" %g"%(ydata[j][i]))
                the_destination.write("\n")
            the_destination=None
            xdata=ydata=None
        finally:
            self.lock.release()

    def write_to_hdf(self, hdffile, where, name, title, compress=None):
        accu_group=hdffile.createGroup(where=where,name=name,title=title)
        accu_group._v_attrs.damaris_type="ADC_result"
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
                # save channel data
                for channel_no in xrange(len(self.y)):
                    y_mean=self.get_ydata(channel_no)
                    for index_no in xrange(len(self.index)):
                        index=self.index[index_no]
                        # set time data
                        timedata=numarray.array(y_mean[index[0]:index[1]+1],
                                                type = numarray.Int32)
                        
                        time_slice_data=None
                        if compress is not None:
                            chunkshape = numarray.shape(timedata)
                            chunkshape = (min(chunkshape[0],1024*8),chunkshape[1])
                            prefered_complib='lzo'
                            if tables.whichLibVersion(prefered_complib) is None:
                                prefered_complib='zlib' #builtin
                            time_slice_data=hdffile.createCArray(accu_group,
                                                                 name="idx%04d_ch%04d"%(index_no,channel_no),
                                                                 shape=timedata.getshape(),
                                                                 atom=tables.Int32Atom(shape=chunkshape),
                                                                                       flavor="numarray"),
                                                                 filters=tables.Filters(complevel=compress,
                                                                                        complib=prefered_complib),
                                                                 title="Index %d, Channel %d"%(index_no,channel_no))
                            time_slice_data[:]=timedata
                        else:
                            time_slice_data=hdffile.createArray(accu_group,
                                                                name="idx%04d_ch%04d"%(index_no,channel_no),
                                                                object=timedata,
                                                                title="Index %d, Channel %d"%(index_no,channel_no))

                        timedata=None
                        # set attributes
                        time_slice_data._f_setAttr("index",numarray.array(index_no, type=numarray.Int32))
                        time_slice_data._f_setAttr("channel",numarray.array(channel_no, type=numarray.Int32))
                        time_slice_data._f_setAttr("dwelltime",numarray.array(1.0/self.sampling_rate,
                                                                               type=numarray.Float64))
                        time_slice_data._f_setAttr("start_time",numarray.array(1.0/self.sampling_rate*index[0],
                                                                                type=numarray.Float64))
            finally:
                time_slice_data=None
                accu_group=None
                self.lock.release()

    # Überladen von Operatoren und Built-Ins -------------------------------------------------------

    def __len__(self):
        "Redefining len(ADC_Result obj), returns the number of samples in one channel"
        return len(self.y[0])

    def __repr__(self):
        tmp_string = "X:                   " + repr(self.x) + "\n"

        for i in range(self.get_number_of_channels()):
            tmp_string += ("Y(%d):                " % i) + repr(self.y[i]) + "\n"

        tmp_string += "Indexes:             " + str(self.index) + "\n"

        tmp_string += "Job ID:              " + str(self.job_id) + "\n"
        tmp_string += "Job Date:            " + str(self.job_date) + "\n"
        tmp_string += "Description:         " + str(self.description) + "\n"
        tmp_string += "Samples per Channel: " + str(len(self.y[0])) + "\n"
        tmp_string += "Samplingfrequency:   " + str(self.sampling_rate)

        return tmp_string


    def __add__(self, other):
        "Redefining self + other"
        if isinstance(other, IntType) or isinstance(other, FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numarray.array(self.y[i], type="Float64") + other)

            r = ADC_Result(x = self.x+0, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
            return r
        else:
            raise ValueError("ValueError: Cannot add \"%s\" to ADC-Result!") % str(other.__class__)


    def __radd__(self, other):
        "Redefining other + self"
        return self.__add__(other)


    def __sub__(self, other):
        "Redefining self - other"
        if isinstance(other, IntType) or isinstance(other, FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numarray.array(self.y[i], type="Float64") - other)

            r = ADC_Result(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
            return r
        else:
            raise ValueError("ValueError: Cannot subtract \"%s\" to ADC-Result!") % str(other.__class__)


    def __rsub__(self, other):
        "Redefining other - self"
        if isinstance(other, IntType) or isinstance(other, FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(other - numarray.array(self.y[i], type="Float64"))

            r = ADC_Result(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
            return r

        else:
            raise ValueError("ValueError: Cannot subtract \"%s\" to ADC-Result!") % str(other.__class__)


    def __mul__(self, other):
        "Redefining self * other"
        if isinstance(other, IntType) or isinstance(other, FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numarray.array(self.y[i], type="Float64") * other)

            r = ADC_Result(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
        else:
            raise ValueError("ValueError: Cannot multiply \"%s\" to ADC-Result!") % str(other.__class__)    


    def __rmul__(self, other):
        "Redefining other * self"
        return self.__mul__(other)


    def __pow__(self, other):
        "Redefining self ** other"
        if isinstance(other, IntType) or isinstance(other, FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numarray.array(self.y[i], type="Float64") ** other)

            r = ADC_Result(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
            return r
        else:
            raise ValueError("ValueError: Cannot multiply \"%s\" to ADC-Result!") % str(other.__class__)

        
    def __div__(self, other):
        "Redefining self / other"
        if isinstance(other, IntType) or isinstance(other, FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numarray.array(self.y[i], type="Float64") / other)

            r = ADC_Result(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
            return r
        else:
            raise ValueError("ValueError: Cannot multiply \"%s\" to ADC-Result!") % str(other.__class__)


    def __rdiv__(self, other):
        "Redefining other / self"
        if isinstance(other, IntType) or isinstance(other, FloatType):
            self.lock.acquire()
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(other / numarray.array(self.y[i], type="Float64"))

            r = ADC_Result(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
            self.lock.release()
        else:
            raise ValueError("ValueError: Cannot multiply \"%s\" to ADC-Result!") % str(other.__class__)


    def __neg__(self):
        "Redefining -self"
        self.lock.acquire()
        tmp_y = []

        for i in range(self.get_number_of_channels()):
            tmp_y.append(numarray.array(-self.y[i]))

        r = ADC_Result(x = numarray.array(self.x), y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)
        self.lock.release()
        return r
