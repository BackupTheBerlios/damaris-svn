# -*- coding: cp1252 -*-

def rf_pulse(time, phase = 0):
    print "Pulse mit %f Länge, %d Phase." % (time, phase)

def wait(time):
    print "Pause für %fs" %time

def record(frequency, samples):
    print "Nehme auf mit: %f, %d samples" % (frequency, samples)

def loop(iterations):
    print "Schleife mit %d Iterationen." %iterations

def endloop():
    print "Ende der Schleife"

pi = float(1e-6)

for accu in range(2):
    for phase in [0,90,180,270]:
        rf_pulse(pi/2,phase)
        wait(1e-3)
        record(1e6, 1024)
        if 1:
            loop(100)
            rf_pulse(pi/2)
            wait(1e-3)
            endloop()
        wait(1e-3)
        write_job(
       
