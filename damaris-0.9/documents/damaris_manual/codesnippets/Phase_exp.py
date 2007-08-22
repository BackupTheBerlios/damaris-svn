import Experiment

def fid_experiment(run):
	e=Experiment.Experiment()
	# set a description, this one is used later in the result script
	e.set_description("run",run)
	# run%4 is run modulo(4), and thus goes like 0,1,2,3,0,1,2 ...
	e.set_frequency(frequency=300.01e6, phase=[0,90,180,270][run%4])
	e.ttl_pulse(length=5e-6, channel=1)		# gate for the RF pulse
	e.ttl_pulse(length=2e-6, channel=1+2)	# RF pulse
	e.wait(10e-6)							# dead-time
	# change phase, CYCLOPS phase cycling
	e.set_phase([0,90,180,270][run%4])
	# record 1024 samples at sampling-rate 2 MHz 
	# with a +/- 2V range 
	e.record(samples=1024, frequency=2e6, sensitivity=2)
	return e

def experiment():
	# should be multiple of 4
	accumulations=8
	for run in xrange(accumulations):
		yield fid_experiment(run)
