import Experiment
from pylab import arange, log, exp, polyval, load, concatenate, array
from random import shuffle
import pickle

def gradient(t_pfg, grad):
	"""
	Calculate dac_value
	"""
    dac_value = (grad)/6.649e-5
    return int(dac_value)

def rf(exp, length):
	"""
	Short form for making a RF pulse
	"""
    exp.ttl_pulse(5e-6, value=1) 	# gate 2**0
    exp.ttl_pulse(length, value=3)   	# gate+rf 2**0+2**1

def pfg_exp(frequency, 		# Frequency
            f0,				# Resonance frequency
            pi,				# Pi pulse length
            pi_h,			# Pi/2 pulse length
            grad_G,			# Gradient stength
            grad_F,			# Gradient strength
	    	t_pfg, 			# Gradient pulse length
            read_gradient,	# Read gradient strength
            delta1,			# Time before gradient
	    	delta2, 		# Time after gradient
            akku,			# Current accumulation
	    	Delta,			# Time Delta between RF pulses 3 and 4
	    	no_akku,		# Total number of accumulation
            t_corr, 		# Correction time length of the read gradient in prep interval
            sample_temp,	# Sample temperature
            pfg_rise_time,	# Rise time of the gradients, needed to center the gradients
            t1,				# T1 longitudinal relaxation time
            t2,				# T2 transversal relaxation time
            use_cor,		# Flag if correction should be used or not
            the_index,		# (next two not used)
            t_pfg_index):
	"""
	The PFG pulse sequence
	"""

    e=Experiment.Experiment()
    e.wait(7*t1)                         # Repetition time
    # Descriptions
    e.set_description("type","13interval")
    e.set_description("no_akku", no_akku)
    e.set_description("pi",pi)
    e.set_description("f",frequency)
    e.set_description("f0",f0)
    e.set_description("t_pfg",t_pfg)
    e.set_description("Akku", akku)
    e.set_description("tau",delta1+delta2+t_pfg)
    e.set_description('delta1', delta1)
    e.set_description('delta2', delta2)
    e.set_description("Delta", Delta)
    e.set_description("SampleTemp", sample_temp)
    e.set_description("G", grad_G)
    e.set_description("F", grad_F)
    e.set_description("t1",t1)
    e.set_description("index",the_index)
    e.set_description("t_pfg_index",t_pfg_index)
    e.set_description("t2",t2)
    e.set_description("use_cor",use_cor)

    # Phase cycling from J. Phys. Chem. B, 105 (25), 5922 -5927, 2001. 10.1021
    phases =   {'ph1' : [270,270,90,90,0,0,180,180,90,90,270,270,180,180,90,90][akku%16],
                'ph2' : [270,270,90,90,0,0,180,180,90,90,270,270,180,180,90,90][akku%16],
                'ph3' : [270,270,90,90,0,0,180,180,90,90,270,270,180,180,90,90][akku%16],
                'ph4' : [270,270,90,90,0,0,180,180,90,90,270,270,180,180,90,90][akku%16],
                'ph5' : [0,180,90,270,90,270,180,0,180,0,270,90,270,90,0,180][akku%16],
                'rec_ph' : [0,0,0,0,90,90,90,90,180,180,180,180,270,270,270,270][akku%16]}
    # 1. pi/2
    ph1 = phases['ph1']
    # 1. pi
    ph2 = phases['ph2']
    # 2. pi/2
    ph3 = phases['ph3']
    # 3. pi/2
    ph4 = phases['ph4']
    # 2. pi
    ph5 = phases['ph5']
    # Receiver
    rec_ph = phases['rec_ph']

    
    # Create the dac_values for the four gradients

    pos_G = gradient(t_pfg,grad_G)
    pos_F = gradient(t_pfg,grad_F)
    neg_G = -pos_G
    neg_F = -pos_F

	# This is necessary for the result script 
	# to identify a zero gradient signal
    if grad_G == 0.0:
        e.set_description("dac_value",0)
    else:
        e.set_description("dac_value",pos_G)    
    
    # Create zero gradient and read_gradient dac_values

    zero_grad = gradient(2e-3, 0.0)

    if (read_gradient != None):		    # Did we set a read gradient ?
        if use_cor:                         # Should we do correction ?
            key = (pos_G, t_pfg_index, the_index) # Generate key for the dictionary
            try:
		CORR_file = open('CORR')
		corr_data = pickle.load(CORR_file)  # Load correction data dictionary
		CORR_file.close()
				# t_corr is decremented by the correction time found in dictionary
                t_corr -= corr_data[key] 
            except:
                keys_tr = str(key[0]) + "_" + str(key[1]) + "_" + str(key[2])
                print "No t_corr found: %s"%keys_tr
        else:
            pass    
	if t_corr < 0:			    
			# If t_corr  becomes negative, reverse the sign of the correction gradient
            t_corr = abs(t_corr)
	    small_const_g = gradient(2e-3, -read_gradient)
	else: 
	    neg_t_corr = False    
            small_const_g = gradient(2e-3, read_gradient)
            
        if abs(t_corr) < 3.78e-6: # Shortest possible pfg length
            t_corr = 3.78e-6
            

        if abs(t_corr) > delta2:
        	# If t_corr bigger than the time between PFG and RF pulse print warning
            print "t_corr too long, shortening it ..."
            t_corr = delta2 
        if grad_G == 0.0:
        	# Set no correction gradient at all if gradient is set to 0 T/m
            small_const_g = None
    
    else:				    
    	# Without read gradient don't set any
        small_const_g = None
    

    ## Preparation part
    
    e.set_pfg(dac_value=zero_grad,is_seq=1) # Zero gradient
    e.set_frequency(frequency, ph1) # Needs 2 microseconds
    ####     Pulse 1
    rf(e,pi_h)
    e.wait(delta1 - pfg_rise_time)
    ## F
    e.set_pfg(dac_value=pos_F, length=t_pfg, is_seq=1)
    e.set_pfg(dac_value=zero_grad, is_seq=1) # Set to zero
    e.wait(delta2 - 2e-6 + pfg_rise_time)
    e.set_phase(ph2)
    ##		Pulse 2
    rf(e,pi)
    e.wait(delta1 - pfg_rise_time)
    ## G
    e.set_pfg(dac_value=pos_G,length=t_pfg, is_seq=1)
    if small_const_g:
    	# The small read gradient
        e.set_pfg(dac_value=small_const_g, length=t_corr, is_seq=1) 
        # Now go back to zero
        e.set_pfg(dac_value=zero_grad, is_seq=1)    
        e.wait(delta2 - 2e-6 + pfg_rise_time - t_corr)
    else:
        e.set_pfg(dac_value=zero_grad, is_seq=1) # Set to zero    
        e.wait(delta2 - 2e-6 + pfg_rise_time)        

    ## Evolution part
    
    e.set_phase(ph3)
    #### 	Pulse 3
    rf(e,pi_h)
    e.wait(Delta-2e-6)       # Evolution time

    ## Refocussing part
    
    e.set_phase(ph4)
    #### 	Pulse 4
    rf(e,pi_h) 
    e.wait(delta1 - pfg_rise_time)
    ## -G
    e.set_pfg(dac_value=neg_G, length=t_pfg, is_seq=1) 
    # now go back to zero
    e.set_pfg(dac_value=zero_grad, is_seq=1)
    e.wait(delta2 - 2e-6 + pfg_rise_time)
    e.set_phase(ph5)
    ##		Pulse 5
    rf(e,pi)
    e.wait(delta1 - pfg_rise_time)
    ## -F
    e.set_pfg(dac_value=neg_F,length=t_pfg)
    e.set_pfg(dac_value=zero_grad, is_seq=1)
    e.wait(delta2 - 2e-6 + pfg_rise_time)
    
    
    ## Recording part
    # Adjust the receiver phase to get a real signal on the real channel 
    e.set_phase(rec_ph-45)
    if small_const_g:
    	# The small read_gradient while recording
		e.set_pfg(dac_value=small_const_g, is_seq=1)
    e.record(4*1024, 1e6, sensitivity=2)
    e.set_pfg(dac_value=zero_grad,is_seq=0)
    # Synchronize Experiment script with the Result script
    if akku+1 == no_akku:
        synchronize()
    return e
    


