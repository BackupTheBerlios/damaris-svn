# -*- coding: iso-8859-1 -*-

from Resultable import Resultable
from Drawable import Drawable

#############################################################################
#                                                                           #
# Name: Class Error_Result                                                  #
#                                                                           #
# Purpose: Specialised class of Resultable                                  #
#          Contains occured error-messages from the core                    #
#                                                                           #
#############################################################################

class Error_Result(Resultable, Drawable):
    def __init__(self, error_msg = None, desc = {}, job_id = None, job_date = None):
        Resultable.__init__(self)
        Drawable.__init__(self)

        if error_msg is not None:
            self.error_message = error_msg
            self.set_title("Error-Result: %s" % error_msg)
        else:
            self.error_message = error_msg
        self.description = desc
        self.job_id = job_id
        self.job_date = job_date



    def get_error_message(self):
        return self.error_message


    def set_error_message(self, error_msg):
        self.set_title("Error-Result: %s" % error_msg)
        self.error_message = error_msg


    # No statistics
    def uses_statistics(self):
        return False

    # Nothing to plot
    def get_ydata(self):
        return [0.0]

    # Nothing to plot
    def get_xdata(self):
        return [0.0]

    # Überladen von Operatoren und Built-Ins -------------------------------------------------------

    def __repr__(self):
        tmp_string = "Core error-message: %s" % self.error_message

        return tmp_string

    def __len__(self):
        return len(self.error_message)
    

    def __str__(self):
        return self.error_message


    # Preventing an error when adding something to an error-result (needed for plotting error-results)
    def __add__(self, other):
        return self


