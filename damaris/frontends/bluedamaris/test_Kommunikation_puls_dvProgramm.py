import Experiment
from DamarisGUI import *
from ResultReader import *
#from Result import *


import glob
import os
import time


def pulsProgramm():

    tau = 1e-3
    pi = 10e-6

    while tau <= 2e-3:

        for accu in range(20):
            exp = Experiment.Experiment()
            exp.setDescription("tau", tau)

            exp.rf_pulse(0, pi/2)
            exp.wait(tau)

            exp.loop_start(100)

            exp.rf_pulse(0, pi)
            exp.wait(tau + 1e-6)

            exp.loop_end()

            exp.record(2048, 1e6)

            yield exp

        tau += 0.5e-3





##gui = DamarisGUI()
##gui.start()
##
experiment_liste = []

print "Erstelle Experimente..."

for experiment in pulsProgramm():
    experiment_liste.append(experiment)

xp0 = Experiment.Experiment()
print xp0.getJobID()

Experiment.reset()

xp1 = Experiment.Experiment()
print xp1.getJobID()

##
##print "Fertig!"
##
##datei_liste = glob.glob("D:\\cygwin\\tmp\\dummyspool\\job*")
##for datei in datei_liste:
##    os.remove(datei)
##
##del datei_liste
##
##print "Schreibe sie auf Festplatte..."
##
##for xp in experiment_liste:
##    datei = open("D:\\cygwin\\tmp\\dummyspool\\job.%09d" % xp.getJobID(), "w")
##    datei.write(xp.writeXMLString())
##    datei.close()
##    
##print "Fertig!"
##
##leser = ResultReader("D:\\cygwin\\tmp\\dummyspool\\")
##leser.start()
##
##tmp = leser.getNextResult()
##while tmp is None:
##    tmp = leser.getNextResult()
##
##
### DV - Programm ----------------------------------------
##
##while 1:
##    gui.drawResult(tmp)
##    tmp = leser.getNextResult()
##
##    while tmp is None:
##        tmp = leser.getNextResult()
##        time.sleep(0.2)
##
##
##leser.join()
##gui.join()
##
##
