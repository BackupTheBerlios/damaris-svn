# import native python modules
import time
import math
import sys
import platform
import cStringIO
import codecs
import os.path
import traceback
import tables
import types
import xml.parsers.expat
import threading
import webbrowser

# import 3rd party modules
# gui graphics
import gobject
gobject.threads_init()
import pygtk
pygtk.require("2.0")
import gtk
gtk_version_missmatch=gtk.check_version(2, 8, 0)
if gtk_version_missmatch:
    raise Exception("insufficient gtk version: "+gtk_version_missmatch)

import gtk.gdk
gtk.gdk.threads_init()
import gtk.glade

import pango
import cairo

# array math
import numpy

# math graphics
import matplotlib
# force noninteractive and use of numpy
matplotlib.rcParams["numerix"]="numpy"
matplotlib.rcParams["interactive"]="False"

if matplotlib.rcParams["backend"]=="GTK":
    from matplotlib.backends.backend_gtk import FigureCanvasGTK as FigureCanvas
    from matplotlib.backends.backend_gtk import NavigationToolbar2GTK as NavigationToolbar
    max_points_to_display=0 # no limit
elif matplotlib.rcParams["backend"]=="GTKAgg":
    from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas
    from matplotlib.backends.backend_gtkagg import NavigationToolbar2GTK as NavigationToolbar
    max_points_to_display=0 # no limit
else:
    # default
    from matplotlib.backends.backend_gtkcairo import FigureCanvasGTKCairo as FigureCanvas
    from matplotlib.backends.backend_gtkcairo import NavigationToolbar2GTK as NavigationToolbar
    max_points_to_display=1<<14

import matplotlib.axes
import matplotlib.figure
#for printing issues
if hasattr(gtk, "PrintOperation"):
    import matplotlib.backends.backend_cairo

# import our own stuff
from damaris.gui import ExperimentWriter, ExperimentHandling
from damaris.gui import ResultReader, ResultHandling
from damaris.gui import BackendDriver
#from damaris.data import Drawable # this is a base class, it should be used...
from damaris.data import DataPool, Accumulation, ADC_Result, MeasurementResult

# default, can be set to true from start script
debug=False

# version info
__version__="0.11-devel"

class logstream:
    gui_log=None
    text_log=sys.__stdout__

    def write(self, message):
        # default for stdout and stderr
        if self.gui_log is not None:
            self.gui_log(message)
        if debug or self.gui_log is None:
            self.text_log.write(message)
            self.text_log.flush()

    def __call__(self,message):
        self.write(message)

    def __del__(self):
        pass

