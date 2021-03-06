# import 3rd party modules
import time
import sys
import StringIO
import codecs
import os.path
import traceback
import tables
import compiler
import types
import xml.parsers.expat

import pygtk
pygtk.require("2.0")
import gtk
import gtk.glade
import gobject
import pango

import numarray
import matplotlib
import matplotlib.axes
import matplotlib.figure
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas
from matplotlib.backends.backend_gtk import NavigationToolbar2GTK as NavigationToolbar

# import our own stuff
import ExperimentWriter
import ExperimentHandling
import ResultReader
import ResultHandling
import BackendDriver
import DataPool
import Accumulation
import ADC_Result
import Drawable
import MeasurementResult

# global log facility, to be extended
def log(message):
    print "DamarisGUI", message

ExperimentHandling.log=log
ResultHandling.log=log
ResultReader.log=log
ExperimentWriter.log=log
BackendDriver.log=log
DataPool.log=log

class DamarisGUI:

    ExpScript_Display=0
    ResScript_Display=1
    Monitor_Display=2
    Log_Display=3
    Config_Display=4

    Edit_State=0
    Run_State=1
    Pause_State=2
    Stop_State=3
    Quit_State=4

    def __init__(self):

        gtk.gdk.threads_init()

        # state: edit, run, stop, quit
        # state transitions:
        # edit -> run|quit
        # run -> pause|stop
        # pause -> run|stop
        # stop -> edit
        self.state=DamarisGUI.Edit_State
        # script execution engines and backend driver
        self.si = None
        # produced and displayed data
        self.data = None
        
        self.glade_layout_init()
        # my notebook
        self.main_notebook = self.xml_gui.get_widget("main_notebook")
        
        self.sw=ScriptWidgets(self.xml_gui)

        self.toolbar_init()

        self.monitor=MonitorWidgets(self.xml_gui)

        self.config=ConfigTab(self.xml_gui)

        self.statusbar_init()

        self.main_window.show_all()

    def glade_layout_init(self):
        glade_file=os.path.join(os.path.dirname(__file__),"damaris.glade")
        self.xml_gui = gtk.glade.XML(glade_file)
        self.main_window = self.xml_gui.get_widget("main_window")
        self.main_window.connect("delete-event", self.quit_event)
        self.main_window.set_icon_from_file(os.path.join(os.path.dirname(__file__),"stock_snap-grid.png"))

    def statusbar_init(self):
        """
        experiment and result thread status, backend state
        """
        self.experiment_script_statusbar_label = self.xml_gui.get_widget("statusbar_experiment_script_label")
        self.data_handling_statusbar_label = self.xml_gui.get_widget("statusbar_data_handling_label")
        self.backend_statusbar_label = self.xml_gui.get_widget("statusbar_core_label")
    
    def toolbar_init(self):
        """
        buttons like save and run...
        """
        self.toolbar_stop_button = self.xml_gui.get_widget("toolbar_stop_button")
        self.toolbar_run_button = self.xml_gui.get_widget("toolbar_run_button")
        self.toolbar_pause_button = self.xml_gui.get_widget("toolbar_pause_button")
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
        # prolong lifetime of clipboard till the very end (avoid error message)
        self.main_clipboard = self.sw.main_clipboard
        gtk.gdk.threads_enter()
        gtk.main()
        gtk.gdk.threads_leave()

        self.si=None
        self.sw=None
        self.config=None
        self.xml_gui=None

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
            self.config=None
            self.sw=None
            self.monitor=None
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
        
        # something running?
        if self.si is not None:
            print "Last Experiment is not clearly stopped!"
        self.si = None

        # get config values:
        actual_config = self.config.get()
        if (not actual_config["start_backend"] and
            not actual_config["start_result_script"] and
            not actual_config["start_experiment_script"]):
            return

        self.data = None

        # prepare to run
        self.state=DamarisGUI.Run_State
        self.sw.disable_editing()
        self.toolbar_run_button.set_sensitive(False)
        self.toolbar_stop_button.set_sensitive(True)
        self.toolbar_pause_button.set_sensitive(True)
        self.toolbar_pause_button.set_active(False)

        # get scripts and start script interface
        exp_script, res_script=self.sw.get_scripts()
        if not actual_config["start_result_script"]:
            res_script=""
        if not actual_config["start_experiment_script"]:
            exp_script=""
        backend=actual_config["backend_executable"]
        if not actual_config["start_backend"]:
            backend=""
        
        self.monitor.observe_data_pool(self.data)
        
        # start experiment
        try:
            self.spool_dir=os.path.abspath(actual_config["spool_dir"])
            # setup script engines
            self.si=ScriptInterface(exp_script,
                                    res_script,
                                    backend,
                                    self.spool_dir,
                                    clear_jobs=actual_config["del_jobs_after_execution"],
                                    clear_results=actual_config["del_results_after_processing"])

            self.data=self.si.data
            # run frontend and script engines
            self.monitor.observe_data_pool(self.data)
            self.si.runScripts()
            # start data dump
            self.dump_filename=actual_config["data_pool_name"]
            self.dump_states(init=True)
        except Exception, e:
            print "ToDo evaluate exception",str(e), "at",traceback.extract_tb(sys.exc_info()[2])[-1][1:3]
            print "Full traceback:"
            traceback_file=StringIO.StringIO()
            traceback.print_tb(sys.exc_info()[2], None, traceback_file)
            print traceback_file.getvalue()
            traceback_file=None

            self.data=None
            if self.si is not None:
                still_running=filter(None,[self.si.exp_handling,self.si.res_handling,self.si.back_driver])
                for r in still_running:
                    r.quit_flag.set()
                print "waiting for threads stoping...",
                still_running=filter(None,[self.si.exp_handling, self.si.res_handling, self.si.back_driver])
                for t in still_running:
                    t.join()
                print "done"

            # cleanup
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
        if self.si.exp_handling is not None:
            self.experiment_script_statusbar_label.set_text("Experiment Script Running (0)")
        else:
            self.experiment_script_statusbar_label.set_text("Experiment Script Idle")
        if self.si.res_handling is not None:
            self.data_handling_statusbar_label.set_text("Result Script Running (0)")
        else:
            self.data_handling_statusbar_label.set_text("Result Script Idle")
            
        if self.si.back_driver is not None:
            self.backend_statusbar_label.set_text("Backend Running")
        else:
            self.backend_statusbar_label.set_text("Backend Idle")
            

        # and observe it...
        gobject.timeout_add(200,self.observe_running_experiment)
        dump_timeinterval=60*60 # in seconds
        self.dump_states_event_id=gobject.timeout_add(dump_timeinterval*1000,self.dump_states)

    def observe_running_experiment(self):
        """
        periodically look at running threads
        """
        # look at components and update them
        # test whether backend and scripts are done
        e=self.si.data.get("__recentexperiment",-1)+1
        r=self.si.data.get("__recentresult",-1)+1
        e_text=None
        r_text=None
        b_text=None
        if self.si.exp_handling is not None:
            if not self.si.exp_handling.isAlive():
                self.si.exp_handling.join()
                if self.si.exp_handling.raised_exception:
                    print "experiment script failed at line %d (function %s): %s"%(self.si.exp_handling.location[0],
                                                                                   self.si.exp_handling.location[1],
                                                                                   self.si.exp_handling.raised_exception)
                    print "Full traceback", self.si.exp_handling.traceback
                    e_text="Experiment Script Failed (%d)"%e
                else:
                    e_text="Experiment Script Finished (%d)"%e
                    print "experiment script finished"
                self.si.exp_handling = None
            else:
                e_text="Experiment Script Running (%d)"%e

        if self.si.res_handling is not None:
            if not self.si.res_handling.isAlive():
                self.si.res_handling.join()
                if self.si.res_handling.raised_exception:
                    print "result script failed at line %d (function %s): %s"%(self.si.res_handling.location[0],
                                                                               self.si.res_handling.location[1],
                                                                               self.si.res_handling.raised_exception)
                    print "Full traceback", self.si.res_handling.traceback
                    r_text="Result Script Failed (%d)"%r
                else:
                    r_text="Result Script Finished (%d)"%r
                self.si.res_handling = None
            else:
                r_text="Result Script Running (%d)"%r

        if self.si.back_driver is not None:
            if not self.si.back_driver.isAlive():
                b_text="Backend Finished"
                self.si.back_driver.join()
                self.si.back_driver = None

        gtk.gdk.threads_enter()
        if e_text:
            self.experiment_script_statusbar_label.set_text(e_text)
        if r_text:
                self.data_handling_statusbar_label.set_text(r_text)
        if b_text:
                self.backend_statusbar_label.set_text(b_text)
        gtk.gdk.threads_leave()

        still_running=filter(None,[self.si.exp_handling, self.si.res_handling, self.si.back_driver])

        if len(still_running)==0:
            print "all subprocesses ended"
            self.state = DamarisGUI.Stop_State

        if self.state == DamarisGUI.Stop_State:
            if len(still_running)!=0:
                print "subprocesses still running:", map(lambda s:s.getName(),still_running)
                return True

            if not self.dump_states_event_id is None:
                gobject.source_remove(self.dump_states_event_id)
            self.dump_states(compress=9)
            self.dump_states_event_id=None
            
            # now everything is stopped
            self.state=DamarisGUI.Edit_State
            gtk.gdk.threads_enter()
            self.sw.enable_editing()
            self.toolbar_run_button.set_sensitive(True)
            self.toolbar_stop_button.set_sensitive(False)
            self.toolbar_pause_button.set_sensitive(False)
            gtk.gdk.threads_leave()

            # keep data to display but throw away everything else
            self.si=None

            return False

        # or look at them again
        return True

    def dump_states(self, init=False, compress=None):
        """
        init: constructs basic structure of this file
        compress: optional argument for zlib compression 0-9
        """

        class dump_file_timeline(tables.IsDescription):
            time=tables.StringCol(length=len("YYYYMMDD HH:MM:SS"))
            experiments=tables.Int64Col()
            results=tables.Int64Col()

        if init:
            # dump all information to a file
            print "ToDo: move away old dump file"
            dump_file=tables.openFile(self.dump_filename,mode="w",title="DAMARIS experiment data")
            # write scripts
            scriptgroup=dump_file.createGroup("/","scripts","Used Scripts")
            if self.si.exp_script:
                dump_file.createArray(scriptgroup,"experiment_script", self.si.exp_script)
            if self.si.res_script:
                dump_file.createArray(scriptgroup,"result_script", self.si.res_script)
            if self.si.backend_executable:
                dump_file.createArray(scriptgroup,"backend_executable", self.si.backend_executable)
            if self.spool_dir:
                dump_file.createArray(scriptgroup,"spool_directory", self.spool_dir)
            timeline_table=dump_file.createTable("/","timeline", dump_file_timeline, title="Timeline of Experiment")
            timeline_row=timeline_table.row
            timeline_row["time"]=time.strftime("%Y%m%d %H:%M:%S")
            timeline_row["experiments"]=0
            timeline_row["results"]=0
            timeline_row.append()
            timeline_table.flush()
        else:
            # repack file
            os.rename(self.dump_filename,self.dump_filename+".bak")
            old_dump_file=tables.openFile(self.dump_filename+".bak",mode="r+")
            if "data_pool" in old_dump_file.root:
                old_dump_file.removeNode(where="/", name="data_pool", recursive=True)
            old_dump_file.copyFile(self.dump_filename)
            old_dump_file.close()
            old_dump_file=None
            os.remove(self.dump_filename+".bak")
            # prepare for update
            dump_file=tables.openFile(self.dump_filename,mode="r+")
            e=self.si.data.get("__recentexperiment",-1)+1
            r=self.si.data.get("__recentresult",-1)+1
            timeline_table=dump_file.root.timeline
            timeline_row=timeline_table.row
            timeline_row["time"]=time.strftime("%Y%m%d %H:%M:%S")
            timeline_row["experiments"]=e
            timeline_row["results"]=r
            timeline_row.append()
            timeline_table.flush()

        self.data.write_hdf5(dump_file, where="/", name="data_pool", compress=compress)
        
        dump_file.flush()
        dump_file.close()
        dump_file=None

        return True
        
    def pause_experiment(self, widget, data = None):
        """
        pause experiment execution (that means delay backend and let others run)
        """
        if self.si is None: return False
        pause_state=self.toolbar_pause_button.get_active()
        if pause_state:
            if self.state!=DamarisGUI.Run_State: return False
            if self.spool_dir is None: return False
            no=self.si.data.get("__recentresult",-1)+1
            result_pattern=os.path.join(self.spool_dir, "job.%09d.result")
            job_pattern=os.path.join(self.spool_dir, "job.%09d")
            while os.path.isfile(result_pattern%no):
                no+=1
            i=0
            self.pause_files=[]
            while i<3 and os.path.isfile(job_pattern%(no+i)):
                pause_file=(job_pattern%(no+i))+".pause"
                os.rename(job_pattern%(no+i), pause_file )
                self.pause_files.append(pause_file)
                i+=1
            self.state=DamarisGUI.Pause_State
            self.backend_statusbar_label.set_text("Backend Paused")
                
        else:
            if self.state!=DamarisGUI.Pause_State: return False
            self.state=DamarisGUI.Run_State
            for f in self.pause_files:
                os.rename(f, f[:-6])
            self.pause_files=None
            self.backend_statusbar_label.set_text("Backend Running")

    def stop_experiment(self, widget, data = None):
        if self.state in [DamarisGUI.Run_State, DamarisGUI.Pause_State]:
            if self.si is None: return
            still_running=filter(None,[self.si.exp_handling,self.si.res_handling,self.si.back_driver])
            for r in still_running:
                r.quit_flag.set()
            self.state=DamarisGUI.Stop_State
            self.toolbar_pause_button.set_sensitive(False)

