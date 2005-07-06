#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-


import pygtk
pygtk.require("2.0")
import gtk
import threading
import gobject

import gtk.glade

# switch comments for gtk over gtkagg
#from matplotlib.backends.backend_gtk import FigureCanvasGTK as FigureCanvas
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas

#import pylab

# Toolbar
from matplotlib.backends.backend_gtk import NavigationToolbar2GTK as NavigationToolbar

from matplotlib.axes import Subplot
from matplotlib.figure import Figure


class Display(threading.Thread):

    def __init__(self):
        threading.Thread.__init__(self)
        gtk.gdk.threads_init()


    def run(self):

        self.xml_gui = gtk.glade.XML("damaris.glade")

        # Alle Signale verbinden

        self.xml_gui.signal_connect("main_window_close", lambda wid, data = None: gtk.main_quit())

##        # 2. Zeile: Matplot -------------------------------------------
##
##        # Neue Abbildung erstellen
##        self.mathplot_figure = Figure()
##
##        # Standartplot erstellen (1 Reihe, 1 Zeile, Beginnend beim 1.) und der Abbildung
##        self.mathplot = self.mathplot_figure.add_subplot(111)
##
##        # Achsen beschriften & Gitternetzlinien sichtbar machen
##        self.mathplot.set_xlabel("x")
##        self.mathplot.set_ylabel("y(x)")
##        self.mathplot.grid(True)
##
##        # Ersten Plot erstellen und Referenz des ersten Eintrags im zurückgegebenen Tupel speichern
##        self.graphen = self.mathplot.plot([1,2], [1,2], "b-", [1,2], [2,1], "r-")
##
##        # Lineare y-Skalierung
##        self.mathplot.set_yscale("linear")
##
##        # Matplot in einen GTK-Rahmen stopfen
##        self.mathplot_canvas = FigureCanvas(self.mathplot_figure)
##
##        self.main_vbox.pack_start(self.mathplot_canvas, True, True, 4)
##
##
##        # 3. Zeile: Toolbar und Navigation ----------------------------
##
##        self.third_row = gtk.HBox()
##
##        self.mathplot_toolbar = NavigationToolbar(self.mathplot_canvas, self.main_window)
##
##        self.history_label = gtk.Label("Graph history: ")
##        self.history_label.set_alignment(0, 0.5)
##        
##        self.history_button_rewind = gtk.Button(" < ")
##        self.history_position_label = gtk.Label("x / x")
##        self.history_button_forward = gtk.Button(" > ")
##
##        self.third_row.pack_start(self.mathplot_toolbar, True, True)
##        self.third_row.pack_start(self.history_label, False, False, 4)
##        self.third_row.pack_start(self.history_button_rewind, False, False, 4)
##        self.third_row.pack_start(self.history_position_label, False, False, 4)
##        self.third_row.pack_start(self.history_button_forward, False, False, 4)
##
##        self.main_vbox.pack_start(self.third_row, False, False)
##
##        # /Mathplot ----------------------------------------------
##
##        self.main_window.add(self.main_vbox)
##
##        self.main_window.show_all()

        gtk.gdk.threads_enter()
        gtk.main()
        gtk.gdk.threads_leave()


    def draw(self, in_result):

        def idle_func():
            try:
                gtk.gdk.threads_enter()
                self.graphen[0].set_data(in_result.getXValues(), in_result.getChannel(0))
                self.graphen[1].set_data(in_result.getXValues(), in_result.getChannel(1))

                self.mathplot.set_xlim(min(in_result.getXValues()), max(in_result.getXValues()))
                self.mathplot.set_ylim(min(in_result.getChannel(0)), max(in_result.getChannel(0)))

                self.mathplot_canvas.queue_resize()
                                   
                return False
            finally:
                gtk.gdk.threads_leave()

        gobject.idle_add(idle_func)


    def watch(self, in_result, in_name):
               
        def idle_func():
            gtk.gdk.threads_enter()
            try:
                appendTextIfNeeded()
                self.draw(in_result)
                
                while gtk.gdk.events_pending():
                    gtk.main_iteration_do()
                    
                return False
            
            finally:
                gtk.gdk.threads_leave()
                
                    
        def appendTextIfNeeded():
            if not self.watch_combobox_text.has_key(in_name):
                self.watch_combobox.append_text(in_name)
                self.watch_combobox_text[in_name] = len(self.watch_combobox_text)

            else:
                print "Eintrag bereits vorhanden!"


        gobject.idle_add(idle_func)

##    def plotRealData(self, in_x, in_y):
##
##        gtk.gdk.threads_enter()
##
##        try:
##
##            if len(in_x) != len(in_y):
##                print "Error: Length of x-Data and y-Data must be the same!"
##                gtk.gdk.threads_leave()
##                return
##
##            self.graph[0].set_data(in_x, in_y)
##
##            self.mathplot.set_xlim(min(in_x), max(in_x))
##            self.mathplot.set_ylim(min(in_y), max(in_y))
##
##            self.mathplot_canvas.queue_resize()
##            gtk.gdk.threads_leave()
##
##        except:
##            gtk.main_quit()
##            gtk.gdk.threads_leave()
##            raise
##
##
##
##    def plotImgData(self, in_x, in_y):
##
##        gtk.gdk.threads_enter()
##
##        try:
##
##            if len(in_x) != len(in_y):
##                print "Error: Length of x-Data and y-Data must be the same!"
##                gtk.gdk.threads_leave()
##                return
##
##            self.graph[1].set_data(in_x, in_y)
##
##            self.mathplot.set_xlim(min(in_x), max(in_x))
##            self.mathplot.set_ylim(min(in_y), max(in_y))
##
##            self.mathplot_canvas.queue_resize()
##            gtk.gdk.threads_leave()
##
##        except:
##            gtk.main_quit()
##            gtk.gdk.threads_leave()
##            raise
##
##
##    def watchData(self, in_adc_result, in_name):
##
##        def idle_func():
##            gtk.gdk.threads_enter()
##
##            try:
##                self.channel_combobox.append(in_name)
##                return False
##            finally:
##                gtk.gdk.threads_leave()
##                
##        gobject.idle_add(idle_func)
