import os
import os.path
import subprocess
import sys
import time
import re
import glob
import ExperimentWriter
import ResultReader
import threading
import types
import signal

if sys.platform=="win32":
    import _winreg

__doc__ = """
This class handles the backend driver
"""

class BackendDriver(threading.Thread):
    
    def __init__(self, executable, spool, clear_jobs=False, clear_results=False):
        threading.Thread.__init__(self, name="Backend Driver")
        self.core_pid = None
        self.core_input = None
        self.core_output = None
        self.statefilename = None

        self.executable=str(executable)
        self.spool_dir=spool
        self.experiment_pattern="job.%09d"
        self.result_pattern=self.experiment_pattern+".result"

        if not os.path.isfile(self.executable):
            raise AssertionError("could not find backend %s "%self.executable)
        if not os.access(self.executable,os.X_OK):
            raise AssertionError("insufficient rights for backend %s execution"%self.executable)
        if not os.path.isdir(self.spool_dir):
            try:
                os.makedirs(os.path.abspath(self.spool_dir))
            except OSError,e:
                print e
                raise AssertionError("could not create backend's spool directory %s "%self.spool_dir)
        
        # remove stale state filenames
        if sys.platform.startswith("linux") or sys.platform.startswith("darwin"):
            old_state_files=glob.glob(os.path.join(self.spool_dir,"*.state"))
            statelinepattern=re.compile("<state name=\"([^\"]+)\" pid=\"([^\"]+)\" starttime=\"([^\"]+)\">")
            for statefilename in old_state_files:
                statefile=file(statefilename,"r")
                statelines=statefile.readlines()
                statefile.close
                del statefile
                core_pid=None
                for l in statelines:
                    matched=statelinepattern.match(l)
                    if matched:
                        core_pid=int(matched.group(2))
                        break
                if core_pid is not None:
                    if os.path.isdir("/proc/%d"%core_pid):
                        raise AssertionError("found backend with pid %d (state file %s) in same spool dir"%(core_pid,statefilename))
                    else:
                        print "removing stale backend state file", statefilename
                        os.remove(statefilename)
        else:
            print "todo: take care of existing backend state files"

        self.result_reader = ResultReader.BlockingResultReader(self.spool_dir,
                                                               no=0,
                                                               result_pattern=self.result_pattern,
                                                               clear_jobs=clear_jobs,
                                                               clear_results=clear_results)
        self.experiment_writer = ExperimentWriter.ExperimentWriterWithCleanup(self.spool_dir,
                                                                              no=0,
                                                                              job_pattern=self.experiment_pattern,
                                                                              inform_last_job=self.result_reader)

        self.quit_flag=threading.Event()
        self.raised_exception=None

    def run(self):
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
        self.core_output=file(self.core_output_filename,"w")

        # again look out for existing state files
        state_files=glob.glob(os.path.join(self.spool_dir,"*.state"))
        if state_files:
            self.raised_exception="found other state file(s) in spool directory: "+",".join(state_files)
            self.quit_flag.set()
            return

        # start backend
        if sys.platform.startswith("linux") or sys.platform.startswith("darwin"):
            self.core_input=subprocess.Popen([self.executable, "--spool", self.spool_dir],
                                             stdout=self.core_output,
                                             stderr=self.core_output)

        if sys.platform=="win32":
            cygwin_root_key=_winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Cygnus Solutions\\Cygwin\\mounts v2\\/")
            cygwin_path=_winreg.QueryValueEx(cygwin_root_key,"native")[0]
            os.environ["PATH"]+=";"+os.path.join(cygwin_path,"bin")+";"+os.path.join(cygwin_path,"lib")
            self.core_input=subprocess.Popen("\"" + self.executable + "\"" + " --spool "+self.spool_dir,
                                             stdout=self.core_output,
                                             stderr=self.core_output)

        # wait till state file shows up
        timeout=10
        # to do: how should I know core's state name????!!!!!
        self.statefilename=None
        state_files=glob.glob(os.path.join(self.spool_dir,"*.state"))
        while len(state_files)==0:
            if timeout<0 or self.core_input is None or self.core_input.poll() is not None or self.quit_flag.isSet():
                # look into core log file and include contents
                log_message=''
                self.core_input=None
                if os.path.isfile(self.core_output_filename):
                    # to do include log data
                    log_message='\n'+''.join(file(self.core_output_filename,"r").readlines()[:10])
                    if not log_message:
                        log_message=" no error message from core"
                self.core_output.close()
                self.raised_exception="no state file appeared or backend died away:"+log_message
                print self.raised_exception
                self.quit_flag.set()
                return
            time.sleep(0.05)
            timeout-=0.05
            state_files=glob.glob(os.path.join(self.spool_dir,"*.state"))

        # save the one
        if len(state_files)>1:
            print "did find more than one state file, taking first one!"
        self.statefilename=state_files[0]

        # read state file
        statefile=file(self.statefilename,"r")
        statelines=statefile.readlines()
	statefile=None
        statelinepattern=re.compile("<state name=\"([^\"]+)\" pid=\"([^\"]+)\" starttime=\"([^\"]+)\">")
        self.core_pid=-1
        for l in statelines:
            matched=statelinepattern.match(l)
            if matched:
                self.core_pid=int(matched.group(2))
                break

        # wait on flag and look after backend
        while not self.quit_flag.isSet() and self.is_busy():
            self.quit_flag.wait(0.1)

        if self.quit_flag.isSet():
            self.stop_queue()
            while self.is_busy():
                time.sleep(0.1)
            
        if not self.is_busy():
            if self.core_input is not None:
                backend_result=self.core_input.poll()
                wait_loop_counter=0
                while backend_result is None:
                    # waiting in tenth of a second
                    time.sleep(0.1)
                    wait_loop_counter+=1
                    backend_result=self.core_input.poll()
                    if backend_result is not None: break
                    if wait_loop_counter==10:
                        print "sending termination signal to backend process"
                        self.send_signal("SIGTERM")
                    elif wait_loop_counter==20:
                        print "sending kill signal to backend process"
                        self.send_signal("SIGKILL")
                    elif wait_loop_counter>30:
                        print "no longer waiting for backend shutdown"
                        break

                if backend_result is None:
                    print "backend dit not end properly, please stop it manually"
                elif backend_result>0:
                    print "backend returned ", backend_result
                elif backend_result<0:
                    sig_name=filter(lambda x: x.startswith("SIG") and \
                                    x[3]!="_" and \
                                    (type(signal.__dict__[x])is types.IntType) and \
                                    signal.__dict__[x]==-backend_result,
                                    dir(signal))
                    if sig_name:
                        print "backend was terminated by signal ",sig_name[0]
                    else:
                        print "backend was terminated by signal no",-backend_result
            self.core_input = None
            self.core_pid = None

            # the experiment handler should stop
            if self.experiment_writer is not None:
                # self.experiment_writer.
                self.experiment_writer=None

            # tell result reader, game is over...
            #self.result_reader.stop_no=self.experiment_writer.no
            if self.result_reader is not None:
                self.result_reader.poll_time=-1
                self.result_reader=None
             
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
        #self.core_pid=None
        #self.core_input=None

    def abort(self):
        # abort execution
        self.send_signal("SIGTERM")
        # assumes success
        #self.core_pid=None
        #self.core_input=None

    def send_signal(self, sig):
        if self.core_pid is None:
            print "BackendDriver.send_signal is called with core_pid=None"
            return
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
            print "could not send signal %s to core: %s"%(sig, str(e))
            

    def is_busy(self):
        "Checks for state file"
        return self.statefilename is not None and os.path.isfile(self.statefilename) and \
               self.core_input is not None and self.core_input.poll() is None
    
        #file_list = glob.glob(os.path.join(self.spool_dir, self.core_state_file))
        #if len(file_list) != 0:
        #    return True
        #else:
        #    return False


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
        self.core_input=None
        if self.core_output:
            self.core_output.close()
            self.core_output=None
