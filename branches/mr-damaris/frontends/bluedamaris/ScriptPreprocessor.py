class ScriptPreprocessor:
    def __init__(self, kind):
        "kind should be 'ES' or 'DH', corresponding the script"

        # GUI-Calls (corresponding methods)
        self.new_log_message = "gui.new_log_message"
        
        if kind == "ES":
            self.kind = "ES"
        elif kind == "DH":
            self.kind = "DH"
        else:
            raise TypeError("'kind' must be 'ES' or 'DH'")
        

    def substitute_print(self, script):
        "Substitutes 'print XX' with gui.new_log_message(XX, kind)"

        if self.kind == "DH":
            return self.substitute_print_dh(script)
        elif self.kind == "ES":
            return self.substitute_print_es(script)
        else:
            pass


    def substitute_print_es(self, script):
        substituted_script = ""
        outer_space_variable = ""

        start = script.find("(", script.find("def experiment_script(")) + 1
        end = script.find(")", start)

        if start == 0:
            raise TypeError("ScriptPreprocessor: Awaiting Experiment-Script, found something else")

        outer_space_variable = script[start:end]
        substituted_print = outer_space_variable + "." + self.new_log_message + "((" + "%s" + "), \"ES\")"

        start = 0
        end = 0

        while script.find("print", start) != -1:

            # Copy from start to just before first occurence of "print"
            substituted_script += script[start:script.find("print", start)]

            # Set start and end to expression after print (if EOF, -1 is returned)
            start = script.find("print", start) + 6
            end = script.find("\n", start)

            # Catch EOF
            if end == -1:
                end = len(script)

            # Add substituted print
            substituted_script += substituted_print % script[start:end]

            # Set startpointer to end
            start = end

        string = substituted_script + "hello"
        return substituted_script


    def substitute_print_dh(self, script):
        substituted_script = ""
        outer_space_variable = ""

        start = script.find("(", script.find("def data_handling(")) + 1
        end = script.find(")", start)

        if start == 0:
            raise TypeError("ScriptPreprocessor: Awaiting Data Handling-Script, found something else")

        outer_space_variable = script[start:end]
        substituted_print = outer_space_variable + "." + self.new_log_message + "((" + "%s" + "), \"DH\")"

        start = 0
        end = 0

        while script.find("print", start) != -1:

            # Copy from start to just before first occurence of "print"
            substituted_script += script[start:script.find("print", start)]

            # Set start and end to expression after print (if EOF, -1 is returned)
            start = script.find("print", start) + 6
            end = script.find("\n", start)

            # Catch EOF
            if end == -1:
                end = len(script)

            # Add substituted print
            substituted_script += substituted_print % script[start:end]

            # Set startpointer to end
            start = end

        return substituted_script
