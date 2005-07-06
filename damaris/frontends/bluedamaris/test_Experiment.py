from Experiment import *
import random

saettigungsfolge = []

for i in range(4):
    saettigungsfolge.append(float(random.randint(1,10) * 100e-6))

px = 0
py = 90
mx = 180
my = 270

pi = 3.5e-6
tau = 1e-3


def saettigungsFolge(in_exp):
    for i in range(len(saettigungsfolge)):
        in_exp.rf_pulse(0, pi/2)
        in_exp.wait(saettigungsfolge[i])        

while tau <= 2e-3:
    
    for accu in range(2):
        for phase in [ px, py, mx, my ]:

            exp = Experiment()
            print "Job %d erstellt!" % exp.getJobID()

            exp.setFrequency(100e6, phase)

            saettigungsFolge(exp)        

            exp.rf_pulse(0, pi/2)
            exp.wait(tau)
            exp.rf_pulse(0, pi)
            exp.wait(tau + 1e-6)

            exp.record(4096, 1e6)

            exp.write()

    tau += 0.5e-3
