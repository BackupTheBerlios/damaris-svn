# -*- coding: iso-8859-1 -*-
# Usage: Result(Channels, Samples per Channel)

from types import *
import numarray

class Accumulation:

    def __init__(self, in_channels, in_samples, **keywords):

        #self.description = { }
        
        # Normaler Konstruktor
        if len(keywords) is 0:

            self.channels = numarray.array([0], type = "Int32")
            self.channels = numarray.resize(self.channels, (in_channels, in_samples))

            self.x = numarray.array([0], type = "Float64")
            self.x = numarray.resize(self.x, (1, in_samples))

            self.samples = in_samples
            self.sampling_rate = None

        # Copy-Konstruktor
        elif len(keywords) == 3:

            for key in keywords.keys():
                if key == "channels":
                    self.channels = numarray.array(keywords["channels"], type = "Int32")
                elif key == "x":
                    self.x = numarray.array(keywords["x"], type="Float64")
                elif key == "sampling_rate":
                    self.sampling_rate = keywords["sampling_rate"]

        # Hä?
        else:
            raise TypeError, "Accumulation: Wrong usage of copy-constructor: expected 3 keywords, got %d" % len(keywords)
                
        

    def get_number_of_samples(self):
        return self.samples


    def get_number_of_channels(self):
        return (self.channels.getshape())[0]


    def set_sampling_rate(self, in_sampling_rate):
        self.sampling_rate = in_sampling_rate


    def get_sampling_rate(self):
        return self.sampling_rate


##    def addDescription(self, in_key, in_value):
##        if self.description.has_key(in_key): print "Warning: Overwriting existing key \"%s\" (%s) with \"%s\"." % (str(in_key), str(self.description[in_key]), str(in_value))
##        self.description[in_key] = in_value
##
##
##    def getDescription(self, in_key):
##        if self.description.has_key(in_key):
##            return self.description[in_key]
##        else:
##            print "Warning: No existing key \"%s\" in descriptions." % in_key


    def getDescriptionList(self):
        return self.description.keys()


    def getChannel(self, in_nr):
        return self.channels[in_nr]


    def getChannels(self):
        return self.channels


    def setValue(self, in_channel, in_pos, in_value):
        try:
            self.channels[in_channel, in_pos] = in_value
            if in_value > self.y_max: self.y_max = in_value
            if in_value < self.y_min: self.y_min = in_value
        except:
            raise


    def setXValue(self, in_pos, in_value):
        try:
            self.x[0, in_pos] = in_value
            if in_value > self.x_max: self.x_max = in_value
            if in_value < self.x_min: self.x_min = in_value
        except:
            raise


    def getXValues(self):
        return self.x[0]


    def getXValue(self, in_pos):
        try:
            return self.x[0, in_pos]
        except:
            raise


    def getYValue(self, in_channel, in_pos):
        try:
            return self.channels[in_channel, in_pos]
        except:
            raise


    def size(self):
        return self.channels.getshape()


    def get_xmax(self):
        return max(self.x)


    def get_ymax(self):
        return max(self.channels)


    def get_xmin(self):
        return min(self.x)


    def get_ymin(self):
        return min(self.channels)


    def __iadd__(self, other):
        if type(other) is IntType:
            self.channels += other
            return Accumulation(self.get_number_of_channels(), self.samples, channels = self.channels, x = self.x, sampling_rate = self.sampling_rate)

        elif type(other) is FloatType:
            print "Accumulation: Warning, converting accumulation to float might result in incorrect data (due to rounding-errors)!"
            self.channels += other
            return Accumulation(self.get_number_of_channels(), self.samples, channels = self.channels, x = self.x, sampling_rate = self.sampling_rate)
        
        else:
            raise TypeError, "Cannot add %s to accumulation-object!" % str(type(other))



