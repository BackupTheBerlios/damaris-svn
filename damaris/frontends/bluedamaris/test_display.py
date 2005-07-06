# -*- coding: cp1252 -*-
from damarisGUI import *
from DisplayPool import *
from ResultReader import *
from Result import *
import random
import time

gui = DamarisGUI()
gui.start()
print "Oberfläche erstellt!"

dp = DisplayPool()

gui.connectDisplayPool(dp)
print "DP connected!"

time.sleep(1)

leser = ResultReader("E:\\Eigene Dateien\\Projekt NMR\\Phase 4\\GUI-Design\\Results")
leser.start()
print "Leser gestartet!"

while 1:

    tmp = leser.getNextResult()

    while tmp is None:
        time.sleep(0.1)
        tmp = leser.getNextResult()
    
    print "Results waiting: " + str(leser.getNumberOfResults())

    dp.watch(tmp, "Zeitsignal")


leser.join()
gui.join()
