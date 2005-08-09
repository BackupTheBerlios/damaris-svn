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
    def __init__(self, x = None, y = None, y_2 = None, n = None, index = None, sampl_freq = None, jobs_added = None, error = False):
        Errorable.__init__(self)
        Drawable.__init__(self)

        self.use_error = error
        
        if self.uses_statistics():
            if (y_2 is not None) and (n is not None):
                self.y_square = y_2
                self.n = n
            elif (y_2 is None) and (n is None):
                self.y_square = []
                self.n = 0
            else:
                raise ValueError("Wrong usage of __init__!")
             

        if (x is None) and (y is None) and (index is None) and (sampl_freq is None) and (jobs_added is None):
            self.sampling_rate = 0
            self.jobs_added = 0
            self.cont_data = False
            self.index = []
            self.x = []
            self.y = []

        elif (x is not None) and (y is not None) and (index is not None) and (sampl_freq is not None) and (jobs_added is not None):
            self.x = x
            self.y = y
            self.sampling_rate = sampl_freq
            self.jobs_added = jobs_added
            self.index = index
            self.cont_data = True

        else:
            raise ValueError("Wrong usage of __init__!")


##    def create_data_space(self, channels, samples):
##        "Initialises the internal data-structures"
##
##        if self.contains_data():
##            print "Warning Accumulation: Tried to run \"create_data_space()\" more than once."
##            return
##        
##        if channels <= 0: raise ValueError("ValueError: You cant create an ADC-Result with less than 1 channel!")
##        if samples <= 0: raise ValueError("ValueError: You cant create an ADC-Result with less than 1 sample!")
##        
##        for i in range(channels):
##            self.y.append(numarray.array([0]*samples, type="Float64"))
##            if self.uses_statistics():
##                self.yerr.append(numarray.array([0]*samples, type="Float64"))
##                self.y_square.append(numarray.array([0]*samples, type="Float64"))
##
##        self.x = numarray.array([0]*samples, type="Float64")
##            
##
##        self.index.append((0, samples-1))
##        self.cont_data = True


    def get_accu_by_index(self, index):
        pass


    def get_ysquare(self, channel):
        if self.uses_statistics():
            try:
                return self.y_square[channel]
            except:
                raise
        else: return None
    
