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
        try:
            start = self.index[index][0]
            end = self.index[index][1]
        except:
            raise

        tmp_x = self.x[start:end+1]
        tmp_y = []

        for i in range(self.get_number_of_channels()):
            tmp_y.append(self.y[i][start:end+1])

        return Accumulation(x = tmp_x, y = tmp_y, n = self.n, index = [(0,len(tmp_y[0])-1)], sampl_freq = self.sampling_rate, error = self.use_error)            


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

        if self.n < 2: return numarray.zeros((len(self.y[0]),),type="Float64")

        # ( E(X^2) - E(X)^2 )^0.5
        try:
            tmp_yerr = (((self.y_square[channel] / self.n) - ((self.y[channel] / self.n)**2)) ** 0.5)
        except:
            print "Warning Accumulation.get_yerr(channel): Channel index does not exist."
            return numarray.zeros((len(self.y[0]),),type="Float64")

        return tmp_yerr


    def get_ydata(self, channel):

        if not self.contains_data(): return []
        if not self.uses_statistics(): self.n = self.jobs_added

        try:
            tmp_y = self.y[channel] / self.n
        except:
            print "Warning Accumulation.get_ydata(channel): Channel index does not exist."
            return numarray.zeros((len(self.y[0]),),type="Float64")

        return tmp_y


    def get_ymin(self):

        if not self.contains_data(): return 0
        
        tmp_min = []
        for i in range(self.get_number_of_channels()):
            tmp_min.append(self.get_ydata(i).min())

        if self.uses_statistics() and self.ready_for_drawing_error():
            for i in range(self.get_number_of_channels()):
                tmp_min.append((self.get_ydata(i) - self.get_yerr(i)).min())            

        return min(tmp_min)

    
    def get_ymax(self):

        if not self.contains_data(): return 0
        
        tmp_max = []
        for i in range(self.get_number_of_channels()):
            tmp_max.append(self.get_ydata(i).max())

        if self.uses_statistics() and self.ready_for_drawing_error():
            for i in range(self.get_number_of_channels()):
                tmp_max.append((self.get_ydata(i) + self.get_yerr(i)).max())

        return max(tmp_max)


    def get_job_id(self):
        return None

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
            if self.uses_statistics() and self.n >= 2: tmp_string += "\ny_err(%d):            " % i + str(self.yerr[i])  + "\n"

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


                for i in range(self.get_number_of_channels()):
                    # Dont change errors and mean value
                    if self.uses_statistics(): tmp_ysquare.append(self.y_square[i] + ( (2*self.y[i]*other) + ((other**2)*self.n) ))
                    tmp_y.append(self.y[i] + (other*self.n))

                if self.uses_statistics():
                    return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = self.n, index = self.index, sampl_freq = self.sampling_rate, error = self.use_error)
                else:
                    return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, n = self.n, index = self.index, sampl_freq = self.sampling_rate, error = self.use_error)

        # ADC_Result
        elif str(other.__class__) == "ADC_Result.ADC_Result":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (copy)
            if not self.contains_data():

                tmp_y = []
                tmp_ysquare = []

                for i in range(other.get_number_of_channels()):
                    tmp_y.append(numarray.array(other.y[i], type="Float64"))
                    if self.uses_statistics(): tmp_ysquare.append(tmp_y[i] ** 2)
                 

                if self.uses_statistics():
                    return Accumulation(x = numarray.array(other.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = 1, index = other.index, sampl_freq = other.sampling_rate, error = True)
                else:
                    return Accumulation(x = numarray.array(other.x, type="Float64"), y = tmp_y, index = other.index, sampl_freq = other.sampling_rate, n = 1, error = False)
                

            # Other and self not empty (self + other)
            else:
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
                    return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = self.n + 1, index = self.index, sampl_freq = self.sampling_rate, error = True)
                else:
                    return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, n = self.n + 1, index = self.index, sampl_freq = self.sampling_rate, error = False)

        # Accumulation
        elif str(other.__class__) == "Accumulation.Accumulation":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (copy)
            if not self.contains_data():

                tmp_y = []
                tmp_ysquare = []

                for i in range(other.get_number_of_channels()):
                    tmp_y.append(other.y[i])
                    tmp_ysquare.append(other.y_square[i])

                if self.uses_statistics():
                    return Accumulation(x = numarray.array(other.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = other.n, index = other.index, sampl_freq = other.sampling_rate, error = True)
                else:
                    return Accumulation(x = numarray.array(other.x, type="Float64"), y = tmp_y, n = other.n, index = other.index, sampl_freq = other.sampling_rate, error = False)


            # Other and self not empty (self + other)
            else:
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
                    return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = other.n + self.n, index = self.index, sampl_freq = self.sampling_rate, error = True)
                else:
                    return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, n = other.n + self.n, index = self.index, sampl_freq = self.sampling_rate, error = False)

            

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
                
                for i in range(self.get_number_of_channels()):
                    #Dont change errors and mean value
                    if self.uses_statistics(): self.y_square[i] += (2*self.y[i]*other) + ((other**2)*self.n)
                    self.y[i] += other*self.n

                return self

        # ADC_Result
        elif str(other.__class__) == "ADC_Result.ADC_Result":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (copy)
            if not self.contains_data():
                
                self.n += 1
                self.index = other.index[0:]
                self.sampling_rate = other.sampling_rate
                self.x = numarray.array(other.x, type="Float64")
                self.cont_data = True

                for i in range(other.get_number_of_channels()):
                    self.y.append(numarray.array(other.y[i], type="Float64"))
                    if self.uses_statistics(): self.y_square.append(self.y[i] ** 2)

                self.set_title(self.__title_pattern % self.n)

                return self


            # Other and self not empty (self + other)
            else:
                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent sampling-rates")
                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of samples")
                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of channels")
                for i in range(len(self.index)):
                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add ADC-Results with diffrent indexing")

                for i in range(self.get_number_of_channels()):
                    self.y[i] += other.y[i]
                    if self.uses_statistics(): self.y_square[i] += numarray.array(other.y[i], type="Float64") ** 2

                self.n += 1

                self.set_title(self.__title_pattern % self.n)

                return self

        # Accumulation
        elif str(other.__class__) == "Accumulation.Accumulation":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (copy)
            if not self.contains_data():
                if self.uses_statistics() and not other.uses_statistics(): raise ValueError("Accumulation: You cant add non-error accumulations to accumulations with error")

                self.n += other.n
                self.index = other.index[0:]
                self.sampling_rate = other.sampling_rate
                self.x = numarray.array(other.x, type="Float64")
                self.cont_data = True

                for i in range(other.get_number_of_channels()):
                    self.y.append(numarray.array(other.y[i], type="Float64"))
                    if self.uses_statistics(): self.y_square.append(self.y[i] ** 2)

                self.set_title(self.__title_pattern % self.n)

                return self
            
            # Other and self not empty (self + other)
            else:
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

                self.set_title(self.__title_pattern % self.n)

                return self


    def __isub__(self, other):
        "Redefining self -= other"
        return self.__iadd__(-other)
        

    def __neg__(self):
        "Redefining -self"

        if not self.contains_data(): return

        tmp_y = []

        for i in range(self.get_number_of_channels()):
            tmp_y.append(numarray.array(-self.y[i], type="Float64"))

        if self.uses_statistics():
            return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, y_2 = numarray.array(self.y_square), n = self.n, index = self.index, sampl_freq = self.sampling_rate, error = True)
        else:
            return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, n = self.n, index = self.index, sampl_freq = self.sampling_rate, error = False)
