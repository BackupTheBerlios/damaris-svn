import Experiment
from pylab import arange, log, exp, polyval, load, concatenate, array
from random import shuffle
import pickle
def gradient(t_pfg, grad):
    #dac_value = (grad - 1.824e-2)/6.649e-5
    dac_value = (grad)/6.649e-5
    #if t_pfg < 1e-3 and abs(grad) > 0:
    #    corr = [-1.00925277e-18, 4.20625114e-13, -6.39939547e-08, 1.00163279e+00]
    #    dac_value = dac_value*polyval(corr, [dac_value])[0]
    return int(dac_value)



def rf(exp, length):
    exp.ttl_pulse(5e-6, value=1) 	# gate 2**0
    exp.ttl_pulse(length, value=3)   	# gate+rf 2**0+2**1

def pfg_exp(frequency,
            f0,
            pi,
            pi_h,
            grad_G,
            grad_F,
	    t_pfg, 
            read_gradient, 
            delta1,
	    delta2, 
            akku,
	    Delta,
	    no_akku,
            t_corr, 
            sample_temp,
            pfg_rise_time,
            t1,
            t2,
            use_cor,
            the_index,
            t_pfg_index):

    e=Experiment.Experiment()
    e.wait(1*t1)                         # repetition time
    ## Descriptions
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
    # Phase cycling
    phases =   {'ph1' : [0,0,180,180][akku%4],
                'ph2' : [0,90,180,270][akku%4],
                'ph3' : [0,180,0,180][akku%4],
                'ph4' : 0,
                'ph5' : [0,90,180,270][akku%4],
                'rec_ph': [0,180,0,180][akku%4]}

    # next ones are from J. Phys. Chem. B, 105 (25), 5922 -5927, 2001. 10.1021
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
    # receiver
    rec_ph = phases['rec_ph']

    
    # Create the dac_value for the four gradients

    pos_G = gradient(t_pfg,grad_G)
    pos_F = gradient(t_pfg,grad_F)
    neg_G = -pos_G
    neg_F = -pos_F
    
    #gradient(t_pfg,-grad/(8.0/(1+1.0/3.0*(t_pfg/(delta1+delta2+t_pfg))**2)-1))
    neg_F = neg_F
    if grad_G == 0.0:
        e.set_description("dac_value",0)
    else:
        e.set_description("dac_value",pos_G)    
    
    # Create zero gradient and read_gradient dac_values

    zero_grad = gradient(2e-3, 0.0)


    if (read_gradient != None):		    # did we set a read gradient ?
        if use_cor:                         # should we do correction?
            #print "correcting echo signal"
            key = (pos_G, t_pfg_index, the_index)
            try:
		CORR_file = open('CORR')
		corr_data = pickle.load(CORR_file)  # load correction data dictionary
		CORR_file.close()
                t_corr -= corr_data[key]  # t_corr is decremented by the correction time found in dictionary
            except:
                keys_tr = str(key[0]) + "_" + str(key[1]) + "_" + str(key[2])
                print "No t_corr found: %s"%keys_tr
        else:
            #print "without correction"
            pass    
	if t_corr < 0:			    # if t_corr  becomes negative, reverse the sign of the correction gradient
            t_corr = abs(t_corr)
	    small_const_g = gradient(2e-3, -read_gradient)
	else: 
	    neg_t_corr = False    
            small_const_g = gradient(2e-3, read_gradient)
            
        if abs(t_corr) < 3.78e-6: # shortest possible pfg length
            t_corr = 3.78e-6
            

        if abs(t_corr) > delta2:	    # if t_corr bigger than the time between PFG and RF pulse print warning
            print "t_corr too long, shortening it ..."
            t_corr = delta2 
        if grad_G == 0.0:		    # set no correction gradient at all if gradient is set to 0 T/m
            small_const_g = None
    
    else:				    # without read gradient don't set any
        small_const_g = None
    

    ## Preparation part
    
    e.set_pfg(dac_value=zero_grad,is_seq=1) # zero gradient
    e.set_frequency(frequency, ph1) # needs 2 microseconds
    rf(e,pi_h) ####     Puls 1   
    e.wait(delta1 - pfg_rise_time)
    ## F
    e.set_pfg(dac_value=pos_F, length=t_pfg, is_seq=1)
    e.set_pfg(dac_value=zero_grad, is_seq=1) # set to zero
    e.wait(delta2 - 2e-6 + pfg_rise_time)
    e.set_phase(ph2)
    ## pi pulse
    rf(e,pi)
    e.wait(delta1 - pfg_rise_time)
    ## G
    e.set_pfg(dac_value=pos_G,length=t_pfg, is_seq=1)
    #e.set_pfg(dac_value=zero_grad,is_seq=1) # set to zero
    if small_const_g:
        e.set_pfg(dac_value=small_const_g, length=t_corr, is_seq=1) # read gradient
        e.set_pfg(dac_value=zero_grad, is_seq=1) # set to zero    
        e.wait(delta2 - 2e-6 + pfg_rise_time - t_corr)
    else:
        e.set_pfg(dac_value=zero_grad, is_seq=1) # set to zero    
        e.wait(delta2 - 2e-6 + pfg_rise_time)        

    ## Evolution part
    
    e.set_phase(ph3)
    rf(e,pi_h) #### 	Puls 3
    e.wait(Delta-2e-6)       # evolution time



    ## Refocussing part
    
    e.set_phase(ph4)
    rf(e,pi_h) #### 	Puls 4
    e.wait(delta1 - pfg_rise_time)
    ## -G
    e.set_pfg(dac_value=neg_G, length=t_pfg, is_seq=1) 
    e.set_pfg(dac_value=zero_grad, is_seq=1)
    e.wait(delta2 - 2e-6 + pfg_rise_time)
    e.set_phase(ph5)
    ## pi pulse
    rf(e,pi)
    e.wait(delta1 - pfg_rise_time)
    ## -F
    e.set_pfg(dac_value=neg_F,length=t_pfg)
    e.set_pfg(dac_value=zero_grad, is_seq=1)
    e.wait(delta2 - 2e-6 + pfg_rise_time)
    
    
    ## Recording part
    
    e.set_phase(rec_ph-45)
    if small_const_g: 
	e.set_pfg(dac_value=small_const_g, is_seq=1) # read_gradient
    e.record(4*1024, 1e6, sensitivity=2)
    e.set_pfg(dac_value=zero_grad,is_seq=0)
    if akku+1 == no_akku:
        #pass
        #print "Exp script",grad_G, Delta, t_pfg      
        synchronize()
    return e
    


