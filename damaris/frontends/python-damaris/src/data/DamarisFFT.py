import numpy
import sys
import autophase

class DamarisFFT:
    def clip(self, start=None, stop=None):
		"""
		Method for clipping data, only the timesignal between start and stop
		is returned.
		start and stop can be either time or frequency. The unit is automaticall determined
		"""
		# check if start/stop order is properly
		if start > stop:
			# I could swap start/stop actually
			# TODO swap values?
			raise
		# if one uses clip as a "placeholder"
		if start==None and stop==None:
			return self

		if start==None:
			start = 0
		if stop==None:
			stop = -1
		# check if data is fft which changes the start/stop units
		# TODO should get nicer(failsafe), i.e. flags in the object?
		if self.xlabel == "Frequency / Hz":
			isfft = True
			start = self.x.size*(0.5 + start/self.sampling_rate)
			stop  = self.x.size*(0.5 +  stop/self.sampling_rate)
		else:
			isfft = False
			# get the corresponding indices
			start *= self.sampling_rate
			stop  *= self.sampling_rate
		# check if boundaries make sense, raise exception otherwise
		if numpy.abs(int(start)-int(stop))<=0:
			raise ValueError("start stop too close: There are no values in the given boundaries!")
		for ch in xrange(len(self.y)):
		# clip the data for each channel
		# TODO multi records
			self.y[ch] = self.y[ch][int(start):int(stop)]
		# TODO what to do with x? Should it start from 0 or from start?
		# self.x = self.x[:int(stop)-int(start)]
		self.x = self.x[int(start):int(stop)]
		return self

    def baseline(self, last_part=0.1):
		"""
		Correct the baseline of your data by subtracting the mean of the 
		last_part fraction of your data.

		last_part defaults to 0.1, i.e. last 10% of your data
		"""
		# TODO baselinecorrection for spectra after:
		# Heuer, A; Haeberlen, U.: J. Mag. Res.(1989) 85, Is 1, 79-94 
		# Should I create an empty object?
		# I deided to do NOT a copy, but 
		# rather modify the object
		n = int(self.x.size*last_part)
		for ch in xrange(len(self.y)):
			self.y[ch] -= self.y[ch][:-n].mean()
		# Skip the following due to design reasons
		# new_object.was_copied = True
		return self


		""" 
		Apodization functions:
		* exp_window and gauss_window are S/N enhancing,
		* dexp_window and traf_window are resolution enhancing
		* standard windows [hamming, hanning, bartlett, blackman, kaiser-bessel] 
		  are also available 
		self.x			=	time points
		elf.aquisition_time    =	aquisition time (no. samples / sampling_rate)
		line_broadening			=	line broadening factor (standard = 10 Hz)
		gaussian_multiplicator  =	Gaussian Multiplication Factor for 
									the double exponential apodization 
									function (standard = 0.3)
		"""
    def exp_window(self, line_broadening=10):
		"""
		exponential window
		"""
		apod = numpy.exp(-numpy.arange(self.x.size)*line_broadening)
		print apod
		for i in range(2):
			self.y[i] = self.y[i]*apod
		return self

    def gauss_window(self, line_broadening=10):
		apod = numpy.exp(-(numpy.arange(self.x.size)*line_broadening)**2)
		for i in range(2):
			self.y[i] = self.y[i]*apod
		return self

    def dexp_window(self, line_broadening=10, gaussian_multiplicator=0.3):
		apod = numpy.exp(-(numpy.arange(self.x.size)*line_broadening - gaussian_multiplicator*self.x.max())**2)
		for i in range(2):
			self.y[i] = self.y[i]*apod 
		return self

    def traf_window(self, line_broadening=10):
		apod = (numpy.exp(-numpy.arange(self.x.size)*line_broadening))**2 / ( (numpy.exp(-numpy.arange(self.x.size)*line_broadening))**3 
		+ (numpy.exp(-self.x.max()*line_broadening))**3  )
		for i in range(2):
			self.y[i] = self.y[i]*apod
		return self

    def hanning_window(self):
		apod = numpy.hanning(self.x.size)
		for i in range(2):
			self.y[i] = self.y[i]*apod
		return self

    def hamming_window(self):
		apod = numpy.hamming(self.x.size)
		for i in range(2):
			self.y[i] = self.y[i]*apod
		return self

    def blackman_window(self):
		apod = numpy.blackman(self.x.size)
		for i in range(2):
			self.y[i] = self.y[i]*apod
		return self

    def bartlett_window(self):
		apod = numpy.bartlett(self.x.size)
		for i in range(2):
			self.y[i] = self.y[i]*apod
		return self

    def kaiser_window(self, beta=4, use_scipy=None):
		if use_scipy == None:
			# modified Bessel function of zero kind order from somewhere
			def I_0(x):
				i0=0
				fac = lambda n:reduce(lambda a,b:a*(b+1),range(n),1)
				for n in xrange(20):
					i0 += ((x/2.0)**n/(fac(n)))**2
				return i0

			t = numpy.arange(self.x.size, type=numpy.Float) - self.x.size/2.0
			T = self.x.size
			# this is the window function array
			apod = I_0(beta*numpy.sqrt(1-(2*t/T)**2))/I_0(beta)
		else:
			# alternative method using scipy
			import scipy 
			apod=scipy.kaiser(self.x.size, beta)

		for i in range(2):
			self.y[i] = self.y[i]*apod
		return	self

    def autophase(self):
        """
        works nice with a SNR above 20 dB
        10 V signal height to 1V noise width
        """
        autophase.get_phase(self)
        return self

    def fft(self, samples=None):
		"""
		Fouriertransform the timesignal inplace.
		For "zerofilling" set "samples" to a value higher than your data length. 
		Shorten "samples" to truncate your data.
		samples takes only integer values

		"""
		# Is this smart performance wise? Should I create an empty object?
		# Tests showed that this try except block performed 3.78ms 
		# timesignal.baseline().fft()
		# with out this it needed 4.41 ms, thus this is justified :-)
		#try: 
		#	if self.was_copied:
		#		new_object = self
		#except:
		#	new_object = self+0
		fft_of_signal = numpy.fft.fft(self.y[0] + 1j*self.y[1], n=samples)
		fft_of_signal = numpy.fft.fftshift(fft_of_signal)
		dwell = 1.0/self.sampling_rate
		fft_frequencies = numpy.fft.fftfreq(self.x.size, dwell)
		self.x = numpy.fft.fftshift(fft_frequencies)
		self.y[0] = fft_of_signal.real
		self.y[1] = fft_of_signal.imag
		self.set_xlabel("Frequency / Hz")
		return self
	
    def magnitude(self):
		# this should calculate the absolute value, and set the imag channel to zero
		self.y[0] = numpy.sqrt(self.y[0]**2+self.y[1]**2)
		self.y[1] *= 0 #self.y[0].copy()
		return self
