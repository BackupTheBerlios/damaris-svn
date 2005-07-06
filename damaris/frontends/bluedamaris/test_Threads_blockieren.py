from Experiment import *
import random
import threading
import time

class DV(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.y = None

    def run(self):
        while 1:
            time.sleep(5)
            if self.y is None: self.y = 2
            self.y = self.y**2
            if self.y > 2**64: self.y = 2


    def getY(self):
        while self.y is None:
            time.sleep(0.2)
        return self.y


dv = DV()
dv.start()

while 1:
    print "Hole y..."
    print dv.getY()
    print "Starte erneut, hole y"
    #dv.start()
    print dv.getY()
    time.sleep(1)

dv.join()
