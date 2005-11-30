
# data pool collects data from data handling script
# provides data to experiment script and display

import UserDict
import threading


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

    def dump_hdf5(self,filename):
        pass

    def register_listener(self, listening_function):
        self.__registered_listeners.append(listening_function)

    def unregister_listener(self, listening_function):
        if listening_function in self.__registered_listeners:
            self.__registered_listeners.remove(listening_function)
