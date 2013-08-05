# -*- coding: iso-8859-1 -*-
import types
import numpy

class StateBase(object):
    def __init__(self):
        pass
    def to_xml(self, indent = ""):
        return indent + "<!-- " + repr(self) + " -->"

class StateSimple(StateBase):
    def __init__(self, time, content=None):
        super(StateSimple, self).__init__()
        if time < 0:
            raise AssertionError("time for state is negative!")
        self.time = time
        self.content = content

    def to_xml(self, indent = ""):
        s = indent + '<state time="%s"' % repr(self.time)
        if self.content is None:
            return s + '/>\n'
        s += '>\n'
        s += indent + '  ' + str(self.content) + '\n'
        s += indent + '</state>\n'
        return s
    def __repr__(self):
        return 'StateSimple(%s, %s)' % (self.time, repr(self.content))

class StateList(StateBase):
    def __init__(self):
        super(StateList, self).__init__()
        self.list = []
    def to_xml(self, indent = "  "):
        s = ""
        for k in self.list:
            if hasattr(k, "to_xml"):
                s += k.to_xml(indent)
            else:
                s += indent + str(k)
        return s
    def append(self, val):
        self.list.append(val)


class StateLoop(StateList):
    """Represents a loop in the state tree"""
    def __init__(self, repeat):
        super(StateLoop, self).__init__()
        self.repeat = repeat
    def to_xml(self, indent = ""):
        s = indent + ('<sequent repeat="%d">\n' % self.repeat)
        s += super(StateLoop, self).to_xml(indent + "  ")
        s += indent + '</sequent>\n'
        return s
    def __repr__(self):
        return 'StateLoop(repeat=%d, %s)' \
            % (self.repeat, repr(self.list))


#############################################################
#                                                           #
# Class: Experiment                                         #
#                                                           #
# Purpose: Represents one full experiment (one program on   #
#          the pulse-card; one file)                        #
#                                                           #
#############################################################
import dac

