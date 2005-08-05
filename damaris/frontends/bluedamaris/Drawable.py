# -*- coding: iso-8859-1 -*-

#############################################################################
#                                                                           #
# Name: Class Drawable                                                      #
#                                                                           #
# Purpose: Base class of everything plottable                               #
#                                                                           #
#############################################################################

class Drawable:
    def __init__(self):

        # Will be set correctly in one of the subclasses
        self.x = []
        self.y = []

        self.styles = { }

        self.xlabel = ""
        self.ylabel = ""

        self.title = ""

        self.legend = { }

        self.labels = { }

        self.xmin = 0
        self.xmax = 0
        self.ymin = 0
        self.ymax = 0


    def get_xdata(self):
        "Returns a reference to the x-Plotdata (numarray)"
        return self.x


    def set_xdata(self, pos, value):
        "Sets a point in x"
        try:
            self.x[pos] = value
            
            if value < self.xmin: self.xmin = value
            if value > self.xmax: self.xmax = value
        except:
            raise


    def get_ydata(self, channel):
        "Returns the y-Plotdata of channel n (numarray)"
        try:
            return self.y[channel]
        except:
            raise


    def set_ydata(self, channel, pos, value):
        "Sets a point in y"
        try:
            self.y[channel][pos] = value
            
            if value < self.ymin: self.ymin = value
            if value > self.ymax: self.ymax = value
        except:
            raise

        
    def get_number_of_channels(self):
        "Returns the number of channels in y"
        return len(self.y)


    def get_style(self):
        "Returns a reference to plot-styles (dictionary)"
        return self.styles


    def set_style(self, channel, value):
        "Sets a channel to a certain plot-style"
        if self.styles.has_key(key):
            print "Drawable Warning: Style key \"%s\" will be overwritten with \"%s\"" % (str(channel), str(value))

        self.styles[channel] = str(value)


    def get_xlabel(self):
        "Returns the label for the x-axis"
        return self.xlabel


    def set_xlabel(self, label):
        "Sets the label for the x-axis"
        self.xlabel = str(label)


    def get_ylabel(self):
        "Gets the label for the y-axis"
        return self.ylabel


    def set_ylabel(self, label):
        "Sets the label for the y-axis"
        self.ylabel = str(label)


    def get_labels(self):
        "Returns labels to be plotted (Dictionary)"
        return self.labels


    def set_lables(self, x, y, value):
        "Sets labels to be plotted "
        if self.labels.has_key((x, y)):
            print "Drawable Warning: Label key \"%s\" will be overwritten with \"%s\"" % (str(key), str(value))

        self.labels[(x,y)] = str(value)


    def get_title(self):
        "Returns the title of the plot"
        return self.title


    def set_title(self, title):
        "Sets the title of the plot"
        self.title = str(title)


    def get_legend(self):
        "Returns the legend of the plot (Dictionary)"
        return self.legend


    def set_legend(self, channel, value):
        "Sets the legend of the plot"
        if self.legend.has_key(key):
            print "Drawable Warning: Legend key \"%s\" will be overwritten with \"%s\"" % (str(channel), str(value))

        self.legend[channel] = str(value)


    def get_xmin(self):
        "Returns minimun of x"
        return self.xmin


    def set_xmin(self, xmin):
        "Sets minimum of x"
        self.xmin = xmin


    def get_xmax(self):
        "Returns maximum of x"
        return self.xmax


    def set_xmax(self, xmax):
        "Sets maximum of x"
        self.xmax = xmax


    def get_ymin(self):
        "Returns minimum of y"
        return self.ymin


    def set_ymin(self, ymin):
        "Sets minimum of y"
        self.ymin = ymin


    def get_ymax(self):
        "Returns maximimum of y"
        return self.ymax


    def set_ymax(self, ymax):
        "Sets maximum of y"
        self.ymax = ymax
        
        
        

        

        
        

        
    
