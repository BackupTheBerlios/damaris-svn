function meanvec=akku(meanvec,vec,shot)
d=size(meanvec);
if d(2)~=1;
    meanvec=meanvec';
end
d=size(vec);
if d(2)~=1;
    vec=vec';
end

if shot==1
    meanvec=zeros(max(d),1);
end

dim=max(size(meanvec));
dim2=max(size(vec));
if dim>dim2
    vec=[zeros(dim-dim2,1); vec];
end
if dim2>dim
    meanvec=[zeros(dim2-dim,1); meanvec];
end

meanvec=meanvec*(shot-1)/shot + vec/shot;


