# -*- coding: iso-8859-1 -*-

from Resultable import Resultable
from Drawable import Drawable

import numarray
from types import *

#############################################################################
#                                                                           #
# Name: Class Temp_Result                                                   #
#                                                                           #
# Purpose: Specialised class of Resultable and Drawable                     #
#          Contains recorded temperature data                               #
#                                                                           #
#############################################################################

class Temp_Result(Resultable, Drawable):
    def __init__(self, x = None, y = None, desc = None, job_id = None, job_date = None):
        Resultable.__init__(self)
        Drawable.__init__(self)
     

        if (x is None) and (y is None) and (desc is None) and (job_id is None) and (job_date is None):
            pass

        elif (x is not None) and (y is not None) and (desc is not None) and (job_id is not None) and (job_date is not None):
            pass

        else:
            raise ValueError("Wrong usage of __init__!")


    # Überladen von Operatoren und Built-Ins -------------------------------------------------------

    # / Überladen von Operatoren und Built-Ins -----------------------------------------------------
