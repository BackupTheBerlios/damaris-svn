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

    def delete_no_files(self,no):
        """
        delete everything with this job number
        """
        for f in glob.glob(os.path.join(self.spool,(self.job_pattern%no))+"*"):
            os.unlink(f)

    def send_next(self, job):
        """
        """
        job.job_id=self.no
        job_filename=os.path.join(self.spool,self.job_pattern%self.no)
        file(job_filename+".tmp","w").write(job.write_xml_string())
        os.rename(job_filename+".tmp", job_filename)
        self.no+=1
