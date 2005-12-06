import time
import sys
import codecs
import os.path

import pygtk
pygtk.require("2.0")
import gtk
import gtk.glade
import gobject
import pango

import matplotlib
import matplotlib.axes
import matplotlib.figure
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas
from matplotlib.backends.backend_gtk import NavigationToolbar2GTK as NavigationToolbar

import script_interface

class DamarisGUI:

    ExpScript_Display=1
    ResScript_Display=2
    Monitor_Display=3
    Log_Display=4

    Edit_State=0
    Run_State=1
    Pause_State=2
    Stop_State=3
    Quit_State=4

    def __init__(self):

        gtk.gdk.threads_init()
        #all my state variables
        # active display... (do I really need it?)
        self.active_display=DamarisGUI.ExpScript_Display

        # state: edit, run, stop, quit
        # state transitions:
        # edit -> run|quit
        # run -> pause|stop
        # pause -> run|stop
        # stop -> edit
        self.state=DamarisGUI.Edit_State
        
        self.glade_layout_init()
        # my notebook
        self.main_notebook = self.xml_gui.get_widget("main_notebook")
        
        self.sw=ScriptWidgets(self.xml_gui)

        self.toolbar_init()

        self.monitor_init()

        self.statusbar_init()

    def glade_layout_init(self):
        glade_file=os.path.join(os.path.dirname(__file__),"damaris.glade")
        self.xml_gui = gtk.glade.XML(glade_file)
        self.main_window = self.xml_gui.get_widget("main_window")

        self.main_window.connect("delete-event", self.quit_event)

    def statusbar_init(self):
        self.experiment_script_statusbar_label = self.xml_gui.get_widget("statusbar_experiment_script_label")
        self.data_handling_statusbar_label = self.xml_gui.get_widget("statusbar_data_handling_label")
        self.backend_statusbar_label = self.xml_gui.get_widget("statusbar_core_label")

    def monitor_init(self):
        """
        initialize matplotlib widget
        """

        # Display footer:
        self.display_x_scaling_combobox = self.xml_gui.get_widget("display_x_scaling_combobox")
        self.display_y_scaling_combobox = self.xml_gui.get_widget("display_y_scaling_combobox")
        self.display_source_combobox = gtk.combo_box_new_text()
        self.display_autoscaling_checkbutton = self.xml_gui.get_widget("display_autoscaling_checkbutton")
        self.display_statistics_checkbutton = self.xml_gui.get_widget("display_statistics_checkbutton")

        # insert monitor
        # Matplot (Display_Table, 1st Row) --------------------------------------------------------

        # Neue Abbildung erstellen
        self.matplot_figure = matplotlib.figure.Figure()

        # Standartplot erstellen (1 Reihe, 1 Zeile, Beginnend beim 1.) und der Abbildung
        self.matplot_axes = self.matplot_figure.add_subplot(111)

        # Achsen beschriften & Gitternetzlinien sichtbar machen
        self.matplot_axes.grid(True)

        # Ersten Plot erstellen und Referenz des ersten Eintrags im zurueckgegebenen Tupel speichern
        # Voerst: graphen[0,1] = Real und Img-Kanal; [2,3] = Real-Fehler, [4,5] = Img-Fehler
        #self.graphen = []
        #self.measurementresultgraph=None

        self.matplot_axes.set_ylim([0.0,1.0])
        self.matplot_axes.set_xlim([0.0,1.0])
        self.matplot_axes.set_autoscale_on(self.display_autoscaling_checkbutton.get_active())
        
        # Lineare y-/x-Skalierung
        self.matplot_axes.set_yscale("linear")
        self.matplot_axes.set_xscale("linear")

        # Matplot in einen GTK-Rahmen stopfen
        self.matplot_canvas = matplotlib.backends.backend_gtkagg.FigureCanvasGTKAgg(self.matplot_figure)

        self.display_table = self.xml_gui.get_widget("display_table")
        self.display_table.attach(self.matplot_canvas, 0, 6, 0, 1, gtk.EXPAND | gtk.FILL, gtk.EXPAND | gtk.FILL, 0, 0)
        self.matplot_canvas.show()

        # Matplot Toolbar hinzufuegen (Display_Table, 2. Zeile) 

        self.matplot_toolbar = matplotlib.backends.backend_gtk.NavigationToolbar2GTK(self.matplot_canvas, self.main_window)
        
        self.display_table.attach(self.matplot_toolbar, 0, 1, 1, 2, gtk.FILL | gtk.EXPAND, 0, 0, 0)
        self.matplot_toolbar.show()

        # /Mathplot --------------------------------------------------------------------------------

        self.display_source_combobox.set_active(0)
        self.display_source_combobox.set_add_tearoffs(1)
        self.display_x_scaling_combobox.set_active(0)
        self.display_y_scaling_combobox.set_active(0)
        self.display_x_scaling_combobox.set_sensitive(False)
        self.display_y_scaling_combobox.set_sensitive(False)

        # and events...
        pass
    
    def toolbar_init(self):
        """
        """
        self.toolbar_stop_button = self.xml_gui.get_widget("toolbar_stop_button")
        self.toolbar_run_button = self.xml_gui.get_widget("toolbar_run_button")
        self.toolbar_pause_button = self.xml_gui.get_widget("toolbar_pause_button")
        self.toolbar_check_scripts_button = self.xml_gui.get_widget("toolbar_check_scripts_button")
        self.toolbar_exec_with_options_togglebutton = self.xml_gui.get_widget("toolbar_execute_with_options_button")

        # prepare for edit state
        self.toolbar_run_button.set_sensitive(True)
        self.toolbar_stop_button.set_sensitive(False)
        self.toolbar_pause_button.set_sensitive(False)

        # and their events
        self.xml_gui.signal_connect("on_toolbar_run_button_clicked", self.start_experiment)
        self.xml_gui.signal_connect("on_toolbar_pause_button_toggled", self.pause_experiment)
        self.xml_gui.signal_connect("on_toolbar_stop_button_clicked", self.stop_experiment)
        self.xml_gui.signal_connect("on_toolbar_execute_with_options_button_clicked", self.start_experiment_with_options)

    def run(self):

        gtk.gdk.threads_enter()
        gtk.main()
        gtk.gdk.threads_leave()

        self.si=None
        self.sw=None

    # event handling: the real acitons in gui programming

    # first global events

    def quit_event(self, widget, data=None):
        """
        expecting quit event for main application
        """
        if self.state in [DamarisGUI.Edit_State, DamarisGUI.Quit_State]:
            self.state=DamarisGUI.Quit_State
            # do a cleanup...
            print "ToDo: Cleanup, Save Dialogs ..."
            # and quit
            gtk.main_quit()
            return False
        else:
            print "Stop Experiment please! (ToDo: Dialog)"
            return True

    # toolbar related events:

    def start_experiment_with_options(self, widget, data = None):
        print "ToDo: start_experiment_with_options"
        
    def start_experiment(self, widget, data = None):
        # prepare to run
        self.state=DamarisGUI.Run_State
        self.sw.disable_editing()
        self.toolbar_run_button.set_sensitive(False)
        self.toolbar_stop_button.set_sensitive(True)
        self.toolbar_pause_button.set_sensitive(True)
        self.toolbar_pause_button.set_active(False)

        # get scripts and start script interface
        exp_script, res_script=self.sw.get_scripts()

        try:
            spool_dir=os.path.abspath("spool")
            self.si=script_interface.ScriptInterface(exp_script,
                                                     res_script,
                                                     "/home/achim/damaris/backends/machines/Mobilecore.exe",
                                                     spool_dir)

            self.si.data.register_listener(self.datapool_listener)
            self.si.runScripts()
        except Exception, e:
            print "ToDo evaluate exception",str(e)
            still_running=filter(None,[self.si.exp_handling,self.si.res_handling,self.si.back_driver])
            for r in still_running:
                r.quit_flag.set()

            self.si=None
            self.state=DamarisGUI.Edit_State
            self.sw.enable_editing()
            self.toolbar_run_button.set_sensitive(True)
            self.toolbar_stop_button.set_sensitive(False)
            self.toolbar_pause_button.set_sensitive(False)
            self.toolbar_pause_button.set_active(False)
            return

        # switch to grapics
        self.main_notebook.set_current_page(2)

        # set running
        self.experiment_script_statusbar_label.set_text("Experiment Script Running (0)")
        self.data_handling_statusbar_label.set_text("Result Script Running (0)")
        self.backend_statusbar_label.set_text("Backend Running")

        # and observe it...
        gobject.timeout_add(200,self.observe_running_experiment)

    def observe_running_experiment(self):
        """
        periodically look at running threads
        """
        # look at components and update them
        # test whether backend and scripts are done
        if self.si.exp_handling is not None:
            e=self.si.data.get("__recentexperiment",-1)+1
            if not self.si.exp_handling.isAlive():
                self.si.exp_handling.join()
                if self.si.exp_handling.raised_exception:
                    print "experiment script failed at line %d (function %s): %s"%(self.si.exp_handling.location[0],
                                                                                   self.si.exp_handling.location[1],
                                                                                   self.si.exp_handling.raised_exception)
                    self.experiment_script_statusbar_label.set_text("Experiment Script Failed (%d)"%e)
                else:
                    self.experiment_script_statusbar_label.set_text("Experiment Script Finished (%d)"%e)
                    print "experiment script finished"
                self.si.exp_handling = None
            else:
                self.experiment_script_statusbar_label.set_text("Experiment Script Running (%d)"%e)


        if self.si.res_handling is not None:
            r=self.si.data.get("__recentresult",-1)+1
            if not self.si.res_handling.isAlive():
                self.si.res_handling.join()
                if self.si.res_handling.raised_exception:
                    print "result script failed at line %d (function %s): %s"%(self.si.res_handling.location[0],
                                                                               self.si.res_handling.location[1],
                                                                               self.si.res_handling.raised_exception)
                    self.data_handling_statusbar_label.set_text("Result Script Failed (%d)"%r)
                else:
                    self.data_handling_statusbar_label.set_text("Result Script Finished (%d)"%r)
                self.si.res_handling = None
            else:
                self.data_handling_statusbar_label.set_text("Result Script Running (%d)"%r)

        if self.si.back_driver is not None:
            if not self.si.back_driver.isAlive():
                self.backend_statusbar_label.set_text("Backend Finished")
                self.si.back_driver.join()
                self.si.back_driver = None

        still_running=filter(None,[self.si.exp_handling, self.si.res_handling, self.si.back_driver])

        if len(still_running)==0:
            print "all subprocesses ended"
            self.state = DamarisGUI.Stop_State

        if self.state == DamarisGUI.Stop_State:
            if len(still_running)!=0:
                print "subprocesses still running:", map(lambda s:s.getName(),still_running)
                return True
            self.state=DamarisGUI.Edit_State
            self.sw.enable_editing()
            self.toolbar_run_button.set_sensitive(True)
            self.toolbar_stop_button.set_sensitive(False)
            self.toolbar_pause_button.set_sensitive(False)
            # now everything is stopped
            return False

        # or look at them again
        return True

    def pause_experiment(self, widget, data = None):
        print "ToDo: pause_experiment"
        pause_state=self.toolbar_pause_button.get_active()
        if pause_state:
            self.state=DamarisGUI.Pause_State
        else:
            self.state=DamarisGUI.Run_State

    def stop_experiment(self, widget, data = None):
        if self.state in [DamarisGUI.Run_State, DamarisGUI.Pause_State]:
            still_running=filter(None,[self.si.exp_handling,self.si.res_handling,self.si.back_driver])
            for r in still_running:
                r.quit_flag.set()
            self.state=DamarisGUI.Stop_State
            self.toolbar_pause_button.set_sensitive(False)

    def datapool_listener(self, event):
        if event.subject[:2]!="__":
            print "Event:", event.subject
        return
        if event.subject=="__recentexperiment":
            e=event.origin.get("__recentexperiment",-1)+1
        if event.subject=="__recentresult":
            r=event.origin.get("__recentresult",-1)+1
