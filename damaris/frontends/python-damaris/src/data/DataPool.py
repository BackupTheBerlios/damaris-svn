# data pool collects data from data handling script
# provides data to experiment script and display

import sys
import types
import tables
import UserDict
import threading
import traceback
import StringIO
import ADC_Result
import Accumulation
import MeasurementResult

class DataPool(UserDict.DictMixin):
    """
    dictionary with sending change events
    """

    # supports tranlation from dictionary keys to pytables hdf node names
    # taken from: Python Ref Manual Section 2.3: Identifiers and keywords
    # things are always prefixed by "dir_" or "dict_"
    translation_table=""
    for i in xrange(256):
        c=chr(i)
        if (c>="a" and c<="z") or \
           (c>="A" and c<="Z") or \
           (c>="0" and c<="9"):
            translation_table+=c
        else:
            translation_table+="_"

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

    def write_hdf5(self,hdffile,where="/",name="data_pool", complib=None, complevel=None):
        if type(hdffile) is types.StringType:
            dump_file=tables.openFile(hdffile, mode="a")
        elif isinstance(hdffile,tables.File):
            dump_file=hdffile
        else:
            raise Exception("expecting hdffile or string")

        dump_group=dump_file.createGroup(where, name, "DAMARIS data pool")
        self.__dictlock.acquire()
        dict_keys=self.__mydict.keys()
        self.__dictlock.release()
        try:
            for key in dict_keys:
                if key[:2]=="__": continue
                dump_dir=dump_group
                # walk along the given path and create groups if necessary
                namelist = key.split("/")
                for part in namelist[:-1]:
                    dir_part="dir_"+str(part).translate(DataPool.translation_table)
                    if not dir_part in dump_dir:
                        dump_dir=dump_file.createGroup(dump_dir,name=dir_part,title=part)
                    else:
                        if dump_dir._v_children[dir_part]._v_title==part:
                            dump_dir=dump_dir._v_children[dir_part]
                        else:
                            extension_count=0
                            while dir_part+"_%03d"%extension_count in dump_dir:
                                extension_count+=1
                            dump_dir=dump_file.createGroup(dump_dir,
                                                           name=dir_part+"_%03d"%extension_count,
                                                           title=part)
                
                # convert last part of key to a valid name
                group_keyname="dict_"+str(namelist[-1]).translate(DataPool.translation_table)
                # avoid double names by adding number extension
                if group_keyname in dump_dir:
                    extension_count=0
                    while group_keyname+"_%03d"%extension_count in dump_dir:
                        extension_count+=1
                    group_keyname+="_%03d"%extension_count
                self.__dictlock.acquire()
                if key not in self.__mydict:
                    # outdated ...
                    self.__dictlock.release()
                    continue
                value=self.__mydict[key]
                self.__dictlock.release()
                # now write data, assuming, the object is constant during write operation
                if "write_to_hdf" in dir(value):
                    try:
                        value.write_to_hdf(hdffile=dump_file,
                                           where=dump_dir,
                                           name=group_keyname,
                                           title=key,
                                           complib=complib,
                                           complevel=complevel)
                    except Exception,e:
                        print "failed to write data_pool[\"%s\"]: %s"%(key,str(e))
	                traceback_file=StringIO.StringIO()
			traceback.print_tb(sys.exc_info()[2], None, traceback_file)
			print "detailed traceback: %s\n"%str(e)+traceback_file.getvalue()
	                traceback_file=None
                else:
                    print "don't know how to store data_pool[\"%s\"]"%key
                value=None

        finally:
            dump_group=None
            if type(hdffile) is types.StringType:
                dump_file.close()
            dump_file=None

    def register_listener(self, listening_function):
        self.__registered_listeners.append(listening_function)

    def unregister_listener(self, listening_function):
        if listening_function in self.__registered_listeners:
            self.__registered_listeners.remove(listening_function)
