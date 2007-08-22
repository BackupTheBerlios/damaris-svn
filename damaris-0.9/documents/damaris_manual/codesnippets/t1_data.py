import ADC_Result
import Accumulation

def result():
	# create accumulation object
	accu = Accumulation.Accumulation()
	# loop over the incoming results
	for timesignal in results:
		# read in variables
		tau = float(timesignal.get_description("tau"))
		run = int(timesignal.get_description("run"))
		accus = int(timesignal.get_description("no_accus"))-1
		# provide the single scan to the display tab
		data["Timesignal"] = timesignal
		accu += timesignal
		# provide the accumulated signal to display tab
		data["Accumulation"] = accu	
		if run == accus:
			# provide the accumulated signal to display tab;
			# in this case all accumulated signals will be
			# available in the data dictionary and thus
			# saved when the experiment is finnished
			data["Accumulation %i"%run] = accu	
			# open a file in append mode,
			# file will be created if not existing
			afile.open('t1.dat','a')
			# get the maximum on the y channel
			amplitude = accu.y[0].max()
			# write tau and amplitude seperated by a <tab> and 
			# ending with a <newline> into the file
			afile.write("%e\t%e\n"%(tau, amplitude))
			# close the file
			afile.close()
			# create a new accumulation object
			accu = Accumulation.Accumulation()
