# -*- coding: ISO-8859-1 -*-

#############################################################
#                                                           #
# Class: Experiment                                         #
#                                                           #
# Purpose: Represents one full experiment (one program on   #
#          the pulse-card; one file)                        #
#                                                           #
#############################################################

class Experiment:

    job_id = 0
    
    def __init__(self):
        self.job_id = Experiment.job_id
        Experiment.job_id += 1

        self.state_list = []
        self.description = { }


    # Commands -------------------------------------------------------------------------------------

    def rf_pulse(self, channel, length = None):
        if length is None:
            self.state_list.append('<ttlout value="%d"/>\n' % channel)

        else:
            self.state_list.append('<state time="%g"><ttlout value="%d"/></state>\n' % (length, channel))


    def state_start(self, time):
        self.state_list.append('<state time="%g">\n' % time)


    def state_end(self):
        self.state_list.append('</state>\n')


    def wait(self, time):
        self.state_list.append('<state time="%g"/>\n' % time)


    def record(self, samples, frequency, timelength=None):
        if timelength is None:
            self.state_list.append('<state time="%g"><analogin s="%d" f="%g"/></state>\n' % (samples / float(frequency)*1.01, samples, frequency))
        else:
            self.state_list.append('<state time="%g"><analogin s="%d" f="%g"/></state>\n' % (timelength, samples, frequency))


    def loop_start(self, iterations):
        self.state_list.append('<sequent repeat="%d">\n' % iterations)


    def loop_end(self):
        self.state_list.append('</sequent>\n')


    def set_frequency(self, frequency, phase):
        "Sets the frequency generator to a desired frequency (Hz)"
        self.state_list.append('<state time="2e-6"><analogout f="%f" phase="%d" /></state>\n' % (frequency, phase))


    def set_description(self, key, value):
        "Sets a description"
        if key in self.description.keys():
            print 'Warning: Overwriting existing description "%s" = "%s" with "%s"' % (key, self.description[key], value)

        self.description[key] = value

    # / Commands -----------------------------------------------------------------------------------


    # Public Methods -------------------------------------------------------------------------------

    def get_job_id(self):
        "Returns the current job-id the experiment got"
        return self.job_id


    def write_xml_string(self):
        "Returns the current program as a string"

        # Standart XML-Kopf
        xml_string = '<?xml version="1.0" encoding="ISO-8859-1"?>\n'

        # Experiment-Start-Tag einfügen
        xml_string += '<experiment id="%d">\n' % self.job_id

        # Descriptions einfügen
        if len(self.description) is 0:
            xml_string += '<description/>\n'
        else:
            xml_string += '<description'

            for key in self.description.keys():
                xml_string += ' %s="%s"' % (key, str(self.description[key]))

            xml_string += "/>\n"

        # Experiment-Inhalt einfügen
        for string in self.state_list:
            xml_string += string

        # Experiment-End-Tag
        xml_string += '</experiment>\n'

        return xml_string


    def write_quit_job(self):
        "Returns a xml quit-job"
        return '<?xml version="1.0" encoding="ISO-8859-1"?>\n<quit/>'


    # /Public Methods ------------------------------------------------------------------------------

def reset():
    "Resets the internal id-inkrementer to 0"
    Experiment.job_id = 0
