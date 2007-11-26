function FID=Phasecorrection(re,im,phase)
samples=max(size(re));
Cos=cos(phase);
Sin=sin(phase);
Re=[];
Im=[];
for i=1:samples
    Re=[Re re(i)*Cos-im(i)*Sin];
    Im=[Im re(i)*Sin+im(i)*Cos];
end
FID=[Re; Im];