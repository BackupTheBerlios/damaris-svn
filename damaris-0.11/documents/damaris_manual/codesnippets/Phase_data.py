import ADC_Result
import Accumulation

def result():
	# create accumulation object
	accu = Accumulation.Accumulation()
	# loop over the incoming results
	for timesignal in results:
		# read in the variable "run" from the result
		run_from_exp_script = timesignal.get_description("run")
		# provide the single scan to the display tab
		data[``Timesignal''] = timesignal
		# add timesignal to the object
		if run%4 in [0,2]:
			accu += timsignal
		elif run%4 in [1,3]:
			accu -= timesignal
		# provide the accumulated signal to display tab
		data["Accumulation"] = accu		
