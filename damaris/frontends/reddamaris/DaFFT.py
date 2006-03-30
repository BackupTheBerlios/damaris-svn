import numarray.fft
import numarray
import ADC_Result
class FFT:
    def __init__(self, one_result):
	# create copy of one_result and work only on the copy
	self.the_result = one_result + 0
	self.timepoints = numarray.array(one_result.x)
	self.sampling_rate = one_result.get_sampling_rate()
	self.data_points = one_result.get_ydata(0).size()
	self.aquisition_time = self.data_points / float(self.sampling_rate)
	
    def rearrange(self, an_array):
        """
	Reorder array from 

	[0,1,2,3...,N,-N...,-3,-2,-1]

	to 

	[-N,...,-3,-2,-1,0,1,2,3,...,N]
	"""
	new_array = an_array*0
	n=new_array.size()
	new_array[n/2:] = an_array[:n/2]
	new_array[:n/2] = an_array[n/2:]
	return new_array

    def base_corr(self, cutoff=0.1, show=0):
	"""
	Subtracts the mean of the last cutoff % of the timsignal
	to get rid of the  DC part in the FFT and returns the
	new data.
	If cutoff is not given, the mean of the last 10% will be
	subtracted.
	If show=1 the result is return and no the instance. This allows to plot the baseline corrected signal
	Example:
	base_corr(cutoff=0.2, show=1)
	"""
	n = self.data_points
	bg = int(cutoff*n)
	#	new_data = numarray.zeros(some_data.shape)
	for i in range(2):
	    self.the_result.y[i] = self.the_result.y[i] - self.the_result.y[i][:-bg].mean()
	#self.the_result.y[1] = self.the_result.y[1] - self.the_result.y[1][:-bg].mean()
	#self.the_result.y = self.the_result.y - self.the_result.y[:-bg].mean()
	if show == 1 :
	    return self.the_result
	return self

    def abs_fft(self, points=None):
	"""
	Fourier transforms the timesignal;
	points is the number of points to transform, if more points given than data points
	the rest is zero padded

	absfft(points=4096)
	"""
	realdata = numarray.array(self.the_result.y[0])
	imdata = numarray.array(self.the_result.y[1])
	data = realdata + 1j*imdata
	fftdata = self.rearrange(numarray.fft.fft(data, points))
	absfft = numarray.sqrt(fftdata.real**2 + fftdata.imag**2)
	# create our x axis
	n = fftdata.size()
	self.the_result.x = numarray.arange(n)*(self.sampling_rate/n)-(self.sampling_rate/2.0)
	self.the_result.y[0] = absfft
	self.the_result.y[1] = numarray.zeros(n)
	return self.the_result

    def fft(self, points=None):
	realdata = numarray.array(self.the_result.y[0])
	imdata = numarray.array(self.the_result.y[1])
	data = realdata + 1j*imdata
	fftdata = self.rearrange(numarray.fft.fft(data, points))
	# create our x axis
	n = fftdata.size()
	self.the_result.x = numarray.arange(n)*(self.sampling_rate/n)-(self.sampling_rate/2.0)
	# create new result
	self.the_result.y[0] = fftdata.real
	self.the_result.y[1] = fftdata.imag
	return self.the_result 

    def realfft(the_result, points=None):
	realdata = numarray.array(self.the_result.y[0])
	imdata = numarray.array(self.the_result.y[1])
	data = realdata + 1j*imdata
	fftdata = self.rearrange(numarray.fft.fft(self.data))
	realfft = numarray.sqrt(fftdata.real**2)
	# create our x axis
	n = fftdata.size()
	self.the_result.x =  numarray.arange(n)*(self.sampling_rate/n)-(self.sampling_rate/2.0)
	self.the_result.y[0] = realfft
	self.the_result.y[1] = numarray.zeros(n)
	return new_result 

    """ 
    Apodization functions:
    exp_window and gauss_window are S/N enhancing,
    dexp_window and traf_window are resolution enhancing
    self.timepoints	    =	time points
    self.aquisition_time    =	aquisition time (no. samples / sampling_rate)
    line_broadening	    =	line broadening factor (Standard = 10 Hz)
    gaussian_multiplicator  =	Gaussian Multiplikation Factor for 
				the double exponential apodization 
				function (Standard=0.3)
    """
    def exp_window(self, line_broadening=10, show=0):
	apod = numarray.exp(-self.timepoints*line_broadening)
	for i in range(2):
	    self.the_result.y[i] = self.the_result.y[i]*apod
	if show == 1 :
	    return self.the_result
	return self

    def gauss_window(self, line_broadening=10, show=0):
	apod = numarray.exp(-(self.timepoints*line_broadening)**2)
	for i in range(2):
	    self.the_result.y[i] = self.the_result.y[i]*apod
	if show == 1 :
	    return self.the_result
	return self

    def dexp_window(self, line_broadening=10, gaussian_multiplicator=0.3, show=0):
	apod = numarray.exp(-(self.timepoints*line_broadening - gaussian_multiplicator*self.aquisition_time)**2)
	for i in range(2):
	    self.the_result.y[i] = self.the_result.y[i]*apod 
	if show == 1:
	    return self.the_result
	return self

    def traf_window(self, line_broadening=10, show=0):
	apod = (numarray.exp(-self.timepoints*line_broadening))**2 / ( (numarray.exp(-self.timepoints*line_broadening))**3 
		+ (numarray.exp(-self.aquisition_time*line_broadening))**3  )
	for i in range(2):
	    self.the_result.y[i] = self.the_result.y[i]*apod
	if show == 1:
	    return self.the_result
	return self
