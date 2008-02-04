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

    def __init__(self, mean=None, mean_err=0, n=0):
        """
        one value with std. deviation
        can be initialized by:
        No argument: no entries
        one argument: first entry
        three arguments: already existing statistics defined by mean, mean's error, n
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
            self.y2=float(mean_err)**2*n*(n-1.0)+float(mean)**2*n

    def __add__(self,y):
        new_one=AccumulatedValue()
        if (type(y) is types.InstanceType and isinstance(y, AccumulatedValue)):
            new_one.y=self.y+y.y
            new_one.y2=self.y2+y.y2
            new_one.n=self.n+y.n
        else:
            new_one.y=self.y+float(y)
            new_one.y2=self.y2+float(y)**2
            new_one.n=self.n+1
        return new_one

    def __iadd__(self,y):
        if (type(y) is types.InstanceType and isinstance(y, AccumulatedValue)):
            self.y+=y.y
            self.y2+=y.y2
            self.n+=y.n
        else:
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
        """
        returns the mean of all added/accumulated values
        """
        if self.n is None:
            return None
        else:
            return self.y/self.n
    
    def sigma(self):
        """
        returns the standard deviation added/accumulated values
        """
        if self.n>1:
            variance=(self.y2-(self.y**2)/float(self.n))/(self.n-1.0)
            if variance<0:
                if variance<-1e-20:
                    print "variance=%g<0! assuming 0"%variance
                return 0.0
            return math.sqrt(variance)
        elif self.n==1:
            return 0
        else:
            return None

    def mean_error(self):
        """
        returns the mean's error (=std.dev/sqrt(n)) of all added/accumulated values
        """
        if self.n>1:
            variance=(self.y2-(self.y**2)/float(self.n))/(self.n-1.0)
            if variance<0:
                if variance<-1e-20:
                    print "variance=%g<0! assuming 0"%variance
                return 0.0
            return math.sqrt(variance/self.n)
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
            return "%g +/- %g (%d accumulations)"%(self.mean(),self.mean_error(),self.n)

    def __repr__(self):
        return str(self)

class MeasurementResult(Drawable.Drawable, UserDict.UserDict):

    def __init__(self, quantity_name):
        """
        convenient accumulation and interface to plot functions

        dictionary must not contain anything but AccumulatedValue instances
        """
        Drawable.Drawable.__init__(self)
        UserDict.UserDict.__init__(self)
        self.quantity_name=quantity_name
        self.lock=threading.RLock()

    # get the selected item, if it does not exist, create an empty one
    def __getitem__(self, key):
        if key not in self:
            a=AccumulatedValue()
            self.data[float(key)]=a
            return a
        else:
            return self.data[float(key)]

    def __setitem__(self,key,value):
        if not (type(value) is types.InstanceType and isinstance(value, AccumulatedValue)):
            value=AccumulatedValue(float(value))
        return UserDict.UserDict.__setitem__(self,
                                             float(key),
                                             value)

    def __add__(self, right_value):
        if right_value==0:
            return self.copy()
        else:
            raise Exception("not implemented")

    def get_title(self):
        return self.quantity_name

    def get_xdata(self):
        """
        sorted array of all dictionary entries
        """
        k=numpy.array(self.data.keys(), dtype="Float64")
        k.sort()
        return k

    def get_ydata(self):
        return self.get_xydata()[1]

    def get_xydata(self):
        k=self.get_xdata()
        v=numpy.array(map(lambda key: self.data[key].mean(),k), dtype="Float64")
        return [k,v]

    def get_errorplotdata(self):
        k=self.get_xdata()
        v=numpy.array(map(lambda key: self.data[key].mean(),k), dtype="Float64")
        e=numpy.array(map(lambda key: self.data[key].mean_error(),k), dtype="Float64")
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
            y=self.data[x]
            if type(y) in [types.FloatType, types.IntType, types.LongType]:
                the_destination.write("%g %g 0 1\n"%(x,y))                
            else:
                the_destination.write("%g %g %g %d\n"%(x,y.mean(),y.mean_error(),y.n))

        the_destination=None


    def write_to_hdf(self, hdffile, where, name, title, complib=None, complevel=None):

        h5_table_format= {
            "x" : tables.Float64Col(),
            "y" : tables.Float64Col(),
            "y_err" : tables.Float64Col(),
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
        mr_table.flavor="numpy"
        mr_table.attrs.damaris_type="MeasurementResult"
        self.lock.acquire()
        try:
            mr_table.attrs.quantity_name=self.quantity_name
                
            row=mr_table.row
            xdata=self.get_xdata()
            if xdata.shape[0]!=0:
                for x in self.get_xdata():
                    y=self.data[x]
                    row["x"]=x
                    if type(y) in [types.FloatType, types.IntType, types.LongType]:
                        row["y"]=y
                        row["y_err"]=0.0
                        row["n"]=1
                    else:
                        row["y"]=y.mean()
                        row["y_err"]=y.mean_error()
                        row["n"]=y.n
                    row.append()

        finally:
            mr_table.flush()
            self.lock.release()

def read_from_hdf(hdf_node):
    """
    reads a MeasurementResult object from the hdf_node
    or None if the node is not suitable
    """

    if not isinstance(hdf_node, tables.Table):
        return None

    if hdf_node._v_attrs.damaris_type!="MeasurementResult":
        return None

    mr=MeasurementResult(hdf_node._v_attrs.quantity_name)
    
    for r in hdf_node.iterrows():
        mr[r["x"]]=AccumulatedValue(r["y"],r["y_err"],r["n"])

    return mr
