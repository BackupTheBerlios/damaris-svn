#!/bin/bash
# -*- coding: iso-8859-1 -*-

#################################################
#                                               #
# Purpose: The place where everything starts!   #
#                                               #
#################################################

import sys
sys.argv.append("--numarray")

from JobWriter import *
from DataHandling import *
from DamarisGUI import *
from CoreInterface import *
from Configuration import *


# Loading configuration (no arguments -> current working directory)
cfg = Configuration()

experiment_script_filename = ""
datahandling_script_filename = ""

for cmd in sys.argv:
    if cmd.find("--experiment-script=") != -1:
        experiment_script_filename = cmd[20:len(cmd)]
        cfg.add_config("gui", "experiment_script", experiment_script_filename)
        
    elif cmd.find("--datahandling-script=") != -1:
        datahandling_script_filename = cmd[22:len(cmd)]
        cfg.add_config("gui", "datahandling_script", datahandling_script_filename)


# creating instances of all components
jw = JobWriter(config_object = cfg)
dh = DataHandling(config_object = cfg)
core= CoreInterface(cfg)
gui = DamarisGUI(config_object = cfg)

# connecting them so they can make use of each other
print "Connecting components..."
dh.connect_job_writer(jw)
jw.connect_data_handler(dh)
dh.connect_gui(gui)
jw.connect_gui(gui)
gui.connect_job_writer(jw)
gui.connect_data_handler(dh)
gui.connect_core(core)

# starting all threads
gui.start()
jw.start()
dh.start()

gui.join()
