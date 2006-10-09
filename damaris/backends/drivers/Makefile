#############################################################################
#
# Author: Achim Gaedke
# Created: June 2004
#
#############################################################################

CXX=g++
CXXFLAGS=-O0 -g -I. -I..

#ToDo PBP-HS3-test.exe
SUBDIRS:=SpinCore-PulseBlaster SpinCore-PulseBlaster24Bit SpinCore-PulseBlasterDDSIII PTS-Synthesizer Eurotherm-2000Series dummy Spectrum-MI40xxSeries Tecmag-DAC20
ifeq ($(shell /bin/uname -o),Cygwin)
SUBDIRS+=TiePie-HS3 Datel-PCI416
endif

all:
	for i in $(SUBDIRS); do make -C $$i all; done	

../tools/add_endline.exe: ../tools/add_endline.cpp
	$(CXX) $< -o $@

.PHONY: SpinCore-PulseBlasterDDSIII/SpinCore-PulseBlasterDDSIII.o TiePie-HS3/TiePie-HS3.a Eurotherm-2000Series/Eurotherm-2000Series.o clean install

tempcont.o: tempcont.cpp tempcont.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

SpinCore-PulseBlasterDDSIII/SpinCore-PulseBlasterDDSIII.o:
	$(MAKE) -C SpinCore-PulseBlasterDDSIII SpinCore-PulseBlasterDDSIII.o

Eurotherm-2000Series/Eurotherm-2000Series.o:
	$(MAKE) -C Eurotherm-2000Series Eurotherm-2000Series.o

TiePie-HS3/TiePie-HS3.a:
	$(MAKE) -C TiePie-HS3 TiePie-HS3.a

Datel-PCI416/Datel-PCI416.o:
	$(MAKE) -C Datel-PCI416 Datel-PCI416.o

clean: ../tools/add_endline.exe
	for f in ADC.h frequgen.h pulsegen.h tempcont.h ; do ../tools/add_endline.exe $$f; done; \
rm -f *~ *.o *.exe *.stackdump core.{0,1,2,3,4,5,6,7,8,9}*; \
for i in $(SUBDIRS); do make -C $$i clean; done

install: all
	for i in $(SUBDIRS); do make PREFIX=$(PREFIX) -C $$i install; done
