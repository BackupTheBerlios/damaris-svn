from Accumulation import Accumulation
from ADC_Result import ADC_Result
from MeasurementResult import MeasurementResult, AccumulatedValue

import numarray
import math

from Experiment import *


def get_mean(timesignal, start, end):
    r_start=max(0,int((start-timesignal.x[0])*timesignal.get_sampling_rate()))
    r_end=min(int((end-timesignal.x[0])*timesignal.get_sampling_rate()), len(timesignal))

    channels=len(timesignal.y)
    means=[0.0,]*channels
    if r_start!=r_end:
        for i in xrange(channels):
            means[i]=timesignal.get_ydata(i)[r_start:r_end].mean()
    return means

def baseline_correction(timesignal, start, end):

    means=get_mean(timesignal, start, end)

    for i in xrange(len(means)):
	timesignal.y[i]-=int(means[i])

def save_accu(filename, data):
    if isinstance(data,Accumulation):
        f=open(filename,"w")
        f.write("# accumulation %d\n"%data.n)
        f.write("# t y1 y1err y2 y2err\n")
        xdata=data.get_xdata()
        ydata1=data.get_ydata(0)
        ydata1err=data.get_yerr(0)
        ydata2=data.get_ydata(1)
        ydata2err=data.get_yerr(1)
        for i in xrange(len(xdata)):
            f.write("%g %g %g %g %g\n"%(xdata[i],ydata1[i],ydata1err[i],ydata2[i],ydata2err[i]))
        f=None
        xdata=ydata1=ydata1err=ydata2=ydata2err=None

def rotate_signal(timesignal, angle):
    # implicit change to float arrays!
    if timesignal.get_number_of_channels()!=2:
        raise Exception("rotation defined only for 2 channels")
    # simple case 0, 90, 180, 270 degree
    reduced_angle=divmod(angle,90.0)
    if abs(reduced_angle[1])<1e-6:
        reduced_angle=reduced_angle[0]%4
        if reduced_angle==0:
            return
        elif reduced_angle==1:
            timesignal.y[1]*=-1            
            timesignal.y=[timesignal.y[1],timesignal.y[0]]
        elif reduced_angle==2:
            timesignal.y[0]*=-1
            timesignal.y[1]*=-1
        elif reduced_angle==3:
            timesignal.y[0]*=-1            
            timesignal.y=[timesignal.y[1],timesignal.y[0]]
    else:
        sin_angle=math.sin(angle/180.0*math.pi)
        cos_angle=math.cos(angle/180.0*math.pi)
        timesignal.y=[cos_angle*timesignal.y[0]-sin_angle*timesignal.y[1],
                      sin_angle*timesignal.y[0]+cos_angle*timesignal.y[1]]


def pick_peak(accu, expected_range, channel=0):
    """
    pick a peak in a certain region
    """
    xdata=accu.get_xdata()
    ydata=accu.get_ydata(channel)
    yerrdata=accu.get_yerr(channel)
    accu_deriv=[]
    for i in xrange(len(xdata)-1):
        deriv=((ydata[i+1]-ydata[i])/(xdata[i+1]-xdata[i]),
               math.sqrt((yerrdata[i+1]**2+yerrdata[i]**2)/(xdata[i+1]-xdata[i])**2))
        accu_deriv.append(deriv)

    trend_range=expected_range[:]

    repeat=3
    expand_range=0
    
    while repeat>0:
        deriv_xsum = deriv_ysum = deriv_xysum = deriv_x2sum = deriv_y2sum = 0.0
        deriv_n = 0.0
        for i in xrange(len(accu_deriv)-1):
            if xdata[i]>trend_range[0] and xdata[i]<trend_range[1]:
                deriv_xsum+=xdata[i]
                deriv_x2sum+=xdata[i]**2
                deriv_ysum+=accu_deriv[i][0]
                deriv_y2sum+=accu_deriv[i][0]**2
                deriv_xysum+=xdata[i]*accu_deriv[i][0]
                deriv_n+=1.0

        try:
            deriv_trend_b=(deriv_n*deriv_xysum-deriv_xsum*deriv_ysum)/(deriv_n*deriv_x2sum-(deriv_xsum)**2)
            deriv_trend_a=(deriv_ysum-deriv_trend_b*deriv_xsum)/deriv_n
            expected_maximum_pos=-deriv_trend_a/deriv_trend_b
            trend_range=[expected_maximum_pos-0.5e-6,expected_maximum_pos+0.5e-6]
        except ZeroDivisionError:
            if expand_range>1: return None
            expected_range=[expected_range[0]-(expected_range[1]-expected_range[0])/2,
                            expected_range[0]+(expected_range[1]-expected_range[0])/2]
            repeat=3
            expand_range+=1
        repeat-=1

    return expected_maximum_pos
    
