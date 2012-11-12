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

#if sys.version_info < (2, 5, 3):
#    log.error("**** ERROR: Install manually: python setup.py install ****")
#    raise ValueError

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

# create doc data file information
distribution_doc_prefix=os.path.join("share","python-damaris","doc")
distribution_data_files = [[ "share", []],
                           [os.path.join("share", "python-damaris", "images"),
                            ["src/gui/DAMARIS.png", "src/gui/DAMARIS.ico"]],
                           [os.path.join("share", "python-damaris"), []],
                           [distribution_doc_prefix, ['doc/index.html']]]

if os.path.isdir(os.path.join("doc","reference-html")):
    # no subdirs, work can be done in simple way
    distribution_data_files.append([os.path.join(distribution_doc_prefix, 'reference-html'),
                                    [os.path.join('doc', 'reference-html', f)
                                     for f in os.listdir(os.path.join('doc', 'reference-html'))]])

if os.path.isdir(os.path.join("doc","tutorial-html")):
    # here, modern style file and attachment directories should be handled
    for d in os.walk(os.path.join("doc","tutorial-html")):
        distribution_data_files.append([os.path.join(os.path.dirname(distribution_doc_prefix),d[0]),
                                        [os.path.join(d[0], f) for f in d[2]]])

LONG_DESCRIPTION="""
DArmstadt MAgnetic Resonance Instrument Software
"""

GPL_LICENCE = "feed licence here"

setup (
    name = 'python-damaris',
    version = "0.14-svn",
    description = 'python frontend for DAMARIS (DArmstadt MAgnetic Resonance Instrument Software)',
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
                 'damaris.gui',
                 'damaris.tools' ],
    package_dir = { 'damaris': 'src',
                    'damaris.data': 'src/data',
                    'damaris.experiments': 'src/experiments',
                    'damaris.gui': 'src/gui',
                    'damaris.tools': 'src/tools' },
    package_data = { 'damaris.gui': ['DAMARIS.png', 'DAMARIS.ico', 'damaris.glade',  'damaris.gladep', 'python.xml']},
    scripts = ['scripts/DAMARIS'],
    cmdclass={"build_scripts": build_damaris_scripts},
    data_files = distribution_data_files
    )