class Experiment:
    ## Experiment class holding the state tree

    job_id = 0
    
    def __init__(self):
        self.job_id = Experiment.job_id
        Experiment.job_id += 1

        self.state_list = StateList()
        self.list_stack = []
        self.description = { }


    # Commands -------------------------------------------------------------------------------------
    ## Deprecated
    def rf_pulse(self, value, length = None):
        """
        deprecated: use ttl_pulse
        """
        s_content = '<ttlout value="0x%06x"/>' % value

        if length is None:
            self.state_list.append(s_content)
        else:
            self.state_list.append(StateSimple(length, s_content))
    
    ## Creates a state with ttl signals of duration *length*.
    #
    # **Example:**
    # ttl_pulse(length=1e-6,value=3) 
    # will create a ttl pulse on channels 0 and 1 (2**0 + 2**1) of duration 1us
    # @param length time length if this state
    # @param channel select a single channel (1...24)
    # @param value select the channels via decimal representation (2**0 + 2**1 ...)
    def ttl_pulse(self, length, channel = None, value = None):
        """
        Creates a state with length *length* and switches 
        some bits of the pulse programmer to HIGH:
         * channel: this selects a single channel (No. 1 - 24)
         * value: this is the integer representation of the 24bit word, 
                  as an example value=3 selects channels 1 and 2 (2**1 + 2**2)
        """
        the_value=0
        if value is not None:
            the_value=int(value)
        elif channel is not None:
            the_value=1<<channel
        self.state_list.append(StateSimple(length, \
            '<ttlout value="0x%06x"/>' % the_value))

    ## Same as ttl_pulse, but no *channel* keyword
    def ttls(self, length = None, value = None):
        """
        same as ttl_pulse, but no *channel* keyword
        """
        the_value=int(value)
        s_content = '<ttlout value="0x%06x"/>' % the_value
        if length is not None:
            self.state_list.append(StateSimple(length, s_content))
        else:
            self.state_list.append(s_content)
    ## Beginning of a new state
    def state_start(self, time):
        """
        starts a state in the pulse programs with duration *time*. 
        This must be closed with state_end
        """
        self.state_list.append('<state time="%s">\n' % repr(time))

    ## End of *state_start*
    def state_end(self):
        """
        closes a state after start_state 
        """
        self.state_list.append('</state>\n')

    ## An empty state doing nothing
    # @param time Duration of this state
    # @param ttls Additinional ttl channels
    def wait(self, time, ttls=None):
        if ttls is not None:
            s_content = '<ttlout value="0x%06x"/>' % ttls
            self.state_list.append(StateSimple(time,s_content))
        else:
            self.state_list.append(StateSimple(time))

    ## Records  data with given number of samples, sampling-frequency frequency and sensitivity
    # @param samples Number of samples to record
    # @param frequency Sampling frequency
    # @param timelength Length of this state, per default calculated automatically
    # @param sensitivity Sensitivity in Umax/V
    # @param ttls Additional ttl channels
    def record(self, samples, frequency, timelength=None, sensitivity = None, ttls=None, channels = 3, offset = None, impedance = None):
        attributes='s="%d" f="%d"'%(samples,frequency)#%g
        if channels != 1 and channels != 3 and channels != 5 and channels != 15:
            raise ValueError, "Channel definition is illegal"
        attributes += ' channels="%i"'%(channels)
        
        nchannels = 0
        if channels == 1:
            nchannels = 1
        elif channels == 3 or channels == 5:
            nchannels = 2
        elif channels == 15:
            nchannels = 4
        if sensitivity is not None:
            # float values are allowed and applied to all channels
            if isinstance(sensitivity, float) or isinstance(sensitivity, int):
                for i in range(nchannels):
                    attributes +=' sensitivity%i="%f"'%(i, float(sensitivity))
            else:
                for i in range(nchannels):
                    attributes +=' sensitivity%i="%f"'%(i, sensitivity[i])
        if offset is not None:
            # int values are allowed and applied to all channels
            if isinstance(offset, int):
                for i in range(nchannels):
                    attributes +=' offset%i="%f"'%(i, offset)
            else:
                for i in range(nchannels):
                    attributes +=' offset%i="%f"'%(i, offset[i])
        if impedance is not None:
            # float values are allowed and applied to all channels
            if isinstance(impedance, float):
                for i in range(nchannels):
                    attributes += ' impedance%i="%i"'%(i, impedance)
            else:
                for i in range(nchannels):
                    attributes += ' impedance%i="%i"'%(i, impedance[i])
                
        s_content = '<analogin %s/>' % attributes
        if ttls is not None:
            s_content+='<ttlout value="0x%06x"/>' % ttls
        if timelength is None:
            timelength = samples / float(frequency)#*1.01
        self.state_list.append(StateSimple(timelength, s_content))

    ## Create a loop on the pulse programmer. Loop contents can not change inside the loop.
    # @params iterations Number of loop iterations
    def loop_start(self, iterations):
        """creates a loop of given number of iterations and has to be closed by loop_end().
        Commands inside the loop can not change, i.e. the parameters are the same for each loop run. 
        This loop is created on the pulse programmer, thus saving commands.
        One must close the loop with loop_end (see below)"""
        l = StateLoop(iterations)
        self.state_list.append(l)
        # (These two lines could probably be guarded by a mutex)
        self.list_stack.append(self.state_list)
        self.state_list = l

    ## End loop state
    def loop_end(self):
        # (This line could probably be guarded by a mutex)
        self.state_list = self.list_stack.pop(-1)

    ## Set the frequency and phase of the frequency source.
    ## This state needs 2us.
    # @param frequency New frequency in Hz
    # @param phase New phase in degrees 
    # @param ttls Additional ttl channels
    def set_frequency(self, frequency, phase, ttls=0):
        """
        Sets the frequency and phase of the frequency source and optionally further channels. 
        The time needed to set the frequency is 2 us.
        Switch pulse programmer line with *ttls* .

        """
        "Sets the frequency generator to a desired frequency (Hz)"
        s_content = '<analogout id="0" f="%f" phase="%f"/>' % (frequency, phase)
        if ttls != 0:
            s_content += '<ttlout value="0x%06x"/>' % ttls
        self.state_list.append(StateSimple(2e-6, s_content))
    ## Creates a, possibly shaped, pulsed gradient.
    # @param dac_value DAC value to set
    # @param length Duration of the state, minimum length is 42*90ns=3.78us (default)
    # @param shape Tuple of (shape, resolution/seconds), shape can be one of: rec (default), sin2, sin
    # @param is_seq If set to *True*, do NOT set DAC to zero after this state
    # @param trigger Additional ttl channels
    def set_pfg(self, dac_value=None, length=None, shape=('rec',0), trigger=4, is_seq=False):
        """
        This sets the value for the PFG, it also sets it back automatically.
        If you don't whish to do so (i.e. line shapes)  set is_seq=1
        If you wnat to set a trigger, set trigger (default=4, i.e. channel 2)
        If you want shaped gradients: shape=(ashape, resolution), ashape can be rec, sin2, sin

        """
        try:
            form, resolution = shape
        except:
            raise  SyntaxError, "shape argument needs to be a tuple, i.e. ('shape',resolution), shape can be sin, sin2, rec"
        
        if length == None:
            # mimimum length
            length=42*9e-8
        if resolution >= length:
            raise ValueError, "Resolution %.3e of shaped gradients can not be longer than total length %.3e"%(resolution, length)

        if resolution < 42*9e-8:
            raise ValueError, "Resulution %.3e can not be smaller than %.3e"%(resolution, 42*9e-8)

        t_steps = numpy.arange(0,length,resolution)
        
        if form == 'rec': # shape==None --> rectangular gradients
            s_content = '<ttlout value="%s"/><analogout id="1" dac_value="%i"/>' % (trigger, dac_value)
            self.state_list.append(StateSimple(length, s_content))
        
            if not is_seq and shape == None:
                s_content = '<analogout id="1" dac_value="0"/>'
                self.state_list.append(StateSimple(42*9e-8, s_content))

        elif form == 'sin2':
            # sin**2 shape
            for t in t_steps:
                dac = int (dac_value*numpy.sin(numpy.pi/length*t)**2)
                s_content = '<ttlout value="%s"/><analogout id="1" dac_value="%i"/>' % (trigger, dac)
                self.state_list.append(StateSimple(resolution, s_content))
            # set it back to zero
            s_content = '<ttlout value="%s"/><analogout id="1" dac_value="0"/>' % (trigger)
            self.state_list.append(StateSimple(resolution, s_content))
            
        elif form == 'sin':
            # sin shape
            for t in t_steps:
                dac = int (dac_value*numpy.sin(numpy.pi/length*t))
                s_content = '<ttlout value="%s"/><analogout id="1" dac_value="%i"/>' % (trigger, dac)
                self.state_list.append(StateSimple(resolution, s_content))
            # set it back to zero
            s_content = '<ttlout value="%s"/><analogout id="1" dac_value="0"/>' % (trigger)
            self.state_list.append(StateSimple(resolution, s_content))

        else: # don't know what to do
            raise SyntaxError , "form is unknown: %s"%form

    ## Deprecated, use set_pfg instead 
    def set_pfg_wt(self, I_out=None, dac_value=None, length=None, is_seq=0, trigger=4):
        """
        This sets the value for the PFG (plus trigger, default=2**2), it also sets it back automatically.
        If you don't whish to do so (i.e. line shapes)  set is_seq=1
        """
