# -*- coding: iso-8859-1 -*-

#########################################################################
#                                                                       #
# Purpose: Supporting GUI-Elements, like easy question-dialogs etc.     #
#                                                                       #
#########################################################################

import pygtk
pygtk.require("2.0")
import gtk


def show_error_dialog(calling_window, title, bodytext):
    "Displays an error dialog"

    def response(self, response_id, response_data = None):
        return True

    def close(self, widget, response_data = None):
        return True

    dialog = gtk.MessageDialog(calling_window,
                               gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                               gtk.MESSAGE_ERROR,
                               (gtk.BUTTONS_OK),
                               bodytext)

    dialog.connect("response", response)
    dialog.connect("close", close)

    dialog.set_title(title)

    dialog.label.set_line_wrap(True)
    dialog.label.set_single_line_mode(False)
    dialog.label.set_width_chars(100)

    dialog.run()
    dialog.destroy()

    return None

def show_question_dialog(calling_window, title, bodytext):
    "Displays a question dialog: 0, 1 or 2 returned (Yes, No, Cancel), [or -1 if window was killed]"

    # Making it a reference
    response_value = []

    def response(self, response_id, response_data = None):
        response_data.append(response_id)

    def close(self, widget, response_data = None):
        response_data.append(-4)
        return True

    dialog = gtk.MessageDialog(calling_window,
                               gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                               gtk.MESSAGE_QUESTION,
                               (gtk.BUTTONS_NONE),
                               bodytext)

    dialog.connect("response", response, response_value)
    dialog.connect("close", close)

    dialog.set_title(title)

    dialog.add_button(gtk.STOCK_YES, gtk.RESPONSE_YES)
    dialog.add_button(gtk.STOCK_NO, gtk.RESPONSE_NO)
    dialog.add_button(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL)

    dialog.label.set_line_wrap(True)
    dialog.label.set_single_line_mode(False)
    
    dialog.run()
    dialog.destroy()

    if response_value[0] == -4:
        # Window killed
        return -1
    
    elif response_value[0] == -8:
        # User pressed "Yes"
        return 0

    elif response_value[0] == -9:
        # User pressed "No"
        return 1

    elif response_value[0] == -6:
        # User pressed "Cancel"
        return 2
    
    return None

def show_question_dialog_compulsive(calling_window, title, bodytext):
    "Displays a compulsive question dialog: 0 or 1 returned (Yes, No), [or -1 if user killed window]"

    # Making it a reference
    response_value = []

    def response(self, response_id, response_data = None):
        response_data.append(response_id)
        return True

    def close(self, widget, response_data = None):
        response_data.append(-4)
        return True

    dialog = gtk.MessageDialog(calling_window,
                               gtk.DIALOG_MODAL | gtk.DIALOG_DESTROY_WITH_PARENT,
                               gtk.MESSAGE_QUESTION,
                               (gtk.BUTTONS_YES_NO),
                               bodytext)

    dialog.connect("response", response, response_value)
    dialog.connect("close", close)

    dialog.set_title(title)

    dialog.label.set_line_wrap(True)
    dialog.label.set_single_line_mode(False)

    dialog.run()
    dialog.destroy()

    if response_value[0] == -4:
        # Window killed
        return 2
    
    elif response_value[0] == -8:
        # User pressed "Yes"
        return 0

    elif response_value[0] == -9:
        # User pressed "No"
        return 1
    
    return None
