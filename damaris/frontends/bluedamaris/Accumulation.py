# -*- coding: iso-8859-1 -*-

#################################################################
#                                                               #
# Class: Accumulation                                           #
#                                                               #
# Purpose: Represents an accumulation of results. Whenever two  #
#          results will be added (or similiar operations),      #
#          an accumulation will be returned.                    #
#                                                               #
#################################################################

from types import *
import numarray

class Accumulation:

    def __init__(self, **keywords):

        # Todo---------------
        self.jobs_added = 0
    
        
        # Normaler Konstruktor
        if len(keywords) is 0:

            self.channels = None
            self.x = None

            self.samples = None
            self.sampling_rate = None

        # Copy-Konstruktor
        elif len(keywords) == 4:
            ctr = 0

            for key in keywords.keys():
                if key == "channel_array":
                    self.channels = numarray.array(keywords["channel_array"], type = "Int32")
                    ctr += 1
                elif key == "x":
                    self.x = numarray.array(keywords["x"], type="Float64")
                    ctr += 1
                elif key == "sampling_rate":
                    self.sampling_rate = keywords["sampling_rate"]
                    ctr += 1
                elif key == "number_of_samples":
                    self.samples = keywords["number_of_samples"]
                    ctr += 1

            if ctr is not 4:
                raise TypeError("Accumulation: Wrong usage of copy-constructor: expected 0 or 4 keywords, got %d" % ctr)            

            self.x = numarray.reshape(self.x, (self.samples,))

        else:
            raise TypeError("Accumulation: Wrong usage of copy-constructor: expected 0 or 4 keywords, got %d" % len(keywords))
                

    # Public Methods -------------------------------------------------------------------------------     

    def get_number_of_samples(self):
        "Returns the number of samples recorded"
        return self.samples


    def is_empty(self):
        "Returns true if the accumulation is still empty"
        if self.channels is None: return True
        else: return False


    def get_number_of_channels(self):
        "Returns the number of channels"
        return (self.channels.getshape())[0]


    def get_sampling_rate(self):
        "Returns the sampling frequency used"
        return self.sampling_rate


    def get_channel(self, in_nr):
        "Gets the number of channels"
        return self.channels[in_nr]


    def get_channels(self):
        "Returns all channels"
        return self.channels


    def get_value(self, in_channel, in_pos):
        "Returns a value"
        try:
            return self.channels[in_channel, in_pos]
        except:
            raise
        
    # /Public Methods ------------------------------------------------------------------------------
    
    # Public methods (internally used) -------------------------------------------------------------

    def set_sampling_rate(self, in_sampling_rate):
        "Sets the sampling frequency (internally used)"
        self.sampling_rate = in_sampling_rate


    def set_value(self, in_channel, in_pos, in_value):
        "Sets a value (internally used)"
        try:
            self.channels[in_channel, in_pos] = in_value
            if in_value > self.y_max: self.y_max = in_value
            if in_value < self.y_min: self.y_min = in_value
        except:
            raise


    def set_xvalue(self, in_pos, in_value):
        "Sets an x-value (internally used for plotting)"
        try:
            self.x[in_pos] = in_value
            if in_value > self.x_max: self.x_max = in_value
            if in_value < self.x_min: self.x_min = in_value
        except:
            raise


    def get_xvalues(self):
        "Returns all x-values (internally used for plotting)"
        return self.x


    def get_xvalue(self, in_pos):
        "Returns a single x-value (internally used for plotting)"
        try:
            return self.x[in_pos]
        except:
            raise

    def get_xmax(self):
        "Returns largest x-value (only used internally for plotting)"
        return max(self.x)


    def get_ymax(self):
        "Returns largest y-value (only used internally for plotting)"
        maximum = []
        for i in range(self.get_number_of_channels()):
            maximum.append(max(self.get_channel(i)))
            
        return max(maximum)


    def get_xmin(self):
        "Returns smallest x-value (only used internally for plotting)"
        return min(self.x)


    def get_ymin(self):
        "Returns smallest y-value (only used internally for plotting)"

        minimum = []
        for i in range(self.get_number_of_channels()):
            minimum.append(min(self.get_channel(i)))        
        
        return min(minimum)
    
    # /Public Methods (internally used) ------------------------------------------------------------

    # Overloading Operators ------------------------------------------------------------------------

    # Overloading += -------------------------
    def __iadd__(self, other):

        try:
            # Integer soll aufaddiert werden
            if isinstance(other, IntType):
                if self.channels is None:
                    raise TypeError("Accumulation: Cannot add integer-offset to an empty accumulation!")
                
                self.channels += other
                return self
            # Float soll aufaddiert werden
            elif isinstance(other, FloatType):
                if self.channels is None:
                    raise TypeError("Accumulation: Cannot add float-offset to an empty accumulation!")

                print "Accumulation: Warning, converting accumulation to float might result in incorrect data (due to rounding-errors)!"
                self.channels += other
                return self

            # Result soll aufaddiert werden
            elif str(other.__class__) == "Result.Result":         
                if self.channels is None:
                    self.channels = numarray.array(other.get_channels(), type="Int32")
                    self.x = numarray.array(other.get_xvalues())
                    self.sampling_rate = other.get_sampling_rate() + 0
                    self.samples = other.get_number_of_samples() + 0
                    return self
                else:
                    if self.sampling_rate != other.get_sampling_rate():
                        print "Accumulation warning: Adding results to accumulation with diffrent sampling-rates (%d, %d)!" % (self.sampling_rate, other.get_sampling_rate())

                    self.channels += other.get_channels()
                    # X-Kanäle neu setzen? self.x = numarray.array(other.getXValues())
                    return self

            # Akkumulation soll aufaddiert werden
            elif str(other.__class__) == "Accumulation.Accumulation":

                if other.is_empty():
                    return self
                
                if self.channels is None:
                    self.channels = numarray.array(other.get_channels(), type="Int32")
                    self.x = numarray.array(other.get_xvalues())
                    self.sampling_rate = other.get_sampling_rate() + 0
                    self.samples = other.get_number_of_samples() + 0
                    return self
                else:
                    if self.sampling_rate is not other.get_sampling_rate():
                        print "Accumulation warning: Adding results to accumulation with diffrent sampling-rates (%d, %d)!" % (self.sampling_rate, other.get_sampling_rate())
                    self.channels += other.get_channels()
                    # X-Kanäle neu setzen? self.x = numarray.array(other.getXValues())
                    return self                

                
            # Anderes geht nicht.   
            else:
                raise TypeError, "Cannot add %s to accumulation-object!" % str(other.__class__)

        except:
            raise


    # Overloading + ----------------------------
    def __add__(self, other):
        try:
            # Integer soll aufaddiert werden
            if isinstance(other, IntType):
                if self.channels is None:
                    return other
                
                return Accumulation(number_of_samples = self.samples, channel_array = self.channels + other, x = self.x, sampling_rate = self.sampling_rate)

            # Float soll aufaddiert werden
            elif isinstance(other, FloatType):
                if self.channels is None:
                    return other

                print "Accumulation: Warning, converting accumulation to float might result in incorrect data (due to rounding-errors)!"
                return Accumulation(number_of_samples = self.samples, channel_array = self.channels + other, x = self.x, sampling_rate = self.sampling_rate)

            # Result soll aufaddiert werden
            elif str(other.__class__) == "Result.Result":
                if self.channels is None:
                    return Accumulation(number_of_samples = other.get_number_of_samples(), channel_array = other.get_channels(), x = other.get_xvalues(), sampling_rate = other.get_sampling_rate())

                else:
                    if self.sampling_rate is not other.get_sampling_rate():
                        print "Accumulation warning: Adding results to accumulation with diffrent sampling-rates!"

                    # X-Kanäle neu setzen? self.x = numarray.array(other.getXValues())
                    return Accumulation(number_of_samples = self.get_number_of_samples(), channel_array = self.channels + other.get_cannels(), x = self.x, sampling_rate = self.sampling_rate)

            # Akkumulation soll aufaddiert werden
            elif str(other.__class__) == "Accumulation.Accumulation":

                # Akku + 0 = Akku
                if other.is_empty() and not self.is_empty():
                    return Accumulation(number_of_samples = self.samples, channel_array = self.channels, sampling_rate = self.sampling_rate, x = self.x)

                # 0 + 0 = 0
                else: return Accumulation()

                # 0 + Akku2 = Akku2
                if self.channels is None:
                    return Accumulation(number_of_samples = other.get_number_of_samples(), channel_array = other.get_channels(), x = other.get_xvalues(), sampling_rate = other.get_sampling_rate()) 

                # Akku1 + Akku2 = Akku3
                else:
                    if self.sampling_rate is not other.get_sampling_rate():
                        print "Accumulation warning: Adding accumulation to accumulation with diffrent sampling-rates!"

                    # X-Kanäle neu setzen? self.x = numarray.array(other.getXValues())
                    return Accumulation(number_of_samples = self.get_number_of_samples(), channel_array = self.channels + other.get_channels(), x = self.x, sampling_rate = self.sampling_rate)               

                
            # Anderes geht nicht.   
            else:
                raise TypeError, "Cannot add %s to accumulation-object!" % str(type(other))

        except:
            raise



    def __str__(self):
        return "<Accumulation-Type: channels = " + str(self.channels) + ", x = " + str(self.x) + ", samples = " + str(self.samples) + ", sampling_rate = " + str(self.sampling_rate) + ">"