#        raise DeprecationWarning, "to be removed in future, use set_pfg instead"
        if I_out == None and dac_value == None:
            dac_value=0
        if I_out != None and dac_value == None:
            dac_value=dac.conv(I_out)
        if I_out == None and dac_value != None:
            dac_value=dac_value
        if I_out !=None and dac_value != None:
            dac_value = 0
            print "WARNING: You can't set both, I_out and dac_value! dac_value set to 0"
        if length==None:
            length=42*9e-8
        s_content = '<analogout id="1" dac_value="%i"/><ttlout value="%s"/>' \
            % (dac_value, trigger)
        self.state_list.append(StateSimple(length, s_content))
        if is_seq == 0:
            s_content = '<analogout id="1" dac_value="0"/><ttlout value="%s"/>' \
                % trigger
            self.state_list.append(StateSimple(42*9e-8, s_content))

    ## sets the value of a DAC
    # @param dac_value DAC value to set
    # @param dac_id ID of the dac in case of multiple DAC(default=1)
    # @param length Duration of the state
    # @param is_seq If set to *True*, do NOT set DAC to zero after this state
    # @param ttls Additional ttl channels
    def set_dac(self, dac_value, dac_id=1, length=None, is_seq=False, ttls=0):
        """
        This sets the value for the DAC and possibly some TTLs.
            It also sets it back automatically.
        If you don't whish to do so (i.e. line shapes)  set is_seq=True
        """
        if length==None:
            length=42*9e-8
        s_content = '<analogout id="%d" dac_value="%i"/><ttlout value="0x%06x"/>' \
            % (dac_id, dac_value, ttls)
        self.state_list.append(StateSimple(length, s_content))
        if not is_seq:
            s_content = '<analogout id="%d" dac_value="0"/><ttlout value="0x%06x"/>' \
                % (dac_id, ttls)
            self.state_list.append(StateSimple(42*9e-8, s_content))

    ## sets the phase of the frequency source.
    ## This state needs 0.5us, though the phase switching time is dependent on the frequency source
    # @param phase New phase to set
    # @param ttls Additional ttl channels
    def set_phase(self, phase, ttls=0):
        s_content = '<analogout phase="%f" />' % (phase)
        if ttls!=0:
            s_content += '<ttlout value="%d"/>' % ttls
        self.state_list.append(StateSimple(0.5e-6, s_content))
        
    ## sets a description which is carried via the back end result 
    ## file to the result script in the front end. In the result script 
    ## you can extract the description with get_description(key)
    # @param key Name of description
    # @param value Value of description
    def set_description(self, key, value):
        """Sets a description which is carried via the back end result 
        file to the result script in the front end. In the result script 
        you can extract the description with get_description"""
        if key in self.description.keys():
            print 'Warning: Overwriting existing description "%s" = "%s" with "%s"' % (key, self.description[key], value)

        self.description[key] = value

    ## set the PTS310/PTS500 frequency source to local mode
    def set_pts_local(self):
        """
        this will set the PTS310/PTS500 frequency source to local mode
        """
        self.state_list.append(StateSimple(1e-6, '<ttlout value="0xf000"/>'))
        self.state_list.append(StateSimple(1e-6, '<ttlout value="0x8000"/>'))

    # / Commands -----------------------------------------------------------------------------------


    # Public Methods -------------------------------------------------------------------------------

    def get_job_id(self):
        "Returns the current job-id the experiment got"
        return self.job_id


    def write_xml_string(self):
        "Returns the current program as a string"

        # Standart XML-Kopf
        xml_string = '<?xml version="1.0" encoding="ISO-8859-1"?>\n'

        # Experiment-Start-Tag einfügen
        xml_string += '<experiment no="%d">\n' % self.job_id

        # Descriptions einfügen
        if len(self.description)==0:
            xml_string += '  <description/>\n'
        else:
            xml_string += '  <description>\n'
            for key,value in self.description.iteritems():
                type_string="repr"
                if value is None:
                    type_string="None"
                    value=""
                elif type(value) is types.FloatType or isinstance(value, numpy.floating):
                    type_string="Float"
                    value=repr(value)
                elif type(value) is types.IntType or isinstance(value, numpy.integer):
                    type_string="Int"
                    value=repr(value)
                elif type(value) is types.LongType:
                    type_string="Long"
                    value=repr(value)
                elif type(value) is types.ComplexType or isinstance(value, numpy.complexfloating):
                    type_string="Complex"
                    value=repr(value)
                elif type(value) is types.BooleanType or isinstance(value, numpy.bool_):
                    type_string="Boolean"
                    value=repr(value)
                elif type(value) in types.StringTypes:
                    type_string="String"
                else:
                   value=repr(value)
                xml_string += '    <item key="%s" type="%s">%s</item>\n'%(key, type_string ,value)
            xml_string += "  </description>\n"

        # Experiment-Inhalt einfügen
        xml_string += self.state_list.to_xml(indent = "  ")

        # Experiment-End-Tag
        xml_string += '</experiment>\n'

        return xml_string

    def write_quit_job(self):
        "Returns a xml quit-job"
        return '<?xml version="1.0" encoding="ISO-8859-1"?>\n<quit/>'


