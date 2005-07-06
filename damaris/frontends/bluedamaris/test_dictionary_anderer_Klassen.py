# -*- coding: iso-8859-1 -*-

from ResultReader import ResultReader
from Result import *
from Accumulation import *

import time

class DataHandling:
    def __init__(self, path):
        try:
            self.result_reader = ResultReader(path)
            self.result_reader.start()
            pass
        except:
            raise


    def get_variable(self, name, blocking = False):
        if blocking:
            if self.__dict__.has_key(name):
                return self.__dict__[name]
            else:
                while self.__dict__.has_key(name) is False:
                    time.sleep(0.1)

                return self.__dict__[name]

        else: # Not Blocking
            if self.__dict__.has_key(name):
                return self.__dict__[name]
            else:
                return None           


    def quit_data_handling(self):
        self.result_reader.quitResultReader()
        self.result_reader.join()


    def join(self):
        self.result_reader.join()


    def get_next_result(blocking = False):
        if blocking:
            while len(self.result_reader.getNumberOfResultsPending()) is 0:
                time.sleep(0.1)

            return self.result_reader.getNextResult()
        else:
            return self.result_reader.getNextResult()


#data_handler = DataHandling("D:\\")

akk1 = Accumulation(2, 10)
akk1 += 5

akk2 = Accumulation(2, 10)
akk2 += 7


akk1 += akk2

print akk1.__dict__


##def data_handling():
##    pass
##
##from Accumulation import *
##from Result import *
##
##acc = Accumulation(2, 10)
##acc += 2.0
##print acc.__dict__
##print acc.__class__
##
##res = Result(2, 10)
##if str(res.__class__) == "Result.Result":
##    print "Jipppieeee!"
