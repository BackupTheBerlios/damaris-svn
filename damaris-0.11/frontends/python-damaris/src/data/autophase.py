from scipy.optimize import fmin_powell, bisect, ridder, brentq
import numpy as N

def calculate_entropy(phi, real, imag, gamma, dwell):
	"""
	Calculates the entropy of the spectrum (real part).
	p = phase
	gamma should be adjusted such that the penalty and entropy are in the same magnitude
	"""
	# This is first order phasecorrection
	# corr_phase = phi[0]+phi[1]*arange(0,len(signal),1.0)/len(signal) # For 0th and 1st correction
	
	# Zero order phase correction
	real_part = real*N.cos(phi)-imag*N.sin(phi)
	
	# Either this for calculating derivatives:
	# Zwei-Punkt-Formel
	# real_diff = (Re[1:]-Re[:-1])/dwell
	# Better this:
	# Drei-Punkte-Mittelpunkt-Formel (Ränder werden nicht beachtet)
	# real_diff = abs((Re[2:]-Re[:-2])/(dwell*2))
	# Even better:
	# Fünf-Punkte-Mittelpunkt-Formel (ohne Ränder)
	real_diff = N.abs((real_part[:-4]-8*real_part[1:-3]
						+8*real_part[3:-1]-2*real_part[4:])/(12*dwell))
	
	# TODO Ränder, sind wahrscheinlich nicht kritisch
	
	# Calculate the entropy
	h = real_diff/real_diff.sum()
	# Set all h with 0 to 1 (log would complain)
	h[h==0]=1
	entropy = N.sum(-h*N.log(h))

	# My version, according the paper
	#penalty = gamma*sum([val**2 for val in Re if val < 0])
	# calculate penalty value: a real spectrum should have positive values
	if real_part.sum() < 0:
		tmp = real_part[real_part<0]
		penalty = N.dot(tmp,tmp)
		if gamma == 0:
			gamma = entropy/penalty
		penalty = N.dot(tmp,tmp)*gamma
	else:
		penalty = 0
	#print "Entropy:",entrop,"Penalty:",penalty # Debugging
	shannon = entropy+penalty
	return shannon

def get_phase(result_object):
	global gamma
	gamma=0
	real = result_object.y[0].copy()
	imag = result_object.y[1].copy()
	dwell = 1.0/result_object.sampling_rate
	# fmin also possible
	xopt = fmin_powell(	func=calculate_entropy,
						x0=N.array([0.0]), 
						args=(real, imag, gamma, dwell),
						disp=0)
	result_object.y[0] = real*N.cos(xopt) - imag*N.sin(xopt)
	result_object.y[1] = real*N.sin(xopt) + imag*N.cos(xopt)
	return 
