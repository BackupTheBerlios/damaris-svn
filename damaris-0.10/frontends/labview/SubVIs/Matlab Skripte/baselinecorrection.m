function blcfid=baselinecorrection(vec,base)

n=max(size(vec));
bl=round(base*n);

baseline=mean(vec(bl:n));
blcfid=vec-baseline;