global log
log=logstream()
sys.stdout=log
sys.stderr=log

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

    def __init__(self, exp_script_filename=None, res_script_filename=None):


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
        
        self.log=LogWindow(self.xml_gui)

        self.sw=ScriptWidgets(self.xml_gui)

        self.toolbar_init()

        self.documentation_init()

        self.monitor=MonitorWidgets(self.xml_gui)

        self.config=ConfigTab(self.xml_gui)

        exp_script=u""
        if exp_script_filename is not None and exp_script_filename!="":
            self.sw.exp_script_filename=exp_script_filename[:]
            if os.path.isfile(exp_script_filename) and os.access(exp_script_filename, os.R_OK):
                script_file = file(exp_script_filename, "rU")
                for line in script_file:
                    exp_script += unicode(line,encoding="iso-8859-15", errors="replace")
                script_file.close()

        res_script=u""
        if res_script_filename is not None and res_script_filename!="":
            self.sw.res_script_filename=res_script_filename[:]
            if os.path.isfile(res_script_filename) and os.access(res_script_filename, os.R_OK):
                script_file = file(res_script_filename, "rU")
                for line in script_file:
                    res_script += unicode(line,encoding="iso-8859-15", errors="replace")
                script_file.close()
        self.sw.set_scripts(exp_script, res_script)

        self.statusbar_init()

        self.main_window.show_all()
        self.main_window.present()

    def glade_layout_init(self):
        glade_file=os.path.join(os.path.dirname(__file__),"damaris.glade")
        self.xml_gui = gtk.glade.XML(glade_file)
        self.main_window = self.xml_gui.get_widget("main_window")
        self.main_window.connect("delete-event", self.quit_event)
        self.main_window.set_icon_from_file(os.path.join(os.path.dirname(__file__),"DAMARIS.png"))
        self.main_window.set_title(u"DAMARIS-%s"%__version__)
        
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

        # print button
        self.toolbar_print_button=self.xml_gui.get_widget("toolbar_print_button")
        if not hasattr(gtk, "PrintOperation"):
            self.toolbar_print_button.set_sensitive(False)
            print "Printing is not supported by GTK+ version in use"
        else:
            self.toolbar_print_button.set_sensitive(True)
            self.xml_gui.signal_connect("on_toolbar_print_button_clicked", self.print_button_switch)

        # prepare for edit state
        self.toolbar_run_button.set_sensitive(True)
        self.toolbar_stop_button.set_sensitive(False)
        self.toolbar_pause_button.set_sensitive(False)

        # and their events
        self.xml_gui.signal_connect("on_toolbar_run_button_clicked", self.start_experiment)
        self.xml_gui.signal_connect("on_toolbar_pause_button_toggled", self.pause_experiment)
        self.xml_gui.signal_connect("on_toolbar_stop_button_clicked", self.stop_experiment)
        self.xml_gui.signal_connect("on_doc_menu_activate",self.show_doc_menu)
        self.xml_gui.signal_connect("on_toolbar_manual_button_clicked",self.show_manual)

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
            self.log=None
            # and quit
            gtk.main_quit()
            return True
        else:
            print "Stop Experiment please! (ToDo: Dialog)"
            return True

    # toolbar related events:

    def start_experiment(self, widget, data = None):
        
        # something running?
        if self.si is not None:
            print "Last Experiment is not clearly stopped!"
        self.si = None

        # get config values:
        actual_config = self.config.get()

        # get scripts and start script interface
        self.sw.disable_editing()
        exp_script, res_script=self.sw.get_scripts()
        if not actual_config["start_result_script"]:
            res_script=""
        if not actual_config["start_experiment_script"]:
            exp_script=""
        backend=actual_config["backend_executable"]
        if not actual_config["start_backend"]:
            backend=""

        if (backend=="" and exp_script=="" and res_script==""):
            print "nothing to do...so doing nothing!"
            self.sw.enable_editing()
            return

        # check whether scripts are syntacticaly valid
        # should be merged with check script function
        exp_code=None
        if exp_script!="":
            try:
                exp_code=compile(exp_script, "Experiment Script", "exec")
            except SyntaxError, e:
                ln=e.lineno
                lo=e.offset
                if type(ln) is not types.IntType:
                    ln=0
                if type(lo) is not types.IntType:
                    lo=0
                print "Experiment Script: %s at line %d, col %d:"%(e.__class__.__name__, ln, lo)
                if e.text!="":
                    print "\"%s\""%e.text
                    # print " "*(e.offset+1)+"^" # nice idea, but needs monospaced fonts
                    pass
                print e

        res_code=None
        if res_script!="":
            try:
                res_code=compile(res_script, "Result Script", "exec")
            except SyntaxError, e:
                ln=e.lineno
                lo=e.offset
                if type(ln) is not types.IntType:
                    ln=0
                if type(lo) is not types.IntType:
                    lo=0
                print "Result script: %s at line %d, col %d:"%(e.__class__.__name__,ln, lo)
                if e.text!="":
                    print "\"%s\""%e.text
                    # print " "*(e.offset+1)+"^" # nice idea, but needs monospaced fonts
                    pass
                print e

        # detect error
        if (exp_script!="" and exp_code is None) or \
           (res_script!="" and res_code is None):
            self.main_notebook.set_current_page(DamarisGUI.Log_Display)
            self.sw.enable_editing()
            return

        # prepare to run
        self.state=DamarisGUI.Run_State
        self.toolbar_run_button.set_sensitive(False)
        self.toolbar_stop_button.set_sensitive(True)
        self.toolbar_pause_button.set_sensitive(True)
        self.toolbar_pause_button.set_active(False)

        # delete old data
        self.data = None
        self.monitor.observe_data_pool(self.data)
        
        # start experiment
        try:
            self.spool_dir=os.path.abspath(actual_config["spool_dir"])
            # setup script engines
            self.si=ScriptInterface(exp_code,
                                    res_code,
                                    backend,
                                    self.spool_dir,
                                    clear_jobs=actual_config["del_jobs_after_execution"],
                                    clear_results=actual_config["del_results_after_processing"])

            self.data=self.si.data
            # run frontend and script engines
            self.monitor.observe_data_pool(self.data)
            self.si.runScripts()
        except Exception, e:
            #print "ToDo evaluate exception",str(e), "at",traceback.extract_tb(sys.exc_info()[2])[-1][1:3]
            #print "Full traceback:"
            traceback_file=cStringIO.StringIO()
            traceback.print_tb(sys.exc_info()[2], None, traceback_file)
            self.main_notebook.set_current_page(DamarisGUI.Log_Display)
            print "Error while executing scripts: %s\n"%str(e)+traceback_file.getvalue()
            traceback_file=None

            self.data=None
            if self.si is not None:
                still_running=filter(None,[self.si.exp_handling,self.si.res_handling,self.si.back_driver])
                for r in still_running:
                    r.quit_flag.set()
                print "waiting for threads stoping...",
                still_running=filter(lambda x:x is not None and x.isAlive(),
                                     [self.si.exp_handling, self.si.res_handling, self.si.back_driver])
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
        self.main_notebook.set_current_page(DamarisGUI.Monitor_Display)

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

        # start data dump
        self.dump_thread=None
        self.save_thread=None
        self.dump_filename=""
        if actual_config["data_pool_name"]!="":
            self.dump_states(init=True)
        gobject.timeout_add(200, self.observe_running_experiment)

    def observe_running_experiment(self):
        """
        periodically look at running threads
        """
        # look at components and update them
        # test whether backend and scripts are done
        r=self.si.data.get("__recentresult",-1)+1
        b=self.si.data.get("__resultsinadvance",-1)+1
        e=self.si.data.get("__recentexperiment",-1)+1
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
                if self.si.back_driver.raised_exception:
                    b_text="Backend Failed"
                else:
                    b_text="Backend Finished"
                self.si.back_driver.join()
                self.si.back_driver = None
            else:
                b_text="Backend Running"
            if b!=0:
                b_text+=" (%d)"%b

        if self.dump_thread is not None:
            if self.dump_thread.isAlive():
                sys.stdout.write(".")
                self.dump_dots+=1
                if self.dump_dots>80:
                    print
                    self.dump_dots=0
            else:
                self.dump_thread.join()
                self.dump_thread=None
                print "done (%.01f s)"%(time.time()-self.dump_start_time)

        gtk.gdk.threads_enter()
        if e_text:
            self.experiment_script_statusbar_label.set_text(e_text)
        if r_text:
                self.data_handling_statusbar_label.set_text(r_text)
        if b_text:
                self.backend_statusbar_label.set_text(b_text)
        gtk.gdk.threads_leave()

        still_running=filter(None,[self.si.exp_handling, self.si.res_handling, self.si.back_driver, self.dump_thread])
        if len(still_running)==0:
            if self.save_thread is None and self.dump_filename!="":
                print "all subprocesses ended, saving data pool"
                # thread to save data...
                self.save_thread=threading.Thread(target=self.dump_states, name="dump states")
                self.save_thread.start()
                self.dump_start_time=time.time()
                self.dump_dots=0
            self.state = DamarisGUI.Stop_State

        if self.state == DamarisGUI.Stop_State:
            gtk.gdk.threads_enter()
            self.toolbar_pause_button.set_sensitive(False)
            self.toolbar_stop_button.set_sensitive(False)
            gtk.gdk.threads_leave()
            if len(still_running)!=0:
                print "subprocess(es) still running: "+', '.join(map(lambda s:s.getName(),still_running))
                return True
            else:
                if self.save_thread is not None:
                    if self.save_thread.isAlive():
                        sys.stdout.write(".")
                        self.dump_dots+=1
                        if self.dump_dots>80:
                            print
                            self.dump_dots=0
                        return True
                    self.save_thread.join()
                    self.save_thread=None
                    print "done (%.01f s)"%(time.time()-self.dump_start_time)

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

        # dump states?
        if self.dump_thread is None and self.dump_filename!="" and \
           self.dump_timeinterval!=0 and self.last_dumped+self.dump_timeinterval<time.time():
            print "dumping data pool"
            self.dump_start_time=time.time()
            self.dump_dots=0
            # thread to save data...
            self.dump_thread=threading.Thread(target=self.dump_states, name="dump states")
            self.dump_thread.start()

        # or look at them again
        return True

    def dump_states(self, init=False):
        """
        init: constructs basic structure of this file
        compress: optional argument for zlib compression 0-9
        """

        if init:
            actual_config = self.config.get()
            extensions_dict={"date": time.strftime("%Y-%m-%d"),
                             "datetime": time.strftime("%Y-%m-%d_%H%M"),
                             "exp": os.path.splitext(os.path.basename(self.sw.exp_script_filename))[0],
                             "res": os.path.splitext(os.path.basename(self.sw.res_script_filename))[0]}
            try:
                self.dump_filename=actual_config.get("data_pool_name")%extensions_dict
            except ValueError,e:
                print "invalid formating character in '"+actual_config.get("data_pool_name")+"'"
                self.dump_filename="DAMARIS_data_pool.h5"
                print "reseting dumpfile to "+self.dump_filename
            del extensions_dict

            self.dump_complib=actual_config.get("data_pool_complib",None)
            if self.dump_complib=="None":
                self.dump_complib=None
            if self.dump_complib is not None:
                self.dump_complevel=int(actual_config.get("data_pool_comprate",0))
            else:
                self.dump_complevel=0

            self.dump_timeinterval=60*60*10
            try:
                self.dump_timeinterval=int(60*float(actual_config["data_pool_write_interval"]))
            except ValueError,e:
                print "configuration provides non-number for dump interval: "+str(e)

            if os.path.split(self.dump_filename)[1]=="" or os.path.isdir(self.dump_filename):
                print "The dump filename is a directory, using filename 'DAMARIS_data_pool.h5'"
                self.dump_filename+=os.sep+"DAMARIS_data_pool.h5"

            # create necessary directories
            dir_stack=[]
            dir_trunk=os.path.dirname(os.path.abspath(self.dump_filename))
            while dir_trunk!="" and not os.path.isdir(dir_trunk):
                dir_stack.append(os.path.basename(dir_trunk))
                dir_trunk=os.path.dirname(dir_trunk)

            try:
                while len(dir_stack):
                    dir_trunk=os.path.join(dir_trunk, dir_stack.pop())
                    if os.path.isdir(dir_trunk): continue
                    os.mkdir(dir_trunk)
            except OSError, e:
                print e
                print "coud not create dump directory '%s', so hdf5 dumps disabled"%dir_trunk
                self.dump_filename=""
                self.dump_timeinterval=0
                return True

            # move away old file
            if os.path.isfile(self.dump_filename):
                # create bakup name pattern
                dump_filename_pattern=None
                (filename,ext)=os.path.splitext(self.dump_filename)
                if ext in [".h5", ".hdf", ".hdf5"]:
                    dump_filename_pattern=filename.replace("%","%%")+"_%d"+ext
                else:
                    dump_filename_pattern=self.dump_filename.replace("%","%%")+"_%d"
                    
                last_backup=0
                cummulated_size=os.stat(self.dump_filename).st_size
                while os.path.isfile(dump_filename_pattern%last_backup):
                    cummulated_size+=os.stat(dump_filename_pattern%last_backup).st_size
                    last_backup+=1
                while last_backup>0:
                    os.rename(dump_filename_pattern%(last_backup-1),dump_filename_pattern%last_backup)
                    last_backup-=1
                os.rename(self.dump_filename,dump_filename_pattern%0)
                if cummulated_size>(1<<30):
                    print "Warning: the cummulated backups size of '%s' is %d MByte"%(self.dump_filename,
                                                                                      cummulated_size/(1<<20))
                
            # dump all information to a file
            dump_file=tables.openFile(self.dump_filename,mode="w",title="DAMARIS experiment data")
            if dump_file.isUndoEnabled():
                dump_file.disableUndo()
            # write scripts
            scriptgroup=dump_file.createGroup("/","scripts","Used Scripts")
            exp_text, res_text=self.sw.get_scripts()
            if self.si.exp_script:
                dump_file.createArray(scriptgroup,"experiment_script", exp_text)
            if self.si.res_script:
                dump_file.createArray(scriptgroup,"result_script", res_text)
            if self.si.backend_executable:
                dump_file.createArray(scriptgroup,"backend_executable", self.si.backend_executable)
            if self.spool_dir:
                dump_file.createArray(scriptgroup,"spool_directory", self.spool_dir)
            timeline_tablecols=numpy.recarray(0,dtype=([("time","S17"),
                                                        ("experiments","int64"),
                                                        ("results","int64")]))
            timeline_table=dump_file.createTable("/","timeline", timeline_tablecols, title="Timeline of Experiment")
            timeline_row=timeline_table.row
            timeline_row["time"]=time.strftime("%Y%m%d %H:%M:%S")
            timeline_row["experiments"]=0
            timeline_row["results"]=0
            timeline_row.append()
            timeline_table.flush()
        else:
            # repack file
            os.rename(self.dump_filename, self.dump_filename+".bak")
            old_dump_file=tables.openFile(self.dump_filename+".bak", mode="r+")
            if "data_pool" in old_dump_file.root:
                old_dump_file.removeNode(where="/", name="data_pool", recursive=True)
            old_dump_file.copyFile(self.dump_filename)
            old_dump_file.close()
            old_dump_file=None
            os.remove(self.dump_filename+".bak")
            # prepare for update
            dump_file=tables.openFile(self.dump_filename, mode="r+")
            if dump_file.isUndoEnabled():
                dump_file.disableUndo()
            e=self.si.data.get("__recentexperiment",-1)+1
            r=self.si.data.get("__recentresult",-1)+1
            timeline_table=dump_file.root.timeline
            timeline_row=timeline_table.row
            timeline_row["time"]=time.strftime("%Y%m%d %H:%M:%S")
            timeline_row["experiments"]=e
            timeline_row["results"]=r
            timeline_row.append()
            timeline_table.flush()

        self.data.write_hdf5(dump_file, where="/", name="data_pool",
                             complib=self.dump_complib, complevel=self.dump_complevel)
        
        dump_file.flush()
        dump_file.close()
        self.last_dumped=time.time()
        del dump_file

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

    def print_button_switch(self, widget):
        """
        decides what to print... and prints, layout is done by responsible class
        """
        if not hasattr(gtk, "PrintOperation"):
            return

        # copied and modified from pygtk-2.10.1/examples/pygtk-demo/demos/print_editor.py

        print_ = gtk.PrintOperation()

        # will come from config
        settings=None
        if settings is not None:
            print_.set_print_settings(settings)

        page_setup=None
        if page_setup is not None:
            print_.set_default_page_setup(page_setup)


        #print_.set_property("allow_async",True)
        current_page=self.main_notebook.get_current_page()
        print_data = {}
        if current_page in [0,1]:
            print_.connect("begin_print", self.sw.begin_print, print_data)
            print_.connect("draw_page", self.sw.draw_page, print_data)
        elif current_page == 2:
            print_.connect("begin_print", self.monitor.begin_print, print_data)
            print_.connect("draw_page", self.monitor.draw_page, print_data)
        else:
            return
        
        try:
            res = print_.run(gtk.PRINT_OPERATION_ACTION_PRINT_DIALOG, self.main_window)
        except gobject.GError, ex:
            error_dialog = gtk.MessageDialog(self.main_window,
                                             gtk.DIALOG_DESTROY_WITH_PARENT,
                                             gtk._MESSAGE_ERROR,
                                             gtk.BUTTONS_CLOSE,
                                             ("Error printing file:\n%s" % str(ex)))
            error_dialog.connect("response", gtk.Widget.destroy)
            error_dialog.show()
        else:
            if res == gtk.PRINT_OPERATION_RESULT_APPLY:
                settings = print_.get_print_settings()

    def documentation_init(self):

        self.doc_urls={
            "Python DAMARIS": None,
            "DAMARIS Homepage": "http://www.fkp.physik.tu-darmstadt.de/damariswiki",
            "Python": "http://www.python.org/doc/%d.%d/"%(sys.version_info[:2]),
            "numpy": "http://www.scipy.org/Documentation#head-9013a0c8c345747e0b152f5125afe50b63177ad6",
            "scipy": "http://www.scipy.org/Documentation#head-737c779c5566aaed848449e5e365542664fb274a",
            "pytables": "http://www.pytables.org/docs/manual/",
            "DAMARIS backednds": None,
            "DAMARIS Repository": "http://element.fkp.physik.tu-darmstadt.de/cgi-bin/viewcvs.cgi/damaris/"
            }

        if os.path.isdir("/usr/share/doc/python%d.%d-doc/html"%(sys.version_info[:2])):
            self.doc_urls["Python"]="file:///usr/share/doc/python%d.%d-doc/html/index.html"%(sys.version_info[:2])

        if os.path.isdir("/usr/share/doc/python-tables-doc/html"):
            self.doc_urls["pytables"]="file:///usr/share/doc/python-tables-doc/html/index.html"

        doc_index_url=None
        # local installation
        installation_base=__file__
        for i in xrange(5):
            installation_base=os.path.dirname(installation_base)
        if os.path.isfile(os.path.join(installation_base, "share", "python-damaris", "doc", "index.html")):
            self.doc_urls["Python DAMARIS"]=os.path.join(installation_base, "share", "python-damaris", "doc", "index.html")
        elif os.path.isfile("/usr/share/doc/python-damaris/html/index.html"):
            # check generic debian location
            self.doc_urls["Python DAMARIS"]="file:///usr/share/doc/python-damaris/html/index.html"
        else:
            self.doc_urls["Python DAMARIS"]="http://www.fkp.physik.tu-darmstadt.de/damariswiki/Tutorial"

        self.doc_browser=None

    def show_doc_menu(self, widget, data=None):
        """
        offer a wide variety of docs, prefer local installations
        """
        if type(widget) is gtk.MenuItem:
            requested_doc=widget.get_child().get_text()
        else:
            requested_doc="Python DAMARIS"
                    
        if requested_doc in self.doc_urls and self.doc_urls[requested_doc] is not None:
            if self.doc_browser is not None:
                if not self.doc_browser.isAlive():
                    self.doc_browser.join()
                if self.doc_browser.my_webbrowser is not None:
                    print "new browser tab"
                    self.doc_browser.my_webbrowser.open_new_tab(self.doc_urls[requested_doc])
                else:
                    del self.doc_browser
                    self.doc_browser=start_browser(self.doc_urls[requested_doc])
                    self.doc_browser.start()
            else:
                self.doc_browser=start_browser(self.doc_urls[requested_doc])
                self.doc_browser.start()
        else:
            print "missing docs for '%s'"%(requested_doc)

    show_manual=show_doc_menu

