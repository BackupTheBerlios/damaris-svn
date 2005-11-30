#! /usr/bin/env python2.4

import DataPool
import ResultReader
import ExperimentWriter
import BackendDriver
import ResultHandling
import ExperimentHandling
import sys
import os

def some_listener(event):
    if event.subject=="__recentexperiment" or event.subject=="__recentresult":
        r=event.origin.get("__recentresult",-1)+1
        e=event.origin.get("__recentexperiment",-1)+1
        if e!=0:
            ratio=100.0*r/e
        else:
            ratio=100.0
        print "\r%d/%d (%.0f%%)"%(r,e,ratio),

if __name__=="__main__":

    if len(sys.argv)==1:
        print "%s: data_handling_script [spool directory]"%sys.argv[0]
        sys.exit(1)
    if len(sys.argv)==2:
        spool_dir=os.getcwd()
    else:
        spool_dir=sys.argv[2]

    if True:
        bd=BackendDriver.BackendDriver("/home/achim/damaris/backends/machines/Mobilecore.exe", spool_dir)
        exp_writer=bd.get_exp_writer()
        res_reader=bd.get_res_reader()
    else:
        bd=None
        exp_writer=ExperimentWriter.ExperimentWriter(spool_dir)
        res_reader=ResultReader.ResultReader(spool_dir)
    
    scriptfile=open(sys.argv[1])
    script=scriptfile.read()
    scriptfile=None
    data=DataPool.DataPool()
    data.register_listener(some_listener)
    exp=ExperimentHandling.ExperimentHandling(script, exp_writer, data)
    res=ResultHandling.ResultHandling(script, res_reader, data)
    exp.start()
    res.start()
    if bd is not None: bd.start()

    while filter(None,[exp,res,bd]):
        if exp is not None:
            if exp.isAlive():
                exp.join(0.1)
            else:
                if exp.raised_exception:
                    print "\rexperiment script failed at line %d (function %s): %s"%(exp.location[0],exp.location[1],exp.raised_exception)
                else:
                    print "\rexperiment script finished"
                exp = None
        if res is not None:
            if res.isAlive():
                res.join(0.1)
            else:
                if res.raised_exception:
                    print "\rresult script failed at line %d (function %s): %s"%(res.location[0],res.location[1],res.raised_exception)
                else:
                    print "\rresult script finished"
                res = None

        if bd is not None:
            if bd.isAlive():
                bd.join(0.1)
            else:
                print "\rbackend finished"
                bd=None
