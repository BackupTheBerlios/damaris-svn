
# data pool collects data from data handling script
# provides data to experiment script and display

import types
import tables
import UserDict
import threading
import ADC_Result
import Accumulation
import MeasurementResult

class DataPool(UserDict.DictMixin):
    """
    dictionary with sending change events
    """

    class Event:
        access=0
        updated_value=1
        new_key=2
        deleted_key=3
        destroy=4
        
        def __init__(self, what, subject="", origin=None):
            self.what=what
            self.subject=subject
            self.origin=origin

        def __repr__(self):
            return "<DataPool.Event origin=%s what=%d subject='%s'>"%(self.origin, self.what,self.subject)

        def copy(self):
            return DataPool.Event(self.what+0, self.subject+"", self.origin)

    def __init__(self):
        self.__mydict={}
        self.__dictlock=threading.Lock()
        self.__registered_listeners=[]

    def __getitem__(self, name):
        try:
            self.__dictlock.acquire()
            return self.__mydict[name]
        finally:
            self.__dictlock.release()

    def __setitem__(self, name, value):
        try:
            self.__dictlock.acquire()
            if name in self.__mydict:
                e=DataPool.Event(DataPool.Event.updated_value,name,self)
            else:
                e=DataPool.Event(DataPool.Event.new_key, name,self)
            self.__mydict[name]=value
        finally:
            self.__dictlock.release()
        self.__send_event(e)


    def __delitem__(self, name):
        try:
            self.__dictlock.acquire()
            del self.__mydict[name]
        finally:
            self.__dictlock.release()
        self.__send_event(DataPool.Event(DataPool.Event.deleted_key,name,self))

    def keys(self):
        try:
            self.__dictlock.acquire()
            return self.__mydict.keys()
        finally:
            self.__dictlock.release()

    def __send_event(self, _event):
        for l in self.__registered_listeners:
            l(_event.copy())

    def __del__(self):
        self.__send_event(DataPool.Event(DataPool.Event.destroy))
        self.__registered_listeners=None

    def write_hdf5(self,hdffile,where="/",name="data_pool", compress=None):
        if type(hdffile) is types.StringType:
            dump_file=tables.openFile(hdffile, mode="a")
        elif isinstance(hdffile,tables.File):
            dump_file=hdffile
        else:
            raise Exception("expecting hdffile or string")

        dump_group=dump_file.createGroup(where, name, "DAMARIS data pool")
        self.__dictlock.acquire()
        try:
            for (key,value) in self.__mydict.iteritems():
                if key[:2]=="__": continue
                # convert key to a valid name
                group_keyname="dict_"
                for character in key:
                    if ((character>='a' and character<='z') or
                        (character>='A' and character<='Z') or
                        (character>='0' and character<='9')):
                        group_keyname+=character
                    else:
                        group_keyname+="_"
                # avoid double names by adding number extension
                if group_keyname in dump_group:
                    extension_count=0
                    while group_keyname+"_%03d"%extension_count in dump_group:
                        extension_count+=1
                    group_keyname+="_%03d"%extension_count
                # now write data
                if isinstance(value, ADC_Result.ADC_Result) or \
                   isinstance(value, MeasurementResult.MeasurementResult) or \
                   isinstance(value, Accumulation.Accumulation):
                    try:
                        value.write_to_hdf(hdffile=dump_file,
                                           where=dump_group,
                                           name=group_keyname,
                                           title=key,
                                           compress=compress)
                    except Exception,e:
                        print "failed to write data_pool[%s]: %s"%(key,str(e))
                else:
                    print "don't know how to store data_pool[%s]"%key
        finally:
            dump_group=None
            self.__dictlock.release()
            dump_file.flush()
            if type(hdffile) is types.StringType:
                dump_file.close()
            dump_file=None

    def register_listener(self, listening_function):
        self.__registered_listeners.append(listening_function)

    def unregister_listener(self, listening_function):
        if listening_function in self.__registered_listeners:
            self.__registered_listeners.remove(listening_function)
