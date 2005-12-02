import os
import os.path
import sys
import time
import re
import glob
import ExperimentWriter
import ResultReader
import threading


if sys.platform[:5]=="linux":
    import signal
if sys.platform=="win32":
    import _winreg

__doc__ = """
This class handles the backend driver
"""

class BackendDriver(threading.Thread):
    
    def __init__(self, executable, spool):
        threading.Thread.__init__(self)

        self.executable=executable
        self.spool_dir=spool
        self.core_state_file = "Mobile core.state"
        self.experiment_pattern="job.%09d"
        self.result_pattern=self.experiment_pattern+".result"
        self.experiment_writer = ExperimentWriter.ExperimentWriterWithCleanup(self.spool_dir, no=0, job_pattern=self.experiment_pattern)
        self.result_reader = ResultReader.BlockingResultReader(self.spool_dir, no=0, result_pattern=self.result_pattern)
        self.quit_flag=threading.Event()

        if not os.path.isfile(self.executable):
            raise AssertionError("could not find backend %s "%self.executable)
        if not os.access(self.executable,os.X_OK):
            raise AssertionError("insufficient rights for backend %s execution"%self.executable)
        if not os.path.isdir(self.spool_dir):
            raise AssertionError("could not find backend's directory %s "%self.spool_dir)
        
        self.core_pid = None

    def run(self):
        # Free remaining handle on file
        self.core_output = None
        # take care of older logfiles
        self.core_output_filename=os.path.join(self.spool_dir,"logdata")
        if os.path.isfile(self.core_output_filename):
            i=0
            max_logs=100
            while os.path.isfile(self.core_output_filename+".%02d"%i):
                i+=1
            while (i>=max_logs):
                i-=1
                os.remove(self.core_output_filename+".%02d"%i)
            for j in xrange(i):
                os.rename(self.core_output_filename+".%02d"%(i-j-1),self.core_output_filename+".%02d"%(i-j))
            os.rename(self.core_output_filename, self.core_output_filename+".%02d"%0)
        # create logfile
        file(self.core_output_filename,"w")
                
        if sys.platform[:5]=="linux":
            self.core_input=os.popen(self.executable+" --spool "+self.spool_dir+" >"+self.core_output_filename+" 2>&1","w")
        if sys.platform=="win32":
            cygwin_root_key=_winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Cygnus Solutions\\Cygwin\\mounts v2\\/")
            cygwin_path=_winreg.QueryValueEx(cygwin_root_key,"native")[0]
            os.environ["PATH"]+=";"+os.path.join(cygwin_path,"bin")+";"+os.path.join(cygwin_path,"lib")
            self.core_input=os.popen("\"" + self.executable + "\"" + " --spool "+self.spool_dir+" >"+self.core_output_filename+" 2>&1","w")

        # look out for state file
        timeout=10
        # to do: how should I know core's state name????!!!!!
        self.statefilename=os.path.join(self.spool_dir,self.core_state_file)
        while not os.path.isfile(self.statefilename) and timeout>0:
            time.sleep(0.1)
            timeout-=0.1
        if timeout<0:
            raise AssertionError("state file %s did not show up"%self.statefilename)

        statefile=file(self.statefilename,"r")
        statelines=statefile.readlines()
        statelinepattern=re.compile("<state name=\"([^\"]+)\" pid=\"([^\"]+)\" starttime=\"([^\"]+)\">")
        self.core_pid=-1
        for l in statelines:
            matched=statelinepattern.match(l)
            if matched:
                self.core_pid=int(matched.group(2))
                break

        # now open output file
        self.core_output=file(self.core_output_filename,"r")

        # wait on flag and look for backend
        while not self.quit_flag.isSet() and self.is_busy():
            self.quit_flag.wait(0.1)
        if not self.is_busy():
            self.core_pid = None
            # tell result reader, game is over...
            self.result_reader.stop_no=self.experiment_writer.no
            self.result_reader.poll_time=-1
            self.result_reader=None
            self.experiment_writer=None
            
    def clear_job(self,no):
        jobfilename=os.path.join(self.spool_dir,"job.%09d")
        resultfilename=os.path.join(self.spool_dir,"job.%09d.result")
        if os.path.isfile(jobfilename):
            os.remove(jobfilename)
        if os.path.isfile(resultfilename):
            os.remove(resultfilename)

    def get_messages(self):
        # return pending messages
        if self.core_output.tell()==os.path.getsize(self.core_output_filename):
            return None
        return self.core_output.read()

    def restart_queue(self):
        self.send_signal("SIGUSR1")

    def stop_queue(self):
        self.send_signal("SIGQUIT")
        # assumes success
        self.core_pid=None
        self.core_input=None

    def abort(self):
        # abort execution
        self.send_signal("SIGTERM")
        # assumes success
        self.core_pid=None
        self.core_input=None

    def send_signal(self, sig):
        if self.core_pid is None: return
        try:
            if sys.platform[:5]=="linux":
                os.kill(self.core_pid,signal.__dict__[sig])
            if sys.platform[:7]=="win32":
                # reg_handle=_winreg.ConnectRegistry(None,_winreg.HKEY_LOCAL_MACHINE)
                cygwin_root_key=_winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Cygnus Solutions\\Cygwin\\mounts v2\\/")
                cygwin_path=_winreg.QueryValueEx(cygwin_root_key,"native")[0]
                kill_command=os.path.join(cygwin_path,"bin","kill.exe")
                os.popen("%s -%s %d"%(kill_command,sig,self.core_pid))
        except OSError, e:
            print "could not send signal %d to core: %s"%(sig, str(e))
            

    def is_busy(self):
        "Checks for a *.state file in core_path"
        file_list = glob.glob(os.path.join(self.spool_dir, self.core_state_file))
        if len(file_list) != 0:
            return True
        else:
            return False


    def get_exp_writer(self):
        return self.experiment_writer
        
    def get_res_reader(self):
        return self.result_reader
        
    def __del__(self):
        # stop core and wait for it
        if self.core_pid is not None:
            try:
                self.abort()
            except OSError:
                pass
