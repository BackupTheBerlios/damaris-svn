# -*- coding: ISO-8859-1 -*-
class Experiment:

    job_id = 0
    
    def __init__(self):
        self.job_id = Experiment.job_id
        Experiment.job_id += 1

        self.state_list = []
        self.description = { }


    def rf_pulse(self, channel, length = None):
        if length is None:
            self.state_list.append('<ttlout value="%d"/>\n' % channel)

        else:
            self.state_list.append('<state time="%f"><ttlout value="%d"/></state>\n' % (length, channel))


    def state_start(self, time):
        self.state_list.append('<state time="%f">\n' % time)


    def state_end(self):
        self.state_list.append('</state>\n')


    def wait(self, time):
        self.state_list.append('<state time="%f"/>\n' % time)


    def record(self, samples, frequency):
        self.state_list.append('<state time="%f"><analogin s="%d" f="%f"/></state>\n' % (samples / float(frequency), samples, frequency))


    def loop_start(self, iterations):
        self.state_list.append('<sequent repeat="%d">\n' % iterations)


    def loop_end(self):
        self.state_list.append('</sequent>\n')


    def set_frequency(self, frequency, phase):
        self.state_list.append('<state time="1e-6"><analogout f="%f" phase="%d" /></state>\n' % (frequency, phase))
        

    def get_job_id(self):
        return self.job_id


    def write_xml_string(self):

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


    def set_description(self, key, value):
        if key in self.description.keys():
            print 'Warning: Overwriting existing description "%s" = "%s" with "%s"' % (key, self.description[key], value)

        self.description[key] = value


    def write_quit_job(self):
        return '<?xml version="1.0" encoding="ISO-8859-1"?>\n<quit/>'



def reset():
    Experiment.job_id = 0
