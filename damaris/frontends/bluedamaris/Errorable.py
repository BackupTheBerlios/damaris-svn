# -*- coding: iso-8859-1 -*-

#############################################################################
#                                                                           #
# Name: Class Errorable                                                     #
#                                                                           #
# Purpose: Base class of everything what could contain a statistic error    #
#                                                                           #
#############################################################################

class Errorable:
    def __init__(self):

        # Will be determined in one of the subclasses
        self.xerr = []
        self.yerr = []

        self.error_color = ""
        self.bars_above = False

        self.cont_changed = True

        self.n = 0


    def get_xerr(self):
        "Returns a reference to x-Error (numarray)"
        return self.xerr


    def set_xerr(self, pos, value):
        "Sets a point in x-Error"
        try:
            self.xerr[pos] = value
        except:
            raise


    def get_yerr(self, channel):
        "Returns a list of y-Errors (list of numarrays, corresponding channels)"
        try:
            if self.content_changed():
                self.calculate_error()
                return self.yerr[channel]
            else:
                return self.yerr[channel]
        except:
            raise


    def set_yerr(self, channel, pos, value):
        "Sets a point in y-Error"
        try:
            self.yerr[channel][pos] = value
        except:
            raise


    def get_error_color(self):
        "Returns the error-bar color"
        return self.error_color


    def set_error_color(self, color):
        "Sets the error-bar color"
        self.error_color = color


    def get_bars_above(self):
        "Gets bars-above property of errorplot"
        return self.bars_above


    def set_bars_above(self, bars_above):
        "Sets bars-above property of errorplot"
        self.bars_above = bool(bars_above)
