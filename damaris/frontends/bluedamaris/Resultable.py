# -*- coding: iso-8859-1 -*-

#############################################################################
#                                                                           #
# Name: Class Resultable                                                    #
#                                                                           #
# Purpose: Base class of everything what could be a core-result             #
#                                                                           #
#############################################################################

class Resultable:
    def __init__(self):

        self.job_id = None
        self.job_date = None

        self.description = { }


    def get_job_id(self):
        "Returns the job-id of this result"
        return self.job_id


    def set_job_id(self, _id):
        "Sets the job-id of this result"
        self.job_id = _id


    def get_job_date(self):
        "Gets the date of this result"
        return self.job_date


    def set_job_date(self, date):
        "Sets the date of this result"
        self.job_date = date


    def get_description_dictionary(self):
        "Returns a reference to the description (Dictionary)"
        return self.description


    def set_description_dictionary(self, dictionary):
        "Sets the entire description"
        self.description = dictionary


    def get_description(self, key):
        "Returns the description value for a given key"
        if self.description.has_key(key):
            return self.description[key]

        else:
            print "Warning Resultable: No value for key \"%s\". Returned None" % str(key)
            return None


    def set_description(self, key, value):
        "Adds a attribute to the description"
        if self.description.has_key(key):
            print "Warning: Result key \"%s\" will be overwritten with \"%s\"." % (str(key), str(value))

        self.description[key] = value
