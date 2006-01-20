Pfad='E:\Holger\Saved\OberflächenNMR260405\5.06 MHz';
tmin=90e-6 ; %Beginn des Echos
tmax=130e-6 ;% Ende des Echos
Variable='TE';  %Variablename
Endung='.akkufid';


addpath(Pfad);
files=dir(strcat(Pfad,'\*',Endung));
numfiles=max(size(files));
strlen1=length(Variable)+1;
strlen2=length(Endung);

S=[];


for j=1:numfiles
    
    data=load(files(j).name);
    Nend=max(size(data));
    tend=data(Nend,1);
    Nmin=round(tmin*Nend/tend);
    Nmax=round(tmax*Nend/tend);
    s = mean(sqrt(data(Nmin:Nmax,2).^2+data(Nmin:Nmax,3).^2));
    n = mean(sqrt(data(Nmax:Nend,2).^2+data(Nmax:Nend,3).^2));
   
    
    Name=files(j).name;
    te =  str2double(Name(strlen1:length(Name)-strlen2));
    
     S=[S; te abs(s/n-1)];
    
    
       
end

result=sortrows(S,1);
loglog(result(:,1),result(:,2),'r+')