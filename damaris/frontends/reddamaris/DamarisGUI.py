import os.path
import sys
import pygtk
pygtk.require("2.0")
import gtk
import gtk.glade
import pango
import matplotlib
import matplotlib.axes
import matplotlib.figure
from matplotlib.backends.backend_gtkagg import FigureCanvasGTKAgg as FigureCanvas
from matplotlib.backends.backend_gtk import NavigationToolbar2GTK as NavigationToolbar


class DamarisGUI:

    ExpScript_Display=1
    ResScript_Display=2
    Monitor_Display=3
    Log_Display=4

    def __init__(self):

        #all my state variables
        self.active_display=DamarisGUI.ExpScript_Display
        
        self.glade_layout_init()
        
        self.text_widgets_init()

        self.toolbar_init()

        self.monitor_init()

    def glade_layout_init(self):
        glade_file=os.path.join(os.path.dirname(__file__),"damaris.glade")
        self.xml_gui = gtk.glade.XML(glade_file)
        self.main_window = self.xml_gui.get_widget("main_window")

        self.main_window.connect("delete-event", self.quit_event)

    def text_widgets_init(self, exp_script="", res_script=""):
        """
        initialize text widgets with text
        """
        # script buffers:
        self.experiment_script_textview = self.xml_gui.get_widget("experiment_script_textview")
        self.data_handling_textview = self.xml_gui.get_widget("data_handling_textview")
        self.experiment_script_textbuffer = self.experiment_script_textview.get_buffer()
        self.data_handling_textbuffer = self.data_handling_textview.get_buffer()
        self.experiment_script_statusbar_label = self.xml_gui.get_widget("statusbar_experiment_script_label")
        self.data_handling_statusbar_label = self.xml_gui.get_widget("statusbar_data_handling_label")

        # load buffers and set cursor to front
        self.experiment_script_textbuffer.set_text(unicode(exp_script))
        self.experiment_script_textbuffer.place_cursor(self.experiment_script_textbuffer.get_start_iter())
        self.experiment_script_textbuffer.set_modified(False)
        self.data_handling_textbuffer.set_text(unicode(res_script))
        self.data_handling_textbuffer.place_cursor(self.data_handling_textbuffer.get_start_iter())
        self.data_handling_textbuffer.set_modified(False)

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

        # init location indicators
        self.textviews_moved(self.experiment_script_textview)
        self.textviews_moved(self.data_handling_textview)

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
        # Toolbar buttons
        self.toolbar_new_button = self.xml_gui.get_widget("toolbar_new_button")
        self.toolbar_open_button = self.xml_gui.get_widget("toolbar_open_file_button")
        self.toolbar_save_button = self.xml_gui.get_widget("toolbar_save_file_button")
        self.toolbar_save_as_button = self.xml_gui.get_widget("toolbar_save_as_button")
        self.toolbar_save_all_button = self.xml_gui.get_widget("toolbar_save_all_button")
        self.toolbar_stop_button = self.xml_gui.get_widget("toolbar_stop_button")
        self.toolbar_run_button = self.xml_gui.get_widget("toolbar_run_button")
        self.toolbar_check_scripts_button = self.xml_gui.get_widget("toolbar_check_scripts_button")
        self.toolbar_exec_with_options_togglebutton = self.xml_gui.get_widget("toolbar_execute_with_options_button")

        # and their events
        self.xml_gui.signal_connect("on_toolbar_run_button_clicked", self.start_experiment)
        self.xml_gui.signal_connect("on_toolbar_open_file_button_clicked", self.open_file)
        self.xml_gui.signal_connect("on_toolbar_new_button_clicked", self.new_file)
        self.xml_gui.signal_connect("on_toolbar_save_as_button_clicked", self.save_file_as)
        self.xml_gui.signal_connect("on_toolbar_save_file_button_clicked", self.save_file)
        self.xml_gui.signal_connect("on_toolbar_save_all_button_clicked", self.save_all_files)
        self.xml_gui.signal_connect("on_toolbar_pause_button_clicked", self.pause_experiment)
        self.xml_gui.signal_connect("on_toolbar_stop_button_clicked", self.stop_experiment)
        self.xml_gui.signal_connect("on_toolbar_execute_with_options_button_clicked", self.start_experiment_with_options)

    def run(self):

        gtk.gdk.threads_enter()
        gtk.main()
        gtk.gdk.threads_leave()

    # event handling: the real acitons in gui programming

    # first global events

    def quit_event(self, widget, data=None):
        """
        expecting quit event for main application
        """
        # do a cleanup...

        # and quit
        gtk.main_quit()

    # text widget related events

    def textviews_modified(self, data = None):
        print "todo: modified"

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


    # toolbar related events:


    def start_experiment_with_options(self, widget, data = None):
        print "ToDo: start_experiment_with_options"
        

    def start_experiment(self, widget, data = None):
        print "ToDo: start_experiment"

    def pause_experiment(self, widget, data = None):
        print "ToDo: pause_experiment"

    def stop_experiment(self, widget, data = None):
        print "ToDo: stop_experiment"

    def open_file(self, widget, Data = None):
        print "ToDo: open_file"

    def save_file(self, widget, Data = None):
        print "ToDo: save_file"

    def save_file_as(self, widget, Data = None):
        print "ToDo: save_file_as"

    def save_all_files(self, widget, Data = None):
        print "ToDo: save_all_files"

    def new_file(self, widget, Data = None):
        print "ToDo: new_file"


if __name__=="__main__":

    d=DamarisGUI()
    d.run()
