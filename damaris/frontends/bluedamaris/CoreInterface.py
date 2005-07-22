import Configuration
import os
import os.path

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

        print "starting core %s"%self.core_executable,
        # save current dir
        my_dir=os.getcwd()
        # change directory
        os.chdir(self.core_dir)
        # popen implementation
        (self.core_input,self.core_output)=os.popen4(self.core_executable+" --spool "+self.core_dir,"rw")
        os.chdir(my_dir)
        print "done"

    def get_messages(self):
        # return pending messages
        return self.core_output.read(1)

    def start_queue(self):
        # start core or reset it
        pass

    def stop_queue(self):
        # stop core but finish job
        pass

    def abort(self):
        # abort execution
        pass

    def __del__(self):
        # stop core and wait for it
        pass

        
if __name__=="__main__":
    # for test purposes only
    conf=Configuration.Configuration(".")
    ci=CoreInterface(conf)
    ci.start_core()
    
    while 1:
        print ci.get_messages()
