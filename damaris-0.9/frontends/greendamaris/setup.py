#!/usr/bin/env python

from distutils.core import setup


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
    scripts = ['scripts/DAMARIS']
    )
