function FFT=Fouriertransfo(time,re,im)
samples=max(size(re));
rate=samples/(max(time));
frequency=-rate/2:rate/(samples-1):rate/2;
fid=vectocompl(re,im);

FFT=fftshift(fft(fid));

fftRe=real(FFT);
fftIm=imag(FFT);
FFT=[frequency;fftRe;fftIm];