class ScriptWidgets:

    def __init__(self, xml_gui):
        """
        initialize text widgets with text
        """
        self.xml_gui=xml_gui

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
        # modify script fonts
        self.experiment_script_textview.modify_font(pango.FontDescription("Courier 14"))
        self.data_handling_textview.modify_font(pango.FontDescription("Courier 14"))
        # clipboard
        self.main_clipboard = gtk.Clipboard(selection = "CLIPBOARD")

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
        self.toolbar_check_scripts_button = self.xml_gui.get_widget("toolbar_check_scripts_button")
        # events
        self.xml_gui.signal_connect("on_toolbar_open_file_button_clicked", self.open_file)
        self.xml_gui.signal_connect("on_toolbar_new_button_clicked", self.new_file)
        self.xml_gui.signal_connect("on_toolbar_save_as_button_clicked", self.save_file_as)
        self.xml_gui.signal_connect("on_toolbar_save_file_button_clicked", self.save_file)
        self.xml_gui.signal_connect("on_toolbar_save_all_button_clicked", self.save_all_files)
        self.xml_gui.signal_connect("on_toolbar_check_scripts_button_clicked",self.check_script)

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
                                                              self.experiment_script_textbuffer.get_end_iter()).rstrip()
        res_script=self.data_handling_textbuffer.get_text(self.data_handling_textbuffer.get_start_iter(),
                                                                 self.data_handling_textbuffer.get_end_iter()).rstrip()
        return (exp_script, res_script)

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
            self.toolbar_check_scripts_button.set_sensitive(True)
        else:
            # disable toolbar
            self.toolbar_new_button.set_sensitive(False)
            self.toolbar_open_button.set_sensitive(False)
            self.toolbar_save_button.set_sensitive(False)
            self.toolbar_save_as_button.set_sensitive(False)
            self.toolbar_save_all_button.set_sensitive(False)
            self.toolbar_check_scripts_button.set_sensitive(False)

    # text widget related events

    def check_script(self, widget, data=None):
        if not self.editing_state: return 0
        
        current_page=self.main_notebook.get_current_page()
        if not current_page in [0,1]: return 0
        script=self.get_scripts()[current_page]
        try:
            compiler.parse(script)
        except SyntaxError, se:
            print "Syntax Error:", str(se), se.lineno, se.offset,"(ToDo: Dialog)"
            if current_page==0:
                new_place=self.experiment_script_textbuffer.get_iter_at_line_offset(se.lineno-1, se.offset-1)
                self.experiment_script_textbuffer.place_cursor(new_place)
                self.experiment_script_textview.scroll_to_iter(new_place, 0.2, False, 0,0)
            elif current_page==1:
                new_place=self.data_handling_textbuffer.get_iter_at_line_offset(se.lineno-1, se.offset-1)
                self.data_handling_textbuffer.place_cursor(new_place)
                self.data_handling_textview.scroll_to_iter(new_place, 0.2, False, 0,0)
        except Exception, e:
            print "Compilation Error:", str(e),"(ToDo: Dialog)"


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
        if event.state&gtk.gdk.CONTROL_MASK!=0:
            if event.keyval==gtk.gdk.keyval_from_name("c"):
                if self.main_notebook.get_current_page() == 0:
                    self.experiment_script_textbuffer.copy_clipboard(self.main_clipboard)
                elif self.main_notebook.get_current_page() == 1:
                    self.data_handling_textbuffer.copy_clipboard(self.main_clipboard)
                return True
            elif event.keyval==gtk.gdk.keyval_from_name("x"):
                # cut_clipboard(clipboard, textview editable?)
                if self.main_notebook.get_current_page() == 0:
                    self.experiment_script_textbuffer.cut_clipboard(self.main_clipboard, True)
                elif self.main_notebook.get_current_page() == 1:
                    self.data_handling_textbuffer.cut_clipboard(self.main_clipboard, True)
                return True
            elif event.keyval==gtk.gdk.keyval_from_name("v"):
                # paste_clipboard(clipboard, textpos (None = Cursor), textview editable?)
                if self.main_notebook.get_current_page() == 0:
                    self.experiment_script_textbuffer.paste_clipboard(self.main_clipboard, None, True)
                elif self.main_notebook.get_current_page() == 1:
                    self.data_handling_textbuffer.paste_clipboard(self.main_clipboard, None, True)
                return True
            return 0

        # indent helpers
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


