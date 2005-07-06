#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-


import pygtk
pygtk.require("2.0")
import gtk

import gtk.glade


class DamarisGUI_GOW:

    def __init__(self, in_xml_gui):

        # Inits

        self.xml_gui = in_xml_gui

        # Widgets holen

        self.graphs_spinbutton = self.xml_gui.get_widget("gow_number_of_graphs_spinbutton")

        self.graph1_source_combobox = self.xml_gui.get_widget("gow_graph1_source_combobox")
        self.graph2_source_combobox = self.xml_gui.get_widget("gow_graph2_source_combobox")
        self.graph3_source_combobox = self.xml_gui.get_widget("gow_graph3_source_combobox")
        self.graph4_source_combobox = self.xml_gui.get_widget("gow_graph4_source_combobox")

        self.graph1_function_combobox = self.xml_gui.get_widget("gow_graph1_function_combobox")
        self.graph2_function_combobox = self.xml_gui.get_widget("gow_graph2_function_combobox")
        self.graph3_function_combobox = self.xml_gui.get_widget("gow_graph3_function_combobox")
        self.graph4_function_combobox = self.xml_gui.get_widget("gow_graph4_function_combobox")

        self.graph1_frame = self.xml_gui.get_widget("gow_graph1_frame")
        self.graph2_frame = self.xml_gui.get_widget("gow_graph2_frame")
        self.graph3_frame = self.xml_gui.get_widget("gow_graph3_frame")
        self.graph4_frame = self.xml_gui.get_widget("gow_graph4_frame")


        # Signale verbinden

        self.xml_gui.signal_connect("on_gow_number_of_graphs_spinbutton_value_changed", self.onValueChanged)


        # Inits
        
        self.graph1_frame.set_sensitive(True)
        self.graph2_frame.set_sensitive(False)
        self.graph3_frame.set_sensitive(False)
        self.graph4_frame.set_sensitive(False)

        self.graph_frames = [ self.graph1_frame, self.graph2_frame, self.graph3_frame, self.graph4_frame ]

        self.graph1_source_combobox.set_active(0)
        self.graph2_source_combobox.set_active(0)
        self.graph3_source_combobox.set_active(0)
        self.graph4_source_combobox.set_active(0)

        self.graph1_function_combobox.set_active(0)
        self.graph2_function_combobox.set_active(0)
        self.graph3_function_combobox.set_active(0)
        self.graph4_function_combobox.set_active(0)


    def onValueChanged(self, widget, Data = None):

        for graph_frame in self.graph_frames:
            graph_frame.set_sensitive(False)
        
        for i in range(0, int(self.graphs_spinbutton.get_value())):
            self.graph_frames[i].set_sensitive(True)
            
        return True
      

        
