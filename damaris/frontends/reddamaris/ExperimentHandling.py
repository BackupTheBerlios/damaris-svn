import threading
import StringIO
import traceback
import sys
import Experiment

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

    def run(self):
        dataspace={}
        dataspace["data"]=self.data
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
        if exp_iterator is None or self.quit_flag.isSet(): return
        while not self.quit_flag.isSet():
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
                return
            # send it
            self.writer.send_next(job)
            # write a note
            if isinstance(job, Experiment.Experiment):
                self.data["__recentexperiment"]=job.job_id+0
                if self.quit_flag.isSet(): return
        job=Experiment.Quit()
        self.writer.send_next(job)
