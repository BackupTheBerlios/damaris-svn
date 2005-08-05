function nullen=Nullen(n,vec)
d=size(vec);
if d(1)==1
    nullen=zeros(1,n);
else
    nullen=zeros(n,1);
end