#             if e!=0:
#                 ratio=100.0*r/e
#             else:
#                 ratio=100.0
#             print "\r%d/%d (%.0f%%)"%(r,e,ratio),

class ScriptWidgets:

    def __init__(self, xml_gui):
        self.xml_gui=xml_gui
        """
        initialize text widgets with text
        """

        # my states
        # editing enabled/disabled
        self.editing_state=True
        # keep in mind which filename was used for experiment script
        self.exp_script_filename=None
        # keep in mind which filename was used for result script
        self.res_script_filename=None
        
        # script buffers:
        self.experiment_script_textview = self.xml_gui.get_widget("experiment_script_textview")
        self.data_handling_textview = self.xml_gui.get_widget("data_handling_textview")
        self.experiment_script_textbuffer = self.experiment_script_textview.get_buffer()
        self.data_handling_textbuffer = self.data_handling_textview.get_buffer()

        # statusbar
        self.experiment_script_statusbar_label = self.xml_gui.get_widget("statusbar_experiment_script_label")
        self.data_handling_statusbar_label = self.xml_gui.get_widget("statusbar_data_handling_label")

        # footline with line and col indicators
        self.experiment_script_line_indicator=self.xml_gui.get_widget("experiment_script_line_textfield")
        self.experiment_script_column_indicator=self.xml_gui.get_widget("experiment_script_column_textfield")
        self.data_handling_line_indicator=self.xml_gui.get_widget("data_handling_line_textfield")
        self.data_handling_column_indicator=self.xml_gui.get_widget("data_handling_column_textfield")

        # some event handlers
        self.experiment_script_textbuffer.connect("modified-changed", self.textviews_modified)
        self.experiment_script_textview.connect_after("move-cursor", self.textviews_moved)
        self.experiment_script_textview.connect("key-press-event", self.textviews_keypress)
        self.experiment_script_textview.connect("button-release-event", self.textviews_clicked)
        self.data_handling_textbuffer.connect("modified-changed", self.textviews_modified)
        self.data_handling_textview.connect_after("move-cursor", self.textviews_moved)
        self.data_handling_textview.connect("key-press-event", self.textviews_keypress)
        self.data_handling_textview.connect("button-release-event", self.textviews_clicked)

        # and the editing toolbar part
        # buttons
        self.toolbar_new_button = self.xml_gui.get_widget("toolbar_new_button")
        self.toolbar_open_button = self.xml_gui.get_widget("toolbar_open_file_button")
        self.toolbar_save_button = self.xml_gui.get_widget("toolbar_save_file_button")
        self.toolbar_save_as_button = self.xml_gui.get_widget("toolbar_save_as_button")
        self.toolbar_save_all_button = self.xml_gui.get_widget("toolbar_save_all_button")
        # events
        self.xml_gui.signal_connect("on_toolbar_open_file_button_clicked", self.open_file)
        self.xml_gui.signal_connect("on_toolbar_new_button_clicked", self.new_file)
        self.xml_gui.signal_connect("on_toolbar_save_as_button_clicked", self.save_file_as)
        self.xml_gui.signal_connect("on_toolbar_save_file_button_clicked", self.save_file)
        self.xml_gui.signal_connect("on_toolbar_save_all_button_clicked", self.save_all_files)

        # my notebook
        self.main_notebook = self.xml_gui.get_widget("main_notebook")
        # config toolbar
        self.main_notebook.connect_after("switch_page", self.notebook_page_switched)
        
        # start with empty scripts
        self.set_scripts("","")
        self.enable_editing()


    # public methods

    def set_scripts(self, exp_script=None, res_script=None):
        # load buffers and set cursor to front
        if exp_script is not None:
            self.experiment_script_textbuffer.set_text(unicode(exp_script))
            self.experiment_script_textbuffer.place_cursor(self.experiment_script_textbuffer.get_start_iter())
            self.experiment_script_textbuffer.set_modified(False)
            self.textviews_moved(self.experiment_script_textview)
        if res_script is not None:
            self.data_handling_textbuffer.set_text(unicode(res_script))
            self.data_handling_textbuffer.place_cursor(self.data_handling_textbuffer.get_start_iter())
            self.data_handling_textbuffer.set_modified(False)
            self.textviews_moved(self.data_handling_textview)
        self.set_toolbuttons_status()

    def get_scripts(self):
        """
        returns scripts
        """
        exp_script=self.experiment_script_textbuffer.get_text(self.experiment_script_textbuffer.get_start_iter(),
                                                              self.experiment_script_textbuffer.get_end_iter())
        res_script=self.data_handling_textbuffer.get_text(self.data_handling_textbuffer.get_start_iter(),
                                                                 self.data_handling_textbuffer.get_end_iter())
        return (exp_script,res_script)

    def disable_editing(self):
        """
        disable editing (for running experiments)
        """
        # disable buffers
        self.editing_state=False
        self.experiment_script_textview.set_sensitive(False)
        self.data_handling_textview.set_sensitive(False)
        self.set_toolbuttons_status()

    def enable_editing(self):
        """
        returns to editable state
        """
        self.editing_state=True
        self.experiment_script_textview.set_sensitive(True)
        self.data_handling_textview.set_sensitive(True)
        self.set_toolbuttons_status()

    # methods to update status and appearance

    def set_toolbuttons_status(self):
        """
        
        ToDo: care about associated file names
        """
        current_page=self.main_notebook.get_current_page()
        if self.editing_state and current_page in [0,1]:
            self.toolbar_new_button.set_sensitive(True)
            self.toolbar_open_button.set_sensitive(True)
            # find visible tab
            exp_modified=self.experiment_script_textbuffer.get_modified()
            res_modified=self.data_handling_textbuffer.get_modified()
            enable_save=True
            if current_page==0:
                enable_save=exp_modified and self.exp_script_filename is not None
            elif current_page==1:
                enable_save=res_modified and self.res_script_filename is not None
            self.toolbar_save_button.set_sensitive(enable_save)
            self.toolbar_save_as_button.set_sensitive(True)
            self.toolbar_save_all_button.set_sensitive(exp_modified or res_modified)
        else:
            # disable toolbar
            self.toolbar_new_button.set_sensitive(False)
            self.toolbar_open_button.set_sensitive(False)
            self.toolbar_save_button.set_sensitive(False)
            self.toolbar_save_as_button.set_sensitive(False)
            self.toolbar_save_all_button.set_sensitive(False)

    # text widget related events

    def notebook_page_switched(self, notebook, page, pagenumber):
        self.set_toolbuttons_status()

    def textviews_modified(self, data = None):
        # mix into toolbar affairs
        self.set_toolbuttons_status()

    def textviews_clicked(self, widget, event):
        return self.textviews_moved(widget)

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

    def textviews_keypress(self, widget, event, data = None):
        """
        helpful tab and return key functions
        """
        # tab keyval 0xff09
        # backspace keyval 0xff08
        # to do check if modified event is called after all
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
                self.textviews_moved(widget)
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
            self.textviews_moved(widget)
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
            self.textviews_moved(widget)
            return 1
        
        self.textviews_moved(widget)
        return 0

    def open_file(self, widget, Data = None):
        """
        do the open file dialog, if necessary ask for save
        """
        # ignore
        if not self.editing_state: return 0
        
        # Determining the tab which is currently open
        current_page=self.main_notebook.get_current_page()
        if current_page == 0:
            open_dialog_title="Open Experiment Script..."
            modified=self.experiment_script_textbuffer.get_modified()
        elif current_page == 1:
            open_dialog_title="Open Result Script..."
            modified=self.data_handling_textbuffer.get_modified()
        else:
            return 0

        if modified:
            print "ToDo: Save First Dialog"

        def response(self, response_id, script_widget):
            if response_id == gtk.RESPONSE_OK:
                file_name = dialog.get_filename()
                if file_name is None:
                    return

                script_filename=os.path.abspath(file_name)
                if not os.access(script_filename, os.R_OK):
                    outer_space.show_error_dialog("File I/O Error","Cannot read from file %s" % script_filename)
                    return True

                script_file = file(script_filename, "r")
                script_string = u""
                for line in script_file:
                    script_string += unicode(line,encoding="iso-8859-1", errors="replace")
                script_file.close()

                if script_widget.main_notebook.get_current_page() == 0:    
                    script_widget.set_scripts(script_string,None)
                    script_widget.exp_script_filename=script_filename
                elif script_widget.main_notebook.get_current_page() == 1:
                    script_widget.set_scripts(None, script_string)
                    script_widget.res_script_filename=script_filename
                
            return True
        

        parent_window=self.xml_gui.get_widget("main_window")
        dialog = gtk.FileChooserDialog(title=open_dialog_title,
                                       parent=parent_window,
                                       action=gtk.FILE_CHOOSER_ACTION_OPEN,
                                       buttons = (gtk.STOCK_OPEN, gtk.RESPONSE_OK, gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL)
                                       )
        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_select_multiple(False)
        # Event-Handler for responce-signal (when one of the button is pressed)
        dialog.connect("response", response, self)

        dialog.run()
        dialog.destroy()

        # update title and so on...

        return True

    def save_file(self, widget = None, Data = None):
        """
        save file to associated filename
        """
        # ignore
        if not self.editing_state: return 0
        
        # Determining the tab which is currently open
        current_page=self.main_notebook.get_current_page()
        if current_page == 0:
            filename=self.exp_script_filename
        elif current_page == 1:
            filename=self.res_script_filename
        else:
            return 0

        if filename is None: return 0

        # save file
        if current_page==0:
            script=self.get_scripts()[0]
        elif current_page==1:
            script=self.get_scripts()[1]
        else:
            return 0

        # encode from unicode to iso-8859-1
        filecontents=codecs.getencoder("iso-8859-1")(script,"replace")[0]
        file(filename,"w").write(filecontents)

        if current_page == 0:
            self.experiment_script_textbuffer.set_modified(False)
        elif current_page == 1:
            self.data_handling_textbuffer.set_modified(False)
        self.set_toolbuttons_status()

        
    def save_file_as(self, widget = None, Data = None):

        def response(self, response_id, script_widget):
            if response_id == gtk.RESPONSE_OK:
                file_name = dialog.get_filename()
                if file_name is None:
                    return True

                absfilename=os.path.abspath(file_name)
                if os.access(file_name, os.F_OK):
                    print "ToDo: Overwrite file question"

                current_page=script_widget.main_notebook.get_current_page()
                if current_page==0:
                    script_widget.exp_script_filename=absfilename
                elif current_page==1:
                    script_widget.res_script_filename=absfilename
                script_widget.save_file()
                
                return True
        
        # Determining the tab which is currently open

        current_page=self.main_notebook.get_current_page()
        if  current_page == 0:
            dialog_title="Save Experiment Script As..."
        elif current_page == 1:
            dialog_title="Save Data Handling As..."
        else:
            return
        
        parent_window=self.xml_gui.get_widget("main_window")
        dialog = gtk.FileChooserDialog(title = dialog_title,
                                       parent = parent_window,
                                       action = gtk.FILE_CHOOSER_ACTION_SAVE,
                                       buttons = (gtk.STOCK_SAVE, gtk.RESPONSE_OK, gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL))

        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_select_multiple(False)

        # Event-Handler for responce-signal (when one of the button is pressed)
        dialog.connect("response", response, self)

        dialog.run()
        dialog.destroy()

        return True

    def save_all_files(self, widget, Data = None):

        current_page=self.main_notebook.get_current_page()
            
        # change page and call save dialog
        self.main_notebook.set_current_page(0)
        if self.exp_script_filename is None:
            self.save_file_as()
        else:
            self.save_file()

        self.main_notebook.set_current_page(1)
        if self.res_script_filename is None:
            self.save_file_as()
        else:
            self.save_file()

        self.main_notebook.set_current_page(current_page)

    def new_file(self, widget, Data = None):
        
        if not self.editing_state: return 0
        current_page=self.main_notebook.get_current_page()
        if current_page==0:
            if self.experiment_script_textbuffer.get_modified():
                print "ToDo: Save before Clear Dialog"
            self.set_scripts("", None)
            self.exp_script_filename=None
        elif current_page==1:
            if self.data_handling_textbuffer.get_modified():
                print "ToDo: Save before Clear Dialog"
            self.set_scripts(None, "")
            self.res_script_filename=None

if __name__=="__main__":

    d=DamarisGUI()
    d.run()