def experiment():
    """
    Conduct the experiment
    """
    # Gradients to measure
    gradient_list = arange(0, 18, 1)
    # Gradient lengths to be measured
    t_pfg_list    = arange(0.2e-3, 1.6e-3, 0.2e-3)
	# Time before the gradient pulses
    delta1 = 0.5e-3
    # How many values for the evolution time
    points = 25
    
    no_akku = 2    	# Number of accumulations for the data
    no_akku_corr = 6 	# Number of accumulations for corrected signal
    
    # calculate approximate duration
    duration = len(gradient_list)*len(t_pfg_list)*points*(no_akku 
    								+ no_akku_corr)*7*load('t1')[0]/3600.0
    print "Duration will be approx. ",duration,"hours"
    if duration > 48:
        print "Warning: load nitrogen"
        
    # Start value for the evolution time
    start = 5.5e-3 #3.5e-3
    # End value for the evolution time
    end = 160e-3
    
    # Logarithmic distribution of the evolution times
    tp_list = [exp(i) for i in arange(log(start), log(end),log(end/start)/points)]
   
    # Create the list of parameters, each entry in the list has 3 values:
    # gradient strength, Delta (tp) and gradient length
    # 
    zero_grad_list = [(grad, tp, t_pfg) for grad in [0] 
    									for tp in tp_list 
    									for t_pfg in t_pfg_list]
    									
    rest_grad_list = [(grad, tp, t_pfg) for grad in gradient_list 
    									for tp in tp_list 
    									for t_pfg in t_pfg_list]
    # Randomize the data aquisition, so any drifts are 
    # statistically distributed/obscured
    shuffle(zero_grad_list)
    shuffle(rest_grad_list)
    # but do the zero gradients first
    parameter_list = concatenate((zero_grad_list,rest_grad_list))
    
    # Bookmarking several variables    
    t_dict={}
    for i,t in enumerate(tp_list):
        t_dict[t]=i
    
    t_pfg_dict = {}
    for i,t in enumerate(t_pfg_list):
        t_pfg_dict[t]=i
        
    # First measure without correction, then correct
    for correction in [False, True]:
        if correction:	
            akkus = no_akku
            print "with correction"
        else:                           
        	# Do more accumulations for corrected signal
            akkus = no_akku_corr
            print "determinig correction"    

        for tpl in parameter_list:
            print tpl
            grad = tpl[0]
            tp	 = tpl[1]
            t_pfg= tpl[2]
            d1 = delta1
            for akku in range(akkus): # Accumulations
                yield pfg_exp(
                        pi = 3.8e-6,
                        pi_h = 1.9e-6,
                        frequency    = load('freq0')[0]+0e4,
                        f0	     = load('freq0')[0],
                        grad_G       = grad,
                        grad_F       = grad/(8.0/(1+1.0/3.0*(t_pfg/(d1*2+t_pfg))**2)-1),  
                        t_pfg        = t_pfg, 
                        t_corr       = 0.2e-3,
                        pfg_rise_time= 120e-6,
                        read_gradient= 0.03,                    
                        delta1       = d1, # Time before Ggradient
                        delta2       = d1, # Time before Ggradient
                        akku         = akku,
                        Delta        = tp, # Evolution time
                        no_akku      = akkus,
                        sample_temp  = 263.3,
                        t1           = load('t1')[0], # Load T1 from file
                        t2           = load('t2')[0], # Load T2 from file
                        use_cor	     = correction,
                        the_index    = t_dict[tp],
                        t_pfg_index  = t_pfg_dict[t_pfg])
