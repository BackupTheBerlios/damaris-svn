import threading
import math
import types
import sys
import tables
import numpy
import exceptions
import UserDict
import Drawable

## provide gaussian statistics for a series of measured data points
#
# AccumulatedValue provides mean and error of mean after being fed with measured data
# internaly it keeps the sum, the sum of squares and the number of data points
class AccumulatedValue:

    def __init__(self, mean=None, mean_err=None, n=None):
        """
        one value with std. deviation
        can be initialized by:
        No argument: no entries
        one argument: first entry
	two arguments: mean and its error, n is set 2
        three arguments: already existing statistics defined by mean, mean's error, n
        """
        if mean is None:
            self.y=0.0
            self.y2=0.0
            self.n=0
        elif mean_err is None and n is None:
            self.y=float(mean)
            self.y2=self.y**2
            self.n=1
	elif mean_err is None:
            self.n=max(1, int(n))
            self.y=float(mean)*self.n
            self.y2=(float(mean)**2)*self.n
	elif n is None:
            self.n=2
            self.y=float(mean)*2
            self.y2=(float(mean_err)**2+float(mean)**2)*2
        else:
            self.n=int(n)
            self.y=float(mean)*self.n
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
        if self.n is None or self.n==0:
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
            return 0.0
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
            return 0.0
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
        sorted array of all dictionary entries without Accumulated Value objects with n==0
        """
        keys=numpy.array(filter(lambda k: not (isinstance(self.data[k], AccumulatedValue) and self.data[k].n==0), self.data.keys()),
	                 dtype="Float64")
        keys.sort()
        return keys

    def get_ydata(self):
        return self.get_xydata()[1]

    def get_xydata(self):
        k=self.get_xdata()
        v=numpy.array(map(lambda key: self.data[key].mean(), k), dtype="Float64")
        return [k,v]

    def get_errorplotdata(self):
        k=self.get_xdata()
        v=numpy.array(map(lambda key: self.data[key].mean(), k), dtype="Float64")
        e=numpy.array(map(lambda key: self.data[key].mean_error(), k), dtype="Float64")
        return [k,v,e]

    def uses_statistics(self):
        """
        drawable interface method, returns True
        """
        return True

    def write_to_csv(self,destination=sys.stdout, delimiter=" "):
        """
        writes the data to a file or to sys.stdout
        destination can be a file or a filename
        suitable for further processing
        """
        # write sorted
        the_destination=destination
        if type(destination) in types.StringTypes:
            the_destination=file(destination, "w")

        the_destination.write("# quantity:"+str(self.quantity_name)+"\n")
        the_destination.write("# x y ysigma n\n")
        for x in self.get_xdata():
            y=self.data[x]
            if type(y) in [types.FloatType, types.IntType, types.LongType]:
                the_destination.write("%e%s%e%s0%s1\n"%(x, delimiter, y, delimiter, delimiter))
            else:
                the_destination.write("%e%s%e%s%e%s%d\n"%(x,
                                                          delimiter,
                                                          y.mean(),
                                                          delimiter,
                                                          y.mean_error(),
                                                          delimiter,
                                                          y.n))
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
