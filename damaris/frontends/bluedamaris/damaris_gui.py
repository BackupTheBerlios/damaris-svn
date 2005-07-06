#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-


import pygtk
pygtk.require("2.0")
import gtk
import threading
import gobject

import gtk.glade

import numarray

# switch comments for gtk over gtkagg
#from matplotlib.backends.backend_gtk import FigureCanvasGTK as FigureCanvas
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas

#import pylab

# Toolbar
from matplotlib.backends.backend_gtk import NavigationToolbar2GTK as NavigationToolbar

from matplotlib.axes import Subplot
from matplotlib.figure import Figure


class DamarisGUI(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        gtk.gdk.threads_init()


    def run(self):
        self.xml_gui = gtk.glade.XML("damaris.glade")

        self.damaris_gui_Init()
        self.graph_options_window_init()
 
        gtk.gdk.threads_enter()
        gtk.main()
        gtk.gdk.threads_leave()

        
    # Inits der einzelnen Fenster ##################################################################

    def damaris_gui_Init(self):

        self.main_window = self.xml_gui.get_widget("main_window")

        # Alle Signale verbinden -------------------------------------------------------------------

        self.xml_gui.signal_connect("main_window_close", lambda wid, data = None: gtk.main_quit())
        self.xml_gui.signal_connect("on_graphoptionen_activate", self.main_onGraphoptionsChanged)


        # Sonstige inits ---------------------------------------------------------------------------

        self.statusbar_label = self.xml_gui.get_widget("statusbar_label")


        # Matplot hinzufügen (Display VBox, 1. Zeile) ----------------------------------------------

        # Neue Abbildung erstellen
        self.matplot_figure = Figure()

        # Standartplot erstellen (1 Reihe, 1 Zeile, Beginnend beim 1.) und der Abbildung
        self.matplot_axes = self.matplot_figure.add_subplot(111)

        # Achsen beschriften & Gitternetzlinien sichtbar machen
        self.matplot_axes.set_xlabel("Time (s)")
        self.matplot_axes.set_ylabel("Samples (14-Bit)")
        self.matplot_axes.grid(True)

        # Ersten Plot erstellen und Referenz des ersten Eintrags im zurückgegebenen Tupel speichern
        self.graphen = self.matplot_axes.plot([1,2], [1,8192], "b-", [1,2], [8192,1], "r-")

        self.matplot_axes.set_ylim([-8192, 8192])

        # Lineare y-Skalierung
        self.matplot_axes.set_yscale("linear")

        # Matplot in einen GTK-Rahmen stopfen
        self.matplot_canvas = FigureCanvas(self.matplot_figure)

        self.display_vbox = self.xml_gui.get_widget("display_vbox")
        self.display_vbox.pack_start(self.matplot_canvas, True, True)
        self.matplot_canvas.show()


        # Matplot Toolbar hinzufügen (Display VBox, 2. Zeile) 

        self.matplot_toolbar = NavigationToolbar(self.matplot_canvas, self.main_window)

        self.display_toolbar_table = self.xml_gui.get_widget("display_toolbar_table")
        
        self.display_toolbar_table.attach(self.matplot_toolbar, 0, 1, 0, 1, gtk.FILL | gtk.EXPAND, 0, 0, 0)
        self.matplot_toolbar.show()

        # /Mathplot --------------------------------------------------------------------------------


    def graph_options_window_init(self):

        # Widgets holen

        self.gow = self.xml_gui.get_widget("graph_options_window")

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

        self.xml_gui.signal_connect("on_gow_number_of_graphs_spinbutton_value_changed", self.gow_onValueChanged)


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


    def gow_onValueChanged(self, widget, Data = None):

        for graph_frame in self.graph_frames:
            graph_frame.set_sensitive(False)
        
        for i in range(0, int(self.graphs_spinbutton.get_value())):
            self.graph_frames[i].set_sensitive(True)
            
        return True

    # /Inits der einzelnen Fenster #################################################################        


    # Interne Funktionen ###########################################################################

    def drawResult(self, in_result):

        def idle_func():
            gtk.gdk.threads_enter()

            try:
                self.graphen[0].set_data(in_result.getXValues(), in_result.getChannel(0))
                self.graphen[1].set_data(in_result.getXValues(), in_result.getChannel(1))

                self.matplot_axes.set_xlim(0, max(in_result.getXValues()))

                self.matplot_canvas.queue_resize()

                while gtk.gdk.events_pending():
                    gtk.main_iteration_do()

                return False

            finally:
                gtk.gdk.threads_leave()

        gobject.idle_add(idle_func)        
            


    # Callbacks ####################################################################################

    def main_onGraphoptionsChanged(self, widget, Data = None):
        if self.xml_gui.get_widget("menu_anzeige_graphoptionen").get_active():
            self.gow.show()
        else:
            self.gow.hide()


##    def draw_loop(self):
##        gtk.gdk.threads_enter()
##        try:
##            if self.display_pool.getChannels() is None: return False
##
##            key_list = self.display_pool.getChannels()
##
##            for key in key_list:
##                if key in self.display_pool_counter:
##                    self.drawResult(self.display_pool.getChannel(key))
##
##            return True           
##            
##        finally: gtk.gdk.threads_leave()

        
        
    # Schnittstellen nach Außen ####################################################################

    def connectDisplayPool(self, in_pool):
        self.display_pool = in_pool
        self.display_pool_counter = { }

        print self.display_pool



