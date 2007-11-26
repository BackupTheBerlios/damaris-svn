#############################################################################
#
# Author: Achim Gaedke
# Created: June 2004
#
#############################################################################


.PHONY: clean install all

SUBDIRS=core drivers machines tests doc tools
PREFIX=$(shell cd && pwd)/nmr_software

all:
	for d in $(SUBDIRS); do $(MAKE) PREFIX=$(PREFIX) -C $$d all; done

clean:
	rm -f *~ && for d in $(SUBDIRS); do $(MAKE) -C $$d clean; done

install:
	install -d $(PREFIX); \
for d in $(SUBDIRS); do $(MAKE) PREFIX=$(PREFIX) -C $$d install; done
