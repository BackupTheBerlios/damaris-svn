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

import sys
import threading
import types
import tables
import numarray
from types import *

class Accumulation(Errorable, Drawable):
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

        if not self.uses_statistics(): return []
        if not self.contains_data(): return []

        self.lock.acquire()
        if self.n < 2:
            self.lock.release()
            return numarray.zeros((len(self.y[0]),),type="Float64")


        # ( E(X^2) - E(X)^2 )^0.5
        try:
            tmp_yerr = (((self.y_square[channel] / self.n) - ((self.y[channel] / self.n)**2))/self.n) ** 0.5
        except e:
            print "Warning Accumulation.get_yerr(channel): Std-Deviation calculation failed (%s)"%(str(e))
            tmp_yerr = numarray.zeros((len(self.y[0]),),type="Float64")

        self.lock.release()
        return tmp_yerr


    def get_ydata(self, channel):

        if not self.contains_data(): return []
        self.lock.acquire()

        try:
            tmp_y = self.y[channel] / self.n
        except:
            print "Warning Accumulation.get_ydata(channel): Channel index does not exist."
            tmp_y = numarray.zeros((len(self.y[0]),),type="Float64")

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

    def write_as_csv(self, destination=sys.stdout):
        """
        writes the data to a file or to sys.stdout
        destination can be a file or a filename
        suitable for further processing
        """
        # write sorted
        the_destination=None
        if isinstance(destination,types.FileType):
            the_destination=destination
        elif isinstance(destination,types.StringTypes):
            the_destination=file(destination,"w")
        else:
            raise Exception("sorry destination %s is not valid"%(repr(destination)))

        the_destination.write("# accumulation %d\n"%self.n)
        the_destination.write("# t y_mean y_err ...\n")
        self.lock.acquire()
        try:
            xdata=self.get_xdata()
            ch_no=self.get_number_of_channels()
            ydata=map(self.get_ydata, xrange(ch_no))
            yerr=map(self.get_yerr, xrange(ch_no))
            for i in xrange(len(xdata)):
                the_destination.write("%g"%xdata[i])
                for j in xrange(ch_no):
                    the_destination.write(" %g %g"%(ydata[j][i],yerr[j][i]))
                the_destination.write("\n")
            the_destination=None
            xdata=yerr=ydata=None
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
                # save channel data
                for channel_no in xrange(len(self.y)):
                    y_mean=self.get_ydata(channel_no)
                    y_sigma=self.get_yerr(channel_no)
                    for index_no in xrange(len(self.index)):
                        index=self.index[index_no]
                        # set time data
                        timedata=numarray.array(type = numarray.Float64,
                                                shape = (2*(index[1]-index[0]+1),))
                            
                        timedata[0::2]=y_mean[index[0]:index[1]+1]
                        if len(y_sigma):
                            timedata[1::2]=y_sigma[index[0]:index[1]+1]
                        else:
                            timedata[1::2]=numarray.zeros(type = numarray.Float64,
                                                          shape = ((index[1]-index[0]+1),))
                        timedata.setshape((index[1]-index[0]+1,2))
                        time_slice_data=None
                        if complib is not None:
                            if complevel is None:
                                complevel=9
                            chunkshape = numarray.shape(timedata)
			    if len(chunkshape) <= 1:
				chunkshape = (min(chunkshape[0],1024*8),)
			    else:
				chunkshape = (min(chunkshape[0],1024*8), chunkshape[1])
                            time_slice_data=hdffile.createCArray(accu_group,
                                                                 name="idx%04d_ch%04d"%(index_no,channel_no),
                                                                 shape=timedata.getshape(),
                                                                 atom=tables.Float64Atom(shape=chunkshape,
                                                                                         flavor="numarray"),
                                                                 filters=tables.Filters(complevel=complevel,
                                                                                        complib=complib,
											shuffle=1),
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
                        time_slice_data._f_setAttr("number",numarray.array(self.n, type=numarray.Int64))
                        time_slice_data._f_setAttr("dwelltime",numarray.array(1.0/self.sampling_rate,
                                                                               type=numarray.Float64))
                        time_slice_data._f_setAttr("start_time",numarray.array(1.0/self.sampling_rate*index[0],
                                                                                type=numarray.Float64))
            finally:
                time_slice_data=None
                accu_group=None
                self.lock.release()

    # / Schnittstellen nach Außen ------------------------------------------------------------------
    
    # Überladen von Operatoren ---------------------------------------------------------------------

    def __len__(self):
        return len(self.y[0])


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
        if isinstance(other, IntType) or isinstance(other, FloatType):
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
                    r = Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = self.n, index = self.index, sampl_freq = self.sampling_rate, error = self.use_error)
                else:
                    r = Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, n = self.n, index = self.index, sampl_freq = self.sampling_rate, error = self.use_error)

                self.lock.release()
                return r

        # ADC_Result
        elif str(other.__class__) == "ADC_Result.ADC_Result":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (copy)
            if not self.contains_data():

                tmp_y = []
                tmp_ysquare = []

                self.lock.acquire()

                for i in range(other.get_number_of_channels()):
                    tmp_y.append(numarray.array(other.y[i], type="Float64"))
                    if self.uses_statistics(): tmp_ysquare.append(tmp_y[i] ** 2)
                 

                if self.uses_statistics():
                    r = Accumulation(x = numarray.array(other.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = 1, index = other.index, sampl_freq = other.sampling_rate, error = True)
                else:
                    r = Accumulation(x = numarray.array(other.x, type="Float64"), y = tmp_y, index = other.index, sampl_freq = other.sampling_rate, n = 1, error = False)
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
                    if self.uses_statistics(): tmp_ysquare.append(self.y_square[i] + (numarray.array(other.y[i], type="Float64") ** 2))

                if self.uses_statistics():
                    r = Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = self.n + 1, index = self.index, sampl_freq = self.sampling_rate, error = True)
                else:
                    r = Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, n = self.n + 1, index = self.index, sampl_freq = self.sampling_rate, error = False)
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
        elif str(other.__class__) == "Accumulation.Accumulation":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (copy)
            if not self.contains_data():

                tmp_y = []
                tmp_ysquare = []

                self.lock.acquire()

                if self.uses_statistics():
                    r = Accumulation(x = numarray.array(other.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = other.n, index = other.index, sampl_freq = other.sampling_rate, error = True)
                else:
                    r = Accumulation(x = numarray.array(other.x, type="Float64"), y = tmp_y, n = other.n, index = other.index, sampl_freq = other.sampling_rate, error = False)
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
                    r = Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = other.n + self.n, index = self.index, sampl_freq = self.sampling_rate, error = True)
                else:
                    r = Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, n = other.n + self.n, index = self.index, sampl_freq = self.sampling_rate, error = False)

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
        return self.__neg__(self.__add__(other))


    def __iadd__(self, other):
        "Redefining self += other"
        # Float or int
        if isinstance(other, IntType) or isinstance(other, FloatType):
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
        elif str(other.__class__) == "ADC_Result.ADC_Result":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (copy)
            if not self.contains_data():
                self.lock.acquire()
                self.n += 1
                self.index = other.index[0:]
                self.sampling_rate = other.sampling_rate
                self.x = numarray.array(other.x, type="Float64")
                self.cont_data = True

                for i in range(other.get_number_of_channels()):
                    self.y.append(numarray.array(other.y[i], type="Float64"))
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
                    if self.uses_statistics(): self.y_square[i] += numarray.array(other.y[i], type="Float64") ** 2

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
        elif str(other.__class__) == "Accumulation.Accumulation":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (copy)
            if not self.contains_data():
                if self.uses_statistics() and not other.uses_statistics(): raise ValueError("Accumulation: You cant add non-error accumulations to accumulations with error")

                self.lock.acquire()
                self.n += other.n
                self.index = other.index[0:]
                self.sampling_rate = other.sampling_rate
                self.x = numarray.array(other.x, type="Float64")
                self.cont_data = True

                for i in range(other.get_number_of_channels()):
                    self.y.append(numarray.array(other.y[i], type="Float64"))
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


    def __isub__(self, other):
        "Redefining self -= other"
        return self.__iadd__(-other)
        

    def __neg__(self):
        "Redefining -self"

        if not self.contains_data(): return

        tmp_y = []

        self.lock.acquire()
        for i in range(self.get_number_of_channels()):
            tmp_y.append(numarray.array(-self.y[i], type="Float64"))

        if self.uses_statistics():
            r = Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, y_2 = numarray.array(self.y_square), n = self.n, index = self.index, sampl_freq = self.sampling_rate, error = True)
        else:
            r = Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, n = self.n, index = self.index, sampl_freq = self.sampling_rate, error = False)
        self.lock.release()
        return r