##        try:
##            start = self.index[index][0]
##            end = self.index[index][1]
##        except:
##            raise
##
##        tmp_x = self.x[start:end+1]
##        tmp_y = []
##
##        for i in range(self.get_number_of_channels()):
##            tmp_y.append(self.y[i][start:end+1])
##
##        return ADC_Result(x = tmp_x, y = tmp_y,  index = [(0,len(tmp_y[0])-1)], sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)            


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

        if not self.uses_statistics(): return None
        if not self.contains_data(): return None

        if self.n < 2: return numarray.zeros((len(self.y[0]),),type="Float64")

        # ( E(X^2) - E(X)^2 )^0.5
        try:
            tmp_yerr = (((self.y_square[channel] / self.n) - ((self.y[channel] / self.n)**2)) ** 0.5)
        except:
            print "Warning Accumulation.get_yerr(channel): Channel index does not exist."
            return numarray.zeros((len(self.y[0]),),type="Float64")

        return tmp_yerr


    def get_ydata(self, channel):

        if not self.contains_data(): return None
        if not self.uses_statistics(): self.n = self.jobs_added

        try:
            tmp_y = self.y[channel] / self.n
        except:
            print "Warning Accumulation.get_ydata(channel): Channel index does not exist."
            return numarray.zeros((len(self.y[0]),),type="Float64")

        return tmp_y


    def get_ymin(self):
        tmp_min = []
        for i in range(self.get_number_of_channels()):
            tmp_min.append(self.get_ydata(i).min())

        if self.uses_statistics() and self.ready_for_drawing_error():
            for i in range(self.get_number_of_channels()):
                tmp_min.append(self.get_yerr(i).min())            

        return min(tmp_min)

    
    def get_ymax(self):
        tmp_max = []
        for i in range(self.get_number_of_channels()):
            tmp_max.append(self.get_ydata(i).max())

        if self.uses_statistics() and self.ready_for_drawing_error():
            for i in range(self.get_number_of_channels()):
                tmp_max.append(self.get_yerr(i).max())

        return max(tmp_max)

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
        tmp_string += "Jobs added:          " + str(self.jobs_added) + "\n"

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
                    return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = self.n, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added, error = self.use_error)
                else:
                    return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added, error = self.use_error)

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
                    return Accumulation(x = numarray.array(other.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = 1, index = other.index, sampl_freq = other.sampling_rate, jobs_added = 1, error = True)
                else:
                    return Accumulation(x = numarray.array(other.x, type="Float64"), y = tmp_y, index = other.index, sampl_freq = other.sampling_rate, jobs_added = 1, error = False)
                

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
                    return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = self.n + 1, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added + 1, error = True)
                else:
                    return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added + 1, error = False)

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
                    return Accumulation(x = numarray.array(other.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = other.n, index = other.index, sampl_freq = other.sampling_rate, jobs_added = other.jobs_added, error = True)
                else:
                    return Accumulation(x = numarray.array(other.x, type="Float64"), y = tmp_y, index = other.index, sampl_freq = other.sampling_rate, jobs_added = other.jobs_added, error = False)


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
                    return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, y_2 = tmp_ysquare, n = other.n, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added + other.jobs_added, error = True)
                else:
                    return Accumulation(x = numarray.array(self.x, type="Float64"), y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added + other.jobs_added, error = False)

            

    def __radd__(self, other):
        "Redefining other + self"
        return self.__add__(other)


    def __sub__(self, other):
        "Redefining self - other"

        # Ints or Floats
        if isinstance(other, IntType) or isinstance(other, FloatType):
            if not self.contains_data(): raise ValueError("Accumulation: You cant add integers/floats to an empty accumulation")
            else:
                tmp_y = []

                for i in range(self.get_number_of_channels()):
                    tmp_y.append(self.y[i] - other)

                return Accumulation(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added, error = self.use_error)

        # ADC-Results
        elif str(other.__class__) == "ADC_Result.ADC_Result":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (self = 0 - other)
            if not self.contains_data():

                tmp_y = []

                for i in range(other.get_number_of_channels()):
                    tmp_y.append(0 - numarray.array(other.y[i], type="Float64"))
                
                return Accumulation(x = other.x, y = tmp_y, index = other.index, sampl_freq = other.sampling_rate, jobs_added = 1, error = False)

            # Self and other not empty (self - other)
            else:
                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent sampling-rates")
                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of samples")
                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of channels")
                for i in range(len(self.index)):
                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add ADC-Results with diffrent indexing")

                tmp_y = []

                for i in range(self.get_number_of_channels()):
                    tmp_y.append(self.y[i] - other.y[i])

                return Accumulation(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added + 1, error = False)

        # Accumulation
        elif str(other.__class__) == "Accumulation.Accumulation":
            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (self = 0 - other)
            if not self.contains_data():

                tmp_y = []

                for i in range(other.get_number_of_channels()):
                    tmp_y.append(0 - other.y[i])
                
                return Accumulation(x = other.x, y = tmp_y, index = other.index, sampl_freq = other.sampling_rate, jobs_added = other.jobs_added, error = False)

            # Other and self not empty (self - other)
            else:
                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add accumulations with diffrent sampling-rates")
                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add accumulations with diffrent number of samples")
                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add accumulations with diffrent number of channels")
                for i in range(len(self.index)):
                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add accumulations with diffrent indexing")

                tmp_y = []

                for i in range(self.get_number_of_channels()):
                    tmp_y.append(self.y[i] - other.y[i])

                return Accumulation(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added + other.jobs_added, error = False)


    def __rsub__(self, other):
        "Redefining other - self"

        # Ints or Floats
        if isinstance(other, IntType) or isinstance(other, FloatType):
            if not self.contains_data(): raise ValueError("Accumulation: You cant add integers/floats to an empty accumulation")
            else:
                tmp_y = []

                for i in range(self.get_number_of_channels()):
                    tmp_y.append(other - self.y[i])

                return Accumulation(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added, error = self.use_error)

        # ADC-Results
        elif str(other.__class__) == "ADC_Result.ADC_Result":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (self = other - 0)
            if not self.contains_data():

                tmp_y = []

                for i in range(other.get_number_of_channels()):
                    tmp_y.append(numarray.array(other.y[i], type="Float64"))
                
                return Accumulation(x = other.x, y = tmp_y, index = other.index, sampl_freq = other.sampling_rate, jobs_added = 1, error = False)

            # Self and other not empty (other - self)
            else:
                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent sampling-rates")
                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of samples")
                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of channels")
                for i in range(len(self.index)):
                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add ADC-Results with diffrent indexing")

                tmp_y = []

                for i in range(self.get_number_of_channels()):
                    tmp_y.append(other.y[i] - self.y[i])

                return Accumulation(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added + 1, error = False)

        # Accumulation
        elif str(other.__class__) == "Accumulation.Accumulation":
            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (self = other - 0)
            if not self.contains_data():

                tmp_y = []

                for i in range(other.get_number_of_channels()):
                    tmp_y.append(other.y[i])
                
                return Accumulation(x = other.x, y = tmp_y, index = other.index, sampl_freq = other.sampling_rate, jobs_added = other.jobs_added, error = False)

            # Other and self not empty (self - other)
            else:
                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add accumulations with diffrent sampling-rates")
                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add accumulations with diffrent number of samples")
                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add accumulations with diffrent number of channels")
                for i in range(len(self.index)):
                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add accumulations with diffrent indexing")

                tmp_y = []

                for i in range(self.get_number_of_channels()):
                    tmp_y.append(other.y[i] - self.y[i])

                return Accumulation(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added + other.jobs_added, error = False)


    def __mul__(self, other):
        "Redefining self * other"

        # Ints or Floats
        if isinstance(other, IntType) or isinstance(other, FloatType):
            if not self.contains_data(): raise ValueError("Accumulation: You cant multiply integers/floats to an empty accumulation")
            else:
                tmp_y = []

                for i in range(self.get_number_of_channels()):
                    tmp_y.append(self.y[i] * other)

                return Accumulation(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added, error = self.use_error)


    def __rmul__(self, other):
        "Redefining other * self"
        return self.__mul__(other)


    def __pow__(self, other):
        "Redefining self ** other"
        
         # Ints or Floats
        if isinstance(other, IntType) or isinstance(other, FloatType):
            if not self.contains_data(): raise ValueError("Accumulation: You cant multiply integers/floats to an empty accumulation")
            else:
                tmp_y = []

                for i in range(self.get_number_of_channels()):
                    tmp_y.append(self.y[i] ** other)

                return Accumulation(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added, error = self.use_error)
       

    def __div__(self, other):
        "Redefining self / other"

        # Ints or Floats
        if isinstance(other, IntType) or isinstance(other, FloatType):
            if not self.contains_data(): raise ValueError("Accumulation: You cant multiply integers/floats to an empty accumulation")
            else:
                tmp_y = []

                for i in range(self.get_number_of_channels()):
                    tmp_y.append(self.y[i] / other)

                return Accumulation(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added, error = self.use_error)


    def __rdiv__(self, other):
        "Redefining other / self"

        # Ints or Floats
        if isinstance(other, IntType) or isinstance(other, FloatType):
            if not self.contains_data(): raise ValueError("Accumulation: You cant multiply integers/floats to an empty accumulation")
            else:
                tmp_y = []

                for i in range(self.get_number_of_channels()):
                    tmp_y.append(other / self.y[i])

                return Accumulation(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added, error = self.use_error)


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
                
                if self.uses_statistics(): self.n += 1
                self.jobs_added = 1
                self.index = other.index[0:]
                self.sampling_rate = other.sampling_rate
                self.x = numarray.array(other.x, type="Float64")
                self.cont_data = True

                for i in range(other.get_number_of_channels()):
                    self.y.append(numarray.array(other.y[i], type="Float64"))
                    if self.uses_statistics(): self.y_square.append(self.y[i] ** 2)

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

                if self.uses_statistics():
                    self.n += 1

                self.jobs_added += 1

                return self

        # Accumulation
        elif str(other.__class__) == "Accumulation.Accumulation":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (copy)
            if not self.contains_data():
                if self.uses_statistics() and not other.uses_statistics(): raise ValueError("Accumulation: You cant add non-error accumulations to accumulations with error")

                if self.uses_statistics(): self.n += other.n
                self.jobs_added = other.jobs_added
                self.index = other.index[0:]
                self.sampling_rate = other.sampling_rate
                self.x = numarray.array(other.x, type="Float64")
                self.cont_data = True

                for i in range(other.get_number_of_channels()):
                    self.y.append(numarray.array(other.y[i], type="Float64"))
                    if self.uses_statistics(): self.y_square.append(self.y[i] ** 2)

                return self
            
            # Other and self not empty (self + other)
            else:
                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add accumulations with diffrent sampling-rates")
                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add accumulations with diffrent number of samples")
                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add accumulations with diffrent number of channels")
                for i in range(len(self.index)):
                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add accumulations with diffrent indexing")
                if self.uses_statistics() and not other.uses_statistics(): raise ValueError("Accumulation: You cant add non-error accumulations to accumulations with error")

                if self.uses_statistics():
                    self.n += other.n
                    
                self.jobs_added += other.jobs_added

                for i in range(self.get_number_of_channels()):
                    self.y[i] += other.y[i]
                    if self.uses_statistics(): self.y_square[i] += other.y_square[i]
                    

                return self


    def __isub__(self, other):
        "Redefining self -= other"
        # Float or int
        if isinstance(other, IntType) or isinstance(other, FloatType):
            if not self.contains_data(): raise ValueError("Accumulation: You cant add integers/floats to an empty accumulation")
            else:
                for i in range(self.get_number_of_channels()):
                    self.y[i] -= other

                return self

        # ADC_Result
        elif str(other.__class__) == "ADC_Result.ADC_Result":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (self = 0 - other)
            if not self.contains_data():

                tmp_y = []
                
                for i in range(other.get_number_of_channels()):
                    tmp_y.append(0 - numarray.array(other.y[i], type="Float64"))
                
                return Accumulation(x = other.x, y = tmp_y, index = other.index, sampl_freq = other.sampling_rate, jobs_added = 1, error = False)

            # Other and self not empty (self -= other)
            else:
                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent sampling-rates")
                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of samples")
                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of channels")
                for i in range(len(self.index)):
                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add ADC-Results with diffrent indexing")

                for i in range(self.get_number_of_channels()):
                   self.y[i] -= other.y[i]

                self.jobs_added += 1

                return self

        # Accumulation
        elif str(other.__class__) == "Accumulation.Accumulation":

            # Other empty (return)
            if not other.contains_data(): return

            # Self empty (self = 0 - other)
            if not self.contains_data():

                tmp_y = []

                for i in range(other.get_number_of_channels()):
                    tmp_y.append(0 - other.y[i])
                
                return Accumulation(x = other.x, y = tmp_y, index = other.index, sampl_freq = other.sampling_rate, jobs_added = other.jobs_added, error = False)

            # Other and self not empty (self + other)
            else:
                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add accumulations with diffrent sampling-rates")
                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add accumulations with diffrent number of samples")
                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add accumulations with diffrent number of channels")
                for i in range(len(self.index)):
                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add accumulations with diffrent indexing")

                for i in range(self.get_number_of_channels()):
                    self.y[i] -= other.y[i]

                self.jobs_added += other.jobs_added

                return self