class ConfigTab:
    """
    by now all values are saved in the GUI widgets
    """

    defaultfilename="damaris_config.xml"

    def __init__(self, xml_gui):
        self.xml_gui=xml_gui

        self.config_start_backend_checkbutton=self.xml_gui.get_widget("start_backend_checkbutton")
        self.config_backend_executable_entry=self.xml_gui.get_widget("backend_executable_entry")
        self.config_spool_dir_entry=self.xml_gui.get_widget("spool_dir_entry")
        self.config_start_experiment_script_checkbutton=self.xml_gui.get_widget("start_experiment_script_checkbutton")
        self.config_start_result_script_checkbutton=self.xml_gui.get_widget("start_result_script_checkbutton")
        self.config_del_results_after_processing_checkbutton=self.xml_gui.get_widget("del_results_after_processing_checkbutton")
        self.config_del_job_after_execution_checkbutton=self.xml_gui.get_widget("del_job_after_execution_checkbutton")
        self.config_data_pool_name_entry=self.xml_gui.get_widget("data_pool_name_entry")
        if sys.platform[:5] == "linux":
            self.defaultfilename=os.path.expanduser("~/.damaris")

        self.xml_gui.signal_connect("on_config_save_button_clicked", self.save_config_handler)
        self.xml_gui.signal_connect("on_config_load_button_clicked", self.load_config_handler)

    def get(self):
        """
        returns a dictionary of actual config values
        """
        return {
            "start_backend": self.config_start_backend_checkbutton.get_active(),
            "start_result_script": self.config_start_result_script_checkbutton.get_active(),
            "start_experiment_script": self.config_start_experiment_script_checkbutton.get_active(),            
            "spool_dir": self.config_spool_dir_entry.get_text(), 
            "backend_executable" : self.config_backend_executable_entry.get_text(),
            "data_pool_name" : self.config_data_pool_name_entry.get_text(),
            "del_results_after_processing" : self.config_del_results_after_processing_checkbutton.get_active(),
            "del_jobs_after_execution" : self.config_del_job_after_execution_checkbutton.get_active()
            }

    def set(self, config):
        if "start_backend" in config:
            self.config_start_backend_checkbutton.set_active(config["start_backend"])
        if "start_experiment_script" in config:
            self.config_start_experiment_script_checkbutton.set_active(config["start_experiment_script"])
        if "start_result_script" in config:
            self.config_start_result_script_checkbutton.set_active(config["start_result_script"])
        if "spool_dir" in config:
            self.config_spool_dir_entry.set_text(config["spool_dir"])
        if "backend_executable" in config:
            self.config_backend_executable_entry.set_text(config["backend_executable"])
        if "del_results_after_processing_checkbutton" in config:
            self.config_del_results_after_processing_checkbutton.set_active(config["del_results_after_processing_checkbutton"])
        if "del_job_after_execution_checkbutton" in config:
            self.config_del_job_after_execution_checkbutton.set_active(config["del_job_after_execution_checkbutton"])

    def load_config_handler(self, widget):
        self.load_config()

    def save_config_handler(self, widget):
        self.save_config()

    def load_config(self, filename=None):
        """
        set config from an xml file
        """
        if filename is None:
            filename=self.defaultfilename

        # parser functions
        def start_element(name, attrs, config):
            if name == "config" and "key" in attrs:
                config["__this_key__"]=attrs["key"]
                if "type" in attrs:
                    config["__this_type__"]=attrs["type"]
                config[attrs["key"]]=""
        
        def end_element(name, config):
            if "__this_type__" in config and "__this_key__" in config:
                if config["__this_type__"] == "Boolean":
                    if config[config["__this_key__"]] == "True":
                        config[config["__this_key__"]]=True
                    else:
                        config[config["__this_key__"]]=False
                elif config["__this_type__"] == "Integer":
                    config[config["__this_key__"]]=int(config[config["__this_key__"]])

            if "__this_type__" in config:
                del config["__this_type__"]
            if "__this_key__" in config:
                del config["__this_key__"]

        
        def char_data(data, config):
            if "__this_key__" in config:
                config[config["__this_key__"]]+=data

        # parse file contents to dictionary
        config={}
        p = xml.parsers.expat.ParserCreate()
        p.StartElementHandler = lambda n,a: start_element(n,a, config)
        p.EndElementHandler = lambda n: end_element(n, config)
        p.CharacterDataHandler = lambda d: char_data(d, config)
        p.ParseFile(file(filename,"r"))

        self.set(config)
        

    def save_config(self, filename=None):
        """
        write config as an xml file
        """
        if filename is None:
            filename=self.defaultfilename
        configfile=file(filename, "w")
        configfile.write("<?xml version='1.0'?>\n")
        configfile.write("<damaris>\n")
        config=self.get()
        for k,v in config.iteritems():
            val=""
            typename=""
            if type(v) is types.BooleanType:
                typename="Boolean"
                if v:
                    val="True"
                else:
                    val="False"
            if type(v) is types.StringType:
                typename="String"
                val=v
            configfile.write("  <config key='%s' type='%s'>%s</config>\n"%(k, typename, val))
        configfile.write("</damaris>\n")
        
        

