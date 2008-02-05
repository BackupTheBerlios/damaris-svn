# -*- coding: iso-8859-1 -*-

from Resultable import Resultable

#############################################################################
#                                                                           #
# Name: Class Error_Result                                                  #
#                                                                           #
# Purpose: Specialised class of Resultable                                  #
#          Contains occured error-messages from the core                    #
#                                                                           #
#############################################################################

class Config_Result(Resultable):
    def __init__(self, config = None, desc = None, job_id = None, job_date = None):
        Resultable.__init__(self)    

        if config is None: self.config = { }
        if desc is None: self.description = { }
        self.job_id = job_id
        self.job_date = job_date



    def get_config_dictionary(self):
        return self.config


    def set_config_dictionary(self, config):
        self.config = config


    def get_config(self, key):
        if self.config.has_key(key): return self.config[key]
        else: return None


    def set_config(self, key, value):
        if self.config.has_key(key):
            print "Warning Config_Result: Key \"%s\" will be overwritten with \"%s\"" % (key, value)

        self.config[key] = value    
        

    # Überladen von Operatoren und Built-Ins -------------------------------------------------------

    def __repr__(self):
        return str(self.config)


    def __str__(self):
        return str(self.config)


