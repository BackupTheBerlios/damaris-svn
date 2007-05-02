import threading
import StringIO
import traceback
import sys
import time
from damaris.experiments.Experiment import Quit
from damaris.experiments import Experiment

class ExperimentHandling(threading.Thread):
    """
    runs the experiment script in sandbox
    """

    def __init__(self, script, exp_writer, data):
        threading.Thread.__init__(self, name="experiment handler")
        self.script=script
        self.writer=exp_writer
        self.data=data
        self.quit_flag = threading.Event()
        if self.data is not None:
            self.data["__recentexperiment"]=-1

    def synchronize(self, before=0, waitsteps=0.1):
        while (self.data["__recentexperiment"]>self.data["__recentresult"]+before) and not self.quit_flag.isSet():
            self.quit_flag.wait(waitsteps)

    def run(self):
        dataspace={}
	exp_classes = __import__('damaris.experiments', dataspace, dataspace, ['Experiment'])
	for name in dir(exp_classes):
            if name[:2]=="__" and name[-2:]=="__": continue
            dataspace[name]=exp_classes.__dict__[name]
        del exp_classes

        dataspace["data"]=self.data
        dataspace["synchronize"]=self.synchronize
        self.raised_exception = None
        self.location = None
        exp_iterator=None
        try:
            exec self.script in dataspace
        except Exception, e:
            self.raised_exception=e
            self.location=traceback.extract_tb(sys.exc_info()[2])[-1][1:3]
            traceback_file=StringIO.StringIO()
            traceback.print_tb(sys.exc_info()[2], None, traceback_file)
            self.traceback=traceback_file.getvalue()
            traceback_file=None
            return
        if "experiment" in dataspace:
            try:
                exp_iterator=dataspace["experiment"]()
            except Exception, e:
                self.raised_exception=e
                self.location=traceback.extract_tb(sys.exc_info()[2])[-1][1:3]
                traceback_file=StringIO.StringIO()
                traceback.print_tb(sys.exc_info()[2], None, traceback_file)
                self.traceback=traceback_file.getvalue()
                traceback_file=None
                return
        while exp_iterator is not None and not self.quit_flag.isSet():
            # get next experiment from script 
            try:
                job=exp_iterator.next()
            except StopIteration:
                break
            except Exception, e:
                self.raised_exception=e
                self.location=traceback.extract_tb(sys.exc_info()[2])[-1][1:3]
                traceback_file=StringIO.StringIO()
                traceback.print_tb(sys.exc_info()[2], None, traceback_file)
                self.traceback=traceback_file.getvalue()
                traceback_file=None
                break
            # send it
            self.writer.send_next(job)
            # write a note
            if isinstance(job, Experiment):
                if self.data is not None:
                    self.data["__recentexperiment"]=job.job_id+0
                    # wait a while
                    if "__resultsinadvance" in self.data and self.data["__resultsinadvance"]+100<job.job_id:
                        self.quit_flag.wait(0.05)
                if self.quit_flag.isSet():
                    data_sapce=None
                    exp_itterator=None
                    return
        job=Quit()
        self.writer.send_next(job)
        # do not count quit job (is this a good idea?)
        dataspace=None
        self.exp_iterator=None
        self.writer=None
