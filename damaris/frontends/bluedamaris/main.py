from JobWriter import *
from DataHandling import *
from DamarisGUI import *
from Configuration import *

import pygtk
import gtk

import time

cfg = Configuration()

jw = JobWriter(config_object = cfg)
dh = DataHandling(config_object = cfg)
gui = DamarisGUI(config_object = cfg)


print "Connecting components..."
dh.connect_job_writer(jw)
jw.connect_data_handler(dh)
dh.connect_gui(gui)
jw.connect_gui(gui)
gui.connect_job_writer(jw)
gui.connect_data_handler(dh)


gui.start()
jw.start()
dh.start()

time.sleep(2)

gui.join()
    

