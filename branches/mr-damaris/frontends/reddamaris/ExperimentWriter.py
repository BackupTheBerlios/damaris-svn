import os
import os.path
import shutil
import Experiment

class ExperimentWriter:
    """
    writes experiments in propper way to spool directory
    """
    def __init__(self, spool, no=0, job_pattern="job.%09d", inform_last_job=None):
        self.spool=spool
        self.job_pattern=job_pattern
        self.no=no
        self.inform_last_job=inform_last_job
        # test if spool exists
        if not os.path.isdir(spool):
            os.mkdir(spool)

    def send_next(self, job):
        """
        """
        job.job_id=self.no
        job_filename=os.path.join(self.spool,self.job_pattern%self.no)
	f=file(job_filename+".tmp","w")
	f.write(job.write_xml_string())
	f.close() # explicit close under windows necessary (don't know why)
	f=None
	# this implementation tries to satisfiy msvc filehandle caching
	# os.rename(job_filename+".tmp", job_filename)
        shutil.copyfile(job_filename+".tmp", job_filename)
	try:
	    os.unlink(job_filename+".tmp")
	except OSError:
	    print "could not delete temporary file %s.tmp"%job_filename

        self.no+=1

    def __del__(self):
        print "del it"
        if self.inform_last_job is not None:
            self.inform_last_job.stop_no=self.no

class ExperimentWriterWithCleanup(ExperimentWriter):
    """
    writes experiments and cleans up in front of queue
    """
    def __init__(self, spool, no=0, job_pattern="job.%09d", inform_last_job=None):
        ExperimentWriter.__init__(self, spool, no, job_pattern, inform_last_job=inform_last_job)
        self.delete_no_files(self.no)

    def send_next(self, job):
        self.delete_no_files(self.no+1)
        ExperimentWriter.send_next(self,job)

    def delete_no_files(self,no):
        """
        delete everything with this job number
        """
        filename=os.path.join(self.spool,(self.job_pattern%no))
        if os.path.isfile(filename): os.unlink(filename)
        if os.path.isfile(filename+".tmp"): os.unlink(filename+".tmp")
        if os.path.isfile(filename+".result"): os.unlink(filename+".result")
