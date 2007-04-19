#! /usr/bin/env python2.4

import time
import sys
import os
import os.path
import tables
import DataPool
import ResultReader
import ExperimentWriter
import BackendDriver
import ResultHandling
import ExperimentHandling

def some_listener(event):
    if event.subject=="__recentexperiment" or event.subject=="__recentresult":
        r=event.origin.get("__recentresult",-1)+1
        e=event.origin.get("__recentexperiment",-1)+1
        if e!=0:
            ratio=100.0*r/e
        else:
            ratio=100.0
        print "\r%d/%d (%.0f%%)"%(r,e,ratio),


class ScriptInterface:

    def __init__(self, exp_script=None, res_script=None, backend_executable=None, spool_dir="spool"):
        self.exp_script=exp_script
        self.res_script=res_script
        self.backend_executable=backend_executable
        self.spool_dir=os.path.abspath(spool_dir)
        self.exp_handling=self.res_handling=None

        self.exp_writer=self.res_reader=self.back_driver=None
        if self.backend_executable is not None:
            self.back_driver=BackendDriver.BackendDriver(self.backend_executable, spool_dir)
            if self.exp_script: self.exp_writer=self.back_driver.get_exp_writer()
            if self.res_script: self.res_reader=self.back_driver.get_res_reader()
        else:
            self.back_driver=None
            if self.exp_script: self.exp_writer=ExperimentWriter.ExperimentWriter(spool_dir)
            if self.res_script: self.res_reader=ResultReader.ResultReader(spool_dir)

        self.data=DataPool.DataPool()


    def runScripts(self):
        # get script engines
        if self.exp_script and self.exp_writer:
            self.exp_handling=ExperimentHandling.ExperimentHandling(self.exp_script, self.exp_writer, self.data)
        if self.res_script and self.res_reader:
            self.res_handling=ResultHandling.ResultHandling(self.res_script, self.res_reader, self.data)

        # start them
        if self.exp_handling: self.exp_handling.start()
        if self.back_driver is not None: self.back_driver.start()
        if self.res_handling: self.res_handling.start()

    def waitForScriptsEnding(self):
        # time of last dump
        dump_interval=600
        next_dump_time=time.time()+dump_interval
        # keyboard interrupts are handled in extra cleanup loop
        try:
            while filter(None,[self.exp_handling,self.res_handling,self.back_driver]):
                time.sleep(0.1)
                if time.time()>next_dump_time:
                    self.dump_data("data_pool.h5")
                    next_dump_time+=dump_interval

                if self.exp_handling is not None:
                    if not self.exp_handling.isAlive():
                        self.exp_handling.join()
                        if self.exp_handling.raised_exception:
                            print ": experiment script failed at line %d (function %s): %s"%(self.exp_handling.location[0],
                                                                                             self.exp_handling.location[1],
                                                                                             self.exp_handling.raised_exception)
                        else:
                            print ": experiment script finished"
                        self.exp_handling = None

                if self.res_handling is not None:
                    if not self.res_handling.isAlive():
                        self.res_handling.join()
                        if self.res_handling.raised_exception:
                            print ": result script failed at line %d (function %s): %s"%(self.res_handling.location[0],
                                                                                         self.res_handling.location[1],
                                                                                         self.res_handling.raised_exception)
                        else:
                            print ": result script finished"
                        self.res_handling = None

                if self.back_driver is not None:
                    if not self.back_driver.isAlive():
                        print ": backend finished"
                        self.back_driver=None

        except KeyboardInterrupt:
            still_running=filter(None,[self.exp_handling,self.res_handling,self.back_driver])
            for r in still_running:
                r.quit_flag.set()

            for r in still_running:
                r.join()

    def dump_data(self, filename):
        try:
            # write data from pool
            dump_file=tables.openFile(filename,mode="w",title="DAMARIS experiment data")
            self.data.write_hdf5(dump_file)
            # write scripts
            scriptgroup=dump_file.createGroup("/","scripts","Used Scripts")
            dump_file.createArray(scriptgroup,"experiment_script", self.exp_script)
            dump_file.createArray(scriptgroup,"result_script", self.res_script)
            dump_file.createArray(scriptgroup,"backend_executable", self.backend_executable)
            dump_file.createArray(scriptgroup,"spool_directory", self.spool_dir)
            dump_file.flush()
            dump_file.close()
            dump_file=None
            # todo
        except Exception,e:
            print "dump failed", e
       


if __name__=="__main__":

    if len(sys.argv)==1:
        print "%s: data_handling_script [spool directory]"%sys.argv[0]
        sys.exit(1)
    if len(sys.argv)==2:
        spool_dir=os.getcwd()
    else:
        spool_dir=sys.argv[2]
    
    scriptfile=open(sys.argv[1])
    script=scriptfile.read()
    scriptfile=None

    si=ScriptInterface(script, script,"/home/achim/damaris/backends/machines/Mobilecore.exe", spool_dir)

    si.data.register_listener(some_listener)

    si.runScripts()

    si.waitForScriptsEnding()

    si.dump_data("data_pool.h5")

    si=None