def experiment():
    
    gradient_list = arange(17,18,1)
    t_pfg_list    = arange(0.8e-3, 1.9e-3,0.2e-3)
    #delta1      = [0.4, 0.8 ,1.2]
    delta1 = 0.5e-3
    points = 2
    
    no_akku = 2#3*16    # accumulations for the data
    no_akku_corr = 2#16 # accumulations for t_corr
    
    duration = len(gradient_list)*len(t_pfg_list)*points*(no_akku + no_akku_corr)*7*load('t1')[0]/3600.0
    print "Duration will be approx. ",duration,"hours"
    if duration > 48:
        print "Warning: load nitrogen"
        
    start = 5.5e-3 #3.5e-3
    end = 160e-3
    
    tp_list = [exp(i) for i in arange(log(start), log(end),log(end/start)/points)]
    

    
    # Randomize the data aquisition
    zero_grad_list = [(grad, tp, t_pfg) for grad in [0] for tp in tp_list for t_pfg in t_pfg_list]
    rest_grad_list = [(grad, tp, t_pfg) for grad in gradient_list for tp in tp_list for t_pfg in t_pfg_list]
    shuffle(zero_grad_list)
    shuffle(rest_grad_list)
    # but do the zero gradients first
    parameter_list = concatenate((zero_grad_list,rest_grad_list))
    
    # bookmarking several variables
    
    t_dict={}
    for i,t in enumerate(tp_list):
        t_dict[t]=i
    
    t_pfg_dict = {}
    for i,t in enumerate(t_pfg_list):
        t_pfg_dict[t]=i
        
    #delta1_dict={}
    #for i,t in enumerate(delta1):
    #    delta1_dict[t]=i
        
    # create original gradient list for t_corr 
    # gradient_list = concatenate(([0], gradient_list))
    # parameter_list_corr = [(grad, tp, t_pfg) for grad in gradient_list for tp in tp_corr for t_pfg in t_pfg_list]
    
    for correction in [False, True]:
        if correction:	
            akkus = no_akku
            print "with correction"
        else:                           # Do more accumulations for corrected signal
            akkus = no_akku_corr
            print "determinig correction"    

        # vary the gradient length
        #for t_pfg in t_pfg_dict.keys():
        #t_pfg = 1.0e-3    
        for tpl in parameter_list: # G_z strength
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
                        delta1       = d1, # Zeit vor  dem Gradienten
                        delta2       = d1, # Zeit nach dem Gradienten
                        akku         = akku,
                        Delta        = tp, # Evolutionszeit
                        no_akku      = akkus,
                        sample_temp  = 263.3,
                        t1           = load('t1')[0],
                        t2           = load('t2')[0],
                        use_cor	     = correction,
                        the_index    = t_dict[tp],
                        t_pfg_index  = t_pfg_dict[t_pfg])