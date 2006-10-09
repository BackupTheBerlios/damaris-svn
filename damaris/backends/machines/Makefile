#############################################################################
#
# Author: Achim Gaedke
# Created: June 2004
#
#############################################################################


CXX=g++
CXXFLAGS=-g -O0 -I. -I.. -I/opt/local/include
LDFLAGS=
LIBS=-lexpat -lxerces-c -lm -L/opt/local/lib

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

../drivers/Tecmag-DAC20/DAC20.o:
	$(MAKE) -C ../drivers/Tecmag-DAC20 DAC20.o

../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.a:
	$(MAKE) -C ../drivers/Spectrum-MI40xxSeries Spectrum-MI40xxSeries.a

../drivers/Eurotherm-2000Series/Eurotherm-2000Series.o:
	$(MAKE) -C ../drivers/Eurotherm-2000Series Eurotherm-2000Series.o

../drivers/tempcont.o:
	$(MAKE) -C ../drivers tempcont.o

../drivers/dummy/dummy.o:
	$(MAKE) -C ../drivers/dummy dummy.o

../core/core.a:
	$(MAKE) -C ../core core.a

hardware.o: hardware.cpp hardware.h
	$(CXX) -c $(CXXFLAGS) $< -o $@


Mobilecore.exe: Mobilecore.o hardware.o \
../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.a ../drivers/PTS-Synthesizer/PTS.o \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.o \
../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.o ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.o  ../core/core.a
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@ -lpthread

Mobilecore.o: Mobilecore.cpp \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h ../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.h ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.h \
../core/stopwatch.h ../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h
	$(CXX) -c $(CXXFLAGS) $< -o $@


bg_backend: bg_backend.o hardware.o \
../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.a ../drivers/PTS-Synthesizer/PTS.o \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.o ../drivers/Eurotherm-2000Series/Eurotherm-2000Series.o ../drivers/tempcont.o \
../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.o ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.o  ../core/core.a
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@ -lpthread

bg_backend.o: bg_backend.cpp  ../drivers/PTS-Synthesizer/PTS.h \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h \
../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h
	$(CXX) -c $(CXXFLAGS) $< -o $@


magnexgrad_backend: magnexgrad_backend.o hardware.o \
../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.a ../drivers/PTS-Synthesizer/PTS.o \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.o ../drivers/Eurotherm-2000Series/Eurotherm-2000Series.o ../drivers/tempcont.o \
../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.o ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.o  ../core/core.a
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@ -lpthread

magnexgrad_backend.o: magnexgrad_backend.cpp \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h \
../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

deuteron_backend.o: deuteron_backend.cpp \
../drivers/SpinCore-PulseBlasterDDSIII/SpinCore-PulseBlasterDDSIII.h \
../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

deuteron_backend.exe: deuteron_backend.o hardware.o \
../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.a ../drivers/PTS-Synthesizer/PTS.o \
../drivers/SpinCore-PulseBlasterDDSIII/SpinCore-PulseBlasterDDSIII.o \
../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.o ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.o  ../core/core.a
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@ -lpthread

dummycore.o: dummycore.cpp ../drivers/dummy/dummy.o
	$(CXX) -c $(CXXFLAGS) $< -o $@

dummycore.exe: dummycore.o  ../drivers/dummy/dummy.o hardware.o ../drivers/tempcont.o ../core/core.a
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@ -lpthread

clean: ../tools/add_endline.exe
	for f in *.cpp *.h; do ../tools/add_endline.exe $$f;done; \
rm -f *~ *.o *.exe *.stackdump magnexgrad_backend bg_backend

install: $(MACHINE)core.exe
	install $(MACHINE)core.exe $(PREFIX)

PFGcore.exe: PFGcore.o hardware.o \
../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.a ../drivers/PTS-Synthesizer/PTS.o \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.o ../drivers/Tecmag-DAC20/DAC20.o\
../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.o ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.o  ../core/core.a
	$(CXX) $(LDFLAGS) $^ $(LIBS) -o $@ -lpthread

PFGcore.o: PFGcore.cpp \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h ../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.h ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.h ../drivers/Tecmag-DAC20/DAC20.h\
../core/stopwatch.h ../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h
	$(CXX) -c $(CXXFLAGS) $< -o $@
