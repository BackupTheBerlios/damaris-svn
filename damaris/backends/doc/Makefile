#############################################################################
#
# Author: Achim Gaedke
# Created: June 2004
#
#############################################################################
DOXYGEN=doxygen
PATH:=$(PATH)

.PHONY: all install docinstall clean doc

all:

install:

doc: backends-html

backends-html: .htmltag

software_design_overview_scaled.png: software_design_overview.png
	convert -resize 635x476 $< $@

.htmltag: software_design_overview_scaled.png
	$(DOXYGEN) Doxyfile && touch .htmltag

clean:
	rm -rf backends-html software_design_overview_scaled.png .htmltag *~ \#*

docinstall: backends-html
	install -d $(PREFIX)/doc && cp -r backends-html/* $(PREFIX)/doc
