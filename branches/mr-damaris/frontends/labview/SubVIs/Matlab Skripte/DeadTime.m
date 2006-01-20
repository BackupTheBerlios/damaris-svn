function cv=DeadTime(v,rate,deadtime)
n=round(deadtime*rate);
if n<1
    n=1;
end
dim=max(size(v));
cv=v(n:dim);
