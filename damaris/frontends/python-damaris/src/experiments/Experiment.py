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

    job_id = 0
    
    def __init__(self):
        self.job_id = Experiment.job_id
        Experiment.job_id += 1

        self.state_list = StateList()
        self.list_stack = []
        self.description = { }


    # Commands -------------------------------------------------------------------------------------

    def rf_pulse(self, channel, length = None):
        s_content = '<ttlout value="0x%06x"/>' % channel

        if length is None:
            self.state_list.append(s_content)
        else:
            self.state_list.append(StateSimple(length, s_content))

    def ttl_pulse(self, length, channel = None, value = None):
        the_value=0
        if value is not None:
            the_value=int(value)
        elif channel is not None:
            the_value=1<<channel
        self.state_list.append(StateSimple(length, \
            '<ttlout value="0x%06x"/>' % the_value))

    def state_start(self, time):
        self.state_list.append('<state time="%s">\n' % repr(time))


    def state_end(self):
        self.state_list.append('</state>\n')


    def wait(self, time):
        self.state_list.append(StateSimple(time))


    def record(self, samples, frequency, timelength=None, sensitivity=None, ttls=None):
        attributes='s="%d" f="%g"'%(samples,frequency)
        if sensitivity is not None:
            attributes+=' sensitivity="%f"'%sensitivity
        s_content = '<analogin %s/>' % attributes
    	if ttls is not None:
	        s_content+='<ttlout value="0x%06x"/>' % ttls
        if timelength is None:
            timelength = samples / float(frequency)*1.01
        self.state_list.append(StateSimple(timelength, s_content))


    def loop_start(self, iterations):
        l = StateLoop(iterations)
        self.state_list.append(l)
        # (These two lines could probably be guarded by a mutex)
        self.list_stack.append(self.state_list)
        self.state_list = l


    def loop_end(self):
        # (This line could probably be guarded by a mutex)
        self.state_list = self.list_stack.pop(-1)


    def set_frequency(self, frequency, phase, ttls=0):
        "Sets the frequency generator to a desired frequency (Hz)"
        s_content = '<analogout id="0" f="%f" phase="%f"/>' % (frequency, phase)
        if ttls != 0:
            s_content += '<ttlout value="0x%06x"/>' % ttls
        self.state_list.append(StateSimple(2e-6, s_content))
    
    def set_pfg(self, dac_value=None, length=None, shape=('rec',0), trigger=4, is_seq=0):
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
        
            if is_seq == 0 and shape == None:
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


    def set_phase(self, phase, ttls=0):
        s_content = '<analogout phase="%f" />' % (phase)
        if ttls!=0:
            s_content += '<ttlout value="%d"/>' % ttls
        self.state_list.append(StateSimple(0.5e-6, s_content))
        

    def set_description(self, key, value):
        "Sets a description"
        if key in self.description.keys():
            print 'Warning: Overwriting existing description "%s" = "%s" with "%s"' % (key, self.description[key], value)

        self.description[key] = value

    def set_pts_local(self):
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
	        if type(value) is types.FloatType \
	        	or isinstance(value, numpy.floating):
		    type_string="Float"
		    value=repr(value)
		elif type(value) is types.IntType \
			or isinstance(value, numpy.integer):
		    type_string="Int"
		    value=repr(value)
		elif type(value) is types.LongType:
		    type_string="Long"
		    value=repr(value)
		elif type(value) is types.ComplexType \
			or isinstance(value, numpy.complexfloating):
		    type_string="Complex"
		    value=repr(value)
		elif type(value) is types.BooleanType \
			or isinstance(value, numpy.bool_):
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
