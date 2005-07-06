# -*- coding: iso-8859-1 -*-
# Usage: Result(Channels, Samples per Channel)

import numarray

class Result:

    def __init__(self, in_channels, in_samples, **keywords):

        self.description = { }
        self.channels = numarray.array([0], type = "Int16")
        self.channels = numarray.resize(self.channels, (in_channels, in_samples))

        self.x = numarray.array([0], type = "Float64")
        self.x = numarray.resize(self.x, (in_samples,))

        self.job_nr = None
        self.samples = in_samples
        self.sampling_rate = None

        self.x_max = -32768
        self.y_max = -32768

        self.x_min = 32768
        self.y_min = 32768

        if len(keywords) is not 0:
            for key in keywords.keys():
                if key == "sampling_rate":
                    self.sampling_rate = keywords[key]


    def set_job_number(self, in_job_nr):
        self.job_nr = in_job_nr


    def get_job_id(self):
        return self.job_nr


    def get_job_number(self):
        return self.job_nr


    def get_number_of_samples(self):
        return self.samples

    def get_samples(self):
        return self.samples


    def get_number_of_channels(self):
        return (self.channels.getshape())[0]


    def set_sampling_rate(self, in_sampling_rate):
        self.sampling_rate = in_sampling_rate


    def get_sampling_rate(self):
        return self.sampling_rate


    def add_description(self, in_key, in_value):
        if self.description.has_key(in_key): print "Warning: Overwriting existing key \"%s\" (%s) with \"%s\"." % (str(in_key), str(self.description[in_key]), str(in_value))
        self.description[in_key] = in_value


    def get_description(self, in_key):
        if self.description.has_key(in_key):
            return self.description[in_key]
        else:
            print "Warning: No existing key \"%s\" in descriptions." % in_key


    def get_description_list(self):
        return self.description.keys()


    def get_channel(self, in_nr):
        return self.channels[in_nr]


    def get_channels(self):
        return self.channels


    def set_value(self, in_channel, in_pos, in_value):
        try:
            self.channels[in_channel, in_pos] = in_value
            if in_value > self.y_max: self.y_max = in_value
            if in_value < self.y_min: self.y_min = in_value
        except:
            raise


    def set_xvalue(self, in_pos, in_value):
        try:
            self.x[in_pos] = in_value
            if in_value > self.x_max: self.x_max = in_value
            if in_value < self.x_min: self.x_min = in_value
        except:
            raise


    def get_xvalues(self):
        return self.x



    def get_xvalue(self, in_pos):
        try:
            return self.x[in_pos]
        except:
            raise


    def get_value(self, in_channel, in_pos):
        try:
            return self.channels[in_channel, in_pos]
        except:
            raise


    def get_xmax(self):
        return self.x_max


    def get_ymax(self):
        return self.y_max


    def get_xmin(self):
        return self.x_min


    def get_ymin(self):
        return self.y_min


    # Überladen der Operatoren ---------------------------------------------------------------------

    # Überladen von += ---------------------------
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
                    if self.sampling_rate is not other.get_sampling_rate():
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
                raise TypeError, "Cannot add %s to accumulation-object!" % str(type(other))

        except:
            raise


    # Überladen von + ----------------------------
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
                        print "Accumulation warning: Adding results to accumulation with diffrent sampling-rates (%d, %d)!" % (self.sampling_rate, other.get_sampling_rate())

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
                        print "Accumulation warning: Adding results to accumulation with diffrent sampling-rates (%d, %d)!" % (self.sampling_rate, other.get_sampling_rate())

                    # X-Kanäle neu setzen? self.x = numarray.array(other.getXValues())
                    return Accumulation(number_of_samples = self.get_number_of_samples(), channel_array = self.channels + other.get_channels(), x = self.x, sampling_rate = self.sampling_rate)               

                
            # Anderes geht nicht.   
            else:
                raise TypeError, "Cannot add %s to accumulation-object!" % str(type(other))

        except:
            raise



    def __str__(self):
        return "<Result-Type: channels = " + str(self.channels) + ", x = " + str(self.x) + ", samples = " + str(self.samples) + ", sampling_rate = " + str(self.sampling_rate) + ">"


