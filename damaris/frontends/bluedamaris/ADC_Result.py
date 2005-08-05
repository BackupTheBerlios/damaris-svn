# -*- coding: iso-8859-1 -*-

from Resultable import Resultable
from Drawable import Drawable
from Accumulation import Accumulation

import numarray
from types import *

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
     

        if (x is None) and (y is None) and (index is None) and (sampl_freq is None) and (desc is None) and (job_id is None) and (job_date is None):
            self.cont_data = False
            self.sampling_rate = 0
            self.index = []

        elif (x is not None) and (y is not None) and (index is not None) and (sampl_freq is not None) and (desc is not None) and (job_id is not None) and (job_date is not None):
            self.x = x
            self.y = y
            self.index = index
            self.sampling_rate = sampl_freq
            self.cont_data = True
            self.description = desc
            self.job_id = job_id
            self.job_date = job_date

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
            self.y.append(numarray.array([0]*samples, type="Int16"))

        self.x = numarray.array([0]*samples, type="Float64")

        self.index.append((0, samples-1))
        self.cont_data = True


    def contains_data(self):
        "Returns true if ADC_Result contains data. (-> create_data_space() was called)"
        return self.cont_data


    def add_sample_space(self, samples):
        "Adds space for n samples, where n can also be negative (deletes space). New space is filled up with \"0\""

        if not self.cont_data:
            print "Warning ADC-Result: Tried to resize empty array!"
            return

        length = len(self.y[0])

        self.x = numarray.resize(self.x, (length+samples))

        for i in range(self.get_number_of_channels()):
            self.y[i] = numarray.resize(self.y[i], (length+samples))

        self.index.append((length, len(self.y[0])-1))


    def get_result_by_index(self, index):

        try:
            start = self.index[index][0]
            end = self.index[index][1]
        except:
            raise

        tmp_x = self.x[start:end+1]
        tmp_y = []

        for i in range(self.get_number_of_channels()):
            tmp_y.append(self.y[i][start:end+1])

        return ADC_Result(x = tmp_x, y = tmp_y,  index = [(0,len(tmp_y[0])-1)], sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)            


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
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numarray.array(self.y[i], type="Float64") + other)

            return ADC_Result(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)

        else:
            raise ValueError("ValueError: Cannot add \"%s\" to ADC-Result!") % str(other.__class__)


    def __radd__(self, other):
        "Redefining other + self"
        return self.__add__(other)


    def __sub__(self, other):
        "Redefining self - other"
        if isinstance(other, IntType) or isinstance(other, FloatType):
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numarray.array(self.y[i], type="Float64") - other)

            return ADC_Result(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)

        else:
            raise ValueError("ValueError: Cannot subtract \"%s\" to ADC-Result!") % str(other.__class__)


    def __rsub__(self, other):
        "Redefining other - self"
        if isinstance(other, IntType) or isinstance(other, FloatType):
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(other - numarray.array(self.y[i], type="Float64"))

            return ADC_Result(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)

        else:
            raise ValueError("ValueError: Cannot subtract \"%s\" to ADC-Result!") % str(other.__class__)


    def __mul__(self, other):
        "Redefining self * other"
        if isinstance(other, IntType) or isinstance(other, FloatType):
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numarray.array(self.y[i], type="Float64") * other)

            return ADC_Result(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)

        else:
            raise ValueError("ValueError: Cannot multiply \"%s\" to ADC-Result!") % str(other.__class__)    


    def __rmul__(self, other):
        "Redefining other * self"
        return self.__mul__(other)


    def __pow__(self, other):
        "Redefining self ** other"
        if isinstance(other, IntType) or isinstance(other, FloatType):
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numarray.array(self.y[i], type="Float64") ** other)

            return ADC_Result(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)

        else:
            raise ValueError("ValueError: Cannot multiply \"%s\" to ADC-Result!") % str(other.__class__)

        
    def __div__(self, other):
        "Redefining self / other"
        if isinstance(other, IntType) or isinstance(other, FloatType):
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(numarray.array(self.y[i], type="Float64") / other)

            return ADC_Result(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)

        else:
            raise ValueError("ValueError: Cannot multiply \"%s\" to ADC-Result!") % str(other.__class__)


    def __rdiv__(self, other):
        "Redefining other / self"
        if isinstance(other, IntType) or isinstance(other, FloatType):
            tmp_y = []

            for i in range(self.get_number_of_channels()):
                tmp_y.append(other / numarray.array(self.y[i], type="Float64"))

            return ADC_Result(x = self.x, y = tmp_y, index = self.index, sampl_freq = self.sampling_rate, desc = self.description, job_id = self.job_id, job_date = self.job_date)

        else:
            raise ValueError("ValueError: Cannot multiply \"%s\" to ADC-Result!") % str(other.__class__)
