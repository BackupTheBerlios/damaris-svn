import ADC_Result
import Accumulation

def result():
	# create accumulation object
	accu = Accumulation.Accumulation()
	# loop over the incoming results
	for timesignal in results:
		# plot the timesignal
		data["Timesignal"] = timesignal
		# add timesignal to the object
		accu += timesignal
		# provide the data to display tab
		data["Accumulation"] = accu
