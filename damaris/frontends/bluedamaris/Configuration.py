import xml.parsers.expat
import os

class Configuration:

    def __init__(self, path = None):

        self.config_job_writer = { }
        self.config_data_handling = { }
        self.config_core = { }
        self.config_gui = { }

        # Path setzen
        if path is None:
            self.path = os.getcwd()
        else:
            self.path = path

 
        if os.access(os.path.join(self.path, "config.xml"), os.R_OK):
            self.config_file = file(os.path.join(self.path, "config.xml"), "r")
        else:
            raise IOError("Configuration: Error, cant open config.xml")

        try:
            # Parser erstellen
            self.xml_parser = xml.parsers.expat.ParserCreate()
            self.xml_parser.StartElementHandler = self.xml_start_tag_found

            # Parsing all cdata as one block
            self.xml_parser.buffer_text = True
            self.xml_parser.ParseFile(self.config_file)      
        except:
            self.config_file.close()


        self.config_file.close()



    def xml_start_tag_found(self, name, attribute):
        if name == "job_writer":
            self.config_job_writer = attribute.copy()

        elif name == "data_handling":
            self.config_data_handling = attribute.copy()

        elif name == "core":
            self.config_core = attribute.copy()

        elif name == "gui":
            self.config_gui = attribute.copy()

        elif name == "config":
            pass

        else:
            print "Configuration: Warning, found unknown config-tag!"
            print "Name: " + str(name) + ", Attribute: " + str(attribute)
         
 

    def get_my_config(self, applicant):
        #print "applicant: " + str(applicant.__class__)

        if str(applicant.__class__) == "JobWriter.JobWriter" or str(applicant.__class__) == "<class 'JobWriter.JobWriter'>":
            return self.config_job_writer.copy()

        if str(applicant.__class__) == "DataHandling.DataHandling" or str(applicant.__class__) == "<class 'DataHandling.DataHandling'>":
            return self.config_data_handling.copy()

        if str(applicant.__class__) == "DamarisGUI.DamarisGUI" or str(applicant.__class__) == "<class 'DamarisGUI.DamarisGUI'>":
            return self.config_gui.copy()
