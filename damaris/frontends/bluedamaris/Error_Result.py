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

class Error_Result(Resultable):
    def __init__(self, error_msg = None, desc = None, job_id = None, job_date = None):
        Resultable.__init__(self)    

        self.error_message = error_msg
        if desc is None: self.description = { }
        self.job_id = job_id
        self.job_date = job_date



    def get_error_message(self):
        return self.error_message


    def set_error_message(self, error_msg):
        self.error_message = err_msg
        

    # Überladen von Operatoren und Built-Ins -------------------------------------------------------

    def __repr__(self):
        tmp_string = "Core error-message: %s" % self.error_message

        return tmp_string


    def __str__(self):
        return self.error_message