class MonitorWidgets:
    
    def __init__(self, xml_gui):
        """
        initialize matplotlib widgets and stuff around
        """

        self.xml_gui=xml_gui
        self.main_window = self.xml_gui.get_widget("main_window")
        self.display_settings_frame = self.xml_gui.get_widget("display_settings_frame")

        # Display footer:
        self.display_x_scaling_combobox = self.xml_gui.get_widget("display_x_scaling_combobox")
        self.display_x_scaling_combobox.set_sensitive(False)
        self.display_y_scaling_combobox = self.xml_gui.get_widget("display_y_scaling_combobox")
        self.display_y_scaling_combobox.set_sensitive(False)
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

        # display source
        self.display_source_liststore = gtk.ListStore(gobject.TYPE_STRING)
        self.display_source_combobox = gtk.ComboBox(self.display_source_liststore)

        self.display_source_cell = gtk.CellRendererText()
        self.display_source_combobox.pack_start(self.display_source_cell, True)
        self.display_source_combobox.add_attribute(self.display_source_cell, 'text', 0)
        self.display_source_liststore.append(['None'])
        self.display_source_combobox.set_active(0)
        self.display_source_combobox.set_add_tearoffs(1)
        self.display_settings_frame.attach(self.display_source_combobox, 1, 2, 0, 1,  gtk.FILL, gtk.FILL, 3, 0)

        # display scaling
        self.display_x_scaling_combobox.set_active(0)
        self.display_y_scaling_combobox.set_active(0)
        self.display_x_scaling_combobox.set_sensitive(False)
        self.display_y_scaling_combobox.set_sensitive(False)

        # and events...
        self.display_source_combobox.connect("changed", self.display_source_changed_event)
        self.xml_gui.signal_connect("on_display_autoscaling_checkbutton_toggled", self.display_autoscaling_toggled)
        self.xml_gui.signal_connect("on_display_statistics_checkbutton_toggled", self.display_statistics_toggled)
        #self.xml_gui.signal_connect("on_display_x_scaling_combobox_changed", self.display_x_scaling_changed)
        #self.xml_gui.signal_connect("on_display_y_scaling_combobox_changed", self.display_y_scaling_changed)
        self.xml_gui.signal_connect("on_display_save_data_as_text_button_clicked", self.save_display_data_as_text)

        # data to observe
        self.data_pool=None
        # name of displayed data and reference to data
        self.displayed_data=[None,None]
        self.__rescale=True
        self.update_counter=0

    def observe_data_pool(self, data_pool):
        """
        register a listener and save reference to data
        assume to be in gtk/gdk lock
        """
        if not self.data_pool is None:
            # maybe some extra cleanup needed
            print "ToDo: cleanup widgets"
            if self.displayed_data[1] is not None and "unregister_listener" in dir(self.displayed_data[1]):
                    self.displayed_data[1].unregister_listener(self.datastructures_listener)
                    self.displayed_data[1]=None
            self.displayed_data=[None,None]
            self.data_pool.unregister_listener(self.datapool_listener)
            self.data_pool=None

            self.display_source_liststore.clear()
            self.display_source_liststore.append(['None'])
            self.display_source_combobox.set_active(0)
            self.clear_display()
            self.update_counter=0

        if data_pool is not None:
            # keep track of data
            self.data_pool=data_pool
            self.data_pool.register_listener(self.datapool_listener)

        # display states
        self.__rescale=True
        self.displayed_data=[None,None]

    #################### observing data structures and produce idle events

    def datapool_listener(self, event):
        """
        sort data as fast as possible and get rid of non-interesting data
        """
        if event.subject[:2]=="__": return
        if event.what==DataPool.DataPool.Event.updated_value:
            if self.displayed_data[0] is None or event.subject!=self.displayed_data[0]:
                # do nothing, forget it
                return
            if self.displayed_data[1] is self.data_pool[event.subject]:
                # oh, another category
                gobject.idle_add(self.update_display_idle_event,self.displayed_data[0][:])
                return

        gobject.idle_add(self.datapool_idle_listener,event)
        
    def datastructures_listener(self, event):
        """
        do fast work selecting important events
        """
        if event.origin is not self.displayed_data[1]: return
        self.update_counter+=1
        gobject.idle_add(self.update_display_idle_event,self.displayed_data[0][:])

    ################### consume idle events

    def datapool_idle_listener(self,event):
        """
        here dictionary changes are done
        """
        if event.what==DataPool.DataPool.Event.updated_value:
            if (self.displayed_data[0] is not None and
                self.displayed_data[0]==event.subject):
                new_data_struct=self.data_pool[self.displayed_data[0]]
                if self.displayed_data[1] is new_data_struct:
                    # update display only
                    gtk.gdk.threads_enter()
                    try:
                        self.update_display()
                    finally:
                        gtk.gdk.threads_leave()
                else:
                    # unregister old one
                    if self.displayed_data[1] is not None and "unregister_listener" in dir(self.displayed_data[1]):
                        self.displayed_data[1].unregister_listener(self.datastructures_listener)
                        self.displayed_data[1]=None
                    # register new one
                    if "register_listener" in dir(new_data_struct):
                        new_data_struct.register_listener(self.datastructures_listener)
                    self.displayed_data[1]=new_data_struct
                    gtk.gdk.threads_enter()
                    try:
                        self.renew_display()
                    finally:
                        gtk.gdk.threads_leave()
                new_data_struct=None
        elif event.what==DataPool.DataPool.Event.new_key:
            # update combo-box by inserting and rely on consistent information
            gtk.gdk.threads_enter()
            self.display_source_liststore.append([event.subject])
            gtk.gdk.threads_leave()
        elif event.what==DataPool.DataPool.Event.deleted_key:
            # update combo-box by removing and rely on consistent information
            gtk.gdk.threads_enter()
            if (not self.displayed_data[0] is None and
                self.displayed_data[0]==event.subject):
                self.displayed_data=[None,None]
                self.display_source_combobox.set_active(0)
                self.clear_display()
            i=self.display_source_liststore.get_iter_first()
            while not i is None:
                if self.display_source_liststore.get(i,0)[0]==event.subject:
                    self.display_source_liststore.remove(i)
                    break
                i=self.display_source_liststore.iter_next(i)
            gtk.gdk.threads_leave()
        elif event.what==DataPool.DataPool.Event.destroy:
            gtk.gdk.threads_enter()
            self.display_source_liststore.clear()
            self.display_source_liststore.append(['None'])
            self.display_source_combobox.set_active(0)
            self.displayed_data=[None,None]
            self.clear_display()
            gtk.gdk.threads_leave()
        return

    def update_display_idle_event(self, subject=None):
        self.update_counter-=1
        if self.update_counter>10:
            print "update queue too long (%d>10): throwing away things"%self.update_counter
            return
        if self.displayed_data[0] is None or subject!=self.displayed_data[0]:
            return
        gtk.gdk.threads_enter()
        try:
            self.update_display()
        finally:
            gtk.gdk.threads_leave()


    ######################## events from buttons

    def display_source_changed_event(self, widget, data=None):
        ai=self.display_source_combobox.get_active_iter()
        new_data_name=str(self.display_source_liststore.get(ai,0)[0])
        if (self.displayed_data[0] is None and new_data_name=="None"): return
        if (self.displayed_data[0]==new_data_name): return
        if self.displayed_data[1] is not None and "unregister_listener" in dir(self.displayed_data[1]):
            self.displayed_data[1].unregister_listener(self.datastructures_listener)
            self.displayed_data[1]=None
            # register new one
        if new_data_name=="None":
            self.displayed_data=[None,None]
            self.clear_display()
        else:
            new_data_struct=self.data_pool[new_data_name]
            if "register_listener" in dir(new_data_struct):
                new_data_struct.register_listener(self.datastructures_listener)
            self.displayed_data=[new_data_name,new_data_struct]
            self.renew_display()

    def display_autoscaling_toggled(self, widget, data=None):
        if self.displayed_data[0] is not None:
            self.update_display(self.displayed_data[0][:])

    def display_statistics_toggled(self, widget, data=None):
        if self.displayed_data[0] is not None:
            self.update_display(self.displayed_data[0][:])

    def save_display_data_as_text(self, widget, data=None):
        """
        copy data to tmp file and show save dialog
        """
        data_to_save=self.displayed_data[:]
        if self.displayed_data[1] is None:
            # nothing to save
            return
        if "write_as_csv" not in dir(data_to_save[1]):
            log("do not know how to save %s of class/type %s"%(data_to_save[0],type(data_to_save[1])))
            return

        # save them to a temporary file (in memory)
        tmpdata=os.tmpfile()
        tmpdata.writeline("# saved from monitor as %s"%data_to_save[0])
        data_to_save[1].write_as_csv(tmpdata)
        
        # show save dialog
        def response(self, response_id, tmpfile):
            if response_id == gtk.RESPONSE_OK:
                file_name = dialog.get_filename()
                if file_name is None:
                    return True

                absfilename=os.path.abspath(file_name)
                if os.access(file_name, os.F_OK):
                    log("ToDo: Overwrite file question")

                textfile=file(absfilename,"w")
                tmpfile.seek(0)
                for l in tmpfile:
                    textfile.write(l)
                textfile.close()
                textfile=None
                tmpfile=None
                return True
        
        # Determining the tab which is currently open
        dialog_title="Save %s in file"%data_to_save[0]
        
        dialog = gtk.FileChooserDialog(title = dialog_title,
                                       parent = self.main_window,
                                       action = gtk.FILE_CHOOSER_ACTION_SAVE,
                                       buttons = (gtk.STOCK_SAVE, gtk.RESPONSE_OK, gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL))

        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_select_multiple(False)

        # Event-Handler for responce-signal (when one of the button is pressed)
        dialog.connect("response", response, tmpdata)
        del tmpdata, data_to_save
        dialog.run()
        dialog.destroy()

        return True

    ##################### functions to feed display

    def clear_display(self):
        """
        unconditionally throw away everything
        we are inside gtk/gdk lock
        """
        if "__rescale" not in dir(self):
            self.__rescale = True
        if "measurementresultgraph" not in dir(self):
            self.measurementresultgraph=None
        elif self.measurementresultgraph is not None:
            # clear errorbars
            self.matplot_axes.lines.remove(self.measurementresultgraph[0])
            for l in self.measurementresultgraph[1]:
                self.matplot_axes.lines.remove(l)
            self.measurementresultgraph=None
            self.matplot_axes.clear()
            self.matplot_axes.grid(True)
        if "graphen" not in dir(self):
            self.graphen=[]
        elif self.graphen:
            for l in self.graphen:
                self.matplot_axes.lines.remove(l)
            self.graphen=[]
            self.matplot_axes.clear()
            self.matplot_axes.grid(True)
        self.matplot_canvas.draw()

    def update_display(self, subject=None):
        """
        try to recycle labels, data, lines....
        assume, object is not changed
        we are inside gtk/gdk lock
        """
        in_result=self.data_pool[self.displayed_data[0]]
        if isinstance(in_result, Accumulation.Accumulation) or isinstance(in_result, ADC_Result.ADC_Result):
            # directly taken from bluedamaris
            if len(self.graphen)==0:
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "b-", linewidth = 2))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "r-", linewidth = 2))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "b-", linewidth = 0.5))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "b-", linewidth = 0.5))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "r-", linewidth = 0.5))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "r-", linewidth = 0.5))

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

            xdata=in_result.get_xdata()
            ydata0=in_result.get_ydata(0)
            ydata1=in_result.get_ydata(1)
            self.graphen[0].set_data(xdata, ydata0)
            self.graphen[1].set_data(xdata, ydata1)

            # Statistics activated?
            if (self.display_statistics_checkbutton.get_active() and
                in_result.uses_statistics() and in_result.ready_for_drawing_error()):
                # Real-Fehler
                self.graphen[2].set_data(xdata, ydata0 + in_result.get_yerr(0))
                self.graphen[3].set_data(xdata, ydata0 - in_result.get_yerr(0))
                # Img-Fehler
                self.graphen[4].set_data(xdata, ydata1 + in_result.get_yerr(1))
                self.graphen[5].set_data(xdata, ydata1 - in_result.get_yerr(1))
            else:
                # Maybe theres a better place for deleting the error-lines
                # Real-Fehler
                self.graphen[2].set_data([0.0],[0.0])
                self.graphen[3].set_data([0.0],[0.0])
                # Img-Fehler
                self.graphen[4].set_data([0.0],[0.0])
                self.graphen[5].set_data([0.0],[0.0])
            xdata=ydata0=ydata1=None

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
            in_result=None
            
        elif isinstance(in_result, MeasurementResult.MeasurementResult):
            # directly taken from bluedamaris
            # remove lines and error bars
            if self.measurementresultgraph is not None:
                self.matplot_axes.lines.remove(self.measurementresultgraph[0])
                for l in self.measurementresultgraph[1]:
                    self.matplot_axes.lines.remove(l)
                self.measurementresultgraph=None

            [k,v,e]=in_result.get_errorplotdata()
            # Initial rescaling needed?
            if self.__rescale or self.display_autoscaling_checkbutton.get_active():
                if len(k):
                    xmin=min(k)
                    xmax=max(k)
                    # fix range and scaling problems
                    if xmin==xmax: (xmin,xmax)=(xmin-1, xmin+1)
                    ymin=min(map(lambda i:v[i]-e[i],xrange(len(v))))
                    ymax=max(map(lambda i:v[i]+e[i],xrange(len(v))))
                    # fix range and scaling problems
                    if ymin==ymax: (ymin,ymax)=(ymin-1,ymin+1)
                    self.__rescale = False
                else:
                    xmin=-1
                    xmax=1
                    ymin=-1
                    ymax=1
                    self.__rescale = True
                
                self.matplot_axes.set_xlim(xmin, xmax)
                self.matplot_axes.set_ylim(ymin, ymax)

            # add error bars
            self.measurementresultgraph=self.matplot_axes.errorbar(x=k, y=v, yerr=e, fmt="b+")
            k=v=e=None

            # Any title to be set?
            title=in_result.get_title()+""
            if title is not None:
                self.matplot_axes.set_title(title)
            else:
                self.matplot_axes.set_title("")

            self.matplot_canvas.draw()
            in_result=None        

    def renew_display(self):
        """
        set all properties of display
        we are inside gtk/gdk lock
        """
        self.clear_display()
        to_draw=self.data_pool[self.displayed_data[0]]

        if to_draw is None: return
        self.update_display()


