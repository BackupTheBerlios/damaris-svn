# -*- coding: iso-8859-1 -*-

#############################################################################
#                                                                           #
# Name: Class Accumulation                                                  #
#                                                                           #
# Purpose: Specialised class of Errorable and Drawable                      #
#          Contains accumulated ADC-Data                                    #
#                                                                           #
#############################################################################

from Errorable import Errorable
from Drawable import Drawable
from DamarisFFT import DamarisFFT
from Signalpath import Signalpath
import sys
import threading
import types
import tables
import numpy
import datetime

class Accumulation(Errorable, Drawable, DamarisFFT, Signalpath):
    def __init__(self, x = None, y = None, y_2 = None, n = None, index = None, sampl_freq = None, error = False):
        Errorable.__init__(self)
        Drawable.__init__(self)

        # Title of this accumulation (plotted in GUI -> look Drawable)
        self.__title_pattern = "Accumulation: n = %d"

        # Axis-Labels (inherited from Drawable)
        self.xlabel = "Time (s)"
        self.ylabel = "Avg. Samples [Digits]"
        self.lock=threading.RLock()

        self.common_descriptions=None
        self.time_period=[]

        self.use_error = error

        if self.uses_statistics():
            if (y_2 is not None):
                self.y_square = y_2
            elif (y_2 is None) :
                self.y_square = []
            else:
                raise ValueError("Wrong usage of __init__!")
             

        if (x is None) and (y is None) and (index is None) and (sampl_freq is None) and (n is None):
            self.sampling_rate = 0

            self.n = 0
            self.set_title(self.__title_pattern % self.n)
            
            self.cont_data = False
            self.index = []
            self.x = []
            self.y = []

        elif (x is not None) and (y is not None) and (index is not None) and (sampl_freq is not None) and (n is not None):
            self.x = x
            self.y = y
            self.sampling_rate = sampl_freq

            self.n = n
            self.set_title(self.__title_pattern % self.n)
        
            self.index = index
            self.cont_data = True

        else:
            raise ValueError("Wrong usage of __init__!")


    def get_accu_by_index(self, index):
        self.lock.acquire()
        try:
            start = self.index[index][0]
            end = self.index[index][1]
        except:
            self.lock.release()
            raise

        tmp_x = self.x[start:end+1]
        tmp_y = []

        for i in range(self.get_number_of_channels()):
            tmp_y.append(self.y[i][start:end+1])

        r = Accumulation(x = tmp_x, y = tmp_y, n = self.n, index = [(0,len(tmp_y[0])-1)], sampl_freq = self.sampling_rate, error = self.use_error)            
        self.lock.release()
        return r

    def get_ysquare(self, channel):
        if self.uses_statistics():
            try:
                return self.y_square[channel]
            except:
                raise
        else: return None
    
    def contains_data(self):
        return self.cont_data


    def get_sampling_rate(self):
        "Returns the samplingfrequency"
        return self.sampling_rate + 0


    def get_index_bounds(self, index):
        "Returns a tuple with (start, end) of the wanted result"
        return self.index[index]


    def uses_statistics(self):
        return self.use_error

    # Schnittstellen nach Außen --------------------------------------------------------------------

    def get_yerr(self, channel):
        """
        return error (std.dev/sqrt(n)) of mean
        """

        if not self.uses_statistics(): return numpy.zeros((len(self.y[0]),),dtype="Float64")
        if not self.contains_data(): return []

        self.lock.acquire()
        if self.n < 2:
            retval=numpy.zeros((len(self.y[0]),),dtype="Float64")
            self.lock.release()
            return retval
        try:
            variance_over_n = (self.y_square[channel] - (self.y[channel]**2 / float(self.n)))/float((self.n-1)*self.n)
        except IndexError:
            print "Warning Accumulation.get_ydata(channel): Channel index does not exist."
            variance_over_n = numpy.zeros((len(self.y[0]),), dtype="Float64")
        self.lock.release()
        # sample standard deviation / sqrt(n)
        return numpy.nan_to_num(numpy.sqrt(variance_over_n))

    def get_ydata(self, channel):
        """
        return mean data
        """

        if not self.contains_data(): return []
        self.lock.acquire()

        try:
            tmp_y = self.y[channel] / self.n
        except IndexError:
            print "Warning Accumulation.get_ydata(channel): Channel index does not exist."
            tmp_y = numpy.zeros((len(self.y[0]),), dtype="Float64")

        self.lock.release()
        return tmp_y


    def get_ymin(self):

        if not self.contains_data(): return 0
        
        tmp_min = []
        self.lock.acquire()
        for i in range(self.get_number_of_channels()):
            tmp_min.append(self.get_ydata(i).min())

        if self.uses_statistics() and self.ready_for_drawing_error():
            for i in range(self.get_number_of_channels()):
                tmp_min.append((self.get_ydata(i) - self.get_yerr(i)).min())
        self.lock.release()

        return min(tmp_min)

    
    def get_ymax(self):

        if not self.contains_data(): return 0
        
        tmp_max = []
        self.lock.acquire()
        for i in range(self.get_number_of_channels()):
            tmp_max.append(self.get_ydata(i).max())

        if self.uses_statistics() and self.ready_for_drawing_error():
            for i in range(self.get_number_of_channels()):
                tmp_max.append((self.get_ydata(i) + self.get_yerr(i)).max())
        self.lock.release()
        return max(tmp_max)

    def get_job_id(self):
        return None

    def write_to_csv(self, destination=sys.stdout, delimiter=" "):
        """
        writes the data to a file or to sys.stdout
        destination can be a file or a filename
        suitable for further processing
        """

        the_destination=destination
        if type(destination) in types.StringTypes:
            the_destination=file(destination, "w")

        the_destination.write("# accumulation %d\n"%self.n)
        self.lock.acquire()
        try:
            if self.common_descriptions is not None:
                for (key,value) in self.common_descriptions.iteritems():
                    the_destination.write("# %s : %s\n"%(key, str(value)))
            the_destination.write("# t")
            ch_no=self.get_number_of_channels()
            if self.use_error:
                for i in xrange(ch_no): the_destination.write(" ch%d_mean ch%d_err"%(i,i))
            else:
                for i in xrange(ch_no): the_destination.write(" ch%d_mean"%i)
            the_destination.write("\n")
            xdata=self.get_xdata()
            ydata=map(self.get_ydata, xrange(ch_no))
            yerr=None
            if self.use_error:
                yerr=map(self.get_yerr, xrange(ch_no))
            for i in xrange(len(xdata)):
                the_destination.write("%e"%xdata[i])
                for j in xrange(ch_no):
                    if self.use_error:
                        the_destination.write("%s%e%s%e"%(delimiter, ydata[j][i], delimiter, yerr[j][i]))
                    else:
                        the_destination.write("%s%e"%(delimiter,ydata[j][i]))                        
                the_destination.write("\n")
            the_destination=None
            xdata=yerr=ydata=None
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
        accu_group._v_attrs.damaris_type="Accumulation"
        if self.contains_data():
            self.lock.acquire()
            try:
                # save time stamps
                if self.time_period is not None and len(self.time_period)>0:
                    accu_group._v_attrs.earliest_time="%04d%02d%02d %02d:%02d:%02d.%03d"%(self.time_period[0].year,
                                                                                          self.time_period[0].month,
                                                                                          self.time_period[0].day,
                                                                                          self.time_period[0].hour,
                                                                                          self.time_period[0].minute,
                                                                                          self.time_period[0].second,
                                                                                          self.time_period[0].microsecond/1000)
                    accu_group._v_attrs.oldest_time="%04d%02d%02d %02d:%02d:%02d.%03d"%(self.time_period[1].year,
                                                                                        self.time_period[1].month,
                                                                                        self.time_period[1].day,
                                                                                        self.time_period[1].hour,
                                                                                        self.time_period[1].minute,
                                                                                        self.time_period[1].second,
                                                                                        self.time_period[1].microsecond/1000)
                if self.common_descriptions is not None:
                    for (key,value) in self.common_descriptions.iteritems():
                        accu_group._v_attrs.__setattr__("description_"+key,str(value))

                # save interval information
                filter=None
                if complib is not None:
                    if complevel is None:
                        complevel=9
                    filter=tables.Filters(complevel=complevel,complib=complib,shuffle=1)

                # tried compression filter, but no effect...
                index_table=hdffile.createTable(where=accu_group,
                                                name="indices",
                                                description={"start": tables.UInt64Col(),
                                                             "length": tables.UInt64Col(),
                                                             "start_time": tables.Float64Col(),
                                                             "dwelltime": tables.Float64Col(),
                                                             "number": tables.UInt64Col()},
                                                title="indices of adc data intervals",
                                                filters=filter,
                                                expectedrows=len(self.index))
                index_table.flavor="numpy"
                # save interval data
                new_row=index_table.row
                for i in xrange(len(self.index)):
                    new_row["start"]=self.index[i][0]
                    new_row["dwelltime"]=1.0/self.sampling_rate
                    new_row["start_time"]=1.0/self.sampling_rate*self.index[i][0]
                    new_row["length"]=self.index[i][1]-self.index[i][0]+1
                    new_row["number"]=self.n
                    new_row.append()

                index_table.flush()
                new_row=None
                index_table=None

                # prepare saving data
                channel_no=len(self.y)
                timedata=numpy.empty((len(self.y[0]),channel_no*2), dtype = "Float64")
                for ch in xrange(channel_no):
                    timedata[:,ch*2]=self.get_ydata(ch)
                    if self.uses_statistics():
                        timedata[:,ch*2+1]=self.get_yerr(ch)
                    else:
                        timedata[:,ch*2+1]=numpy.zeros((len(self.y[0]),),dtype = "Float64")
                
                # save data
                time_slice_data=None
                if filter is not None:
                    chunkshape=timedata.shape
                    if len(chunkshape) <= 1:
                        chunkshape = (min(chunkshape[0],1024*8),)
                    else:
                        chunkshape = (min(chunkshape[0],1024*8), chunkshape[1])
                    if tables.__version__[0]=="1":
                        time_slice_data=hdffile.createCArray(accu_group,
                                                             name="accu_data",
                                                             shape=timedata.shape,
                                                             atom=tables.Float64Atom(shape=chunkshape,
                                                                                     flavor="numpy"),
                                                             filters=filter,
                                                             title="accu data")
                    else:
                        time_slice_data=hdffile.createCArray(accu_group,
                                                             name="accu_data",
                                                             shape=timedata.shape,
                                                             chunkshape=chunkshape,
                                                             atom=tables.Float64Atom(),
                                                             filters=filter,
                                                             title="accu data")
                        
                    time_slice_data[:]=timedata
                else:
                    time_slice_data=hdffile.createArray(accu_group,
                                                        name="accu_data",
                                                        object=timedata,
                                                        title="accu data")



            finally:
                time_slice_data=None
                accu_group=None
                self.lock.release()

    # / Schnittstellen nach Außen ------------------------------------------------------------------
    
    # Überladen von Operatoren ---------------------------------------------------------------------

    def __len__(self):
        """
        return number of samples per channel, 0 if empty
        """
        if len(self.y)>0:
            return len(self.y[0])
        return 0

    def __repr__(self):
        "Redefining repr(Accumulation)"

        if not self.contains_data(): return "Empty"
        
        tmp_string = "X:                   " + repr(self.x) + "\n"

        for i in range(self.get_number_of_channels()):
            tmp_string += ("Y(%d):                " % i) + repr(self.y[i]) + "\n"
            if self.uses_statistics(): tmp_string += "y_square(%d):         " % i + str(self.y_square[i]) + "\n"

        tmp_string += "Indexes:             " + str(self.index) + "\n"

        tmp_string += "Samples per Channel: " + str(len(self.y[0])) + "\n"
        tmp_string += "Samplingfrequency:   " + str(self.sampling_rate) + "\n"

        tmp_string += "n:                   " + str(self.n)

        return tmp_string


    def __add__(self, other):
        "Redefining self + other"
        # Float or int
        if isinstance(other, types.IntType) or isinstance(other, types.FloatType):
            if not self.contains_data(): raise ValueError("Accumulation: You cant add integers/floats to an empty accumulation")
            else:

                tmp_y = []
                tmp_ysquare = []


                self.lock.acquire()
                for i in range(self.get_number_of_channels()):
                    # Dont change errors and mean value
                    if self.uses_statistics(): tmp_ysquare.append(self.y_square[i] + ( (2*self.y[i]*other) + ((other**2)*self.n) ))
                    tmp_y.append(self.y[i] + (other*self.n))

                if self.uses_statistics():
                    r = Accumulation(x = numpy.array(self.x, dtype="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = self.n, index = self.index, sampl_freq = self.sampling_rate, error = self.use_error)
                else:
                    r = Accumulation(x = numpy.array(self.x, dtype="Float64"), y = tmp_y, n = self.n, index = self.index, sampl_freq = self.sampling_rate, error = self.use_error)

                self.lock.release()
                return r

        # ADC_Result
        elif str(other.__class__) == "damaris.data.ADC_Result.ADC_Result":

            # Other empty (return)
            # todo: this is seems to be bugy!!!! (Achim)
            if not other.contains_data(): return

            # Self empty (copy)
            if not self.contains_data():

                tmp_y = []
                tmp_ysquare = []

                self.lock.acquire()

                for i in range(other.get_number_of_channels()):
                    tmp_y.append(numpy.array(other.y[i], dtype="Float64"))
                    if self.uses_statistics(): tmp_ysquare.append(tmp_y[i] ** 2)
                 

                if self.uses_statistics():
                    r = Accumulation(x = numpy.array(other.x, dtype="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = 1, index = other.index, sampl_freq = other.sampling_rate, error = True)
                else:
                    r = Accumulation(x = numpy.array(other.x, dtype="Float64"), y = tmp_y, index = other.index, sampl_freq = other.sampling_rate, n = 1, error = False)
                r.time_period=[other.job_date,other.job_date]
                r.common_descriptions=other.description.copy()
                self.lock.release()
                return r

            # Other and self not empty (self + other)
            else:
                self.lock.acquire()
                
                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent sampling-rates")
                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of samples")
                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of channels")
                for i in range(len(self.index)):
                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add ADC-Results with diffrent indexing")

                tmp_y = []
                tmp_ysquare = []

                for i in range(self.get_number_of_channels()):
                    tmp_y.append(self.y[i] + other.y[i])
                    if self.uses_statistics(): tmp_ysquare.append(self.y_square[i] + (numpy.array(other.y[i], dtype="Float64") ** 2))

                if self.uses_statistics():
                    r = Accumulation(x = numpy.array(self.x, dtype="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = self.n + 1, index = self.index, sampl_freq = self.sampling_rate, error = True)
                else:
                    r = Accumulation(x = numpy.array(self.x, dtype="Float64"), y = tmp_y, n = self.n + 1, index = self.index, sampl_freq = self.sampling_rate, error = False)
                r.time_period=[min(self.time_period[0],other.job_date),
                               max(self.time_period[1],other.job_date)]
                if self.common_descriptions is not None:
                    r.common_descriptions={}
                    for key in self.common_descriptions.keys():
                        if (key in other.description and self.common_descriptions[key]==other.description[key]):
                            r.common_descriptions[key]=value

                self.lock.release()
                return r

        # Accumulation
        elif str(other.__class__) == "damaris.data.Accumulation.Accumulation":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (copy)
            if not self.contains_data():

                tmp_y = []
                tmp_ysquare = []

                self.lock.acquire()

                if self.uses_statistics():
                    r = Accumulation(x = numpy.array(other.x, dtype="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = other.n, index = other.index, sampl_freq = other.sampling_rate, error = True)
                else:
                    r = Accumulation(x = numpy.array(other.x, dtype="Float64"), y = tmp_y, n = other.n, index = other.index, sampl_freq = other.sampling_rate, error = False)
                for i in range(other.get_number_of_channels()):
                    tmp_y.append(other.y[i])
                    tmp_ysquare.append(other.y_square[i])
                r.time_period=other.time_period[:]
                if other.common_descriptions is not None:
                    r.common_descriptions=othter.common_descriptions.copy()
                else:
                    r.common_descriptions=None

                self.lock.release()
                return r

            # Other and self not empty (self + other)
            else:
                self.lock.acquire()

                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add accumulations with diffrent sampling-rates")
                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add accumulations with diffrent number of samples")
                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add accumulations with diffrent number of channels")
                for i in range(len(self.index)):
                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add accumulations with diffrent indexing")
                if self.uses_statistics() and not other.uses_statistics(): raise ValueError("Accumulation: You cant add non-error accumulations to accumulations with error")

                tmp_y = []
                tmp_ysquare = []

                for i in range(self.get_number_of_channels()):
                    tmp_y.append(self.y[i] + other.y[i])
                    tmp_ysquare.append(self.y_square[i] + other.y_square[i])

                if self.uses_statistics():
                    r = Accumulation(x = numpy.array(self.x, dtype="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = other.n + self.n, index = self.index, sampl_freq = self.sampling_rate, error = True)
                else:
                    r = Accumulation(x = numpy.array(self.x, dtype="Float64"), y = tmp_y, n = other.n + self.n, index = self.index, sampl_freq = self.sampling_rate, error = False)

                r.time_period=[min(self.time_period[0],other.time_period[0]),
                               max(self.time_period[1],other.time_period[1])]
                r.common_descriptions={}
                if self.common_descriptions is not None and other.common_descriptions is not None:
                    for key in self.common_descriptions.keys():
                        if (key in other.common_descriptions and
                            self.common_descriptions[key]==other.common_descriptions[key]):
                            r.common_descriptions[key]=value

                self.lock.release()
                return r
            

    def __radd__(self, other):
        "Redefining other + self"
        return self.__add__(other)


    def __sub__(self, other):
        "Redefining self - other"
        return self.__add__(-other)
            

    def __rsub__(self, other):
        "Redefining other - self"
        return self.__neg__(self.__add__(-other))


    def __iadd__(self, other):
        "Redefining self += other"
        # Float or int
        if isinstance(other, types.IntType) or isinstance(other, types.FloatType):
            if not self.contains_data(): raise ValueError("Accumulation: You cant add integers/floats to an empty accumulation")
            else:
                
                self.lock.acquire()
                for i in range(self.get_number_of_channels()):
                    #Dont change errors and mean value
                    if self.uses_statistics(): self.y_square[i] += (2*self.y[i]*other) + ((other**2)*self.n)
                    self.y[i] += other*self.n
                self.lock.release()

                return self

        # ADC_Result
        elif str(other.__class__) == "damaris.data.ADC_Result.ADC_Result":

            # Other empty (return)
            if not other.contains_data(): return self

            # Self empty (copy)
            if not self.contains_data():
                self.lock.acquire()
                self.n += 1
                self.index = other.index[0:]
                self.sampling_rate = other.sampling_rate
                self.x = numpy.array(other.x, dtype="Float64")
                self.cont_data = True

                for i in range(other.get_number_of_channels()):
                    self.y.append(numpy.array(other.y[i], dtype="Float64"))
                    if self.uses_statistics(): self.y_square.append(self.y[i] ** 2)

                self.set_title(self.__title_pattern % self.n)
                self.lock.release()

                self.time_period=[other.job_date,other.job_date]
                self.common_descriptions=other.description.copy()

                return self


            # Other and self not empty (self + other)
            else:
                self.lock.acquire()

                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent sampling-rates")
                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of samples")
                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of channels")
                for i in range(len(self.index)):
                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add ADC-Results with diffrent indexing")

                for i in range(self.get_number_of_channels()):
                    self.y[i] += other.y[i]
                    if self.uses_statistics(): self.y_square[i] += numpy.array(other.y[i], dtype="Float64") ** 2

                self.n += 1
                self.time_period=[min(self.time_period[0],other.job_date),
                                  max(self.time_period[1],other.job_date)]
                if self.common_descriptions is not None:
                    for key in self.common_descriptions.keys():
                        if not (key in other.description and self.common_descriptions[key]==other.description[key]):
                            del self.common_descriptions[key]

                self.set_title(self.__title_pattern % self.n)
                self.lock.release()

                return self

        # Accumulation
        elif str(other.__class__) == "damaris.data.Accumulation.Accumulation":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (copy)
            if not self.contains_data():
                if self.uses_statistics() and not other.uses_statistics(): raise ValueError("Accumulation: You cant add non-error accumulations to accumulations with error")

                self.lock.acquire()
                self.n += other.n
                self.index = other.index[0:]
                self.sampling_rate = other.sampling_rate
                self.x = numpy.array(other.x, dtype="Float64")
                self.cont_data = True

                for i in range(other.get_number_of_channels()):
                    self.y.append(numpy.array(other.y[i], dtype="Float64"))
                    if self.uses_statistics(): self.y_square.append(self.y[i] ** 2)

                self.set_title(self.__title_pattern % self.n)
                self.common_descriptions=other.common_desriptions.copy()
                self.time_period=other.time_period[:]
                self.lock.release()

                return self
            
            # Other and self not empty (self + other)
            else:
                self.lock.acquire()
                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add accumulations with diffrent sampling-rates")
                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add accumulations with diffrent number of samples")
                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add accumulations with diffrent number of channels")
                for i in range(len(self.index)):
                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add accumulations with diffrent indexing")
                if self.uses_statistics() and not other.uses_statistics(): raise ValueError("Accumulation: You cant add non-error accumulations to accumulations with error")
                    
                for i in range(self.get_number_of_channels()):
                    self.y[i] += other.y[i]
                    if self.uses_statistics(): self.y_square[i] += other.y_square[i]

                self.n += other.n
                self.time_period=[min(self.time_period[0],other.time_period[0]),
                                  max(self.time_period[1],other.time_period[1])]
                if self.common_descriptions is not None and other.common_descriptions is not None:
                    for key in self.common_descriptions.keys():
                        if not (key in other.description and
                                self.common_descriptions[key]==other.common_descriptions[key]):
                            del self.common_descriptions[key]

                self.set_title(self.__title_pattern % self.n)
                self.lock.release()
                
                return self

        elif other is None:
            # Convenience: ignore add of None
            return self
        else:
            raise ValueError("can not add "+repr(type(other))+" to Accumulation")


    def __isub__(self, other):
        "Redefining self -= other"
        return self.__iadd__(-other)
        

    def __neg__(self):
        "Redefining -self"

        if not self.contains_data(): return

        tmp_y = []

        self.lock.acquire()
        for i in range(self.get_number_of_channels()):
            tmp_y.append(numpy.array(-self.y[i], dtype="Float64"))

        if self.uses_statistics():
            r = Accumulation(x = numpy.array(self.x, dtype="Float64"), y = tmp_y, y_2 = numpy.array(self.y_square), n = self.n, index = self.index, sampl_freq = self.sampling_rate, error = True)
        else:
            r = Accumulation(x = numpy.array(self.x, dtype="Float64"), y = tmp_y, n = self.n, index = self.index, sampl_freq = self.sampling_rate, error = False)
        self.lock.release()
        return r


def read_from_hdf(hdf_node):
    """
    read accumulation data from HDF node and return it.
    """

    # formal checks first
    if not isinstance(hdf_node, tables.Group):
        return None

    if hdf_node._v_attrs.damaris_type!="Accumulation":
        return None

    if not (hdf_node.__contains__("indices") and hdf_node.__contains__("accu_data")):
        print "no accu data"
        return None

    accu=Accumulation()

    # populate description dictionary
    accu.common_descriptions={}
    for attrname in hdf_node._v_attrs._v_attrnamesuser:
        if attrname.startswith("description_"):
            accu.common_descriptions[attrname[12:]]=hdf_node._v_attrs.__getattr__(attrname)

    eariliest_time=None
    if "earliest_time" in dir(hdf_node._v_attrs):
        timestring=hdf_node._v_attrs.__getattr__("earliest_time")
        earliest_time=datetime.datetime(int(timestring[:4]),  # year
                                        int(timestring[4:6]), # month
                                        int(timestring[6:8]), # day
                                        int(timestring[9:11]), # hour
                                        int(timestring[12:14]), # minute
                                        int(timestring[15:17]), # second
                                        int(timestring[18:21])*1000 # microsecond
                                        )

    oldest_time=None
    if "oldest_time" in dir(hdf_node._v_attrs):
        timestring=hdf_node._v_attrs.__getattr__("oldest_time")
        oldest_time=datetime.datetime(int(timestring[:4]),  # year
                                      int(timestring[4:6]), # month
                                      int(timestring[6:8]), # day
                                      int(timestring[9:11]), # hour
                                      int(timestring[12:14]), # minute
                                      int(timestring[15:17]), # second
                                      int(timestring[18:21])*1000 # microsecond
                                      )

    if oldest_time is None or earliest_time is None:
        accu.time_period=None
        if len(accu.common_descriptions)==0:
            # no accus inside, so no common description expected
            accu.common_descriptions=None
            accu.cont_data=False
    else:
        accu.time_period=[oldest_time, earliest_time]
        accu.cont_data=True

    # start with indices
    for r in hdf_node.indices.iterrows():
        accu.index.append((r["start"],r["start"]+r["length"]-1))
        accu.n=r["number"]
        accu.sampling_rate=1.0/r["dwelltime"]

    # now really belief there are no data
    if len(accu.index)==0 or accu.n==0:
        accu.cont_data=False
        return accu

    # now do the real data
    accu_data=hdf_node.accu_data.read()
    
    accu.x=numpy.arange(accu_data.shape[0], dtype="Float64")/accu.sampling_rate
    # assume error information, todo: save this information explicitly
    accu.y_square=[]
    accu.use_error=False

    for ch in xrange(accu_data.shape[1]/2):
        accu.y.append(accu_data[:,ch*2]*accu.n)
        if accu.n<2 or numpy.all(accu_data[:,ch*2+1]==0.0):
            accu.y_square.append(numpy.zeros((accu_data.shape[0]) ,dtype="Float64"))
        else:
            accu.use_error=True
            accu.y_square.append((accu_data[:,ch*2+1]**2)*float((accu.n-1.0)*accu.n)+(accu_data[:,ch*2]**2)*accu.n)

    if not accu.use_error:
        del accu.y_square

    return accu