class start_browser(threading.Thread):

    def __init__(self, url):
        threading.Thread.__init__(self, name="manual browser")
        self.my_webbrowser=None
        self.my_webbrowser_process=None
        self.start_url=url
                
    def run(self):
        """
        start a webbrowser
        """
        if sys.hexversion>=0x02050000:
            if sys.platform=="linux2" and self.my_webbrowser is None:
                # try for debian linux
                self.my_webbrowser=webbrowser.get("x-www-browser")
            if self.my_webbrowser is None:
                # this is what it should be everywhere!
                self.my_webbrowser=webbrowser.get()
            if self.my_webbrowser is not None:
                print "starting web browser (module webbrowser)"
                self.my_webbrowser.open(self.start_url)
                return True
        # last resort
        print "starting web browser (webbrowser.py)"
        self.my_webbrowser_process=os.spawnl(os.P_NOWAIT,
                                             sys.executable,
                                             os.path.basename(sys.executable),
                                             "-c",
                                             "import webbrowser\nwebbrowser.open('%s')"%self.start_url)
        return True

class LogWindow:
    """
    writes messages to the log window
    """

    def __init__(self, xml_gui):
        
        self.xml_gui=xml_gui
        self.textview=self.xml_gui.get_widget("messages_textview")
        self.textbuffer=self.textview.get_buffer()
        self.logstream=log
        self.logstream.gui_log=self
        self.last_timetag=None

    def __call__(self, message):
        timetag=time.time()
        gobject.idle_add(self.add_message_callback,timetag,message,priority=gobject.PRIORITY_LOW)

    def add_message_callback(self, timetag, message):
        date_tag=u""
        if self.last_timetag is None or (message!="\n" and self.last_timetag+60<timetag):
            self.last_timetag=timetag
            timetag_int=math.floor(timetag)
            timetag_ms=int((timetag-timetag_int)*1000)
            date_tag=time.strftime(u"%Y%m%d  %H:%M:%S.%%03d:\n",time.localtime(timetag_int))%timetag_ms
        gtk.gdk.threads_enter()
        self.textbuffer.place_cursor(self.textbuffer.get_end_iter())
        self.textbuffer.insert_at_cursor(date_tag+unicode(message))
        self.textview.scroll_to_mark(self.textbuffer.get_insert(),0.1)
        gtk.gdk.threads_leave()

    def __del__(self):
        self.logstream.gui_log=None
        self.logstream=None

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
        # script fonts are atlered by configuration
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
        # also listen to location values changed
        self.xml_gui.signal_connect("on_data_handling_column_textfield_value_changed",
                                    self.column_line_widgets_changed_event)
        self.xml_gui.signal_connect("on_data_handling_line_textfield_value_changed",
                                    self.column_line_widgets_changed_event)
        self.xml_gui.signal_connect("on_experiment_script_line_textfield_value_changed",
                                    self.column_line_widgets_changed_event)
        self.xml_gui.signal_connect("on_experiment_script_column_textfield_value_changed",
                                    self.column_line_widgets_changed_event)

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

        if self.exp_script_filename:
            exp_titlename=unicode(os.path.splitext(os.path.basename(self.exp_script_filename))[0])
        else:
            exp_titlename=u"unnamed"
        if self.res_script_filename:
            res_titlename=unicode(os.path.splitext(os.path.basename(self.res_script_filename))[0])
        else:
            res_titlename=u"unnamed"
        window_title=u"DAMARIS-%s %s,%s"%(__version__, exp_titlename, res_titlename)
        self.xml_gui.get_widget("main_window").set_title(window_title)

    # text widget related events

    def check_script(self, widget, data=None):
        if not self.editing_state: return 0
        
        current_page=self.main_notebook.get_current_page()
        if not current_page in [0,1]: return 0
        script=self.get_scripts()[current_page]
        try:
            compile(script, ("Experiment Script", "Result Script")[current_page], "exec")
        except SyntaxError, se:
            if current_page==0:
                tb=self.experiment_script_textbuffer
                tv=self.experiment_script_textview
            elif current_page==1:
                tb=self.data_handling_script_textbuffer
                tv=self.data_handling_script_textview

            ln=se.lineno
            lo=se.offset
            # reguard http://bugs.python.org/issue1778
            if type(ln) is not types.IntType:
                ln=0
            if type(lo) is not types.IntType:
                lo=0
            print "Syntax Error:\n%s in %s at line %d, offset %d"%(str(se), se.filename, ln, lo)+"\n(ToDo: Dialog)"
            if ln>0 and ln<=tb.get_line_count():
                new_place=tb.get_iter_at_line_offset(ln-1,0)
                if lo>0 and lo<=new_place.get_chars_in_line():
                    new_place.set_line_offset(lo)
                tb.place_cursor(new_place)
                tv.scroll_to_iter(new_place, 0.2, False, 0,0)
        except Exception, e:
            print "Compilation Error:\n"+str(e)+"\n(ToDo: Dialog)"

    def notebook_page_switched(self, notebook, page, pagenumber):
        self.set_toolbuttons_status()

    def column_line_widgets_changed_event(self, data=None):
        widget_name=data.name
        text_name=None
        if widget_name.startswith("data_handling"):
            text_name="data_handling"
        elif widget_name.startswith("experiment_script"):
            text_name="experiment_script"
        else:
            print "unknown line/column selector"
            return
        textview=self.__dict__[text_name+"_textview"]
        textbuffer=textview.get_buffer()
        newline=self.__dict__[text_name+"_line_indicator"].get_value_as_int()-1
        #if newline>textbuffer.get_end_iter().get_line():
        #    return
        new_place=textbuffer.get_iter_at_line(newline)
        newcol=self.__dict__[text_name+"_column_indicator"].get_value_as_int()-1
        if newcol>new_place.get_chars_in_line():
            new_place.forward_to_line_end()
        else:
            new_place.set_line_offset(newcol)
        # todo: find out whether chang was issued by program or by user
        
        textbuffer.place_cursor(new_place)
        textview.scroll_to_iter(new_place, 0.2, False, 0,0)
        textview.grab_focus()

    def textviews_modified(self, data = None):
        # mix into toolbar affairs
        self.set_toolbuttons_status()

    def textviews_clicked(self, widget, event):
        return self.textviews_moved(widget)

    def textviews_moved(self, widget, text=None, count=None, ext_selection=None, data = None):
        textbuffer=widget.get_buffer()
        cursor_mark=textbuffer.get_insert()
        cursor_iter=textbuffer.get_iter_at_mark(cursor_mark)
        # todo limits! indicator.set_range(1, ...)
        # fortunately set_text does not emit a change value event
        if textbuffer==self.experiment_script_textbuffer:
            self.experiment_script_line_indicator.set_text(str(cursor_iter.get_line()+1))
            self.experiment_script_line_indicator.set_range(1, textbuffer.get_end_iter().get_line()+1)
            self.experiment_script_column_indicator.set_text(str(cursor_iter.get_line_offset()+1))
            self.experiment_script_column_indicator.set_range(1,cursor_iter.get_chars_in_line()+1)
        if textbuffer==self.data_handling_textbuffer:
            self.data_handling_line_indicator.set_text(str(cursor_iter.get_line()+1))
            self.data_handling_line_indicator.set_range(1, textbuffer.get_end_iter().get_line()+1)
            self.data_handling_column_indicator.set_text(str(cursor_iter.get_line_offset()+1))
            self.data_handling_column_indicator.set_range(1,cursor_iter.get_chars_in_line()+1)
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
            elif event.keyval==gtk.gdk.keyval_from_name("s"):
                # save buffer
                page=self.main_notebook.get_current_page()
                if (self.exp_script_filename,self.res_script_filename)[page] is None:
                    self.save_file_as()
                else:
                    self.save_file()
                return True
            elif event.keyval==gtk.gdk.keyval_from_name("S"):
                # save both buffers
                print "ToDo: save both buffers"
                self.save_all_files(None, None)
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
                if (event.keyval==0xFF08):
                    self.textviews_moved(widget)
                    return 0
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

                script_file = file(script_filename, "rU")
                script_string = u""
                for line in script_file:
                    script_string += unicode(line,encoding="iso-8859-15", errors="replace")
                script_file.close()

                if script_widget.main_notebook.get_current_page() == 0:    
                    script_widget.exp_script_filename=script_filename
                    script_widget.set_scripts(script_string,None)
                elif script_widget.main_notebook.get_current_page() == 1:
                    script_widget.res_script_filename=script_filename
                    script_widget.set_scripts(None, script_string)
                
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

        # encode from unicode to iso-8859-15
        filecontents=codecs.getencoder("iso-8859-15")(script,"replace")[0]
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
            dialog_title="Save Result Script As..."
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
        self.set_toolbuttons_status()

    def begin_print(self, operation, context, print_data):
        """
        layout of all pages
        """
        # copied and modified from pygtk-2.10.1/examples/pygtk-demo/demos/print_editor.py

        # Determining the tab which is currently open
        current_page=self.main_notebook.get_current_page()
        script=""
        # get script text
        if current_page==0:
            script=self.get_scripts()[0]
        elif current_page==1:
            script=self.get_scripts()[1]
        
        width = context.get_width()
        height = context.get_height()
        layout = context.create_pango_layout()
        layout.set_font_description(pango.FontDescription("Monospace 12"))
        layout.set_width(int(width*pango.SCALE))
        layout.set_text(script)
        num_lines = layout.get_line_count()

        page_breaks = []
        page_height = 0

        for line in xrange(num_lines):
            layout_line = layout.get_line(line)
            ink_rect, logical_rect = layout_line.get_extents()
            lx, ly, lwidth, lheight = logical_rect
            line_height = lheight / 1024.0
            if page_height + line_height > height:
                page_breaks.append(line)
                page_height = 0
            page_height += line_height

        operation.set_n_pages(len(page_breaks) + 1)
        print_data["page_breaks"] = page_breaks
        print_data["layout"]=layout

    def draw_page(self, operation, context, page_nr, print_data):
        """
        render a single page
        """
        # copied and modified from pygtk-2.10.1/examples/pygtk-demo/demos/print_editor.py
        assert isinstance(print_data["page_breaks"], list)
        if page_nr == 0:
            start = 0
        else:
            start = print_data["page_breaks"][page_nr - 1]

        try:
            end = print_data["page_breaks"][page_nr]
        except IndexError:
            end = print_data["layout"].get_line_count()
    
        cr = context.get_cairo_context()

        cr.set_source_rgb(0, 0, 0)
  
        i = 0
        start_pos = 0
        iter = print_data["layout"].get_iter()
        while 1:
            if i >= start:
                line = iter.get_line()
                _, logical_rect = iter.get_line_extents()
                lx, ly, lwidth, lheight = logical_rect
                baseline = iter.get_baseline()
                if i == start:
                    start_pos = ly / 1024.0;
                cr.move_to(lx / 1024.0, baseline / 1024.0 - start_pos)
                cr.show_layout_line(line)
            i += 1
            if not (i < end and iter.next_line()):
                break

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
        self.config_del_jobs_after_execution_checkbutton=self.xml_gui.get_widget("del_jobs_after_execution_checkbutton")
        self.config_data_pool_name_entry=self.xml_gui.get_widget("data_pool_name_entry")
        self.config_data_pool_write_interval_entry=self.xml_gui.get_widget("data_pool_write_interval_entry")
        self.config_data_pool_complib=self.xml_gui.get_widget("CompLibs")
        self.config_data_pool_comprate=self.xml_gui.get_widget("CompRatio")
        self.config_info_textview=self.xml_gui.get_widget("info_textview")
        self.config_script_font_button=self.xml_gui.get_widget("script_fontbutton")
        self.config_printer_setup_button=self.xml_gui.get_widget("printer_setup_button")
        if not hasattr(gtk, "print_run_page_setup_dialog" ):
            self.config_printer_setup_button.set_sensitive(False)

        # insert version informations
        components_text=u"""
operating system %(os)s
gtk version %(gtk)s
glib version %(glib)s
python version %(python)s
matplotlib version %(matplotlib)s, using %(matplotlib_numerix)s, %(matplotlib_backend)s
numarray version %(numarray)s
numpy version %(numpy)s
pytables version %(pytables)s, using %(pytables_libs)s
pygtk version %(pygtk)s
pygobject version %(pygobject)s
"""
        if hasattr(gobject, "glib_version"):
            glib_version="%d.%d.%d"%gobject.glib_version
        else:
            glib_version="? (no pygobject module)"
        if hasattr(gobject, "pygobject_version"):
            pygobject_version="%d.%d.%d"%gobject.pygobject_version
        else:
            pygobject_version="? (no gobject version number)"

        numpy_version="none"
        try:
            import numpy
        except ImportError:
            pass
        else:
            numpy_version=numpy.__version__            

        numarray_version="none"
        try:
            import numarray
        except ImportError:
            pass
        else:
            numarray_version=numarray.__version__            

        components_versions = {
            "os":         platform.platform() ,
            "gtk":        "%d.%d.%d"%gtk.gtk_version,
            "glib":       glib_version,
            "python":     sys.version ,
            "matplotlib": matplotlib.__version__,
            "matplotlib_numerix": matplotlib.rcParams["numerix"],
            "matplotlib_backend": FigureCanvas.__name__[12:],
            "numarray":   numarray_version,
            "numpy":      numpy_version,
            "pytables":   tables.getPyTablesVersion(),
            "pytables_libs": "",
            "pygtk":      "%d.%d.%d"%gtk.pygtk_version,
            "pygobject":  pygobject_version
            }

        # pytables modules:
        # find compression extensions for combo box and write version numbers
        # list is taken from ValueError output of tables.whichLibVersion("")
        model=self.config_data_pool_complib.get_model()
        for  libname in ('hdf5', 'zlib', 'lzo', 'ucl', 'bzip2'):
            version_info=None
            try:
                version_info=tables.whichLibVersion(libname)
            except ValueError:
                continue
            if version_info:
                components_versions["pytables_libs"]+="\n  %s: %s"%(libname, str(version_info))
                if libname!="hdf5":
                    # a compression library, add it to combo box
                    if isinstance(model,gtk.ListStore):
                        model.append([libname])
                    elif isinstance(model,gtk.TreeStore):
                        model.append(None,[libname])
                    else:
                        print "cannot append compression lib name to %s"%model.__class__.__name__


        # debug message
        if debug:
            print "DAMARIS", __version__
            print components_text%components_versions
        
        # set no compression as default...
        self.config_data_pool_complib.set_active(0)

        info_textbuffer=self.config_info_textview.get_buffer()
        info_text=info_textbuffer.get_text(info_textbuffer.get_start_iter(),info_textbuffer.get_end_iter())
        info_text%={"moduleversions": components_text%components_versions, "damarisversion": __version__ }
        info_textbuffer.set_text(info_text)
        del info_textbuffer, info_text, components_text, components_versions
        
        if sys.platform[:5] == "linux":
            self.defaultfilename=os.path.expanduser("~/.damaris")

        self.xml_gui.signal_connect("on_config_save_button_clicked", self.save_config_handler)
        self.xml_gui.signal_connect("on_config_load_button_clicked", self.load_config_handler)
        self.xml_gui.signal_connect("on_backend_executable_browse_button_clicked",
                                    self.browse_backend_executable_dialog)
        self.xml_gui.signal_connect("on_fontbutton_font_set",self.set_script_font_handler)
        self.xml_gui.signal_connect("on_printer_setup_button_clicked", self.printer_setup_handler)

        if os.path.isfile(self.defaultfilename) and os.access(self.defaultfilename,os.R_OK):
            self.load_config()

    def get(self):
        """
        returns a dictionary of actual config values
        """
        complib_iter=self.config_data_pool_complib.get_active_iter()
        complib=self.config_data_pool_complib.get_model().get_value(complib_iter,0)
        return {
            "start_backend": self.config_start_backend_checkbutton.get_active(),
            "start_result_script": self.config_start_result_script_checkbutton.get_active(),
            "start_experiment_script": self.config_start_experiment_script_checkbutton.get_active(),            
            "spool_dir": self.config_spool_dir_entry.get_text(), 
            "backend_executable" : self.config_backend_executable_entry.get_text(),
            "data_pool_name" : self.config_data_pool_name_entry.get_text(),
            "del_results_after_processing" : self.config_del_results_after_processing_checkbutton.get_active(),
            "del_jobs_after_execution" : self.config_del_jobs_after_execution_checkbutton.get_active(),
            "data_pool_write_interval" : self.config_data_pool_write_interval_entry.get_text(),
            "data_pool_complib": complib,
            "data_pool_comprate": self.config_data_pool_comprate.get_value_as_int(),
            "script_font": self.config_script_font_button.get_font_name()
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
        if "del_results_after_processing" in config:
            self.config_del_results_after_processing_checkbutton.set_active(config["del_results_after_processing"])
        if "del_jobs_after_execution" in config:
            self.config_del_jobs_after_execution_checkbutton.set_active(config["del_jobs_after_execution"])
        if "data_pool_write_interval" in config:
            self.config_data_pool_write_interval_entry.set_text(config["data_pool_write_interval"])
        if "data_pool_name" in config:
            self.config_data_pool_name_entry.set_text(config["data_pool_name"])
        if "data_pool_comprate" in config:
            self.config_data_pool_comprate.set_value(float(config["data_pool_comprate"]))
        if "script_font" in config:
            self.config_script_font_button.set_font_name(config["script_font"])
            self.set_script_font_handler(None)
        if "data_pool_complib" in config:
            # find combo-box entry and make it active...
            model=self.config_data_pool_complib.get_model()
            iter=model.get_iter_first()
            while iter is not None:
                if model.get(iter,0)[0]==config["data_pool_complib"]:
                    self.config_data_pool_complib.set_active_iter(iter)
                    break
                iter=model.iter_next(iter)
            # if this compression method is not supported, warn and do nothing
            if iter is None:
                print "compression method %s is not supported"%config["data_pool_complib"]

    def load_config_handler(self, widget):
        self.load_config()

    def save_config_handler(self, widget):
        self.save_config()

    def set_script_font_handler(self, widget):
        """
        handles changes in font name
        also sets the fonts to the text views (fast implementation: breaking encapsulation)
        """
        font=self.config_script_font_button.get_font_name()
        experiment_script_textview = self.xml_gui.get_widget("experiment_script_textview")
        if experiment_script_textview:
            experiment_script_textview.modify_font(pango.FontDescription(font))
        data_handling_textview = self.xml_gui.get_widget("data_handling_textview")
        if data_handling_textview:
            data_handling_textview.modify_font(pango.FontDescription(font))

    def printer_setup_handler(self, widget):
        """
        changes to printer setup
        """
        if not (hasattr(gtk, "PrintSettings") and hasattr(gtk, "print_run_page_setup_dialog")):
            return
        if not hasattr(self, "printer_setup"):
            self.printer_setup = gtk.PrintSettings()

        if not hasattr(self, "page_setup"):
            self.page_setup = None

        self.page_setup = gtk.print_run_page_setup_dialog(self.xml_gui.get_widget("main_window"),
                                                          self.page_setup, self.printer_setup)


    def browse_backend_executable_dialog(self, widget):
        """
        do the open file dialog
        """
        backend_filename_dialog_title="find backend"

        def response(self, response_id, script_widget):
            if response_id == gtk.RESPONSE_OK:
                file_name = self.get_filename()
                if file_name is None:
                    return
                script_widget.config_backend_executable_entry.set_text(file_name)
            return True


        parent_window=self.xml_gui.get_widget("main_window")
        dialog = gtk.FileChooserDialog(title=backend_filename_dialog_title,
                                       parent=parent_window,
                                       action=gtk.FILE_CHOOSER_ACTION_OPEN,
                                       buttons = (gtk.STOCK_OPEN,
                                                  gtk.RESPONSE_OK,
                                                  gtk.STOCK_CANCEL,
                                                  gtk.RESPONSE_CANCEL))
        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_select_multiple(False)
        dialog.set_filename(os.path.abspath(self.config_backend_executable_entry.get_text()))
        system_backend_folder="/usr/lib/damaris/backends/"
        if os.path.isdir(system_backend_folder) and os.access(system_backend_folder, os.R_OK):
            dialog.add_shortcut_folder(system_backend_folder)
        # Event-Handler for responce-signal (when one of the button is pressed)
        dialog.connect("response", response, self)
        f=gtk.FileFilter()
        f.add_custom(gtk.FILE_FILTER_FILENAME, lambda x:os.access(x[0],os.X_OK))
        dialog.set_filter(f)

        dialog.run()
        dialog.destroy()

        return True

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
        config=self.get()
        if filename is None:
            filename=self.defaultfilename
        configfile=file(filename, "w")
        configfile.write("<?xml version='1.0'?>\n")
        configfile.write("<damaris>\n")
        for k,v in config.iteritems():
            val=""
            typename=""
            if type(v) is types.BooleanType:
                typename="Boolean"
                if v:
                    val="True"
                else:
                    val="False"
            elif type(v) is types.StringType:
                typename="String"
                val=v
            elif type(v) is types.IntType:
                typename="Integer"
                val=str(v)
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
        self.display_x_scaling_combobox.remove_text(1) # remove base-e log
        self.display_x_scaling_combobox.set_sensitive(False)
        self.display_y_scaling_combobox = self.xml_gui.get_widget("display_y_scaling_combobox")
        self.display_y_scaling_combobox.set_sensitive(False)
        self.display_autoscaling_checkbutton = self.xml_gui.get_widget("display_autoscaling_checkbutton")
        self.display_statistics_checkbutton = self.xml_gui.get_widget("display_statistics_checkbutton")

        # insert monitor
        # Matplot (Display_Table, 1st Row) --------------------------------------------------------

        # Neue Abbildung erstellen
        self.matplot_figure = matplotlib.figure.Figure()

        # the plot area surrounded by axes
        self.matplot_axes = self.matplot_figure.add_axes([0.1,0.15,0.8,0.7])
        
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
        self.matplot_canvas = FigureCanvas(self.matplot_figure)

        self.display_table = self.xml_gui.get_widget("display_table")
        self.display_table.attach(self.matplot_canvas, 0, 6, 0, 1, gtk.EXPAND | gtk.FILL, gtk.EXPAND | gtk.FILL, 0, 0)
        self.matplot_canvas.show()

        # Matplot Toolbar hinzufuegen (Display_Table, 2. Zeile) 
        self.matplot_toolbar = matplotlib.backends.backend_gtk.NavigationToolbar2GTK(self.matplot_canvas, self.main_window)
        
        self.display_table.attach(self.matplot_toolbar, 0, 1, 1, 2, gtk.FILL | gtk.EXPAND, 0, 0, 0)
        self.matplot_toolbar.show()

        # /Mathplot --------------------------------------------------------------------------------

        # display source
        self.display_source_combobox = self.xml_gui.get_widget("display_source_combobox")
        self.display_source_treestore = gtk.TreeStore(gobject.TYPE_STRING)
        self.display_source_combobox.set_model(self.display_source_treestore)
        display_source_cell = gtk.CellRendererText()
        self.display_source_combobox.pack_start(display_source_cell, True)
        self.display_source_combobox.add_attribute(display_source_cell, 'text', 0)
        self.source_list_reset()
        self.display_source_path_label = self.xml_gui.get_widget("display_source_path_label")
        
        # display scaling: ToDo enable scaling
        self.display_x_scaling_combobox.set_active(0)
        self.display_y_scaling_combobox.set_active(0)
        self.display_x_scaling_combobox.set_sensitive(True)
        self.display_y_scaling_combobox.set_sensitive(False)

        # and events...
        self.display_source_combobox.connect("changed", self.display_source_changed_event)
        self.xml_gui.signal_connect("on_display_autoscaling_checkbutton_toggled", self.display_autoscaling_toggled)
        self.xml_gui.signal_connect("on_display_statistics_checkbutton_toggled", self.display_statistics_toggled)
        self.xml_gui.signal_connect("on_display_x_scaling_combobox_changed", self.display_x_scaling_changed)
        #self.xml_gui.signal_connect("on_display_y_scaling_combobox_changed", self.display_y_scaling_changed)
        self.xml_gui.signal_connect("on_display_save_data_as_text_button_clicked", self.save_display_data_as_text)
        self.xml_gui.signal_connect("on_display_copy_data_to_clipboard_button_clicked", self.copy_display_data_to_clipboard)

        # data to observe
        self.data_pool=None
        # name of displayed data and reference to data
        self.displayed_data=[None,None]
        self.__rescale=True
        self.update_counter=0
        self.update_counter_lock=threading.Lock()

    def source_list_reset(self):
        self.display_source_treestore.clear()
        self.source_list_add('None')
        self.display_source_combobox.set_active(0)

    def source_list_find_one(self, model, iter, what):
        """find node in subcategory"""
        while iter is not None:
            if model.get(iter,0)[0] == what:
                return iter
            iter = model.iter_next(iter)
        return iter

    def source_list_find(self, namelist):
        """
        namelist sequence of names, e.g. ["a", "b", "c" ] for "a/b/c"
        """
        model = self.display_source_treestore
        retval = None
        iter = model.get_iter_root()
        while iter is not None and len(namelist) > 0:
            name = namelist[0]
            iter = self.source_list_find_one(model, iter, name)
            if iter is not None:
                retval = iter
                namelist.pop(0)
                iter = model.iter_children(retval)
        return retval

    def source_list_add(self, source_name, parent=None):
        namelist = source_name.split("/")
        found = self.source_list_find(namelist)
        if parent is None:
            parent = found
        for rest_name in namelist:
            # append() returns iter for the new row
            parent = self.display_source_treestore.append(parent, [rest_name])
 
    def source_list_remove(self, source_name):
        namelist = source_name.split("/")
        pwd = namelist[:]
        iter = self.source_list_find(namelist)
        if iter is None or len(namelist) > 0:
            print "source_list_remove: WARNING: Not found"
            return
        model = self.display_source_treestore
        if model.iter_has_child(iter):
            print "source_list_remove: WARNING: Request to delete a tree"
            return
        while True:
            parent = model.iter_parent(iter)
            model.remove(iter)
            pwd.pop()
            # We now test, if we want to remove parent too
            if parent is None:
                break
            if model.iter_has_child(parent):
                # The parent has other children
                break
            if "/".join(pwd) in self.data_pool:
                # The parent has data connected to it
                break
            iter = parent

    def source_list_current(self):
        ai = self.display_source_combobox.get_active_iter()
        namelist = []
        while ai is not None:
            namelist.insert(0, str(self.display_source_treestore.get(ai,0)[0]))
            ai = self.display_source_treestore.iter_parent(ai)
        cur_source_name = "/".join(namelist)
        return cur_source_name

    def observe_data_pool(self, data_pool):
        """
        register a listener and save reference to data
        assume to be in gtk/gdk lock
        """
        if not self.data_pool is None:
            # maybe some extra cleanup needed
            print "ToDo: cleanup widgets"
            if self.displayed_data[1] is not None and hasattr(self.displayed_data[1], "unregister_listener"):
                    self.displayed_data[1].unregister_listener(self.datastructures_listener)
                    self.displayed_data[1]=None
            self.displayed_data=[None,None]
            self.data_pool.unregister_listener(self.datapool_listener)
            self.data_pool=None

            self.source_list_reset()
            self.clear_display()
            self.update_counter_lock.acquire()
            self.update_counter=0
            self.update_counter_lock.release()

        # display states
        self.__rescale=True
        self.displayed_data=[None,None]
        self.display_source_path_label.set_label(u"")

        if data_pool is not None:
            # keep track of data
            self.data_pool=data_pool
            self.data_pool.register_listener(self.datapool_listener)


    #################### observing data structures and produce idle events

    def datapool_listener(self, event):
        """
        sort data as fast as possible and get rid of non-interesting data
        """
        if event.subject.startswith("__"): return
        if debug and self.update_counter<0:
            print "negative event count!", self.update_counter
        if self.update_counter>5:
            if debug:
                print "sleeping to find time for grapics updates"
            threading.Event().wait(0.05)
            while self.update_counter>15:
                threading.Event().wait(0.05)
        if event.what==DataPool.Event.updated_value:
            if self.displayed_data[0] is None or event.subject!=self.displayed_data[0]:
                # do nothing, forget it
                return

        displayed_object=self.displayed_data[1]
        object_to_display=self.data_pool.get(event.subject)
        if displayed_object is None or object_to_display is None:
            self.update_counter_lock.acquire()
            self.update_counter+=1
            self.update_counter_lock.release()
            gobject.idle_add(self.datapool_idle_listener,event,priority=gobject.PRIORITY_DEFAULT_IDLE)
        else:
            if event.what==DataPool.Event.updated_value and \
                   (displayed_object is object_to_display or displayed_object.__class__ is object_to_display.__class__):
                # oh, another category
                self.update_counter+=1
                gobject.idle_add(self.update_display_idle_event,self.displayed_data[0][:],
                                 priority=gobject.PRIORITY_DEFAULT_IDLE)
            else:
                self.update_counter_lock.acquire()
                self.update_counter+=1
                self.update_counter_lock.release()
                gobject.idle_add(self.datapool_idle_listener,event,priority=gobject.PRIORITY_DEFAULT_IDLE)

        del displayed_object
        del object_to_display
        
    def datastructures_listener(self, event):
        """
        do fast work selecting important events
        """
        if debug and self.update_counter<0:
            print "negative event count!", self.update_counter
        if self.update_counter>5:
            if debug:
                print "sleeping to find time for graphics updates"
            threading.Event().wait(0.05)
            while self.update_counter>15:
                threading.Event().wait(0.05)
        if event.origin is not self.displayed_data[1]: return
        self.update_counter_lock.acquire()
        self.update_counter+=1
        self.update_counter_lock.release()
        gobject.idle_add(self.update_display_idle_event,self.displayed_data[0][:],priority=gobject.PRIORITY_DEFAULT_IDLE)

    ################### consume idle events

    def datapool_idle_listener(self,event):
        """
        here dictionary changes are done
        """

        self.update_counter_lock.acquire()
        self.update_counter-=1
        self.update_counter_lock.release()

        if event.what==DataPool.Event.updated_value:
            if (self.displayed_data[0] is not None and
                self.displayed_data[0]==event.subject):
                new_data_struct=self.data_pool[self.displayed_data[0]]
                if self.displayed_data[1] is new_data_struct:
                    # update display only
                    if self.update_counter>10:
                        print "update queue too long (%d>10): skipping one update"%self.update_counter
                    else:
                        gtk.gdk.threads_enter()
                        try:
                            self.update_display()
                        finally:
                            gtk.gdk.threads_leave()
                else:
                    # unregister old one
                    if self.displayed_data[1] is not None and hasattr(self.displayed_data[1], "unregister_listener"):
                        self.displayed_data[1].unregister_listener(self.datastructures_listener)
                        self.displayed_data[1]=None
                    # register new one
                    if hasattr(new_data_struct, "register_listener"):
                        new_data_struct.register_listener(self.datastructures_listener)
                    self.displayed_data[1]=new_data_struct
                    if self.update_counter>10:
                        print "update queue too long (%d>10): skipping one update"%self.update_counter
                    else:
                        gtk.gdk.threads_enter()
                        try:
                            self.renew_display()
                        finally:
                            gtk.gdk.threads_leave()
                new_data_struct=None
        elif event.what==DataPool.Event.new_key:
            # update combo-box by inserting and rely on consistent information
            gtk.gdk.threads_enter()
            self.source_list_add(event.subject)
            gtk.gdk.threads_leave()
        elif event.what==DataPool.Event.deleted_key:
            # update combo-box by removing and rely on consistent information
            gtk.gdk.threads_enter()
            if (not self.displayed_data[0] is None and
                self.displayed_data[0]==event.subject):
                self.displayed_data=[None,None]
                self.display_source_combobox.set_active(0)
                self.clear_display()
            self.source_list_remove(event.subject)
            gtk.gdk.threads_leave()
        elif event.what==DataPool.Event.destroy:
            gtk.gdk.threads_enter()
            self.source_list_reset('None')
            self.displayed_data=[None,None]
            self.clear_display()
            gtk.gdk.threads_leave()
        return

    def update_display_idle_event(self, subject=None):

        self.update_counter_lock.acquire()
        self.update_counter-=1
        self.update_counter_lock.release()
        # print "update display", self.update_counter
        if self.update_counter>10:
            print "update queue too long (%d>10): skipping one update"%self.update_counter
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
        
        new_data_name = self.source_list_current()
        if (self.displayed_data[0] is None and new_data_name=="None"): return
        if (self.displayed_data[0]==new_data_name): return
        if self.displayed_data[1] is not None and hasattr(self.displayed_data[1], "unregister_listener"):
            self.displayed_data[1].unregister_listener(self.datastructures_listener)
            self.displayed_data[1]=None
            # register new one
        if new_data_name=="None":
            self.display_source_path_label.set_label(u"")
            self.displayed_data=[None,None]
            self.clear_display()
        elif self.data_pool is None or new_data_name not in self.data_pool:
            self.display_source_combobox.set_active(0)
            self.display_source_path_label.set_label(u"")
        else:
            new_data_struct=self.data_pool[new_data_name]
            if hasattr(new_data_struct, "register_listener"):
                new_data_struct.register_listener(self.datastructures_listener)
            self.displayed_data=[new_data_name, new_data_struct]
            dirpart=new_data_name.rfind("/")
            if dirpart>=0:
                self.display_source_path_label.set_label(u"in "+new_data_name[:dirpart])
            else:
                self.display_source_path_label.set_label(u"")
            self.renew_display()

    def display_autoscaling_toggled(self, widget, data=None):
        self.matplot_axes.set_autoscale_on(self.display_autoscaling_checkbutton.get_active())
        if self.displayed_data[0] is not None:
            self.update_display(self.displayed_data[0][:])

    def display_x_scaling_changed(self, widget, data=None):
        self.__rescale=True
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
        if not hasattr(data_to_save[1], "write_as_csv"):
            log("do not know how to save %s of class/type %s"%(data_to_save[0],type(data_to_save[1])))
            return

        # save them to a temporary file (in memory)
        tmpdata=os.tmpfile()
        tmpdata.write("# saved from monitor as %s\n"%data_to_save[0])
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
        dialog.set_current_name(data_to_save[0])
        dialog.set_select_multiple(False)

        # Event-Handler for responce-signal (when one of the button is pressed)
        dialog.connect("response", response, tmpdata)
        del tmpdata, data_to_save
        dialog.run()
        dialog.destroy()

        return True

    def copy_display_data_to_clipboard(self, widget, data=None):
        data_to_save=self.displayed_data[:]
        if self.displayed_data[1] is None:
            # nothing to save
            return
        if not hasattr(data_to_save[1], "write_as_csv"):
            log("do not know how to save %s of class/type %s"%(data_to_save[0],type(data_to_save[1])))
            return

        # tested with qtiplot, labplot, openoffice
        # tab delimiters necessary
        # comments are not welcome :-(
        tmpdata=cStringIO.StringIO()
        data_to_save[1].write_as_csv(tmpdata, delimiter=u"\t")
        # cut away comments
        tmpstring=u""
        tmpdata.seek(0)
        for line in tmpdata:
            if line[0]=="#": continue
            tmpstring+=line        
        del tmpdata
        clipboard=gtk.clipboard_get()
        clipboard.set_text(tmpstring)
        del tmpstring, clipboard

    ##################### functions to feed display

    def clear_display(self):
        """
        unconditionally throw away everything
        we are inside gtk/gdk lock
        """
        if not hasattr(self, "__rescale"):
            self.__rescale = True
        if not hasattr(self,"measurementresultgraph"):
            self.measurementresultgraph=None
        elif self.measurementresultgraph is not None:
            # clear errorbars
            self.matplot_axes.lines.remove(self.measurementresultgraph[0])
            for l in self.measurementresultgraph[1]:
                self.matplot_axes.lines.remove(l)
            for l in self.measurementresultgraph[2]:
                self.matplot_axes.collections.remove(l)
            self.measurementresultgraph=None
            self.matplot_axes.clear()
            self.matplot_axes.grid(True)
        if not hasattr(self,"graphen"):
            self.graphen=[]
        elif self.graphen:
            for l in self.graphen:
                self.matplot_axes.lines.remove(l)
            self.graphen=[]
            self.matplot_axes.clear()
            self.matplot_axes.grid(True)
        self.matplot_canvas.draw_idle()

    def update_display(self, subject=None):
        """
        try to recycle labels, data, lines....
        assume, object is not changed
        we are inside gtk/gdk lock
        """
        in_result=self.data_pool.get(self.displayed_data[0])
        if in_result is None:
            self.clear_display()
            return
        if isinstance(in_result, Accumulation) or isinstance(in_result, ADC_Result):
            # directly taken from bluedamaris

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
                self.display_y_scaling_combobox.set_sensitive(False)


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
            moving_average=data_slice=None
            if max_points_to_display>0 and len(xdata)>max_points_to_display:
                print "decimating data to %d points by moving average (prevent crash of matplotlib)"%max_points_to_display
                n=numpy.ceil(len(xdata)/max_points_to_display)
                moving_average=numpy.ones(n, dtype="float")/n
                data_slice=numpy.array(numpy.floor(numpy.arange(max_points_to_display, dtype="float") \
                                                   /max_points_to_display*len(xdata)),
                                       dtype="int")
                xdata=xdata.take(data_slice) # no average !?
                ydata0=numpy.convolve(ydata0, moving_average, "same").take(data_slice)
                ydata1=numpy.convolve(ydata1, moving_average, "same").take(data_slice)

            if len(self.graphen)==0:
                self.graphen.extend(self.matplot_axes.plot(xdata, ydata0, "b-", linewidth = 2))
                self.graphen.extend(self.matplot_axes.plot(xdata, ydata1, "r-", linewidth = 2))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "b-", linewidth = 0.5))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "b-", linewidth = 0.5))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "r-", linewidth = 0.5))
                self.graphen.extend(self.matplot_axes.plot([0.0], [0.0], "r-", linewidth = 0.5))
            else:
                self.graphen[0].set_data(xdata, ydata0)
                self.graphen[1].set_data(xdata, ydata1)

            # Statistics activated?
            if (self.display_statistics_checkbutton.get_active() and
                in_result.uses_statistics() and in_result.ready_for_drawing_error()):
                yerr0=in_result.get_yerr(0)
                yerr1=in_result.get_yerr(1)
                if moving_average is not None:
                    yerr0=numpy.convolve(yerr0, moving_average, "same").take(data_slice)
                    yerr1=numpy.convolve(yerr1, moving_average, "same").take(data_slice)
                # Real-Errors
                self.graphen[2].set_data(xdata, ydata0 + yerr0)
                self.graphen[3].set_data(xdata, ydata0 - yerr0)
                # Img-Errors
                self.graphen[4].set_data(xdata, ydata1 + yerr1)
                self.graphen[5].set_data(xdata, ydata1 - yerr1)
                yerr0=yerr1=None
            else:
                # Maybe theres a better place for deleting the error-lines
                # Real-Errors
                self.graphen[2].set_data([0.0],[0.0])
                self.graphen[3].set_data([0.0],[0.0])
                # Img-Errors
                self.graphen[4].set_data([0.0],[0.0])
                self.graphen[5].set_data([0.0],[0.0])
            moving_average=data_mask=None
            xdata=ydata0=ydata1=None

            # Any title to be set?
            in_result_title=in_result.get_title()
            if in_result_title is not None:
                col=101
                while len(in_result_title)-col>10:
                    in_result_title=in_result_title[:col]+"\n"+in_result_title[col:]
                    col+=101
                self.matplot_axes.set_title(in_result_title)
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
            self.matplot_canvas.draw_idle()
            del in_result
            
        elif isinstance(in_result, MeasurementResult):

            # remove lines and error bars
            if self.measurementresultgraph is not None:
                self.matplot_axes.lines.remove(self.measurementresultgraph[0])
                # remove caps
                for l in self.measurementresultgraph[1]:
                    self.matplot_axes.lines.remove(l)
                # and columns
                for l in self.measurementresultgraph[2]:
                    self.matplot_axes.collections.remove(l)
                self.measurementresultgraph=None

            [k,v,e]=in_result.get_errorplotdata()
            if k.shape[0]!=0:
                xmin=k.min()
                ymin=(v-e).min()

                if xmin>0:
                    self.display_x_scaling_combobox.set_sensitive(True)
                else:
                    # force switch to lin scale
                    self.display_x_scaling_combobox.set_sensitive(False)
                    if self.display_x_scaling_combobox.get_active_text()!="lin":
                        self.__rescale=True
                        # and reset to linear
                        self.display_x_scaling_combobox.set_active(0)
                x_scale=self.display_x_scaling_combobox.get_active_text()
                y_scale=self.display_y_scaling_combobox.get_active_text()

                # Initial rescaling needed?
                if self.__rescale: # or self.display_autoscaling_checkbutton.get_active():
                    xmax=k.max()
                    ymax=(v+e).max()
                    # is there a range problem?
                    if xmin==xmax or ymin==ymax:
                        # fix range and scaling problems
                        if xmin==xmax:
                            if xmin!=0:
                                (xmin,xmax)=(xmin-abs(xmin/10.0), xmin+abs(xmin/10.0))
                            else:
                                (xmin,xmax)=(-1,1)
                        if ymin==ymax:
                            if ymin==0:
                                (ymin,ymax)=(ymin-abs(ymin/10.0), ymin+abs(ymin/10.0))
                            else:
                                (ymin, ymax)=(-1,1)

                    if xmin<=0 or x_scale=="lin":
                        self.matplot_axes.set_xscale("linear")
                    if ymin<=0 or y_scale=="lin":
                        self.matplot_axes.set_yscale("linear")

                    self.matplot_axes.set_xlim(xmin, xmax)
                    self.matplot_axes.set_ylim(ymin, ymax)

                    # finally decide about x log plot
                    if x_scale=="log10" and xmin>0:
                        self.matplot_axes.set_xscale("log", basex=10.0)
                        self.matplot_axes.fmt_xdata = lambda x: "%g" % x
                    #elif x_scale=="log" and xmin>0:
                    # e scaling implementation not really useful
                    #    self.matplot_axes.set_xscale("log", basex=numpy.e)

                    self.__rescale=False

                self.measurementresultgraph=self.matplot_axes.errorbar(x=k, y=v, yerr=e, fmt="b+")

            # Any title to be set?
            title=in_result.get_title()+""
            if title is not None:
                self.matplot_axes.set_title(title)
            else:
                self.matplot_axes.set_title("")

            self.matplot_canvas.draw_idle()
            del k,v,e
            del in_result

    def renew_display(self):
        """
        set all properties of display
        we are inside gtk/gdk lock
        """
        self.clear_display()
        to_draw=self.data_pool[self.displayed_data[0]]

        if to_draw is None: return
        self.update_display()

    def begin_print(self, operation, context, print_data):
        """
        layout of one page with matplotlib graph
        """
        operation.set_n_pages( 1 )


    def draw_page(self, operation, context, page_nr, print_data):
        """
        render a single page
        """
        # copied and modified from pygtk-2.10.1/examples/pygtk-demo/demos/print_editor.py

        if page_nr != 0:
            return

        # check page dimensions
        # all lengths in inch: name *_in
        page_setup=context.get_page_setup()
        dpi=context.get_dpi_x()
        if dpi!=context.get_dpi_y():
            print "draw_page: dpi_x!=dpi_y, I am not prepared for that"
        freewidth_in = float(context.get_width())/dpi
        freeheight_in = float(context.get_height())/dpi
        fc = self.matplot_canvas.switch_backends(matplotlib.backends.backend_cairo.FigureCanvasCairo)
        fc.figure.dpi.set(dpi)
        orig_w_in, orig_h_in = fc.figure.get_size_inches()
        orig_f_color=fc.figure.get_facecolor()
        orig_e_color=fc.figure.get_edgecolor()

        # scale to maximum
        fc.figure.set_facecolor("w")
        fc.figure.set_edgecolor("w")

        #  maximum scale with constant aspect
        scale=min(freewidth_in/orig_w_in, freeheight_in/orig_h_in)
        fc.figure.set_size_inches(orig_w_in*scale, orig_h_in*scale)
        width_in_points, height_in_points = orig_w_in * dpi * scale, orig_h_in * dpi * scale
        renderer = matplotlib.backends.backend_cairo.RendererCairo (fc.figure.dpi)
        renderer.width  = width_in_points
        renderer.height = height_in_points
        # centered picture
        renderer.matrix_flipy = cairo.Matrix (yy=-1,xx=1,
                                              y0=page_setup.get_top_margin(gtk.UNIT_POINTS)+(height_in_points+freeheight_in*dpi)/2.0,
                                              x0=page_setup.get_left_margin(gtk.UNIT_POINTS)+(freewidth_in*dpi-width_in_points)/2.0)
        
        renderer.set_ctx_from_surface (context.get_cairo_context().get_target())
        # unfortunateley there is need for extra treatment of text
        renderer.ctx.translate(page_setup.get_left_margin(gtk.UNIT_POINTS)+(freewidth_in*dpi-width_in_points)/2.0,
                               page_setup.get_top_margin(gtk.UNIT_POINTS)-height_in_points/2.0+freeheight_in*dpi/2.0)
        renderer.ctx.save() # important! there will be no effect of previous statement without save
        fc.figure.draw(renderer)

        # restore the figure's settings
        fc.figure.set_size_inches(orig_w_in, orig_h_in)
        fc.figure.set_facecolor(orig_f_color)
        fc.figure.set_edgecolor(orig_e_color)
        
class ScriptInterface:
    """
    texts or code objects are executed as experiment and result script the backend is started with sufficient arguments
    """
    
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
        self.data=DataPool()

    def runScripts(self):

        try:
            # get script engines
            self.exp_handling=self.res_handling=None
            if self.exp_script and self.exp_writer:
                self.exp_handling=ExperimentHandling.ExperimentHandling(self.exp_script, self.exp_writer, self.data)
            if self.res_script and self.res_reader:
                self.res_handling=ResultHandling.ResultHandling(self.res_script, self.res_reader, self.data)

            # start them
            if self.back_driver is not None:
                self.back_driver.start()
                while (not self.back_driver.quit_flag.isSet() and \
                       self.back_driver.core_pid is None and self.back_driver.core_pid<=0):
                    self.back_driver.quit_flag.wait(0.1)
            if self.exp_handling: self.exp_handling.start()
            if self.res_handling: self.res_handling.start()
        finally:
            self.exp_writer=self.res_reader=None

    def __del__(self):
        self.exp_writer=None
        self.res_reader=None
        self.back_driver=None
        self.data=None
        self.exp_handling=None
        self.res_handling=None