class Quit(Experiment):
    def write_xml_string(self):
        return '<?xml version="1.0" encoding="ISO-8859-1"?>\n<quit no="%d"/>'%self.job_id
        

# /Public Methods ------------------------------------------------------------------------------



def self_test():
    e = Experiment()
    e.set_description("key", "value")
    e.set_frequency(85e6, 90, ttls=16)
    e.wait(1e-6)
    e.rf_pulse(1, 1e-6/3)        # val = 1
    e.ttl_pulse(1e-6/3, 1)       # val = 2
    e.ttl_pulse(1e-6/3, None, 7) # val = 7
    if True:
        e.loop_start(30)
        e.set_pfg(dac_value=1024, is_seq = True)
        e.set_pfg_wt(dac_value=2048)
        e.loop_start(400)
        e.set_phase(270, ttls = 32)
        e.loop_end()
        e.ttl_pulse(5e-6, channel = 6)
        e.loop_end()
    else:
        l = StateLoop(3)
        l.append(StateSimple(5e-6, '<ttlout value="1"/>'))
        e.state_list.append(l)
    e.set_dac(12345, dac_id=2, is_seq = True, ttls=16)
    e.record(1024, 20e6)
    try:
        e.wait(-1)
    except AssertionError:
        pass
    else:
        raise AssertionError("An exception should happen")
    e.set_pts_local()
    print e.write_xml_string()

if __name__ == '__main__':
    self_test()
