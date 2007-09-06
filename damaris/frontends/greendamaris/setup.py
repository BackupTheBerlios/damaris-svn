#!/usr/bin/env python

import shutil
import os
import os.path
import sys
from distutils.core import setup
from distutils.command.build_scripts import build_scripts as _build_scripts
from distutils import log
from distutils.util import convert_path
from distutils.dep_util import newer

class build_damaris_scripts(_build_scripts):

    #user_options=_build_scripts.user_options[:]
    #user_options.append(('install-dir=', 'd', "directory to install scripts to"))

    def initialize_options (self):
        _build_scripts.initialize_options(self)
        self.damaris_dir = None

    def finalize_options (self):
        _build_scripts.finalize_options(self)
        self.set_undefined_options('install',
                                   ('install_lib', 'damaris_dir'))

    def run (self):
        "change PYTHON_PATH for DAMARIS executable"
        _build_scripts.run(self)
        script="scripts/DAMARIS"
        script = convert_path(script)
        outfile = os.path.join(self.build_dir, os.path.basename(script))
        self.damaris_dir=os.path.normpath(self.damaris_dir)
        if self.damaris_dir in sys.path:
            log.debug("not changing %s (this path is on standard path)", script)
            # nothing to do for us
            return

        # now change PATH in DAMARIS script
        # copy backup
        log.info("adapting DAMARIS script to use local installation")
        shutil.copyfile(outfile, outfile+".bak")
        # the file should keep all its attributes (executable...)
        inf=file(outfile+".bak","r")
        outf=file(outfile,"w")
        l=inf.readline()
        while not l.startswith("import sys") and l!="":
            outf.write(l)
            l=inf.readline()

        if l!="":
            outf.write(l)
            l=inf.readline()
            while l.endswith("# inserted by setup.py\n"):
                l=inf.readline()
            outf.write("sys.path.insert(0,\"%s\") # inserted by setup.py\n"%self.damaris_dir)
            outf.write(l)
        
        outf.writelines(inf.readlines())
        inf.close()
        outf.close()
        os.remove(outfile+".bak")

LONG_DESCRIPTION="""
DArmstadt MAgnetic Resonance Instrument System
"""

GPL_LICENCE = "feed licence here"

setup (
    name = 'DAMARIS',
    version = "0.9",

    description = 'DArmstadt MAgnetic Resonance Instrument System',
    long_description = LONG_DESCRIPTION,

    author = 'Achim Gaedke',
    author_email = 'Achim.Gaedke@physik.tu-darmstadt.de',

    maintainer = 'Achim Gaedke',
    maintainer_email = 'Achim.Gaedke@physik.tu-darmstadt.de',

    url = 'http://www.fkp.physik.tu-darmstadt.de/damaris/',
    license = GPL_LICENCE,

    platforms = ('Any',),
    keywords = ('NMR', 'data-processing'),

    packages = [ 'damaris',
                 'damaris.data',
                 'damaris.experiments',
                 'damaris.gui'],
    
    package_dir = { 'damaris': 'src',
                    'damaris.data': 'src/data',
                    'damaris.experiments': 'src/experiments',
                    'damaris.gui': 'src/gui'},
    package_data = { 'damaris.gui': ['DAMARIS.png', 'DAMARIS.ico', 'damaris.glade',  'damaris.gladep']},
    scripts = ['scripts/DAMARIS'],
    cmdclass={"build_scripts": build_damaris_scripts}
    
    )