# Bedarf genauerer Betrachtung (Multiplizieren von ADC / Accu)
##        # ADC-Results
##        elif str(other.__class__) == "ADC_Result.ADC_Result":
##
##            # Other empty (return)
##            if not other.contains_data(): return
##
##            # Self empty (self = 0 * other)
##            if not self.contains_data():
##
##                tmp_y = []
##
##                for i in range(other.get_number_of_channels()):
##                    tmp_y.append(0 * numarray.array(other.get_ydata(i), type="Float64"))
##                
##                return Accumulation(x = other.x, y = tmp_y, index = other.index, sampl_freq = other.sampling_rate, jobs_added = 1, error = False)
##
##            # Self and other not empty (self - other)
##            else:
##                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent sampling-rates")
##                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of samples")
##                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add ADC-Results with diffrent number of channels")
##                for i in range(len(self.index)):
##                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add ADC-Results with diffrent indexing")
##
##                tmp_y = []
##
##                for i in range(self.get_number_of_channels()):
##                    tmp_y.append(self.get_ydata(i) * other.get_ydata(i))
##
##                return Accumulation(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added + 1, error = False)
##
##        # Accumulation
##        elif str(other.__class__) == "Accumulation.Accumulation":
##            # Other empty (return)
##            if not other.contains_data(): return
##
##            # Self empty (self = 0 - other)
##            if not self.contains_data():
##
##                tmp_y = []
##
##                for i in range(other.get_number_of_channels()):
##                    tmp_y.append(0 - numarray.array(other.get_ydata(i), type="Float64"))
##                
##                return Accumulation(x = other.x, y = tmp_y, index = other.index, sampl_freq = other.sampling_rate, jobs_added = other.jobs_added, error = False)
##
##            # Other and self not empty (self - other)
##            else:
##                if self.sampling_rate != other.get_sampling_rate(): raise ValueError("Accumulation: You cant add accumulations with diffrent sampling-rates")
##                if len(self.y[0]) != len(other): raise ValueError("Accumulation: You cant add accumulations with diffrent number of samples")
##                if len(self.y) != other.get_number_of_channels(): raise ValueError("Accumulation: You cant add accumulations with diffrent number of channels")
##                for i in range(len(self.index)):
##                    if self.index[i] != other.get_index_bounds(i): raise ValueError("Accumulation: You cant add accumulations with diffrent indexing")
##
##                tmp_y = []
##
##                for i in range(self.get_number_of_channels()):
##                    tmp_y.append(self.get_ydata(i) - other.get_ydata(i))
##
##                return Accumulation(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, jobs_added = self.jobs_added + other.jobs_added, error = False)
