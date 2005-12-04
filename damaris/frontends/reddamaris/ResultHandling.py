import threading
import sys
import os
import os.path
import traceback
import Resultable

class ResultHandling(threading.Thread):
    """
    runs the result script in sandbox
    """

    def __init__(self, script_data, result_iterator, data_pool):
        threading.Thread.__init__(self,name="result handler")
        self.script=script_data
        self.results=result_iterator
        self.data_space=data_pool

    def run(self):
        # execute it
        self.quit_flag=threading.Event()
        dataspace={}
        dataspace["results"]=self
        dataspace["data"]=self.data_space
        self.raised_exception=None
        self.location = None
        try:
            exec self.script in dataspace
        except Exception, e:
            self.raised_exception=e
            self.location=traceback.extract_tb(sys.exc_info()[2])[-1][1:3]
            return
        if not "result" in dataspace: return
        try:
            dataspace["result"]()
        except Exception, e:
            self.raised_exception=e
            self.location=traceback.extract_tb(sys.exc_info()[2])[-1][1:3]

    def __iter__(self):
        if self.quit_flag.isSet(): return
        for i in self.results:
            if self.quit_flag.isSet(): return
            if isinstance(i, Resultable.Resultable):
                self.data_space["__recentresult"]=i.job_id+0
            yield i
            if self.quit_flag.isSet(): return

    def stop(self):
        self.quit_flag.set()