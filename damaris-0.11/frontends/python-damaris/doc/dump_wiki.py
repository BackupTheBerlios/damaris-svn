# -*- coding: iso-8859-1 -*-
"""
    MoinMoin - Dump a MoinMoin wiki to static pages

    based on "moin.py export dump" command

"""

import sys, os, time, StringIO, codecs, shutil, re, errno

from MoinMoin import config, wikiutil, Page
from MoinMoin.request import RequestCLI
from MoinMoin.action import AttachFile

HTML_SUFFIX = ".html"

page_template = u'''<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<meta http-equiv="content-type" content="text/html; charset=%(charset)s">
<title>%(pagename)s</title>
<link rel="stylesheet" type="text/css" media="all" charset="utf-8" href="%(theme)s/css/common.css">
<link rel="stylesheet" type="text/css" media="screen" charset="utf-8" href="%(theme)s/css/screen.css">
<link rel="stylesheet" type="text/css" media="print" charset="utf-8" href="%(theme)s/css/print.css">
</head>
<body>
<div id="page">
<h1 id="title">%(pagename)s</h1>
%(pagehtml)s
</div>
<hr>
%(timestamp)s
</body>
</html>
'''

def _attachment(request, pagename, filename, outputdir):
    filename = filename.encode(config.charset)
    source_dir = AttachFile.getAttachDir(request, pagename)
    source_file = os.path.join(source_dir, filename)
    dest_dir = os.path.join(outputdir, "attachments", wikiutil.quoteWikinameFS(pagename))
    dest_file = os.path.join(dest_dir, filename)
    dest_url = "attachments/%s/%s" % (wikiutil.quoteWikinameFS(pagename), wikiutil.url_quote(filename))
    if os.access(source_file, os.R_OK):
        if not os.access(dest_dir, os.F_OK):
            try:
                os.makedirs(dest_dir)
            except:
                print ("Cannot create attachment directory '%s'" % dest_dir)
                raise
        elif not os.path.isdir(dest_dir):
            print ("'%s' is not a directory" % dest_dir)

        shutil.copyfile(source_file, dest_file)
        print ('Writing "%s"...' % dest_url)
        return dest_url
    else:
        return ""
  

class PluginScript: #(MoinScript):
    """ Dump script class """

    def __init__(self):
        pass
    
    def mainloop(self):
        """ moin-dump's main code. """

        # Prepare output directory
        outputdir=os.path.join(os.curdir,"tutorial-html")
        try:
            os.mkdir(outputdir)
            print "Created output directory '%s'!" % outputdir
        except OSError, err:
            if err.errno != errno.EEXIST:
                print "Cannot create output directory '%s'!" % outputdir
                raise

        sys.path.insert(0, os.path.abspath(os.curdir))

        wikiconfig_template="""
from MoinMoin.multiconfig import DefaultConfig
class Config(DefaultConfig):
    sitename = u'DAMARIS Homepage and Usergroup'
    logo_string = u'<img src="/damaris/wiki/damaris/DAMARIS.png" alt="DAMARIS Logo">'
    page_front_page = u"Welcome"
    interwikiname = 'damaris'
    data_dir = '%(pwd)s/wikidata/'
    data_underlay_dir = '%(pwd)s/wikiunderlay'
    url_prefix = '/damaris/wiki'
    theme_default = 'modern'
"""%{"pwd": os.curdir, "underlay": "/home/achim/underlay" }

        config_file = open("wikiconfig.py","w")
        print >>config_file, wikiconfig_template
        config_file.close()

        # start with wiki entry page
        request = RequestCLI(pagename="Welcome")

        # fix url_prefix so we get relative paths in output html
        url_prefix = "."
        request.cfg.url_prefix = url_prefix

        pages = request.rootpage.getPageList(user='') # get list of all pages in wiki
        pages.sort()

        # extract a list of pages to be extracted
        # trial session to fat!!!
        try:
            pages_match = re.compile("^(Tutorial|auxiliary tools|overview|installation|code snippets)")
            pages = [page for page in pages if pages_match.match(page)]
        except:
            print "did not find suitable pages"
            raise

        wikiutil.quoteWikinameURL = lambda pagename, qfn=wikiutil.quoteWikinameFS: (qfn(pagename) + HTML_SUFFIX)

        AttachFile.getAttachUrl = lambda pagename, filename, request, addts=0, escaped=0: (_attachment(request, pagename, filename, outputdir))

        errfile = os.path.join(outputdir, 'error.log')
        errlog = open(errfile, 'w')
        errcnt = 0

        page_front_page = wikiutil.getSysPage(request, request.cfg.page_front_page).page_name
        page_title_index = wikiutil.getSysPage(request, 'TitleIndex').page_name
        page_word_index = wikiutil.getSysPage(request, 'WordIndex').page_name
        
        navibar_html = ''
        for p in [page_front_page, page_title_index, page_word_index]:
            navibar_html += '&nbsp;[<a href="%s">%s</a>]' % (wikiutil.quoteWikinameURL(p), wikiutil.escape(p))

        urlbase = request.url # save wiki base url
        for pagename in pages:
            # we have the same name in URL and FS
            file = wikiutil.quoteWikinameURL(pagename) 
            print ('Writing "%s"...' % file)
            try:
                pagehtml = ''
                request.url = urlbase + pagename # add current pagename to url base 
                page = Page.Page(request, pagename)
                request.page = page
                try:
                    request.reset()
                    pagehtml = request.redirectedOutput(page.send_page, request, count_hit=0, content_only=1)
                except:
                    errcnt = errcnt + 1
                    print >>sys.stderr, "*** Caught exception while writing page!"
                    print >>errlog, "~" * 78
                    print >>errlog, file # page filename
                    import traceback
                    traceback.print_exc(None, errlog)
            finally:
                timestamp = time.strftime("%Y-%m-%d %H:%M")
                filepath = os.path.join(outputdir, file)
                fileout = codecs.open(filepath, 'w', config.charset)
                logo_html = '<img src="logo.png">'
                fileout.write(page_template % {
                    'charset': config.charset,
                    'pagename': pagename,
                    'pagehtml': pagehtml,
                    'logo_html': logo_html,
                    'navibar_html': navibar_html,
                    'timestamp': timestamp,
                    'theme': request.cfg.theme_default,
                })
                fileout.close()

        # ToDo: insert style sheets and logo

        errlog.close()
        if errcnt:
            print >>sys.stderr, "*** %d error(s) occurred, see '%s'!" % (errcnt, errfile)

if __name__=="__main__":
    PluginScript().mainloop()
