import threading
import math
import types
import sys
import tables
import numpy
import exceptions
import UserDict
import Drawable

class AccumulatedValue:

    def __init__(self, mean=None, sigma=0, n=0):
        """
        one value with std. deviation
        can be initialized by:
        No argument: no entries
        one argument: first entry
        three arguments: already existing statistics defined by mean, sigma, n
        """
        if mean is None:
            self.y=0.0
            self.y2=0.0
            self.n=0
        elif n==0:
            self.y=mean
            self.n=1
            self.y2=mean*mean
        else:
            self.n=int(n)
            self.y=float(mean*self.n)
            self.y2=float(self.n*(sigma*sigma+mean*mean))

    def __add__(self,y):

        if (type(y) is instance and issubclass(y,AccumulatedValue)):
            raise Exception("Not implemented")
        else:
            new_one=AccumulatedValue()
            new_one.y=self.y+float(y)
            new_one.y2=self.y2+float(y)**2
            new_one.n=self.n+1
            return new_one

    def __iadd__(self,y):
        self.y+=float(y)
        self.y2+=float(y)**2
        self.n+=1
        return self

    def copy(self):
        a=AccumulatedValue()
        a.y=self.y
        a.y2=self.y2
        a.n=self.n
        return a

    def mean(self):
        if self.n is None:
            return None
        else:
            return self.y/self.n
    
    def sigma(self):
        if self.n>1:
            variance=((self.y2)-(self.y*self.y)/float(self.n))/(self.n-1.0)
            if variance<0:
                if variance<-1e-20:
                    print "variance=%g<0! assuming 0"%variance
                return 0.0
            return math.sqrt(variance)
        elif self.n==1:
            return 0
        else:
            return None

    def mean_sigma(self):
        if self.n>1:
            variance=((self.y2)-(self.y*self.y)/float(self.n))/(self.n-1.0)/float(self.n)
            if variance<0:
                if variance<-1e-20:
                    print "variance=%g<0! assuming 0"%variance
                return 0.0
            return math.sqrt(variance)
        elif self.n==1:
            return 0
        else:
            return None

    def __str__(self):
        if self.n==0:
            return "no value"
        elif self.n==1:
            return str(self.y)
        else:
            return "%g +/- %g (%d accumulations)"%(self.mean(),self.sigma(),self.n)

    def __repr__(self):
        return str(self)

class MeasurementResult(Drawable.Drawable, UserDict.UserDict):

    def __init__(self, quantity_name):
        """
        convenient accumulation and interface to plot functios
        """
        Drawable.Drawable.__init__(self)
        UserDict.UserDict.__init__(self)
        self.quantity_name=quantity_name
        self.lock=threading.RLock()

    # get the selected item, if it does not exist, create an empty one
    def __getitem__(self, key):
        if key not in self:
            a=AccumulatedValue()
            UserDict.UserDict.__setitem__(self, key, a)
            return a
        else:
            return UserDict.UserDict.__getitem__(self, key)

    def __setitem__(self,key,value):
        if isinstance(value, int) or isinstance(value,float):
            return UserDict.UserDict.__setitem__(self,key,AccumulatedValue(value))
        else:
            return UserDict.UserDict.__setitem__(self,key,value)

    def __add__(self, right_value):
        if right_value==0:
            return self.copy()
        else:
            raise Exception("not implemented")

    def get_title(self):
        return self.quantity_name

    def get_xdata(self):
        k=self.keys()
        k=filter(lambda key: (type(UserDict.UserDict.__getitem__(self,key)) is types.InstanceType and
                              isinstance(UserDict.UserDict.__getitem__(self,key), AccumulatedValue)) or
                 type(UserDict.UserDict.__getitem__(self,key)) is types.FloatType,
                 k)
        k.sort()
        return k

    def get_ydata(self):
        return map(lambda key: UserDict.UserDict.__getitem__(self,key),self.get_xdata())

    def get_errorplotdata(self):
        k=self.keys()
        k=filter(lambda key: (type(UserDict.UserDict.__getitem__(self,key)) is types.InstanceType and
                              isinstance(UserDict.UserDict.__getitem__(self,key), AccumulatedValue)) or
                 type(UserDict.UserDict.__getitem__(self,key)) is types.FloatType,
                 k)
        k=numpy.array(k, dtype="Float64")
        k.sort()
        v=numpy.array(map(lambda key: UserDict.UserDict.__getitem__(self,key).mean(),k), dtype="Float64")
        e=numpy.array(map(lambda key: UserDict.UserDict.__getitem__(self,key).mean_sigma(),k), dtype="Float64")
        return [k,v,e]

    def uses_statistics(self):
        """
        drawable interface method, returns True
        """
        return True

    def write_as_csv(self,destination=sys.stdout):
        """
        writes the data to a file or to sys.stdout
        destination can be a file or a filename
        suitable for further processing
        """
        # write sorted
        the_destination=None
        if isinstance(destination,types.FileType):
            the_destination=destination
        elif isinstance(destination,types.StringTypes):
            the_destination=file(destination,"w")
        else:
            raise Exception("sorry destination %s is not valid"%(repr(destination)))

        the_destination.write("# quantity:"+str(self.quantity_name)+"\n")
        the_destination.write("# x y ysigma n\n")
        for x in self.get_xdata():
            y=UserDict.UserDict.__getitem__(self,x)
            if type(y) is types.FloatType:
                the_destination.write("%g %g 0 1\n"%(x,y))                
            else:
                the_destination.write("%g %g %g %d\n"%(x,y.mean(),y.mean_sigma(),y.n))

        the_destination=None


    def write_to_hdf(self, hdffile, where, name, title, complib=None, complevel=None):

        h5_table_format= {
            "x" : tables.Float64Col(),
            "y_mean" : tables.Float64Col(),
            "y_sigma" : tables.Float64Col(),
            "n" : tables.Int64Col()
            }
        filter=None
        if complib is not None:
            if complevel is None:
                complevel=9
            filter=tables.Filters(complevel=complevel,complib=complib,shuffle=1)

        mr_table=hdffile.createTable(where=where,name=name,
                                     description=h5_table_format,
                                     title=title,
                                     filters=filter,
                                     expectedrows=len(self))
        mr_table.attrs.damaris_type="MeasurementResult"
        self.lock.acquire()
        try:
            mr_table.attrs.quantity_name=self.quantity_name
                
            row=mr_table.row
            for x in self.get_xdata():
                y=UserDict.UserDict.__getitem__(self,x)
                row["x"]=x
                if type(y) is types.FloatType:
                    row["y_mean"]=y
                    row["y_sigma"]=0.0
                    row["n"]=1
                else:
                    row["y_mean"]=y.mean()
                    row["y_sigma"]=y.mean_sigma()
                    row["n"]=y.n
                row.append()

        finally:
            mr_table.flush()
            self.lock.release()
