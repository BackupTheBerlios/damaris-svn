#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-


import pygtk
pygtk.require("2.0")
import gtk
import threading
import gobject

import gtk.glade
import pango

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

    def __init__(self, config_object = None):
        threading.Thread.__init__(self)
        gtk.gdk.threads_init()

        if config_object is not None:
            self.config = config_object.get_my_config(self)


    def run(self):
        self.xml_gui = gtk.glade.XML("damaris.glade")

        self.damaris_gui_init()

        gtk.gdk.threads_enter()
        gtk.main()
        gtk.gdk.threads_leave()

        
    # Inits der einzelnen Fenster ##################################################################

    def damaris_gui_init(self):

        self.main_window = self.xml_gui.get_widget("main_window")

        # Alle Signale verbinden -------------------------------------------------------------------

        self.xml_gui.signal_connect("main_window_close", self.quit_application)
        self.xml_gui.signal_connect("on_toolbar_run_button_clicked", self.start_experiment)
        self.xml_gui.signal_connect("on_toolbar_open_file_button_clicked", self.open_file)
        self.xml_gui.signal_connect("on_experiment_script_textview_event", self.experiment_script_textview_changed)
        self.xml_gui.signal_connect("on_data_handling_textview_event", self.data_handling_textview_changed)
        
        # Sonstige inits ---------------------------------------------------------------------------

        self.statusbar_label = self.xml_gui.get_widget("statusbar_label")
        self.display_x_scaling_combobox = self.xml_gui.get_widget("display_x_scaling_combobox")
        self.display_y_scaling_combobox = self.xml_gui.get_widget("display_y_scaling_combobox")

        self.display_x_scaling_combobox.set_active(0)
        self.display_y_scaling_combobox.set_active(0)

        self.experiment_script_textview = self.xml_gui.get_widget("experiment_script_textview")
        self.data_handling_textview = self.xml_gui.get_widget("data_handling_textview")

        self.experiment_script_textview.modify_font(pango.FontDescription("Courier 10"))
        self.data_handling_textview.modify_font(pango.FontDescription("Courier 10"))

        self.experiment_script_textbuffer = self.experiment_script_textview.get_buffer()
        self.experiment_script_textbuffer.set_text("def experiment_script(input):\n    ")

        self.data_handling_textbuffer = self.data_handling_textview.get_buffer()
        self.data_handling_textbuffer.set_text("def data_handling(input):\n    ")

        self.toolbar_stop_button = self.xml_gui.get_widget("toolbar_stop_button")
        self.toolbar_run_button = self.xml_gui.get_widget("toolbar_run_button")

        self.main_notebook = self.xml_gui.get_widget("main_notebook")

        # Matplot hinzufügen (Display_Table, 1. Zeile) ----------------------------------------------

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

        self.matplot_axes.set_ylim([-1280, 1280])

        # Lineare y-Skalierung
        self.matplot_axes.set_yscale("linear")

        # Matplot in einen GTK-Rahmen stopfen
        self.matplot_canvas = FigureCanvas(self.matplot_figure)

        self.display_table = self.xml_gui.get_widget("display_table")
        self.display_table.attach(self.matplot_canvas, 0, 5, 0, 1, gtk.EXPAND | gtk.FILL, gtk.EXPAND | gtk.FILL, 0, 0)
        self.matplot_canvas.show()


        # Matplot Toolbar hinzufügen (Display_Table, 2. Zeile) 

        self.matplot_toolbar = NavigationToolbar(self.matplot_canvas, self.main_window)
        
        self.display_table.attach(self.matplot_toolbar, 0, 1, 1, 2, gtk.FILL | gtk.EXPAND, 0, 0, 0)
        self.matplot_toolbar.show()

        # /Mathplot --------------------------------------------------------------------------------


    # /Inits der einzelnen Fenster #################################################################        


    # Interne Funktionen ###########################################################################

    
            


    # Callbacks ####################################################################################

    def quit_application(self, widget, Data = None):
        self.job_writer.quit_job_writer()
        self.data_handler.quit_data_handling()
        gtk.main_quit()


    def start_experiment(self, widget, Data = None):
        gtk.gdk.threads_enter()
 
        #self.toolbar_stop_button.set_sensitive(True)
        #self.toolbar_run_button.set_sensitive(False)

        gtk.gdk.threads_leave()

        self.job_writer.start_job_writing()
        self.data_handler.start_data_handling()

        gobject.timeout_add(100, self.sync_job_writer_data_handler)
        return True


    def sync_job_writer_data_handler(self):
        if self.job_writer.is_ready() and self.data_handler.is_ready():
            self.job_writer.wake_up()
            self.data_handler.wake_up()

            return False

        else:
            return True


    def open_file(self, widget, Data = None):

        main_notebook = self.main_notebook
        experiment_buffer = self.experiment_script_textview.get_buffer()
        data_buffer = self.data_handling_textview.get_buffer()

        def response(self, response_id, Data = None):
            if response_id == 1:
                file_name = dialog.get_filename()
                if file_name is None:
                    return
                
                script_file = file(file_name, "r")

                if main_notebook.get_current_page() == 0:
                    experiment_script_string = ""

                    for line in script_file:
                        experiment_script_string += line

                    script_file.close()

                    experiment_buffer.set_text(experiment_script_string)

                elif main_notebook.get_current_page() == 1:
                    data_handling_string = ""

                    for line in script_file:
                        data_handling_string += line

                    script_file.close()

                    data_buffer.set_text(data_handling_string)                    
                    
                
            else:
                return
        
        print "Page No. %d open" % self.main_notebook.get_current_page()
        if self.main_notebook.get_current_page() == 0:
            dialog = gtk.FileChooserDialog(title="Open Experiment Script...", parent=self.main_window, action=gtk.FILE_CHOOSER_ACTION_OPEN, buttons = ("Open", 1, "Cancel", 0))
        elif self.main_notebook.get_current_page() == 1:
            dialog = gtk.FileChooserDialog(title="Open Data Handling Script...", parent=self.main_window, action=gtk.FILE_CHOOSER_ACTION_OPEN, buttons = ("Open", 1, "Cancel", 0))

        dialog.set_select_multiple(False)
        dialog.connect("response", response)

        dialog.run()
        dialog.destroy()

        return True


    def experiment_script_textview_changed(self, event, Data = None):
        print "Experiment Script changed!"


    def data_handling_textview_changed(self, event, Data = None):
        print "Data Handling changed!"
    

    # Schnittstellen nach Außen ####################################################################

    def draw_result(self, in_result):

        def idle_func():
            gtk.gdk.threads_enter()

            try:
                self.graphen[0].set_data(in_result.get_xvalues(), in_result.get_channel(0))
                self.graphen[1].set_data(in_result.get_xvalues(), in_result.get_channel(1))

                self.matplot_axes.set_xlim(0, in_result.get_xmax())
                #self.matplot_axes.set_ylim(in_result.get_ymin(), in_result.get_ymax())

                self.matplot_canvas.queue_resize()

                return False

            finally:
                gtk.gdk.threads_leave()

        gobject.idle_add(idle_func)  


    def get_experiment_script(self):
        return self.experiment_script_textbuffer.get_text(self.experiment_script_textbuffer.get_start_iter(), self.experiment_script_textbuffer.get_end_iter())


    def get_data_handling_script(self):
        return self.data_handling_textbuffer.get_text(self.data_handling_textbuffer.get_start_iter(), self.data_handling_textbuffer.get_end_iter())


    def connect_data_handler(self, data_handler):
        self.data_handler = data_handler


    def connect_job_writer(self, job_writer):
        self.job_writer = job_writer


    def show_syntax_error_dialog(self, error_message):
        def idle_func():
            try:
                gtk.gdk.threads_enter()
                dialog = gtk.MessageDialog(parent = self.main_window,
                                           flags = gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                                           type = gtk.MESSAGE_ERROR,
                                           buttons = gtk.BUTTONS_OK,
                                           message_format = error_message)

                dialog.set_title("Syntax Error")
                dialog.set_position(gtk.WIN_POS_CENTER)
                                                          
                dialog.run()
                dialog.destroy()

                return False
                
            finally:
                gtk.gdk.threads_leave()

        gobject.idle_add(idle_func)
            

##    def flush(self):
##        gtk.gdk.threads_enter()
##        while gtk.gdk.events_pending():
##            gtk.main_iteration_do()
##        gtk.gdk.threads_leave()

        
