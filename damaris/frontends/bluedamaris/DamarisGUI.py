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

import NiftyGuiElements

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
        #self.display_statistics_checkbutton.set_sensitive(False)

        # Sonstiges:
        self.main_window = self.xml_gui.get_widget("main_window")
        self.main_notebook = self.xml_gui.get_widget("main_notebook")
        self.statusbar_label = self.xml_gui.get_widget("statusbar_label")
        self.main_clipboard = gtk.Clipboard(selection = "CLIPBOARD")
        self.display_settings_frame = self.xml_gui.get_widget("display_settings_frame")
        
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

        # Display:      
        self.display_source_combobox.connect("changed", self.display_source_changed)
        self.xml_gui.signal_connect("on_display_autoscaling_checkbutton_toggled", self.display_autoscaling_toggled)
        self.xml_gui.signal_connect("on_display_statistics_checkbutton_toggled", self.display_statistics_toggled)

        # Scripts:        
        self.experiment_script_textbuffer.connect("modified-changed", self.textviews_modified)
        self.experiment_script_textview.connect_after("move-cursor", self.textviews_moved)
        self.experiment_script_textview.connect("key-press-event", self.textviews_keypress)
        self.experiment_script_textview.connect("button-release-event", self.textviews_clicked)
        self.data_handling_textbuffer.connect("modified-changed", self.textviews_modified)
        self.data_handling_textview.connect_after("move-cursor", self.textviews_moved)
        self.data_handling_textview.connect("key-press-event", self.textviews_keypress)
        self.data_handling_textview.connect("button-release-event", self.textviews_clicked)  

        # Misc:
        self.main_window.connect("delete-event", self.quit_application)
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
        self.display_x_scaling_combobox.set_active(0)
        self.display_y_scaling_combobox.set_active(0)
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

    pi = 1e-3

    tau = 1e-3
    px = 0
    py = 90
    mx = 180
    my = 270

    while tau <= 5e-3:

        for accu in range(1):

            exp = Experiment()
            print "Job %d erstellt!" % exp.get_job_id()
            exp.set_description("tau", tau)

            exp.set_frequency(100e6, 0)

            exp.rf_pulse(0, pi/2)
            exp.wait(tau)
            exp.rf_pulse(0, pi)
            exp.wait(tau + 1e-6)

            exp.record(512, 1e6)

            yield exp

        tau += 1e-3""")

        self.experiment_script_textbuffer.set_modified(False)

        # For faster testing...
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

    acc = Accumulation(error = True)

    while 1:
        timesignal = outer_space.get_next_result()
        if timesignal is None: break
        
	acc = acc + timesignal

        print "Drawing %d..." % timesignal.get_job_id()
        outer_space.watch(timesignal, "Zeitsignal")
	outer_space.watch(acc, "Akkumulation")""")

        self.data_handling_textbuffer.set_modified(False)
        self.main_window.set_title(self.main_window_title % (self.experiment_script_textview.associated_filename, self.data_handling_textview.associated_filename))

        # Matplot (Display_Table, 1. Zeile) --------------------------------------------------------

        # Neue Abbildung erstellen
        self.matplot_figure = Figure()

        # Standartplot erstellen (1 Reihe, 1 Zeile, Beginnend beim 1.) und der Abbildung
        self.matplot_axes = self.matplot_figure.add_subplot(111)

        # Achsen beschriften & Gitternetzlinien sichtbar machen
        self.matplot_axes.set_xlabel("Time (s)")
        self.matplot_axes.set_ylabel("Samples (14-Bit)")
        self.matplot_axes.grid(True)

        # Ersten Plot erstellen und Referenz des ersten Eintrags im zur�ckgegebenen Tupel speichern
        # Voerst: graphen[0,1] = Real und Img-Kanal; [2,3] = Real-Fehler, [4,5] = Img-Fehler
        self.graphen = []
        self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "b-", linewidth = 2))
        self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "r-", linewidth = 2))
        self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "b-", linewidth = 0.5))
        self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "b-", linewidth = 0.5))
        self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "r-", linewidth = 0.5))
        self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "r-", linewidth = 0.5))


        self.matplot_axes.set_ylim([-8192.0, 8192.0])
        self.matplot_axes.set_xlim([0.0,1.0])

        # Lineare y-/x-Skalierung
        self.matplot_axes.set_yscale("linear")
        self.matplot_axes.set_xscale("linear")

        # Matplot in einen GTK-Rahmen stopfen
        self.matplot_canvas = FigureCanvas(self.matplot_figure)

        self.display_table = self.xml_gui.get_widget("display_table")
        self.display_table.attach(self.matplot_canvas, 0, 5, 0, 1, gtk.EXPAND | gtk.FILL, gtk.EXPAND | gtk.FILL, 0, 0)
        self.matplot_canvas.show()

        # Matplot Toolbar hinzuf�gen (Display_Table, 2. Zeile) 

        self.matplot_toolbar = NavigationToolbar(self.matplot_canvas, self.main_window)
        
        self.display_table.attach(self.matplot_toolbar, 0, 1, 1, 2, gtk.FILL | gtk.EXPAND, 0, 0, 0)
        self.matplot_toolbar.show()

        # /Mathplot --------------------------------------------------------------------------------

        self.display_source_combobox.set_active(0)

        self.main_window.show_all()
        

    # /Inits der einzelnen Fenster #################################################################        


    # Callbacks ####################################################################################

    def quit_application(self, widget, Data = None):
        "Callback for everything that quits the application"

        self.quit_timeout = 0.0

        # Contents changed?

        if self.experiment_script_textbuffer.get_modified() or self.data_handling_textbuffer.get_modified():
            answer = NiftyGuiElements.show_question_dialog_compulsive(self.main_window, "Unsaved changes", "Do you want so save your changes?")
            
            if answer == 0:
                self.save_all_files(None)
            elif answer == 2:
                self.save_all_files(None)

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


    def start_experiment(self, widget, Data = None):
        """Callback for the "Start-Experiment" button"""

        self.__experiment_running = True

        # Remove all entries except "None"
        self.display_source_combobox.get_model().clear()

        self.display_source_combobox.insert_text(0, "None")
        self.display_source_combobox.set_active(0)

        # Deleting all history
        self.__display_channels = { }
        
        try:
            self.experiment_script_statusbar_label.set_text("Experiment Script: Busy...")
            self.data_handling_statusbar_label.set_text("Data Handling: Busy...")
            
            self.toolbar_run_button.set_sensitive(False)
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
            

            # Delete existing job-files
            file_list = glob.glob(os.path.join(self.job_writer.get_job_writer_path(), "job*"))
            try:
                for job_file in file_list:
                    if job_file.find(".state") != -1:
                        continue
                    os.remove(job_file)

                print "\nWarning: Deleted %d job/result files" % len(file_list)
            except IOError, e:
                self.gui.show_error_dialog("File Error", "IOError: Cannot delete Job-Files" + str(e))
                return True

            # Waking both threads up
            self.core_interface.clear_job(0)
            print "Waking jw up"
            self.job_writer.wake_up()
            print "(re)starting core"
            self.core_interface.start()
            print "Waking dh up"
            self.data_handler.wake_up()

            gobject.timeout_add(100, self.sync_job_writer_data_handler)
            gobject.timeout_add(500, self.check_job_writer_data_handler_finished)
            
            return True
        except:
            # todo: stop successfully started threads
            self.__experiment_running = False
            raise
        

    def sync_job_writer_data_handler(self):
        "Timeout-Callback for synchronising Job-Writer and Data-Handler (for example restarting everthing if an error occured)"
        try:
            gtk.gdk.threads_enter()
            # Check if still parsing
            if self.job_writer.error_occured() == None:
                return True

            if self.data_handler.error_occured() == None:
                return True

            # Check if an error occured
            if self.job_writer.error_occured() == True or self.data_handler.error_occured() == True:
                self.job_writer.start_writing(False)
                self.data_handler.start_handling(False)            
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

            if not self.data_handler.is_busy() and not self.job_writer.is_busy():
                self.toolbar_run_button.set_sensitive(True)
                self.toolbar_stop_button.set_sensitive(False)
                
                self.toolbar_new_button.set_sensitive(True)
                self.toolbar_open_button.set_sensitive(True)
                self.toolbar_save_as_button.set_sensitive(True)

                self.menu_new_item.set_sensitive(True)
                self.menu_open_item.set_sensitive(True)
                self.menu_save_as_item.set_sensitive(True)
                
                
                self.__experiment_running = False
                self.__rescale = True

                print "\n\nDone."
                
                return False

            return True
            
        finally:
            gtk.gdk.threads_leave()



    def stop_experiment(self, widget, data = None):
        print "\nStopping Experiment... (Waiting for components to stop safely)\n"       

        self.core_interface.stop_queue()
        self.job_writer.stop_experiment()
        self.data_handler.stop_experiment()

        return True
    

    def open_file(self, widget, Data = None):
        "Callback for the open-file dialog"

        # Save changes made...
        if self.experiment_script_textbuffer.get_modified() or self.data_handling_textbuffer.get_modified():
            answer = NiftyGuiElements.show_question_dialog(self.main_window, "Unsaved changes", "Do you want to save your changes made to the scripts?")

            if answer == 0:
                # User wants to save
                self.save_all_files(widget)

            elif answer == 2:
                # User cancels
                return True

        # No changes made or user answered "No"

        def response(self, response_id, outer_space):
            if response_id == gtk.RESPONSE_OK:
                file_name = dialog.get_filename()
                if file_name is None:
                    return

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
                return
        
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

        self.main_window.set_title(self.main_window_title % (self.experiment_script_textview.associated_filename, self.data_handling_textview.associated_filename))

        return True



    def save_file(self, widget, Data = None):

        if self.main_notebook.get_current_page() == 0:
            if self.experiment_script_textview.associated_filename == "Unnamed":
                self.save_file_as(widget)
            else:
                script_file = file(self.experiment_script_textview.associated_filename, "w")
                script_file.write(self.experiment_script_textbuffer.get_text(self.experiment_script_textbuffer.get_start_iter(), self.experiment_script_textbuffer.get_end_iter()))
                script_file.close()
                self.experiment_script_textbuffer.set_modified(False)

        elif self.main_notebook.get_current_page() == 1:
            if self.data_handling_textview.associated_filename == "Unnamed":
                self.save_file_as(widget)
            else:
                script_file = file(self.data_handling_textview.associated_filename, "w")
                script_file.write(self.data_handling_textbuffer.get_text(self.data_handling_textbuffer.get_start_iter(), self.data_handling_textbuffer.get_end_iter()))
                script_file.close()
                self.data_handling_textbuffer.set_modified(False)
        else:
            pass
        
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
                
                script_file = file(file_name, "w")

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
                    
                
            else:
                return True
        
        # Determining the tab which is currently open
        if self.main_notebook.get_current_page() == 0:
            dialog = gtk.FileChooserDialog(title="Save Experiment Script As...", parent=self.main_window, action=gtk.FILE_CHOOSER_ACTION_SAVE, buttons = (gtk.STOCK_SAVE, gtk.RESPONSE_OK, gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL))
        elif self.main_notebook.get_current_page() == 1:
            dialog = gtk.FileChooserDialog(title="Save Data Handling As...", parent=self.main_window, action=gtk.FILE_CHOOSER_ACTION_SAVE, buttons = (gtk.STOCK_SAVE, gtk.RESPONSE_OK, gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL))

        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_select_multiple(False)

        # Event-Handler for responce-signal (when one of the button is pressed)
        dialog.connect("response", response, self)

        dialog.run()
        dialog.destroy()

        self.main_window.set_title(self.main_window_title % (self.experiment_script_textview.associated_filename, self.data_handling_textview.associated_filename))

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

        self.main_window.set_title(self.main_window_title % (self.experiment_script_textview.associated_filename, self.data_handling_textview.associated_filename))        
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
            linebegin=textbuffer.get_text(linestart_iter,cursor_iter)
            linebegin.expandtabs()
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
            linebegin=textbuffer.get_text(linestart_iter,spaceend_iter)
            linebegin.expandtabs()
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
        channel = self.display_source_combobox.get_active_text()

        # Event triggers when we init the box -> Catch Channel "None"
        if channel == "None":
            self.graphen[0].set_data([0], [0])
            self.graphen[1].set_data([0], [0])
            self.matplot_canvas.draw()
            return True
            
        self.draw_result(self.__display_channels[channel][0])
        return True


    def display_autoscaling_toggled(self, widget, Data = None):

        channel = self.display_source_combobox.get_active_text()

        if channel == "None":
            return True

        else:
            if self.display_autoscaling_checkbutton.get_active():
                self.matplot_axes.set_xlim(self.__display_channels[channel][0].get_xmin(), self.__display_channels[channel][0].get_xmax())
                self.matplot_axes.set_ylim(self.__display_channels[channel][0].get_ymin(), self.__display_channels[channel][0].get_ymax())
                self.matplot_canvas.draw()
        
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

    # Schnittstellen nach Au�en ####################################################################

    def draw_result(self, in_result):
        "Interface to surface for drawing results/accumulated results"
        gobject.idle_add(self.draw_result_idle_func, in_result)


    def draw_result_idle_func(self, in_result):
        gtk.gdk.threads_enter()

        try:
            self.graphen[0].set_data(in_result.get_xdata(), in_result.get_ydata(0))
            self.graphen[1].set_data(in_result.get_xdata(), in_result.get_ydata(1))
  
            if self.__rescale:

                self.matplot_axes.set_xlim(float(in_result.get_xmin()), float(in_result.get_xmax()))
                self.__rescale = False

            if self.display_autoscaling_checkbutton.get_active():

                self.matplot_axes.set_xlim(float(in_result.get_xmin()), float(in_result.get_xmax()))
                self.matplot_axes.set_ylim(float(in_result.get_ymin()), float(in_result.get_ymax()))

            if self.display_statistics_checkbutton.get_active() and in_result.uses_statistics() and in_result.ready_for_drawing_error():
                # Real-Fehler
                self.graphen[2].set_data(in_result.get_xdata(), in_result.get_ydata(0) + in_result.get_yerr(0))
                self.graphen[3].set_data(in_result.get_xdata(), in_result.get_ydata(0) - in_result.get_yerr(0))
                # Img-Fehler
                self.graphen[4].set_data(in_result.get_xdata(), in_result.get_ydata(1) + in_result.get_yerr(1))
                self.graphen[5].set_data(in_result.get_xdata(), in_result.get_ydata(1) - in_result.get_yerr(1))
            else:
                # Maybe theres a better place for deleting the error-lines
                # Real-Fehler
                self.graphen[2].set_data([0.0],[0.0])
                self.graphen[3].set_data([0.0],[0.0])
                # Img-Fehler
                self.graphen[4].set_data([0.0],[0.0])
                self.graphen[5].set_data([0.0],[0.0])

            self.matplot_canvas.draw()

            return False

        finally:
            gtk.gdk.threads_leave()
        


    def watch_result(self, result, channel):
        "Interface to surface for watching some data (under a certain name)"
     
        gobject.idle_add(self.watch_result_idle_func, result + 0, channel + "")

    def watch_result_idle_func(self, result, channel):
        gtk.gdk.threads_enter()

        try:
            # Check if channel exists or needs to be added
            if not self.__display_channels.has_key(channel):
                self.__display_channels[channel] = [ ]
                self.__display_channels[channel].insert(0,result)
                self.display_source_combobox.append_text(channel)
            else:
                self.__display_channels[channel].insert(0,result)  
            
            # Getting active text in Combobox and compairing it
            if channel == self.display_source_combobox.get_active_text():
                self.draw_result(self.__display_channels[channel][0])
                 
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
            print "\nWarning: Dropped error-dialog due too many opened windows.\nError Title: %s\nError Message: %s" % (title, error_message)
            return None

        self.__error_dialogs_open += 1
        gobject.idle_add(self.show_error_dialog_idle_func, title, error_message)

    def show_error_dialog_idle_func(self, title, error_message):
        try:
            gtk.gdk.threads_enter()
            NiftyGuiElements.show_error_dialog(self.main_window, title, error_message)

            return False
            
        finally:
            self.__error_dialogs_open -= 1
            gtk.gdk.threads_leave()
        

##    def flush(self):
##        gtk.gdk.threads_enter()
##        while gtk.gdk.events_pending():
##            gtk.main_iteration_do()
##        gtk.gdk.threads_leave()

