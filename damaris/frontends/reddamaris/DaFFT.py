import numarray.fft
import numarray.mlab
import numarray
import ADC_Result

class FFT:
    def __init__(self, one_result):
	# create copy of one_result and work only on the copy
	# also extract some informations
	self.the_result = one_result + 0
	self.timepoints = numarray.array(one_result.x)
	self.sampling_rate = one_result.get_sampling_rate()
	self.data_points = one_result.get_ydata(0).size()
	self.aquisition_time = self.data_points / float(self.sampling_rate)
	self.the_result.set_xlabel('Frequency [Hz]')
		
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

    def base_corr(self, cutoff=0.3, show=0):
	"""
	Subtracts the mean of the last cutoff % of the timsignal
	to get rid of the  DC part in the FFT and returns the
	new data.
	If cutoff is not given, the mean of the last 30% will be
	subtracted.
	If show=1 the result is return and not the instance. This allows to plot the baseline corrected signal
	Example:
	base_corr(cutoff=0.2, show=1)
	"""
	n = self.data_points
	last_points = int(cutoff*n)
	for i in range(2):
	    self.the_result.y[i] = self.the_result.y[i] - self.the_result.y[i][:-last_points].mean()
	if show == 1 :
	    return self.the_result
	return self

    def abs_fft(self, points=None, zoom=None):
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
	if zoom is None: return self.the_result
	else:
	    center, width = zoom
	    return self.zoom(self.the_result, center, width)

    def fft(self, points=None, zoom=None):
	realdata = numarray.array(self.the_result.y[0])
	imdata = numarray.array(self.the_result.y[1])
	data = realdata + 1j*imdata
	fftdata = self.rearrange(numarray.fft.fft(data, points))
	#phase = numarray.arctan(fftdata.real.sum() / fftdata.imag.sum())
	#print "PHASE:", phase*180/numarray.pi
	# create our x axis
	n = fftdata.size()
	self.the_result.x = numarray.arange(n)*(self.sampling_rate/n)-(self.sampling_rate/2.0)
	# create new result
	self.the_result.y[0] = fftdata.real
	self.the_result.y[1] = fftdata.imag
	if zoom is None: return self.the_result
	else:
	    center, width = zoom
	    return self.zoom(self.the_result, center, width)

    def realfft(self, points=None, zoom=None):
	realdata = numarray.array(self.the_result.y[0])
	imdata = numarray.array(self.the_result.y[1])
	data = realdata + 1j*imdata
	fftdata = self.rearrange(numarray.fft.fft(data, points))
	realfft = fftdata.real
	# create our x axis
	n = fftdata.size()
	self.the_result.x =  numarray.arange(n)*(self.sampling_rate/n)-(self.sampling_rate/2.0)
	self.the_result.y[0] = realfft
	self.the_result.y[1] = numarray.zeros(n)
	if zoom is None: return self.the_result
	else:
	    center, width = zoom
	    return self.zoom(self.the_result, center, width)

    def zoom(self,some_result, center="auto", width=1000):
	if center == "auto":
	    i_center = int(self.the_result.y[0].argmax())
	    maximum = self.the_result.y[0][i_center]
	    print "Maximum at Frequency:", self.the_result.x[i_center]
	else:
	    i_center = int(self.data_points/2.0+self.data_points*center/self.sampling_rate)
	#print "TODO: set width automagically"
	#if width == "auto":
	#    i_width = int(self.data_points*width)
	i_width = int(self.data_points*width/self.sampling_rate)
	some_result.x=some_result.x[i_center-i_width/2:i_center+i_width/2]
	some_result.y[0]=some_result.y[0][i_center-i_width/2:i_center+i_width/2]
	some_result.y[1]=some_result.y[1][i_center-i_width/2:i_center+i_width/2]
	return some_result

    """ 
    Apodization functions:
    * exp_window and gauss_window are S/N enhancing,
    * dexp_window and traf_window are resolution enhancing
    * standard windows [hamming, hanning, bartlett, blackman, kaiser-bessel] are also available 
    self.timepoints	    =	time points
    self.aquisition_time    =	aquisition time (no. samples / sampling_rate)
    line_broadening	    =	line broadening factor (standard = 10 Hz)
    gaussian_multiplicator  =	Gaussian Multiplication Factor for 
				the double exponential apodization 
				function (standard = 0.3)
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

    def hanning_window(self, show=0):
	apod = numarray.mlab.hanning(self.data_points)
	for i in range(2):
	    self.the_result.y[i] = self.the_result.y[i]*apod
	if show == 1:
	    return self.the_result
	return self

    def hamming_window(self, show=0):
	apod = numarray.mlab.hamming(self.data_points)
	for i in range(2):
	    self.the_result.y[i] = self.the_result.y[i]*apod
	if show == 1:
	    return self.the_result
	return self

    def blackman_window(self, show=0):
	apod = numarray.mlab.blackman(self.data_points)
	for i in range(2):
	    self.the_result.y[i] = self.the_result.y[i]*apod
	if show == 1:
	    return self.the_result
	return self

    def bartlett_window(self, show=0):
	apod = numarray.mlab.bartlett(self.data_points)
	for i in range(2):
	    self.the_result.y[i] = self.the_result.y[i]*apod
	if show == 1:
	    return self.the_result
	return self

    def kaiser_window(self, beta=4, show=0, use_scipy=None):
	if use_scipy == None:
	    # modified Bessel function of zero kind order
	    def I_0(x):
		i0=0
		fac = lambda n:reduce(lambda a,b:a*(b+1),range(n),1)
		for n in range(20):
		    i0 += ((x/2.0)**n/(fac(n)))**2
		return i0
	
	    t = numarray.arange(self.data_points, type=numarray.Float) - self.data_points/2.0
	    T = self.data_points
	    # this is the window function array
	    apod = I_0(beta*numarray.sqrt(1-(2*t/T)**2))/I_0(beta)
	else:
	    # alternative method using scipy
	    import scipy 
	    apod=scipy.kaiser(self.data_points, beta)
	
	for i in range(2):
	    self.the_result.y[i] = self.the_result.y[i]*apod
	if show == 1:
	    return self.the_result
	return self
						
