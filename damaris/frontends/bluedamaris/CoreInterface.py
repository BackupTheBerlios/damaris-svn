import Configuration
import os
import os.path
import sys
import time
import re

try:
    import signal
except ImportException:
    pass

try:
    import _winreg
except ImportException:
    pass

__doc__ = """
This class handles the backend driver
"""


class CoreInterface:

    core_executable=None
    core_dir=None
    
    def __init__(self, config_object = None):
        if config_object is None:
            raise AssertionError("expecting Configuration object")
        
        self.config = config_object.get_my_config(self)

        self.core_executable=self.config["path"]
        self.core_dir=self.config["dir"]

        if not os.path.isfile(self.core_executable):
            raise AssertionError("could not find backend %s "%self.core_executable)
        if not os.path.isdir(self.core_dir):
            raise AssertionError("could not find backend's directory %s "%self.core_dir)
        if not os.access(self.core_executable,os.X_OK):
            raise AssertionError("insufficient rights for backend %s execution"%self.core_executable)

    def start_core(self):

        # take care of older logfiles
        self.core_output_filename=os.path.join(self.core_dir,"logdata")
        if os.path.isfile(self.core_output_filename):
            i=0
            max_logs=100
            while i<max_logs-1 and os.path.isfile(self.core_output_filename+".%02d"%i):
                i+=1
            for j in xrange(i):
                os.rename(self.core_output_filename+".%02d"%(i-j-1),self.core_output_filename+".%02d"%(i-j))
            os.rename(self.core_output_filename, self.core_output_filename+".00")
        # create logfile
        file(self.core_output_filename,"w")
        
        print "starting core %s ..."%self.core_executable,
        if sys.platform[:5]=="linux":
            self.core_input=os.popen(self.core_executable+" --spool "+self.core_dir+" >"+self.core_output_filename+" 2>&1","w")
        if sys.platform=="win32":
            cygwin_root_key=_winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Cygnus Solutions\\Cygwin\\mounts v2\\/")
            cygwin_path=_winreg.QueryValueEx(cygwin_root_key,"native")[0]
            os.environ["PATH"]+=";"+os.path.join(cygwin_path,"bin")+";"+os.path.join(cygwin_path,"lib")
            self.core_input=os.popen(self.core_executable+" --spool "+self.core_dir+" >"+self.core_output_filename+" 2>&1","w")

        # look out for state file
        timeout=1
        # to do: how should I know core's state name????!!!!!
        self.statefilename=os.path.join(self.core_dir,"dummy core.state")
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
        print "done (pid=%d)"%self.core_pid

    def get_messages(self):
        # return pending messages
        if self.core_output.tell()==os.path.getsize(self.core_output_filename):
            return None
        return self.core_output.read()

    def start_queue(self):
        self.send_signal("SIGUSR1")

    def stop_queue(self):
        self.send_signal("SIGQUIT")

    def abort(self):
        # abort execution
        self.send_signal("SIGTERM")

    def send_signal(self, signal):
        if sys.platform[:5]=="linux":
            os.kill(self.core_pid,signal.__dict__(signal))
        if sys.platform[:7]=="win32":
            # reg_handle=_winreg.ConnectRegistry(None,_winreg.HKEY_LOCAL_MACHINE)
            cygwin_root_key=_winreg.OpenKey(_winreg.HKEY_LOCAL_MACHINE, "SOFTWARE\\Cygnus Solutions\\Cygwin\\mounts v2\\/")
            cygwin_path=_winreg.QueryValueEx(cygwin_root_key,"native")[0]
            kill_command=os.path.join(cygwin_path,"bin","kill.exe")
            os.popen("%s -%s %d"%(kill_command,signal,self.core_pid))
                    
    def __del__(self):
        # stop core and wait for it
        try:
            self.abort()
        except OSError:
            pass
        
if __name__=="__main__":
    # for test purposes only
    conf=Configuration.Configuration(".")
    ci=CoreInterface(conf)
    ci.start_core()
    
    for i in xrange(100):
        m=ci.get_messages()
        if m is not None:
            print m,
        time.sleep(0.1)

    ci.abort()
    time.sleep(1)
