import os
import os.path
import glob
import Experiment

class ExperimentWriter:
    """
    writes experiments in propper way to spool directory
    """
    def __init__(self, spool, no=0, job_pattern="job.%09d"):
        self.spool=spool
        self.job_pattern=job_pattern
        self.no=no
        # test if spool exists
        if not os.path.isdir(spool):
            os.mkdir(spool)


    def send_next(self, job):
        """
        """
        job.job_id=self.no
        job_filename=os.path.join(self.spool,self.job_pattern%self.no)
        file(job_filename+".tmp","w").write(job.write_xml_string())
        os.rename(job_filename+".tmp", job_filename)
        self.no+=1


class ExperimentWriterWithCleanup(ExperimentWriter):
    """
    writes experiments and cleans up in front of queue
    """
    def __init__(self, spool, no=0, job_pattern="job.%09d"):
        ExperimentWriter.__init__(self, spool, no, job_pattern)
        self.delete_no_files(self.no)

    def send_next(self, job):
        self.delete_no_files(self.no+1)
        ExperimentWriter.send_next(self,job)

    def delete_no_files(self,no):
        """
        delete everything with this job number
        """
        filename=os.path.join(self.spool,(self.job_pattern%no))
        if os.path.isfile(filename): os.unlink(f)
        if os.path.isfile(filename+".result"): os.unlink(f+".result")