def result():

    name="tau1"
    accu_val1={}
    accu_val2={}
    overview_height1=MeasurementResult("Echo Height 1(%s)"%name)
    overview_height2=MeasurementResult("Echo Height 2(%s)"%name)
    overview_pos1=MeasurementResult("Echo Pos 1(%s)"%name)
    overview_pos2=MeasurementResult("Echo Pos 2(%s)"%name)
    
    for timesignal in results:
        if not isinstance(timesignal, ADC_Result): continue
 
        this_val=float(timesignal.get_description(name))
        if this_val not in accu_val1.keys():
            accu_val1[this_val]=Accumulation(error=True)
            accu_val2[this_val]=Accumulation(error=True)
        # input.watch(timesignal, "time signal")
        timesignal1=timesignal.get_result_by_index(0)
        timesignal2=timesignal.get_result_by_index(1)
        # correct phases
        p1=float(timesignal.get_description("det_phase1"))
        rotate_signal(timesignal1, -p1)
        p2=float(timesignal.get_description("det_phase2"))
        rotate_signal(timesignal2, -p2)
        baseline_correction(timesignal1, 30e-6, 400e-6)
        baseline_correction(timesignal2, timesignal2.x[0]+30e-6, timesignal2.x[0]+400e-6)
        accu_val1[this_val]+=timesignal1
        accu_val2[this_val]+=timesignal2
        if accu_val1[this_val].n%100==0:
            # save data
            save_accu("accu_echo1_%s%g"%(name,this_val),accu_val1[this_val])
            save_accu("accu_echo2_%s%g"%(name,this_val),accu_val2[this_val])
            p1=pick_peak(accu_val1[this_val], [timesignal1.x[0]+10e-6, timesignal1.x[0]+12e-6])
            p2=pick_peak(accu_val2[this_val], [timesignal2.x[0]+10e-6, timesignal2.x[0]+12e-6])
            if p1 is not None:
                m1=get_mean(accu_val1[this_val], p1-0.2e-6, p1+0.2e-6)[0]
            else:
                p1 = m1 = 0
            if p2 is not None:
                m2=get_mean(accu_val2[this_val], p2-0.2e-6, p2+0.2e-6)[0]
            else:
                p2 = m2 = 0
            print p1, m1, p2, m2
            # overview[this_val]+=math.sqrt(magnetization[0]**2+magnetization[1]**2)
            # [0] ist blau, [1] ist rot
            overview_height1[this_val]=AccumulatedValue(m1,0)
            overview_height2[this_val]=AccumulatedValue(m2,0)
            overview_pos1[this_val]=AccumulatedValue(p1,0)
            overview_pos2[this_val]=AccumulatedValue(p2,0)
            overview_file=file("overview","w")
            overview_file.write("# %s EchoHeight1 EchoPos1 EchoHeight2 EchoPos2\n"%name)
            for x in overview_height1.keys():
                overview_file.write("%g %g %g %g %g\n"%(x,
                                                        overview_height1[x].mean(),
                                                        overview_pos1[x].mean(),
                                                        overview_height2[x].mean(),
                                                        overview_pos2[x].mean()))
            overview_file=None
            

        data[overview_height1.get_title()]=overview_height1
        data[overview_height2.get_title()]=overview_height2
        data[overview_pos1.get_title()]=overview_pos1
        data[overview_pos2.get_title()]=overview_pos2
        data["accu1(%s=%e)"%(name,this_val)]=accu_val1[this_val]
        data["accu2(%s=%e)"%(name,this_val)]=accu_val2[this_val]


def stim_echo(t1, tau1, tau2, pi, det_phase=0, cycle=0, frequency=0):
    if pi>1e-4:
        raise Exception("sorry, pulse too long")
    exp=Experiment()
    exp.set_description("type","stim_echo")
    exp.set_description("tau1",tau1)
    exp.set_description("tau2",tau2)
    exp.set_description("pi",pi)
    exp.set_description("f",frequency)
    exp.set_description("det_phase", det_phase)

    cycle_phase1=(0,0,180,180)[cycle]
    cycle_phase2=(0,180,0,180)[cycle]

    exp.set_description("det_phase1", (0,0,180,180)[cycle])
    exp.set_description("det_phase2", (0,180,0,180)[cycle])

    # set frequency (paranoid version)
    exp.rf_pulse(0xf000,1e-6)
    exp.rf_pulse(0x8000,1e-6)
    exp.wait(1e-4)
    exp.set_frequency(frequency,0)

    # wait necessary mulitple of t1
    exp.set_frequency(0,cycle_phase1)
    exp.wait(t1*3-2e-6)
    
    # first pulse
    exp.rf_pulse(1, 2e-6)
    exp.rf_pulse(3, pi/2.0)
    
    # wait dephasing time
    exp.wait(tau1-(2e-6)-pi/2.0)

    # second pulse
    exp.rf_pulse(1, 2e-6)
    exp.rf_pulse(3, pi/2.0)
    
    # set detection phase and wait echo time
    exp.set_frequency(0,det_phase)
    exp.wait(tau1-(10e-6)-(2e-6)-pi/4.0)
    exp.record(1024*2, 10e6, 260e-6, sensitivity=0.5)
    exp.set_frequency(0,cycle_phase2)
    # wait rest of mixing time
    exp.wait(tau2-(254e-6)-tau1-pi/4.0)

    exp.rf_pulse(1, 2e-6)
    exp.rf_pulse(3, pi/2.0)

    # set detection phase and wait echo time
    exp.set_frequency(0,det_phase)
    exp.wait(tau1-(10e-6)-(2e-6)-pi/4.0)
    exp.record(1024*2, 10e6, sensitivity=0.5)
    
    return exp
    
def experiment():
    pi = 1.3e-6 # bei 15db
    t1 = 0.11 # bei 500K
    tau1 = 40e-6
    tau2 = 90e-3
    f = 75.0e6
    phase0 = 120.0 # vorher 150 für 540K Serie

    for tau1 in staggered_range(log_range(60e-6, 0.5e-3, 5), 5):
    # for t in staggered_range(log_range(1e-3,3,40)):
    # for pi in lin_range(0.5e-6,2e-6,0.2e-6):
        accus=4
        if tau1>5e-5: accus*=2
        for accu in xrange(accus):
            # def stim_echo(t1, tau1, tau2, pi, phase0, cycle=0, frequency=0)
            e=stim_echo(t1, tau1, tau2, pi, phase0, accu%4, f)
            #def sat_recovery(t1, t, tau, pi, frequency, phase):
            #e=sat_recovery(0.3, t, 60e-6, pi, f, phase)
            yield e
