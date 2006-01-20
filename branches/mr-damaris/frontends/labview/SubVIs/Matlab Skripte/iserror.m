function error=iserror(Re,Im,samples)
error=0;
if max(size(Re))~=samples
    error=1;
end
if max(size(Im))~=samples
    error=1;
end
