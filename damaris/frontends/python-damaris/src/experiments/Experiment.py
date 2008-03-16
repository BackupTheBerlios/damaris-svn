# -*- coding: iso-8859-1 -*-
import types
import numpy as N

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


    def record(self, samples, frequency, timelength=None, sensitivity=None):
        attributes='s="%d" f="%g"'%(samples,frequency)
        if sensitivity is not None:
            attributes+=' sensitivity="%f"'%sensitivity
        s_content = '<analogin %s/>' % attributes
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
    
    def set_pfg(self, I_out=None, dac_value=None, length=None, is_seq=0):
	"""This sets the value for the PFG, it also sets it back automatically.
	If you don't whish to do so (i.e. line shapes)  set is_seq=1"""
	
	if I_out == None and dac_value == None:
	    dac_value=0
	if I_out != None and dac_value == None:
	    dac_value=dac.conv(I_out)
	if I_out == None and dac_value != None:
	    dac_value=dac_value
	if I_out !=None and dac_value != None:
	    dac_value = 0
	    print "WARNING: You can't set both, I_out and dac_value! dac_value set to 0"
	if length == None:
	    # mimimum length
	    length=42*9e-8
	s_content = '<analogout id="1" dac_value="%i"/>' % dac_value
	self.state_list.append(StateSimple(length, s_content))
	if is_seq == 0:
	    s_content = '<analogout id="1" dac_value="0"/>'
	    self.state_list.append(StateSimple(42*9e-8, s_content))
 
    def set_pfg_wt(self, I_out=None, dac_value=None, length=None, is_seq=0, trigger=4):
	"""This sets the value for the PFG (plus trigger, default=2**2), it also sets it back automatically.
	If you don't whish to do so (i.e. line shapes)  set is_seq=1"""
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

        # Experiment-Start-Tag einf�gen
        xml_string += '<experiment no="%d">\n' % self.job_id

        # Descriptions einf�gen
        if len(self.description)==0:
            xml_string += '  <description/>\n'
        else:
            xml_string += '  <description>\n'
            for key,value in self.description.iteritems():
	        type_string="repr"
		if value is None:
		    type_string="None"
		    value=""
	        if type(value) is types.FloatType:
		    type_string="Float"
		    value=repr(value)
		elif type(value) is types.IntType:
		    type_string="Int"
		    value=repr(value)
		elif type(value) is types.LongType:
		    type_string="Long"
		    value=repr(value)
		elif type(value) is types.ComplexType:
		    type_string="Complex"
		    value=repr(value)
		elif type(value) is types.BooleanType:
		    type_string="Boolean"
		    value=repr(value)
		elif type(value) in types.StringTypes:
		    type_string="String"
	        else:
		    value=repr(value)
		xml_string += '    <item key="%s" type="%s">%s</item>\n'%(key, type_string ,value)
            xml_string += "  </description>\n"

        # Experiment-Inhalt einf�gen
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


# These methods return lists

lin_range = N.arange

def log_range(start, stop, stepno):
	if (start<=0 or stop<=0 or stepno<1):
		raise ValueError("start, stop must be positive and stepno must be >=1")
	return N.logspace(N.log10(start),N.log10(stop), num=stepno)


def staggered_range(some_range, size=3):
	m=0
	if isinstance(some_range, N.ndarray):
		print "da"
		is_numpy = True
		some_range = list(some_range)
	new_list=[]
	for k in xrange(len(some_range)):
		for i in xrange(size):
			try:
				index = (m*size)
				new_list.append(some_range.pop(index))
			except IndexError:
				break
		m+=1
	if is_numpy:
		new_list = N.asarray(new_list+some_range)
	else:
		new_list+=some_range
	return new_list
		

def combined_ranges(*ranges):
    new_list = []
    for r in ranges:
        new_list+=r
    return new_list

combine_ranges=combined_ranges

def interleaved_range(some_list, left_out):
	"""
	in first run, do every n-th, then do n-1-th of the remaining values and so on...
	"""
	m=0
	new_list = []
	for j in xrange(left_out):
		for i in xrange(len(some_list)):
			if (i*left_out+m) < len(some_list):
				new_list.append(some_list[i*left_out+m])
			else:
				m+=1
				break
	if isinstance(some_list, N.ndarray):
		new_list = N.array(new_list) 
	return new_list


# These are the generators
def lin_range_iter(start,stop, step):
    this_one=float(start)+0.0
    if step>0:
        while (this_one<=float(stop)):
            yield this_one
            this_one+=float(step)
    else:
        while (this_one>=float(stop)):
            yield this_one
            this_one+=float(step)
        

def log_range_iter(start, stop, stepno):
    if (start<=0 or stop<=0 or stepno<1):
        raise ValueError("start, stop must be positive and stepno must be >=1")
    if int(stepno)==1:
        factor=1.0
    else:
        factor=(stop/start)**(1.0/int(stepno-1))
    for i in xrange(int(stepno)):
        yield start*(factor**i)

def staggered_range_iter(some_range, size = 1):
    """
    size=1: do one, drop one, ....
    size=n: do 1 ... n, drop n+1 ... 2*n
    in a second run the dropped values were done
    """
    left_out=[]
    try:
        while True:
            for i in xrange(size):
                yield some_range.next()
            for i in xrange(size):
                left_out.append(some_range.next())
    except StopIteration:
        pass
    
    # now do the droped ones
    for i in left_out:
        yield i

def combined_ranges_iter(*ranges):
    """
    iterate over one range after the other
    """
    for r in ranges:
        for i in r:
            yield i

combine_ranges_iter=combined_ranges_iter

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