class ScriptInterface:
    
    def __init__(self, exp_script=None, res_script=None, backend_executable=None, spool_dir="spool", clear_jobs=True, clear_results=True):
        """
        run experiment scripts and result scripts
        """

        self.exp_script=exp_script
        self.res_script=res_script
        self.backend_executable=str(backend_executable)
        self.spool_dir=os.path.abspath(spool_dir)
        self.clear_jobs=clear_jobs
        self.clear_results=clear_results
        self.exp_handling=self.res_handling=None

        self.exp_writer=self.res_reader=self.back_driver=None
        if self.backend_executable is not None and self.backend_executable!="":
            self.back_driver=BackendDriver.BackendDriver(self.backend_executable, spool_dir, clear_jobs, clear_results)
            if self.exp_script: self.exp_writer=self.back_driver.get_exp_writer()
            if self.res_script: self.res_reader=self.back_driver.get_res_reader()
        elif self.exp_script and self.res_script:
            self.back_driver=None
            self.res_reader=ResultReader.BlockingResultReader(spool_dir, clear_jobs=self.clear_jobs, clear_results=self.clear_results)
            self.exp_writer=ExperimentWriter.ExperimentWriter(spool_dir, inform_last_job=self.res_reader)
        else:
            self.back_driver=None
            if self.exp_script: self.exp_writer=ExperimentWriter.ExperimentWriter(spool_dir)
            if self.res_script: self.res_reader=ResultReader.ResultReader(spool_dir, clear_jobs=self.clear_jobs, clear_results=self.clear_results)

        self.data=DataPool.DataPool()


    def runScripts(self):
        # get script engines
        self.exp_handling=self.res_handling=None
        if self.exp_script and self.exp_writer:
            self.exp_handling=ExperimentHandling.ExperimentHandling(self.exp_script, self.exp_writer, self.data)
        if self.res_script and self.res_reader:
            self.res_handling=ResultHandling.ResultHandling(self.res_script, self.res_reader, self.data)

        # start them
        if self.exp_handling: self.exp_handling.start()
        if self.back_driver is not None: self.back_driver.start()
        if self.res_handling: self.res_handling.start()

        self.exp_writer=self.res_reader=None


if __name__=="__main__":

    d=DamarisGUI()
    d.run()
