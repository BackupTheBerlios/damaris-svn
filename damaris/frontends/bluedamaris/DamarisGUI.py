# -*- coding: iso-8859-1 -*-

#####################################################################
#                                                                   #
# Purpose: GUI-Class, manages everything neither                    #
#          JobWriter nor DataHandling does (espacally GUI-Stuff)    #
#                                                                   #
#####################################################################

import pygtk
pygtk.require("2.0")
import gtk
import threading
import gobject

import gtk.glade
import pango

import numarray
import os
import os.path
import sys
import glob
import time
import datetime

import NiftyGuiElements
import MeasurementResult

# switch comments for gtk over gtkagg
#from matplotlib.backends.backend_gtk import FigureCanvasGTK as FigureCanvas
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas

#import pylab

# Toolbar
from matplotlib.backends.backend_gtk import NavigationToolbar2GTK as NavigationToolbar

from matplotlib.axes import Subplot
from matplotlib.figure import Figure

import pylab


class DamarisGUI(threading.Thread):

    def __init__(self, config_object = None):
        threading.Thread.__init__(self, name = "Thread-GUI")
        gtk.gdk.threads_init()

        if config_object is not None:
            self.config = config_object.get_my_config(self)

        self.__experiment_running = False

        # Place where graphs get saved internally
        self.__display_channels = { }

        # Determines, wether x-scale needs to be adjusted
        self.__rescale = True

        # Number of currently open error-dialogs
        self.__error_dialogs_open = 0

        # Saving the old extrema
        self.__old_ymax = 0
        self.__old_ymin = 0

        # Flag which determines if the GUI is busy with plotting a result
        self.__drawing_busy = False # False: Surface idle (concerns plotting)
                                   # True: Surface busy drawing

        
    def run(self):
        "Starting thread and GTK-mainloop"
        self.xml_gui = gtk.glade.XML(os.path.join(sys.path[0],"damaris.glade"))

        self.damaris_gui_init()

        gtk.gdk.threads_enter()
        gtk.main()
        gtk.gdk.threads_leave()

        
    # Inits der einzelnen Fenster ##################################################################

    def damaris_gui_init(self):
        "Initialises the GUI-elements (connecting signals, referencing elements...)"


        # Widgets mit Variablen verbinden ----------------------------------------------------------

        # Menu:
        self.menu_new_item = self.xml_gui.get_widget("menu_new_file_item")
        self.menu_open_item = self.xml_gui.get_widget("menu_open_file_item")
        self.menu_save_item = self.xml_gui.get_widget("menu_save_file_item")
        self.menu_save_as_item = self.xml_gui.get_widget("menu_save_file_as_item")
        self.menu_save_all_item = self.xml_gui.get_widget("menu_save_all_files_item")

        # Toolbar:
        self.toolbar_new_button = self.xml_gui.get_widget("toolbar_new_button")
        self.toolbar_open_button = self.xml_gui.get_widget("toolbar_open_file_button")
        self.toolbar_save_button = self.xml_gui.get_widget("toolbar_save_file_button")
        self.toolbar_save_as_button = self.xml_gui.get_widget("toolbar_save_as_button")
        self.toolbar_save_all_button = self.xml_gui.get_widget("toolbar_save_all_button")
        self.toolbar_stop_button = self.xml_gui.get_widget("toolbar_stop_button")
        self.toolbar_run_button = self.xml_gui.get_widget("toolbar_run_button")
        self.toolbar_check_scripts_button = self.xml_gui.get_widget("toolbar_check_scripts_button")
        self.toolbar_exec_with_options_togglebutton = self.xml_gui.get_widget("toolbar_execute_with_options_button")

        # Scripts:
        self.experiment_script_textview = self.xml_gui.get_widget("experiment_script_textview")
        self.data_handling_textview = self.xml_gui.get_widget("data_handling_textview")
        self.experiment_script_textbuffer = self.experiment_script_textview.get_buffer()
        self.data_handling_textbuffer = self.data_handling_textview.get_buffer()
        self.experiment_script_statusbar_label = self.xml_gui.get_widget("statusbar_experiment_script_label")
        self.data_handling_statusbar_label = self.xml_gui.get_widget("statusbar_data_handling_label")
        self.experiment_script_line_indicator=self.xml_gui.get_widget("experiment_script_line_textfield")
        self.experiment_script_column_indicator=self.xml_gui.get_widget("experiment_script_column_textfield")
        self.data_handling_line_indicator=self.xml_gui.get_widget("data_handling_line_textfield")
        self.data_handling_column_indicator=self.xml_gui.get_widget("data_handling_column_textfield")
        
        # Display:
        self.display_x_scaling_combobox = self.xml_gui.get_widget("display_x_scaling_combobox")
        self.display_y_scaling_combobox = self.xml_gui.get_widget("display_y_scaling_combobox")
        self.display_source_combobox = gtk.combo_box_new_text()
        self.display_autoscaling_checkbutton = self.xml_gui.get_widget("display_autoscaling_checkbutton")
        self.display_statistics_checkbutton = self.xml_gui.get_widget("display_statistics_checkbutton")

        # Log-Messages
        self.log_messages_textview = self.xml_gui.get_widget("messages_textview")
        self.log_messages_textbuffer = self.log_messages_textview.get_buffer()
        self.log_messages_tab_label = self.xml_gui.get_widget("log_messages_tab_label")
        self.log_messages_lines_label = self.xml_gui.get_widget("log_messages_lines_label")
        self.log_messages_filename_label = self.xml_gui.get_widget("log_messages_filename_label")

        # Execute with options pop-up window
        self.execute_with_options_window = self.xml_gui.get_widget("execute_with_options_window")
        self.execute_with_options_path_label = self.xml_gui.get_widget("execute_with_options_path_label")
        self.execute_with_options_es_checkbutton = self.xml_gui.get_widget("execute_with_options_experiment_script_checkbutton")
        self.execute_with_options_dh_checkbutton = self.xml_gui.get_widget("execute_with_options_datahandling_checkbutton")
        self.execute_with_options_backend_checkbutton = self.xml_gui.get_widget("execute_with_options_backend_checkbutton")
        self.execute_with_options_sync_checkbutton = self.xml_gui.get_widget("execute_with_options_sync_run_checkbutton")

        # Sonstiges:
        self.main_window = self.xml_gui.get_widget("main_window")
        self.main_notebook = self.xml_gui.get_widget("main_notebook")
        self.statusbar_label = self.xml_gui.get_widget("statusbar_label")
        self.main_clipboard = gtk.Clipboard(selection = "CLIPBOARD")
        self.display_settings_frame = self.xml_gui.get_widget("display_settings_frame")
        self.backend_statusbar_label = self.xml_gui.get_widget("statusbar_core_label")
 
        
        # / Widgets mit Variablen verbinden --------------------------------------------------------

        # Alle Signale verbinden -------------------------------------------------------------------

        # Menu:
        self.xml_gui.signal_connect("on_menu_new_file_item_activate", self.new_file)
        self.xml_gui.signal_connect("on_menu_open_file_item_activate", self.open_file)
        self.xml_gui.signal_connect("on_menu_save_file_item_activate", self.save_file)
        self.xml_gui.signal_connect("on_menu_save_all_files_item_activate", self.save_all_files)
        self.xml_gui.signal_connect("on_menu_save_file_as_item_activate", self.save_file_as)
        self.xml_gui.signal_connect("on_menu_quit_item_activate", self.quit_application)

        self.xml_gui.signal_connect("on_menu_edit_paste_item_activate", self.edit_paste)
        self.xml_gui.signal_connect("on_menu_edit_copy_item_activate", self.edit_copy)
        self.xml_gui.signal_connect("on_menu_edit_cut_item_activate", self.edit_cut)

        # Toolbar:
        self.xml_gui.signal_connect("on_toolbar_run_button_clicked", self.start_experiment)
        self.xml_gui.signal_connect("on_toolbar_open_file_button_clicked", self.open_file)
        self.xml_gui.signal_connect("on_toolbar_new_button_clicked", self.new_file)
        self.xml_gui.signal_connect("on_toolbar_save_as_button_clicked", self.save_file_as)
        self.xml_gui.signal_connect("on_toolbar_save_file_button_clicked", self.save_file)
        self.xml_gui.signal_connect("on_toolbar_save_all_button_clicked", self.save_all_files)
        self.xml_gui.signal_connect("on_toolbar_stop_button_clicked", self.stop_experiment)
        self.xml_gui.signal_connect("on_toolbar_execute_with_options_button_clicked", self.start_experiment_with_options)

        # Display:      
        self.display_source_combobox.connect("changed", self.display_source_changed)
        self.xml_gui.signal_connect("on_display_autoscaling_checkbutton_toggled", self.display_autoscaling_toggled)
        self.xml_gui.signal_connect("on_display_statistics_checkbutton_toggled", self.display_statistics_toggled)
        self.xml_gui.signal_connect("on_display_x_scaling_combobox_changed", self.display_x_scaling_changed)
        self.xml_gui.signal_connect("on_display_y_scaling_combobox_changed", self.display_y_scaling_changed)
        self.xml_gui.signal_connect("on_display_save_data_as_text_button_clicked", self.save_display_data_as_text)
        
        # Scripts:        
        self.experiment_script_textbuffer.connect("modified-changed", self.textviews_modified)
        self.experiment_script_textview.connect_after("move-cursor", self.textviews_moved)
        self.experiment_script_textview.connect("key-press-event", self.textviews_keypress)
        self.experiment_script_textview.connect("button-release-event", self.textviews_clicked)
        self.data_handling_textbuffer.connect("modified-changed", self.textviews_modified)
        self.data_handling_textview.connect_after("move-cursor", self.textviews_moved)
        self.data_handling_textview.connect("key-press-event", self.textviews_keypress)
        self.data_handling_textview.connect("button-release-event", self.textviews_clicked)

        #Log-Messages
        self.log_messages_textbuffer.connect("modified-changed", self.textviews_modified)

        # Execute-with-options window
        self.xml_gui.signal_connect("on_execute_with_options_sync_run_checkbutton_toggled", self.execute_with_options_sync_toggled)
        self.xml_gui.signal_connect("on_execute_with_options_run_button_clicked", self.execute_with_options_button_clicked)

        # Misc:
        self.main_window.connect("delete-event", self.quit_application)
        self.execute_with_options_window.connect("delete-event", self.execute_with_options_hide)
        self.xml_gui.signal_connect("on_main_notebook_switch_page", self.main_notebook_page_changed)
                
        # / Signale --------------------------------------------------------------------------------
       
        # Sonstige inits ---------------------------------------------------------------------------

        # Toolbar:
        self.toolbar_stop_button.set_sensitive(False)
        self.toolbar_new_button.set_sensitive(True)
        self.toolbar_open_button.set_sensitive(True)
        self.toolbar_save_button.set_sensitive(False)
        self.toolbar_save_as_button.set_sensitive(True)
        self.toolbar_check_scripts_button.set_sensitive(False)
        self.toolbar_save_all_button.set_sensitive(False)
        
        # Display:
        #self.display_x_scaling_combobox.set_active(0) moved below initialising matplotlib (prevents AttributeError)
        #self.display_y_scaling_combobox.set_active(0) moved below initialising matplotlib (prevents AttributeError)
        #self.display_source_combobox.set_active(0) moved below initialising matplotlib (prevents AttributeError)
        self.display_autoscaling_checkbutton.set_active(False)
        self.display_statistics_checkbutton.set_active(False)
        self.display_source_combobox.insert_text(0,"None")
        self.display_settings_frame.attach(self.display_source_combobox, 1, 2, 0, 1,  gtk.FILL, gtk.FILL, 3, 0)

        # Sonstige:
        self.main_window_title = "DAMARIS - %s, %s"

        # Scripts:
        self.experiment_script_textview.modify_font(pango.FontDescription("Courier 12"))
        self.data_handling_textview.modify_font(pango.FontDescription("Courier 12"))

        self.experiment_script_textview.associated_filename = "Unnamed"
        self.data_handling_textview.associated_filename = "Unnamed"

        if self.config.has_key("experiment_script"):
            script_file = file(self.config["experiment_script"], "r")
            self.experiment_script_textview.associated_filename = self.config["experiment_script"]

            experiment_script_string = ""

            for line in script_file:
                experiment_script_string += line

            script_file.close()

            self.experiment_script_textbuffer.set_text(experiment_script_string)

        else:
                    
            self.experiment_script_textbuffer.set_text("""def experiment_script(outer_space):
    pass
""")

        self.experiment_script_textbuffer.set_modified(False)

        if self.config.has_key("datahandling_script"):
            script_file = file(self.config["datahandling_script"], "r")
            self.data_handling_textview.associated_filename = self.config["datahandling_script"]

            datahandling_script_string = ""

            for line in script_file:
                datahandling_script_string += line

            script_file.close()

            self.data_handling_textbuffer.set_text(datahandling_script_string)

        else:

            self.data_handling_textbuffer.set_text("""def data_handling(outer_space):
    pass
""")

        self.data_handling_textbuffer.set_modified(False)
        self.main_window.set_title(self.main_window_title % (os.path.basename(self.experiment_script_textview.associated_filename),
                                                             os.path.basename(self.data_handling_textview.associated_filename)))

        # Log-Messages Window
        self.log_messages_textview.modify_font(pango.FontDescription("Courier 10"))
        
        self.log_messages_textview_tag_table = self.log_messages_textview.get_buffer().get_tag_table()
        gui_tag = gtk.TextTag("GUI")
        core_tag = gtk.TextTag("CORE")
        es_tag = gtk.TextTag("ES")
        dh_tag = gtk.TextTag("DH")
        norm_tag = gtk.TextTag("NORMAL")

        gui_tag.set_property("foreground", "#F5D000")   #Yellowish
        core_tag.set_property("foreground", "#05D400")  #Greenish
        es_tag.set_property("foreground", "#0090C4")    #Blueish
        dh_tag.set_property("foreground", "#E72100")    #Redish
        norm_tag.set_property("foreground", "#000000")  #Black

        self.log_messages_textview_tag_table.add(gui_tag)
        self.log_messages_textview_tag_table.add(core_tag)
        self.log_messages_textview_tag_table.add(es_tag)
        self.log_messages_textview_tag_table.add(dh_tag)
        self.log_messages_textview_tag_table.add(norm_tag)

        self.log_messages_textbuffer.set_modified(False)
        self.log_messages_textview.associated_filename = "Unnamed"

        # Matplot (Display_Table, 1. Zeile) --------------------------------------------------------

        # Neue Abbildung erstellen
        self.matplot_figure = Figure()

        # Standartplot erstellen (1 Reihe, 1 Zeile, Beginnend beim 1.) und der Abbildung
        self.matplot_axes = self.matplot_figure.add_subplot(111)

        # Achsen beschriften & Gitternetzlinien sichtbar machen
        self.matplot_axes.grid(True)

        # Ersten Plot erstellen und Referenz des ersten Eintrags im zurückgegebenen Tupel speichern
        # Voerst: graphen[0,1] = Real und Img-Kanal; [2,3] = Real-Fehler, [4,5] = Img-Fehler
        self.graphen = []
        self.measurementresultgraph=None

        self.matplot_axes.set_ylim([0.0,1.0])
        self.matplot_axes.set_xlim([0.0,1.0])
        self.matplot_axes.set_autoscale_on(self.display_autoscaling_checkbutton.get_active())
        
        # Lineare y-/x-Skalierung
        self.matplot_axes.set_yscale("linear")
        self.matplot_axes.set_xscale("linear")

        # Matplot in einen GTK-Rahmen stopfen
        self.matplot_canvas = FigureCanvas(self.matplot_figure)

        self.display_table = self.xml_gui.get_widget("display_table")
        self.display_table.attach(self.matplot_canvas, 0, 6, 0, 1, gtk.EXPAND | gtk.FILL, gtk.EXPAND | gtk.FILL, 0, 0)
        self.matplot_canvas.show()

        # Matplot Toolbar hinzufügen (Display_Table, 2. Zeile) 

        self.matplot_toolbar = NavigationToolbar(self.matplot_canvas, self.main_window)
        
        self.display_table.attach(self.matplot_toolbar, 0, 1, 1, 2, gtk.FILL | gtk.EXPAND, 0, 0, 0)
        self.matplot_toolbar.show()

        # /Mathplot --------------------------------------------------------------------------------

        self.display_source_combobox.set_active(0)
        self.display_source_combobox.set_add_tearoffs(1)
        self.display_x_scaling_combobox.set_active(0)
        self.display_y_scaling_combobox.set_active(0)
        self.display_x_scaling_combobox.set_sensitive(False)
        self.display_y_scaling_combobox.set_sensitive(False)

        self.main_window.show_all()
        
    # /Inits der einzelnen Fenster #################################################################        


    # Callbacks ####################################################################################

    def quit_application(self, widget, Data = None):
        "Callback for everything that quits the application"

        self.quit_timeout = 0.0

        # Contents changed?

        if self.experiment_script_textbuffer.get_modified() or self.data_handling_textbuffer.get_modified():
            answer = NiftyGuiElements.show_question_dialog(self.main_window, "Unsaved changes", "Do you want so save your changes?")

            # Answer yes
            if answer == 0:
                self.save_all_files(None)

            # Cancel / Exit-Window clicked
            elif answer == 2 or answer == -1:
                return True

        if self.__experiment_running:
            self.stop_experiment(widget)

        gobject.timeout_add(200, self.quit_application_part_2)


    def quit_application_part_2(self, Data = None):

        # If we got somehow stuck inside an running experiment (what doesnt need to mean we really run one...)
        # activate a 5 second timeout, before quitting
        if self.__experiment_running:
            self.quit_timeout += 0.2
            if self.quit_timeout <= 5:
                return True

        #Jobwriter will also quit core
        self.job_writer.quit_job_writer()
        self.job_writer.join()

        #print threading.enumerate()

        self.data_handler.quit_data_handling()
        self.data_handler.join()

        #print threading.enumerate()

        gtk.main_quit()
        
        return False

    def execute_with_options_hide(self, widget, data = None):
        self.execute_with_options_window.hide_all()
        return True


    def start_experiment_with_options(self, widget, data = None):
        self.execute_with_options_path_label.set_text("Path: " + self.job_writer.get_job_writer_path())
        self.execute_with_options_window.show_all()
        return True

    def execute_with_options_button_clicked(self, widget, data = None):
        # (JW, BE, DH)
        self.execute_with_options_choice = ((int(self.execute_with_options_es_checkbutton.get_active()))<<2) + ((int(self.execute_with_options_backend_checkbutton.get_active()))<<1) + ((int(self.execute_with_options_dh_checkbutton.get_active()))<<0)
        self.execute_with_options_window.hide_all()
        self.start_experiment(widget, self.execute_with_options_choice)
        return True


    def execute_with_options_sync_toggled(self, data = None):
        if self.execute_with_options_sync_checkbutton.get_active():
            self.execute_with_options_es_checkbutton.set_active(True)
            self.execute_with_options_dh_checkbutton.set_active(True)
            self.execute_with_options_backend_checkbutton.set_active(True)
            self.execute_with_options_backend_checkbutton.set_sensitive(False)
            self.execute_with_options_dh_checkbutton.set_sensitive(False)
            self.execute_with_options_es_checkbutton.set_sensitive(False)

            # Setting synchronous run to true
            self.job_writer.write_jobs_synchronous(True)
            self.data_handler.read_jobs_synchronous(True)

        else:
            self.execute_with_options_es_checkbutton.set_sensitive(True)
            self.execute_with_options_dh_checkbutton.set_sensitive(True)
            self.execute_with_options_backend_checkbutton.set_sensitive(True)

            # Setting synchronous run to false
            self.job_writer.write_jobs_synchronous(False)
            self.data_handler.read_jobs_synchronous(False)

        return True
        

    def start_experiment(self, widget, data = None):
        """Callback for the "Start-Experiment" button"""

        if data is None:
            choice = 7
        else:
            choice = data
            if choice == 0:
                self.new_log_message("Experiment done.", "GUI")
                return True
            elif choice == 5:
                self.new_log_message("Warning: Running experiment- and datahandling script without backend is not allowed yet! (prevents lockup)", "GUI")
                return True

       
        # Remove all entries except "None"
        self.display_source_combobox.set_active(0)
        for i in xrange(len(self.__display_channels)):
            self.display_source_combobox.remove_text(1)

        # Deleting all history
        self.__display_channels = { }
        self.__rescale = True

        self.display_x_scaling_combobox.set_active(0)
        self.display_y_scaling_combobox.set_active(0)
        self.display_x_scaling_combobox.set_sensitive(False)
        self.display_y_scaling_combobox.set_sensitive(False)
        
        self.__experiment_running = True

        try:
            
            self.toolbar_run_button.set_sensitive(False)
            self.toolbar_exec_with_options_togglebutton.set_sensitive(False)
            # stop_button.set_sensitive(True) -> look sync_job_writer_data_handler
            self.toolbar_new_button.set_sensitive(False)
            self.toolbar_open_button.set_sensitive(False)
            self.toolbar_save_button.set_sensitive(False)
            self.toolbar_save_as_button.set_sensitive(False)
            self.toolbar_save_all_button.set_sensitive(False)
            
            self.menu_new_item.set_sensitive(False)
            self.menu_open_item.set_sensitive(False)
            self.menu_save_item.set_sensitive(False)
            self.menu_save_as_item.set_sensitive(False)
            self.menu_save_all_item.set_sensitive(False)

            self.experiment_script_textview.set_sensitive(False)
            self.data_handling_textview.set_sensitive(False)
            
            # Delete existing job-files
            if bool(choice & 4):
                file_list = glob.glob(os.path.join(self.job_writer.get_job_writer_path(), "job.[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]"))
                try:
                    for job_file in file_list:
                        os.remove(job_file)

                    if not len(file_list) == 0:
                        self.new_log_message("Warning: Deleted %d job-files" % len(file_list), "GUI")
                except IOError, e:
                    self.gui.show_error_dialog("File Error", "IOError: Cannot delete job-files" + str(e))
                    self.new_log_message("IOError occurred: Unable to delete job-files (%s)" % str(e), "GUI")
                    return True
                except Exception, e:
                    self.gui.show_error_dialog("Error", "Exception: Unable to delete job-files" + str(e))
                    self.new_log_message("Exception occurred: Unable to delete Job-Files (%s)" % str(e), "GUI")
                    return True

            # Delete existing result-files
            if bool(choice & 2):
                file_list = glob.glob(os.path.join(self.job_writer.get_job_writer_path(), "*.result"))
                try:
                    for job_file in file_list:
                        os.remove(job_file)

                    if not len(file_list) == 0:
                        self.new_log_message("Warning: Deleted %d result-files" % len(file_list), "GUI")
                except IOError, e:
                    self.gui.show_error_dialog("File Error", "IOError: Cannot delete result-files" + str(e))
                    self.new_log_message("IOError occurred: Unable to delete result-files (%s)" % str(e), "GUI")
                    return True
                except Exception, e:
                    self.gui.show_error_dialog("Error", "Exception: Cannot delete result-files" + str(e))
                    self.new_log_message("Exception occurred: Unable to delete result-files (%s)" % str(e), "GUI")
                    return True  

            # Depending on execute_with_options run threads
            if bool(choice & 2):
                try:
                    self.backend_statusbar_label.set_text("Backend: Busy...")
                    if len(glob.glob(os.path.join(self.job_writer.get_job_writer_path(), "*.state"))) != 0:
                        self.new_log_message("Warning: Found another core already started. Trying to abort...", "GUI")
                        self.core_interface.abort()
                                    
                    self.core_interface.clear_job(0)
                    self.new_log_message("(Re)Starting backend...", "GUI")
                    self.core_interface.start()
                except EnvironmentError, env_e:
                    self.show_error_dialog("Core Exception (clear_job / start)", str(env_e) + "\n\n(Maybe there is still a (dummy)core process active)")
                    self.new_log_message("Environment Error (Start): %s" % str(env_e), "CORE")
                    self.check_job_writer_data_handler_finished()
                    return True
                except Exception, e:
                    self.show_error_dialog("Core Exception (clear_job / start)", str(e) + "\n\n(Maybe there is still a (dummy)core process active)")
                    self.new_log_message("Exception (Start): %s" % str(e), "CORE")
                    self.check_job_writer_data_handler_finished()
                    return True
            
            if bool(choice & 4):
                self.new_log_message("Executing Experiment Script...", "GUI")
                self.experiment_script_statusbar_label.set_text("Experiment Script: Busy...")
                self.job_writer.wake_up()
            
            
            if bool(choice & 1):
                self.new_log_message("Executing Data Handling Script...", "GUI")
                self.data_handling_statusbar_label.set_text("Data Handling: Busy...")
                self.data_handler.wake_up()

            gobject.timeout_add(100, self.sync_job_writer_data_handler)
            gobject.timeout_add(500, self.check_job_writer_data_handler_finished)
            
            return True
        except:
            self.stop_experiment(None)
            self.__experiment_running = False
            raise
        

    def sync_job_writer_data_handler(self):
        "Timeout-Callback for synchronising Job-Writer and Data-Handler (for example restarting everthing if an error occured)"
        try:
            gtk.gdk.threads_enter()
            # Check if still parsing
            if (self.job_writer.error_occured()) == None and (self.job_writer.is_busy() == True):
                return True

            if self.data_handler.error_occured() == None and (self.data_handler.is_busy() == True):
                return True

            # Check if an error occured
            if self.job_writer.error_occured() == True or self.data_handler.error_occured() == True:
                self.job_writer.start_writing(False)
                self.data_handler.start_handling(False)
                self.core_interface.stop_queue()
                return False
            else:
                self.job_writer.start_writing(True)
                self.data_handler.start_handling(True)
                self.toolbar_stop_button.set_sensitive(True)
                self.main_notebook.set_current_page(2)
                return False
        finally:
            gtk.gdk.threads_leave()


    def check_job_writer_data_handler_finished(self):
        "Timeout-Callback for checking the state of the Job-Writer and Data-Handler"
        try:
            gtk.gdk.threads_enter()
            
            if not self.job_writer.is_busy():
                self.experiment_script_statusbar_label.set_text("Experiment Script: Idle.")

            if not self.data_handler.is_busy():
                self.data_handling_statusbar_label.set_text("Data Handling: Idle.")

            if not self.core_interface.is_busy():
                self.backend_statusbar_label.set_text("Backend: Idle.")

            if not self.data_handler.is_busy() and not self.job_writer.is_busy() and not self.core_interface.is_busy():
                self.toolbar_run_button.set_sensitive(True)
                self.toolbar_exec_with_options_togglebutton.set_sensitive(True)
                self.toolbar_stop_button.set_sensitive(False)
                
                self.toolbar_new_button.set_sensitive(True)
                self.toolbar_open_button.set_sensitive(True)
                self.toolbar_save_as_button.set_sensitive(True)

                self.menu_new_item.set_sensitive(True)
                self.menu_open_item.set_sensitive(True)
                self.menu_save_as_item.set_sensitive(True)

                self.experiment_script_textview.set_sensitive(True)
                self.data_handling_textview.set_sensitive(True)               
                
                self.__experiment_running = False

                self.new_log_message("Experiment Done.", "GUI")
                
                return False

            return True
            
        finally:
            gtk.gdk.threads_leave()



    def stop_experiment(self, widget, data = None):
        "Sends all Threads the stop-commando"
        self.new_log_message("Stopping Experiment... (Waiting for components to stop safely)", "GUI")       

        try:
            self.core_interface.abort()
        except Exception, e:
            self.show_error_dialog("Core Exception (Abort)", str(e) + "\n(for more information look into the logfile)")
            self.new_log_message("Exception (Abort): %s" % str(e), "CORE")
        
        self.job_writer.stop_experiment()
        self.data_handler.stop_experiment()
        

        return True


    def open_file(self, widget, Data = None):
        "Callback for the open-file dialog"

        # Save changes made...
        if self.main_notebook.get_current_page() == 0:
            if self.experiment_script_textbuffer.get_modified():
                answer = NiftyGuiElements.show_question_dialog(self.main_window, "Unsaved changes in Experiment Script", "Do you want to save your changes made to the script?")

                if answer == 0:
                    # User wants to save
                    self.save_file(widget)

                elif answer == 2:
                    # User cancels
                    return True
        elif self.main_notebook.get_current_page() == 1:
            if self.data_handling_textbuffer.get_modified():
                answer = NiftyGuiElements.show_question_dialog(self.main_window, "Unsaved changes in Data Handling Script", "Do you want to save your changes made to the script?")

                if answer == 0:
                    # User wants to save
                    self.save_file(widget)

                elif answer == 2:
                    # User cancels
                    return True
            

        # No changes made or user answers no:
        def response(self, response_id, outer_space):
            if response_id == gtk.RESPONSE_OK:
                file_name = dialog.get_filename()
                if file_name is None:
                    return

                if not os.access(file_name, os.R_OK):
                    outer_space.show_error_dialog("File I/O Error","Cannot read from file %s" % file_name)
                    return True

                script_file = file(file_name, "r")
                
                if outer_space.main_notebook.get_current_page() == 0:
                    experiment_script_string = ""

                    for line in script_file:
                        experiment_script_string += line
                    script_file.close()

                    outer_space.experiment_script_textbuffer.set_text(experiment_script_string)
                    outer_space.experiment_script_textbuffer.set_modified(False)
                    outer_space.experiment_script_textview.associated_filename = file_name


                elif outer_space.main_notebook.get_current_page() == 1:
                    data_handling_string = ""

                    for line in script_file:
                        data_handling_string += line

                    script_file.close()

                    outer_space.data_handling_textbuffer.set_text(data_handling_string)
                    outer_space.data_handling_textbuffer.set_modified(False)
                    outer_space.data_handling_textview.associated_filename = file_name
                    
                
            else:
                return True
        
        # Determining the tab which is currently open
        if self.main_notebook.get_current_page() == 0:
            dialog = gtk.FileChooserDialog(title="Open Experiment Script...", parent=self.main_window, action=gtk.FILE_CHOOSER_ACTION_OPEN, buttons = (gtk.STOCK_OPEN, gtk.RESPONSE_OK, gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL))
        elif self.main_notebook.get_current_page() == 1:
            dialog = gtk.FileChooserDialog(title="Open Data Handling Script...", parent=self.main_window, action=gtk.FILE_CHOOSER_ACTION_OPEN, buttons = (gtk.STOCK_OPEN, gtk.RESPONSE_OK, gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL))

        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_select_multiple(False)

        # Event-Handler for responce-signal (when one of the button is pressed)
        dialog.connect("response", response, self)

        dialog.run()
        dialog.destroy()

        self.main_window.set_title(self.main_window_title % (os.path.basename(self.experiment_script_textview.associated_filename),
                                                             os.path.basename(self.data_handling_textview.associated_filename)))

        return True



    def save_file(self, widget, Data = None):

        if self.main_notebook.get_current_page() == 0:
            if self.experiment_script_textview.associated_filename == "Unnamed":
                self.save_file_as(widget)
            else:
                if not os.access(self.experiment_script_textview.associated_filename, os.W_OK):
                    self.show_error_dialog("File I/O Error", "Cannot save to file %s" % self.experiment_script_textview.associated_filename)
                    return True

                script_file = file(self.experiment_script_textview.associated_filename, "w")
                script_file.write(self.experiment_script_textbuffer.get_text(self.experiment_script_textbuffer.get_start_iter(), self.experiment_script_textbuffer.get_end_iter()))
                script_file.close()
                self.experiment_script_textbuffer.set_modified(False)

        elif self.main_notebook.get_current_page() == 1:
            if self.data_handling_textview.associated_filename == "Unnamed":
                self.save_file_as(widget)
            else:
                if not os.access(self.data_handling_textview.associated_filename, os.W_OK):
                    self.show_error_dialog("File I/O Error", "Cannot save to file %s" % self.data_handling_textview.associated_filename)
                    return True

                script_file = file(self.data_handling_textview.associated_filename, "w")
                script_file.write(self.data_handling_textbuffer.get_text(self.data_handling_textbuffer.get_start_iter(), self.data_handling_textbuffer.get_end_iter()))
                script_file.close()
                self.data_handling_textbuffer.set_modified(False)

        elif self.main_notebook.get_current_page() == 3:
            if self.log_messages_textview.associated_filename == "Unnamed":
                self.save_file_as(widget)
            else:
                if not os.access(self.log_messages_textview.associated_filename, os.W_OK):
                    self.show_error_dialog("File I/O Error", "Cannot save to file %s" % self.log_messages_textview.associated_filename)
                    return True

                log_file = file(self.log_messages_textview.associated_filename, "w")
                log_file.write(self.log_messages_textbuffer.get_text(self.log_messages_textbuffer.get_start_iter(), self.log_messages_textbuffer.get_end_iter()))
                log_file.close()
                self.log_messages_textbuffer.set_modified(False)
                
        else:
            pass

        self.toolbar_save_button.set_sensitive(False)
       
        return True


    def save_file_as(self, widget, Data = None):
        "Callback for the save-file-as dialog"

        def response(self, response_id, outer_space):
            if response_id == gtk.RESPONSE_OK:
                file_name = dialog.get_filename()
                if file_name is None:
                    return True

                if os.access(file_name, os.F_OK):
                    answer = NiftyGuiElements.show_question_dialog_compulsive(outer_space.main_window, "File Exists", "Do you want to overwrite the existing file %s?" % file_name)

                    if answer == 1:
                        return True

                if os.access(file_name, os.F_OK) and not os.access(file_name, os.W_OK):
                    outer_space.show_error_dialog("File I/O Error", "Cannot save file to %s" % file_name)
                    return True

                try:
                    script_file = file(file_name, "w")
                except:
                    outer_space.show_error_dialog("File I/O Error", "Cannot save file to %s" % file_name)
                    return True

                if outer_space.main_notebook.get_current_page() == 0:
                    script_file.write(outer_space.experiment_script_textbuffer.get_text(outer_space.experiment_script_textbuffer.get_start_iter(), outer_space.experiment_script_textbuffer.get_end_iter()))

                    script_file.close()

                    outer_space.experiment_script_textbuffer.set_modified(False)
                    outer_space.experiment_script_textview.associated_filename = file_name

                elif outer_space.main_notebook.get_current_page() == 1:
                    script_file.write(outer_space.data_handling_textbuffer.get_text(outer_space.data_handling_textbuffer.get_start_iter(), outer_space.data_handling_textbuffer.get_end_iter()))

                    script_file.close()

                    outer_space.data_handling_textbuffer.set_modified(False)
                    outer_space.data_handling_textview.associated_filename = file_name

                elif outer_space.main_notebook.get_current_page() == 3:
                    script_file.write(outer_space.log_messages_textbuffer.get_text(outer_space.log_messages_textbuffer.get_start_iter(), outer_space.log_messages_textbuffer.get_end_iter()))

                    script_file.close()

                    outer_space.log_messages_textbuffer.set_modified(False)
                    outer_space.log_messages_textbuffer.associated_filename = file_name
                    outer_space.log_messages_filename_label.set_text("File: " + file_name)
                    
                
            else:
                return True
        
        # Determining the tab which is currently open
        if self.main_notebook.get_current_page() == 0:
            dialog = gtk.FileChooserDialog(title="Save Experiment Script As...", parent=self.main_window, action=gtk.FILE_CHOOSER_ACTION_SAVE, buttons = (gtk.STOCK_SAVE, gtk.RESPONSE_OK, gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL))
        elif self.main_notebook.get_current_page() == 1:
            dialog = gtk.FileChooserDialog(title="Save Data Handling As...", parent=self.main_window, action=gtk.FILE_CHOOSER_ACTION_SAVE, buttons = (gtk.STOCK_SAVE, gtk.RESPONSE_OK, gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL))
        elif self.main_notebook.get_current_page() == 3:
            dialog = gtk.FileChooserDialog(title="Save Log Window As...", parent=self.main_window, action=gtk.FILE_CHOOSER_ACTION_SAVE, buttons = (gtk.STOCK_SAVE, gtk.RESPONSE_OK, gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL))
        

        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_select_multiple(False)

        # Event-Handler for responce-signal (when one of the button is pressed)
        dialog.connect("response", response, self)

        dialog.run()
        dialog.destroy()

        self.main_window.set_title(self.main_window_title % (os.path.basename(self.experiment_script_textview.associated_filename),
                                                             os.path.basename(self.data_handling_textview.associated_filename)))

        self.toolbar_save_button.set_sensitive(False)
        return True



    def save_all_files(self, widget, Data = None):

        ye_olde_page = self.main_notebook.get_current_page()

        if self.experiment_script_textbuffer.get_modified():
            self.main_notebook.set_current_page(0)
            self.save_file(widget)

        if self.data_handling_textbuffer.get_modified():
            self.main_notebook.set_current_page(1)
            self.save_file(widget)

        self.main_notebook.set_current_page(ye_olde_page)
       
        return True
    

    def new_file(self, widget, Data = None):

        if self.main_notebook.get_current_page() == 0:
            if self.experiment_script_textbuffer.get_modified():
                answer = NiftyGuiElements.show_question_dialog(self.main_window, "Unsaved changes in experiment script", "Do you want so save your changes?")

                if answer == 0:
                    self.save_file(widget)

                elif answer == 2:
                    return True

            self.experiment_script_textbuffer.set_text("def experiment_script(outer_space):\n    pass")
            self.experiment_script_textview.associated_filename = "Unnamed"
            self.experiment_script_textbuffer.set_modified(False)

        elif self.main_notebook.get_current_page() == 1:
            if self.data_handling_textbuffer.get_modified():
                answer = NiftyGuiElements.show_question_dialog(self.main_window, "Unsaved changes in experiment script", "Do you want so save your changes?")

                if answer == 0:
                    self.save_file(widget)

                elif answer == 2:
                    return True

            self.data_handling_textbuffer.set_text("def data_handling(outer_space):\n    pass")
            self.data_handling_textview.associated_filename = "Unnamed"
            self.data_handling_textbuffer.set_modified(False)

        elif self.main_notebook.get_current_page() == 3:
            if self.log_messages_textbuffer.get_modified():
                answer = NiftyGuiElements.show_question_dialog(self.main_window, "Unsaved changes in log-window", "Do you want so save changes?")

                if answer == 0:
                    self.save_file(widget)

                elif answer == 2:
                    return True

            self.log_messages_textbuffer.set_text("")
            self.log_messages_textview.associated_filename = "Unnamed"
            self.log_messages_filename_label.set_text("File: " + self.log_messages_textview.associated_filename)
            self.log_messages_textbuffer.set_modified(False)

        self.main_window.set_title(self.main_window_title % (os.path.basename(self.experiment_script_textview.associated_filename),
                                                             os.path.basename(self.data_handling_textview.associated_filename)))
        return True


    def save_display_data_as_text(self, widget, data = None):
        "Saves the currently active figure as textdata"

        def response(self, response_id, outer_space):
            if response_id == gtk.RESPONSE_OK:

                file_name = dialog.get_filename()
                if file_name is None:
                    return True

                if os.access(file_name, os.F_OK):
                    answer = NiftyGuiElements.show_question_dialog_compulsive(outer_space.main_window,
                                                                              "File Exists",
                                                                               "Do you want to overwrite the existing file %s?" % file_name)

                    if answer == 1:
                        return True

                if os.access(file_name, os.F_OK) and not os.access(file_name, os.W_OK):
                    outer_space.show_error_dialog("File I/O Error", "Cannot save file to %s" % file_name)
                    return True

                try:
                    text_file = file(file_name, "w")
                except:
                    outer_space.show_error_dialog("File I/O Error", "Cannot save file to %s" % file_name)
                    return True

                # get information what to save
                channel = outer_space.display_source_combobox.get_active_text()
                data=outer_space.__display_channels[channel][0]
                # decide how to save
                if (isinstance(data,MeasurementResult.MeasurementResult)):
                    text_file.write("# \"%s\" saved from display at %s\n"%(channel,time.ctime()))
                    data.write_as_csv(text_file)
                else:
                    x_re = outer_space.graphen[0].get_xdata()
                    y_re = outer_space.graphen[0].get_ydata()
                    x_im = outer_space.graphen[1].get_xdata()
                    y_im = outer_space.graphen[1].get_ydata()

                    y_re_err_p = outer_space.graphen[2].get_ydata()
                    y_re_err_m = outer_space.graphen[3].get_ydata()
                    y_im_err_p = outer_space.graphen[4].get_ydata()
                    y_im_err_m = outer_space.graphen[5].get_ydata()

                    text_file.write("X_RE,Y_RE,Y_RE_ERR+,Y_RE_ERR-,X_IM,Y_IM,Y_IM_ERR+,Y_IM_ERR-\n")

                    tmp_string = []

                    max_of_all = max([len(x_re), len(y_re), len(x_im), len(y_im), len(y_re_err_p), len(y_re_err_m), len(y_im_err_p), len(y_im_err_m)])

                    # Creating space for the largest array
                    for i in range(max_of_all):
                        tmp_string.append("")

                        if len(x_re) > i:
                            tmp_string[i] += str(x_re[i])
                        else:
                            tmp_string[i] += ","

                        if len(y_re) > i:
                            tmp_string[i] += "," + str(y_re[i])
                        else:
                            tmp_string[i] += ","
                    
                        if len(y_re_err_p) > i:
                            tmp_string[i] += "," + str(y_re_err_p[i])
                        else:
                            tmp_string[i] += ",0.0"

                        if len(y_re_err_m) > i:
                            tmp_string[i] += "," + str(y_re_err_m[i])
                        else:
                            tmp_string[i] += ",0.0"

                        if len(x_im) > i:
                            tmp_string[i] += "," + str(x_im[i])
                        else:
                            tmp_string[i] += ","

                        if len(y_im) > i:
                            tmp_string[i] += "," + str(y_im[i])
                        else:
                            tmp_string[i] += ","
                    
                        if len(y_im_err_p) > i:
                            tmp_string[i] += "," + str(y_im_err_p[i])
                        else:
                            tmp_string[i] += ",0.0"

                        if len(y_im_err_m) > i:
                            tmp_string[i] += "," + str(y_im_err_m[i])
                        else:
                            tmp_string[i] += ",0.0"

                        tmp_string[i] += "\n"
                        
                    text_file.writelines(tmp_string)
                    text_file.close()

            else:
                return True
        

        dialog = gtk.FileChooserDialog(title="Save figure as text...", parent=self.main_window, action=gtk.FILE_CHOOSER_ACTION_SAVE, buttons = (gtk.STOCK_SAVE, gtk.RESPONSE_OK, gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL))

        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_select_multiple(False)

        # Event-Handler for responce-signal (when one of the button is pressed)
        dialog.connect("response", response, self)

        dialog.run()
        dialog.destroy()

        return True

        
    def main_notebook_page_changed(self, widget, page, page_num, data = None):
        "Make sure you can only access toolbar-buttons usable for the open notebook-tab"

        # In Zukunft auch "Running" beachten

        if self.__experiment_running: return True
        
        if page_num == 0:
            self.toolbar_new_button.set_sensitive(True)
            self.menu_new_item.set_sensitive(True)
            self.toolbar_open_button.set_sensitive(True)
            self.menu_open_item.set_sensitive(True)

            if self.experiment_script_textbuffer.get_modified():
                self.toolbar_save_button.set_sensitive(True)
                self.menu_save_item.set_sensitive(True)
                
                self.toolbar_save_all_button.set_sensitive(True)
                self.menu_save_all_item.set_sensitive(True)
            else:
                self.toolbar_save_button.set_sensitive(False)
                self.menu_save_item.set_sensitive(False)

                
            self.toolbar_save_as_button.set_sensitive(True)
            self.menu_save_as_item.set_sensitive(True)
            self.toolbar_run_button.set_sensitive(True)
            self.toolbar_check_scripts_button.set_sensitive(True)
            
        elif page_num == 1:
            self.toolbar_new_button.set_sensitive(True)
            self.menu_new_item.set_sensitive(True)
            self.toolbar_open_button.set_sensitive(True)
            self.menu_open_item.set_sensitive(True)
            
            if self.data_handling_textbuffer.get_modified():
                self.toolbar_save_button.set_sensitive(True)
                self.menu_save_item.set_sensitive(True)
                
                self.toolbar_save_all_button.set_sensitive(True)
                self.menu_save_all_item.set_sensitive(True)
            else:
                self.toolbar_save_button.set_sensitive(False)
                self.menu_save_item.set_sensitive(False)
                
            self.toolbar_save_as_button.set_sensitive(True)
            self.menu_save_as_item.set_sensitive(True)
            self.toolbar_run_button.set_sensitive(True)
            self.toolbar_check_scripts_button.set_sensitive(True)
            
        elif page_num == 2:
            self.toolbar_new_button.set_sensitive(False)
            self.menu_new_item.set_sensitive(False)
            self.toolbar_open_button.set_sensitive(False)
            self.menu_open_item.set_sensitive(False)
            self.toolbar_save_button.set_sensitive(False)
            self.menu_save_item.set_sensitive(False)
            self.toolbar_save_as_button.set_sensitive(False)
            self.menu_save_as_item.set_sensitive(False)
            self.toolbar_run_button.set_sensitive(True)
            self.toolbar_check_scripts_button.set_sensitive(True)

        elif page_num == 3:
            self.toolbar_new_button.set_sensitive(True)
            self.menu_new_item.set_sensitive(True)
            self.toolbar_open_button.set_sensitive(False)
            self.menu_open_item.set_sensitive(False)
            self.menu_save_all_item.set_sensitive(False)
            self.toolbar_save_all_button.set_sensitive(False)
            
            if self.log_messages_textbuffer.get_modified():
                self.toolbar_save_button.set_sensitive(True)
                self.menu_save_item.set_sensitive(True)

            else:
                self.toolbar_save_button.set_sensitive(False)
                self.menu_save_item.set_sensitive(False)
                
            self.toolbar_save_as_button.set_sensitive(True)
            self.menu_save_as_item.set_sensitive(True)
            self.toolbar_run_button.set_sensitive(True)
            self.toolbar_check_scripts_button.set_sensitive(True)
            self.log_messages_tab_label.set_markup("Log")

        return True


    def textviews_modified(self, data = None):

        if self.experiment_script_textbuffer.get_modified() or self.data_handling_textbuffer.get_modified():
            self.toolbar_save_button.set_sensitive(True)
            self.menu_save_item.set_sensitive(True)
            self.toolbar_save_all_button.set_sensitive(True)
            self.menu_save_all_item.set_sensitive(True)
        else:
            self.toolbar_save_button.set_sensitive(False)
            self.menu_save_item.set_sensitive(False)
            self.toolbar_save_all_button.set_sensitive(False)
            self.menu_save_all_item.set_sensitive(False)

        if self.log_messages_textbuffer.get_modified():
            self.toolbar_save_button.set_sensitive(True)
            self.menu_save_item.set_sensitive(True)
            self.toolbar_open_button.set_sensitive(False)
            self.menu_open_item.set_sensitive(False)

        return True

    def textviews_clicked(self, widget, event):
        self.textviews_moved(widget)
        return False

    def textviews_keypress(self, widget, event, data = None):
        # tab keyval 0xff09
        # backspace keyval 0xff08
        if(event.keyval==0xFF09 or event.keyval==0xFF08):
            textbuffer=widget.get_buffer()
            # do not do things during selection
            if (textbuffer.get_selection_bounds()): return 0
            cursor_mark=textbuffer.get_insert()
            cursor_iter=textbuffer.get_iter_at_mark(cursor_mark)
            if (cursor_iter.starts_line()):
                # backspace with normal function at line start
                if (event.keyval==0xFF08): return 0
            # now get iterator at line start
            linestart_iter=cursor_iter.copy()
            linestart_iter.set_line_offset(0)
            linebegin=textbuffer.get_text(linestart_iter,cursor_iter).expandtabs()
            if (len(linebegin)!=0 and not linebegin.isspace()):
                # just make the spaces go away
                textbuffer.delete(linestart_iter,cursor_iter)
                textbuffer.insert(linestart_iter,linebegin)
                return 0
            # find all space at the begin
            while(not cursor_iter.ends_line()
                  and not cursor_iter.is_end()
                  and cursor_iter.get_char().isspace()):
                cursor_iter.forward_char()
            linebegin=textbuffer.get_text(linestart_iter,cursor_iter)
            if (event.keyval==0xFF08):
                # backspace shortens space
                linebegin=u' '*((len(linebegin)-1)/4)*4
            elif (event.keyval==0xFF09):
                # tab widens space
                linebegin=u' '*((len(linebegin)+4)/4)*4
 
            textbuffer.delete(linestart_iter,cursor_iter)
            textbuffer.insert(linestart_iter,linebegin)
            return 1
        # implement convenience function for enter key
        elif (event.keyval==0xFF0D):
            textbuffer=widget.get_buffer()
            # do not do things during selection
            if (textbuffer.get_selection_bounds()): return 0
            cursor_mark=textbuffer.get_insert()
            cursor_iter=textbuffer.get_iter_at_mark(cursor_mark)
            # determine this line's indent count
            linestart_iter=cursor_iter.copy()
            linestart_iter.set_line_offset(0)
            spaceend_iter=linestart_iter.copy()
            while(not spaceend_iter.ends_line()
                  and not spaceend_iter.is_end()
                  and spaceend_iter.get_char().isspace()):
                spaceend_iter.forward_char()
            linebegin=textbuffer.get_text(linestart_iter,spaceend_iter).expandtabs()
            indent_length=len(linebegin)
            textbuffer.delete(linestart_iter,spaceend_iter)
            textbuffer.insert(linestart_iter,u' '*indent_length)
            # start with the real work
            cursor_iter=textbuffer.get_iter_at_mark(cursor_mark)
            if (not cursor_iter.starts_line()):
                # find last char before cursor
                lastchar_iter=cursor_iter.copy()
                lastchar_iter.backward_char()
                if (lastchar_iter.get_char()==u":"): indent_length+=4
            # now find indent of next line...
            textbuffer.insert(cursor_iter,u'\n'+(u' '*indent_length))
            widget.scroll_to_mark(cursor_mark,0.0,0)
            return 1
        return 0

    def textviews_moved(self, widget, text=None, count=None, ext_selection=None, data = None):
        textbuffer=widget.get_buffer()
        cursor_mark=textbuffer.get_insert()
        cursor_iter=textbuffer.get_iter_at_mark(cursor_mark)
        if textbuffer==self.experiment_script_textbuffer:
            self.experiment_script_line_indicator.set_text("%d"%(cursor_iter.get_line()+1))
            self.experiment_script_column_indicator.set_text("%d"%(cursor_iter.get_line_offset()+1))
        if textbuffer==self.data_handling_textbuffer:
            self.data_handling_line_indicator.set_text("%d"%(cursor_iter.get_line()+1))
            self.data_handling_column_indicator.set_text("%d"%(cursor_iter.get_line_offset()+1))
        return False


    def display_source_changed(self, Data = None):
        if self.main_notebook.get_current_page()!=2:
            return
        channel = self.display_source_combobox.get_active_text()

        # Event triggers when we init the box -> Catch Channel "None"
        if channel == "None":
            self.draw_result(None)
            return True
            
        self.draw_result(self.__display_channels[channel][0])
        return True


    def display_autoscaling_toggled(self, widget, Data = None):

        channel = self.display_source_combobox.get_active_text()
        self.matplot_axes.set_autoscale_on(self.display_autoscaling_checkbutton.get_active())

        if channel == "None":
            self.draw_result(None)
            return True
        self.draw_result(self.__display_channels[channel][0])
        return True

    def display_statistics_toggled(self, widget, Data = None):

        channel = self.display_source_combobox.get_active_text()
        
        if channel == "None":
            return True

        else:
            if self.display_statistics_checkbutton.get_active():
                self.draw_result(self.__display_channels[channel][0])
            else:
                self.graphen[2].set_data([0.0],[0.0])
                self.graphen[3].set_data([0.0],[0.0])
                self.graphen[4].set_data([0.0],[0.0])
                self.graphen[5].set_data([0.0],[0.0])
                self.matplot_canvas.draw()
        return True


    def display_x_scaling_changed(self, data = None):
        scaling = self.display_x_scaling_combobox.get_active_text()
        
        if scaling == "lin":
            self.matplot_axes.set_xscale("linear")

        elif scaling == "log":
            self.matplot_axes.set_xscale("log")

        else:
            return True

        self.matplot_canvas.draw()
        

    def display_y_scaling_changed(self, data = None):
        scaling = self.display_y_scaling_combobox.get_active_text()
        
        if scaling == "lin":
            self.matplot_axes.set_yscale("linear")

        elif scaling == "log":
            self.matplot_axes.set_yscale("log")

        else:
            return True

        self.matplot_canvas.draw()


    def edit_copy(self, widget, data = None):

        if self.main_notebook.get_current_page() == 0:
            self.experiment_script_textbuffer.copy_clipboard(self.main_clipboard)
        elif self.main_notebook.get_current_page() == 1:
            self.data_handling_textbuffer.copy_clipboard(self.main_clipboard)

        return True


    def edit_cut(self, widget, data = None):

        # cut_clipboard(clipboard, textview editable?)
        if self.main_notebook.get_current_page() == 0:
            self.experiment_script_textbuffer.cut_clipboard(self.main_clipboard, True)
        elif self.main_notebook.get_current_page() == 1:
            self.data_handling_textbuffer.cut_clipboard(self.main_clipboard, True)

        return True


    def edit_paste(self, widget, data = None):

        # paste_clipboard(clipboard, textpos (None = Cursor), textview editable?)
        if self.main_notebook.get_current_page() == 0:
            self.experiment_script_textbuffer.paste_clipboard(self.main_clipboard, None, True)
        elif self.main_notebook.get_current_page() == 1:
            self.data_handling_textbuffer.paste_clipboard(self.main_clipboard, None, True)

        return True
    
    # / Callbacks ##################################################################################

    # Schnittstellen nach Außen ####################################################################

    def draw_result(self, in_result):
        "Interface to surface for drawing results/accumulated results"
        gobject.idle_add(self.draw_result_idle_func, in_result)


    def draw_result_idle_func(self, in_result):
        if in_result is None:
            gtk.gdk.threads_enter()
            self.matplot_axes.clear()
            self.matplot_axes.grid(True)
            self.matplot_canvas.draw()
            self.graphen=[]
            self.measurementresultgraph=None
            gtk.gdk.threads_leave()
            return False
        if isinstance(in_result,MeasurementResult.MeasurementResult):
            if self.graphen:
                for l in self.graphen:
                    self.matplot_axes.lines.remove(l)
                self.graphen=[]
            if self.measurementresultgraph is not None:
                # clear errorbars before redrawing
                self.matplot_axes.lines.remove(self.measurementresultgraph[0])
                for l in self.measurementresultgraph[1]:
                    self.matplot_axes.lines.remove(l)
                self.measurementresultgraph=None
            return self.draw_result_idle_func_achim(in_result)
        else:
            if self.measurementresultgraph is not None:
                # clear errorbars
                self.matplot_axes.lines.remove(self.measurementresultgraph[0])
                for l in self.measurementresultgraph[1]:
                    self.matplot_axes.lines.remove(l)
                self.measurementresultgraph=None
            if len(self.graphen)==0:
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "b-", linewidth = 2))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "r-", linewidth = 2))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "b-", linewidth = 0.5))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "b-", linewidth = 0.5))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "r-", linewidth = 0.5))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "r-", linewidth = 0.5))
            if not in_result.uses_statistics():
                # Maybe theres a better place for deleting the error-lines
                # Real-Fehler
                self.graphen[2].set_data([0.0],[0.0])
                self.graphen[3].set_data([0.0],[0.0])
                # Img-Fehler
                self.graphen[4].set_data([0.0],[0.0])
                self.graphen[5].set_data([0.0],[0.0])
            return self.draw_result_idle_func_orig(in_result)

    def draw_result_idle_func_achim(self, in_result):
        gtk.gdk.threads_enter()

        try:
            # Initial rescaling needed?
            [k,v,e]=in_result.get_errorplotdata()
            if self.__rescale or self.display_autoscaling_checkbutton.get_active():
                xmin=min(k)
                xmax=max(k)
                ymin=min(map(lambda i:v[i]-e[i],xrange(len(v))))
                ymax=max(map(lambda i:v[i]+e[i],xrange(len(v))))
                self.matplot_axes.set_xlim(xmin, xmax)
                self.matplot_axes.set_ylim(ymin, ymax)
                self.__rescale = False

            # add error bars
            self.measurementresultgraph=self.matplot_axes.errorbar(x=k, y=v, yerr=e, fmt="b+")

            # Any title to be set?
            title=in_result.get_title()
            if title is not None:
                self.matplot_axes.set_title(title)
            else:
                self.matplot_axes.set_title("")

            self.matplot_canvas.draw()
            gtk.gdk.threads_leave()
            return False
        
        finally:
            gtk.gdk.threads_leave()
            

    def draw_result_idle_func_orig(self, in_result):
        gtk.gdk.threads_enter()

        try:
            self.graphen[0].set_data(in_result.get_xdata(), in_result.get_ydata(0))
            self.graphen[1].set_data(in_result.get_xdata(), in_result.get_ydata(1))

            xmin = in_result.get_xmin()
            xmax = in_result.get_xmax()
            ymin = in_result.get_ymin()
            ymax = in_result.get_ymax()

            # Check for log-scale
            if xmin <= 0 or xmax <= 0:
                self.display_x_scaling_combobox.set_sensitive(False)
                self.display_x_scaling_combobox.set_active(0)
            else:
                self.display_x_scaling_combobox.set_sensitive(True)

            if ymin <= 0 or ymax <= 0:
                self.display_y_scaling_combobox.set_sensitive(False)
                self.display_y_scaling_combobox.set_active(0)
            else:
                self.display_y_scaling_combobox.set_sensitive(True)   


            # Initial rescaling needed?
            if self.__rescale:
                self.matplot_axes.set_xlim(xmin, xmax)
                self.matplot_axes.set_ylim(ymin, ymax)
                self.__rescale = False

            # Autoscaling activated?
            elif self.display_autoscaling_checkbutton.get_active():
                if [xmin, xmax] != self.matplot_axes.get_xlim():
                    self.matplot_axes.set_xlim(xmin, xmax)

                # Rescale if new max is larger than old_ymax, simialar rules apply to ymin
                [self.__old_ymin, self.__old_ymax] = self.matplot_axes.get_ylim()
                ydiff=ymax-ymin
                if (self.__old_ymax < ymax or self.__old_ymin > ymin or
                    self.__old_ymax > ymax+0.2*ydiff or self.__old_ymin < ymin-0.2*ydiff):
                    self.matplot_axes.set_ylim(ymin, ymax)


            # Statistics activated?
            if self.display_statistics_checkbutton.get_active() and in_result.uses_statistics() and in_result.ready_for_drawing_error():
                # Real-Fehler
                self.graphen[2].set_data(in_result.get_xdata(), in_result.get_ydata(0) + in_result.get_yerr(0))
                self.graphen[3].set_data(in_result.get_xdata(), in_result.get_ydata(0) - in_result.get_yerr(0))
                # Img-Fehler
                self.graphen[4].set_data(in_result.get_xdata(), in_result.get_ydata(1) + in_result.get_yerr(1))
                self.graphen[5].set_data(in_result.get_xdata(), in_result.get_ydata(1) - in_result.get_yerr(1))

            # Any title to be set?
            if in_result.get_title() is not None:
                self.matplot_axes.set_title(in_result.get_title())
            else:
                self.matplot_axes.set_title("")


            # Any labels to be set?
            if in_result.get_xlabel() is not None:
                self.matplot_axes.set_xlabel(in_result.get_xlabel())
            else:
                self.matplot_axes.set_xlabel("")

            if in_result.get_ylabel() is not None:
                self.matplot_axes.set_ylabel(in_result.get_ylabel())
            else:
                self.matplot_axes.set_ylabel("")
                
               
            # Draw it!
            self.matplot_canvas.draw()

            gtk.gdk.threads_leave()
            return False

        finally:
            gtk.gdk.threads_leave()
        


    def watch_result(self, result, channel):
        "Interface to surface for watching some data (under a certain name)"
        # Copy result and channel
        self.__drawing_busy = True
        
        gobject.idle_add(self.watch_result_idle_func, result + 0, channel + "")
        

    def watch_result_idle_func(self, result, channel):
        gtk.gdk.threads_enter()

        try:
            # Check if channel exists or needs to be added
            if not self.__display_channels.has_key(channel):
                self.__display_channels[channel] = [ ]
                self.__display_channels[channel].insert(0,result) # inserts a refernce 
                self.display_source_combobox.append_text(channel)
            else:
                self.__display_channels[channel].insert(0,result) # inserts a refernce

                # Save at max 5 results per channel (implemented for History; "Watchpoint-managing class" should be implemented in future versions"
                if len(self.__display_channels[channel]) > 5: del self.__display_channels[channel][-1]

            # Getting active text in Combobox and compairing it
            if channel == self.display_source_combobox.get_active_text():
                self.draw_result(self.__display_channels[channel][0])

            self.__drawing_busy = False
                 
        finally:
            gtk.gdk.threads_leave()


    def new_log_message(self, text, source):
        "Adds a new line to the log-window"

        if not (str(source) == "ES" or str(source) == "DH" or str(source) == "GUI" or str(source) == "CORE"):
            print "Error DamarisGui.py (new_log_message(text, source)): Source \"%s\" unknown. Valid values: DH, ES, CORE, GUI\n" % str(source)
        else:
            #self.log_messages_textbuffer.set_modified(True)
            gobject.idle_add(self.new_log_message_idle_func, str(text), source)


    def new_log_message_idle_func(self, text, source):
        gtk.gdk.threads_enter()

        try:
            (sob, eob) = self.log_messages_textbuffer.get_bounds()
            if source == "ES":
                self.log_messages_textbuffer.insert_with_tags_by_name(eob, "[%s] " % str(datetime.datetime.now()), "NORMAL")
                (sob, eob) = self.log_messages_textbuffer.get_bounds()
                self.log_messages_textbuffer.insert_with_tags_by_name(eob, "Experiment Script:\n", "ES")
                (sob, eob) = self.log_messages_textbuffer.get_bounds()
                self.log_messages_textbuffer.insert_with_tags_by_name(eob, text + "\n\n", "NORMAL")
            elif source == "DH":
                self.log_messages_textbuffer.insert_with_tags_by_name(eob, "[%s] " % str(datetime.datetime.now()), "NORMAL")
                (sob, eob) = self.log_messages_textbuffer.get_bounds()
                self.log_messages_textbuffer.insert_with_tags_by_name(eob, "Data Handling Script:\n", "DH")
                (sob, eob) = self.log_messages_textbuffer.get_bounds()
                self.log_messages_textbuffer.insert_with_tags_by_name(eob, text + "\n\n", "NORMAL")             
            elif source == "CORE":
                self.log_messages_textbuffer.insert_with_tags_by_name(eob, "[%s] " % str(datetime.datetime.now()), "NORMAL")
                (sob, eob) = self.log_messages_textbuffer.get_bounds()
                self.log_messages_textbuffer.insert_with_tags_by_name(eob, "Backend:\n", "CORE")
                (sob, eob) = self.log_messages_textbuffer.get_bounds()
                self.log_messages_textbuffer.insert_with_tags_by_name(eob, text + "\n\n", "NORMAL")
            elif source == "GUI":
                self.log_messages_textbuffer.insert_with_tags_by_name(eob, "[%s] " % str(datetime.datetime.now()), "NORMAL")
                (sob, eob) = self.log_messages_textbuffer.get_bounds()
                self.log_messages_textbuffer.insert_with_tags_by_name(eob, "GUI:\n", "GUI")
                (sob, eob) = self.log_messages_textbuffer.get_bounds()
                self.log_messages_textbuffer.insert_with_tags_by_name(eob, text + "\n\n", "NORMAL")

            self.log_messages_tab_label.set_markup("<b>Log</b>")
            self.log_messages_lines_label.set_text("Lines in Buffer: %d" % self.log_messages_textbuffer.get_line_count())

            return False
                
        finally:
            gtk.gdk.threads_leave()


    def get_experiment_script(self):
        "Interface for getting the content of the experiment-script textarea"
        return self.experiment_script_textbuffer.get_text(self.experiment_script_textbuffer.get_start_iter(), self.experiment_script_textbuffer.get_end_iter())


    def get_data_handling_script(self):
        "Interface for getting the content of the data-handling-script textarea"
        return self.data_handling_textbuffer.get_text(self.data_handling_textbuffer.get_start_iter(), self.data_handling_textbuffer.get_end_iter())


    def connect_data_handler(self, data_handler):
        "Referencing the data-handler"
        self.data_handler = data_handler


    def connect_job_writer(self, job_writer):
        "Referencing job-writer"
        self.job_writer = job_writer

    def connect_core(self, core):
        "Referencing core"
        self.core_interface=core


    def show_error_dialog(self, title, error_message):
        "Displays an error dialog"

        if self.__error_dialogs_open > 2:
            self.new_log_message("Warning: Dropped error-dialog due too many opened windows.\nError Title: %s\nError Message: %s" % (title, error_message), "GUI")
            return None

        self.__error_dialogs_open += 1
        gobject.idle_add(self.show_error_dialog_idle_func, title + "", error_message + "")

    def show_error_dialog_idle_func(self, title, error_message):
        try:
            gtk.gdk.threads_enter()
            NiftyGuiElements.show_error_dialog(self.main_window, title, error_message)

            return False
            
        finally:
            self.__error_dialogs_open -= 1
            gtk.gdk.threads_leave()


    def get_selected_channel(self):
        "Returns the currently selected channel"
        return self.display_source_combobox.get_active_text()


    def busy_with_drawing(self):
        "Returns True if the surface is busy with drawing"
        return self.__drawing_busy

##    def flush(self):
##        gtk.gdk.threads_enter()
##        while gtk.gdk.events_pending():
##            gtk.main_iteration_do()
##        gtk.gdk.threads_leave()

