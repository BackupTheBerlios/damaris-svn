def fid_experiment(pi, tau, run, accus):
	e=Experiment()
	e.wait(9) # repetition time (in s)
	# set the descriptions, they are used later in the result script
	e.set_description("run",run)
	e.set_description("tau",tau)
	e.set_description("no_accus", accus)
	# run%4 is run modulo(4), and thus goes like 0,1,2,3,0,1,2 ...
	e.set_frequency(frequency=300.01e6, phase=[0,180][run%2])
	e.ttl_pulse(length=5e-6, channel=1)		# gate for the RF pulse
	e.ttl_pulse(length=pi, channel=1+2)		# RF pulse
	e.wait(tau)
	e.set_phase(phase=[0,0,180,180,90,90,270,270][run%8])
	e.ttl_pulse(length=5e-6, channel=1)		# gate for the RF pulse
	e.ttl_pulse(length=pi/2, channel=1+2)	# RF pulse
	e.wait(10e-6)							# dead-time
	# change phase, CYCLOPS phase cycling
	e.set_phase([0,0,180,180,90,90,270,270][run%8])
	# record 1024 samples at sampling-rate 2 MHz 
	# with a +/- 2V range
	e.record(samples=1024, frequency=2e6, sensitivity=2)
	return e

def experiment():
	# should be multiple of 8
	accumulations=8
	# here we loop over the tau values starting from 1ms to 10s
	# in a logarithmic 
	for t in log_range(start=1e-3, stop=10, step_no=20):
		# this inner loop does the accumulation
		for r in xrange(accumulations):
			yield fid_experiment(pi=4e-6, tau=t, run=r, accus=accumualations)
