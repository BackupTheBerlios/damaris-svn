function z=vectocompl(re,im)
samples=max(size(re));
z=[];
for j=1:samples
    z=[z re(j)+im(j)*i];
end
