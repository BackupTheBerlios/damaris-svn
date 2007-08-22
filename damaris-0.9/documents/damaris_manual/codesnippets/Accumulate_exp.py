import Experiment

def fid_experiment():
	e=Experiment.Experiment()
	e.set_frequency(frequency=300.01e6, phase=0)
	e.ttl_pulse(length=5e-6, channel=1)		# gate for the RF pulse
	e.ttl_pulse(length=2e-6, channel=1+2)	# RF pulse
	e.wait(10e-6)							# dead-time
	# record 1024 samples at sampling-rate 2 MHz 
	# with 2V sensitivity
	e.record(samples=1024, frequency=2e6, sensitivity=2)
	return e

def experiment():
	accumulations=8
	for run in xrange(accumulations):
		yield fid_experiment()