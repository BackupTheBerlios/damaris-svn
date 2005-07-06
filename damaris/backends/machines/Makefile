#############################################################################
#
# Author: Achim Gaedke
# Created: June 2004
#
#############################################################################


CXX=g++
CXXFLAGS=-g -O0 -I. -I..
LDFLAGS=
LIBS=-lexpat -lxerces-c -lm

MACHINE=dummy

.PHONY: all clean install

all: $(MACHINE)core.exe

../tools/add_endline.exe: ../tools/add_endline.cpp
	$(CXX) $< -o $@

ifeq ($(shell uname -o),Cygwin)
../drivers/TiePie-HS3/TiePie-HS3.a:
	$(MAKE) -C ../drivers/TiePie-HS3 TiePie-HS3.a

../drivers/Datel-PCI416/Datel-PCI416.o:
	$(MAKE) -C ../drivers/Datel-PCI416 Datel-PCI416.o


NQRcore.exe: NQRcore.o ../core/core.a hardware.o \
../drivers/Datel-PCI416/Datel-PCI416.o ../drivers/SpinCore-PulseBlasterDDSIII/SpinCore-PulseBlasterDDSIII.o \
../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.o ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.o
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@

NQRcore.o: NQRcore.cpp
	$(CXX) -c $(CXXFLAGS) $^ -o $@

endif

../drivers/SpinCore-PulseBlasterPlus/SpinCore-PulseBlasterPlus.o::
	$(MAKE) -C ../drivers/SpinCore-PulseBlasterPlus SpinCore-PulseBlasterPlus.o

../drivers/SpinCore-PulseBlasterDDSIII/SpinCore-PulseBlasterDDSIII.o::
	$(MAKE) -C ../drivers/SpinCore-PulseBlasterDDSIII SpinCore-PulseBlasterDDSIII.o

../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.o::
	$(MAKE) -C ../drivers/SpinCore-PulseBlaster24Bit SpinCore-PulseBlaster24Bit.o

../drivers/PTS-Synthesizer/PTS.o:
	$(MAKE) -C ../drivers/PTS-Synthesizer PTS.o

../drivers/dummy/dummy.o:
	$(MAKE) -C ../drivers/dummy dummy.o

../core/core.a:
	$(MAKE) -C ../core core.a

hardware.o: hardware.cpp hardware.h
	$(CXX) -c $(CXXFLAGS) $< -o $@


Mobilecore.exe: Mobilecore.o ../core/core.a hardware.o \
../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.o ../drivers/PTS-Synthesizer/PTS.o \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.o \
../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.o ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.o
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@ -lpthread

Mobilecore.o: Mobilecore.cpp \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h ../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.h ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.h \
../core/stopwatch.h ../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

dummycore.o: dummycore.cpp ../drivers/dummy/dummy.o
	$(CXX) -c $(CXXFLAGS) $< -o $@

dummycore.exe: dummycore.o ../core/core.a ../drivers/dummy/dummy.o hardware.o ../drivers/tempcont.o
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@ -lpthread

clean: ../tools/add_endline.exe
	for f in *.cpp *.h; do ../tools/add_endline.exe $$f;done; \
rm -f *~ *.o NQRcore.exe Mobilecore.exe dummycore.exe *.stackdump

install: $(MACHINE)core.exe
	install $(MACHINE)core.exe $(PREFIX)
