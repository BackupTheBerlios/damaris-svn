#############################################################################
#
# Author: Achim Gaedke
# Created: June 2004
#
#############################################################################


CXX=g++
CXXFLAGS=-g -O0 -Wall -Wshadow -pedantic
CXXCPPFLAGS= -I. -I.. 
LDFLAGS=
LIBS=-lexpat -lxerces-c -lm

EXEEXT = .exe

MACHINES = \
	dummycore$(EXEEXT) \
	deuteron_backend$(EXEEXT) \
	magnexgrad_backend \
	magnexgrad_backend_dds \
	PFGcore$(EXEEXT) \
	fc2_backend$(EXEEXT) \
	fc1neu_backend$(EXEEXT) \
	bg_backend \
	Mobilecore$(EXEEXT) \
	Mobile_wo_sync_backend$(EXEEXT) \
	berta$(EXEEXT)

DRV_SPC_MI40xx    = ../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.a
DRV_PTS           = ../drivers/PTS-Synthesizer/PTS.o
DRV_DAC20         = ../drivers/Tecmag-DAC20/DAC20.o
DRV_PB_24BIT      = ../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.o
DRV_PB_DDSIII     = ../drivers/SpinCore-PulseBlasterDDSIII/SpinCore-PulseBlasterDDSIII.o
DRV_PB            = ../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.o
DRV_PB_PROG       = ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.o
DRV_EUROTHERM2000 = ../drivers/Eurotherm-2000Series/Eurotherm-2000Series.o
DRV_TEMPCONT      = ../drivers/tempcont.o


LINK = $(CXX) $(LDFLAGS)
LINK_MACHINE = echo "  Linking    $@" && $(LINK)

.cpp.o:
	@echo "  Compiling  $<"
	@$(CXX) -c $(CXXFLAGS) $(CXXCPPFLAGS) $< -o $@


.PHONY: all clean install

all: $(MACHINES)

../tools/add_endline$(EXEEXT): ../tools/add_endline.cpp
	$(CXX) $< -o $@

ifeq ($(shell uname -o),Cygwin)
../drivers/TiePie-HS3/TiePie-HS3.a:
	$(MAKE) -C ../drivers/TiePie-HS3 TiePie-HS3.a

../drivers/Datel-PCI416/Datel-PCI416.o:
	$(MAKE) -C ../drivers/Datel-PCI416 Datel-PCI416.o


NQRcore$(EXEEXT): NQRcore.o \
		hardware.o \
		../drivers/Datel-PCI416/Datel-PCI416.o \
		$(DRV_PB_DDSIII) $(DRV_PB) $(DRV_PB_PROG) \
		../core/core.a
	@$(LINK_MACHINE) $^ $(LIBS) -o $@

NQRcore.o: NQRcore.cpp
	$(CXX) -c $(CXXFLAGS) $^ -o $@

endif

../drivers/SpinCore-PulseBlasterPlus/SpinCore-PulseBlasterPlus.o:
	$(MAKE) -C ../drivers/SpinCore-PulseBlasterPlus SpinCore-PulseBlasterPlus.o

$(DRV_PB_DDSIII):
	$(MAKE) -C ../drivers/SpinCore-PulseBlasterDDSIII SpinCore-PulseBlasterDDSIII.o

$(DRV_PB_24BIT):
	$(MAKE) -C ../drivers/SpinCore-PulseBlaster24Bit SpinCore-PulseBlaster24Bit.o

$(DRV_PTS):
	$(MAKE) -C ../drivers/PTS-Synthesizer PTS.o

$(DRV_DAC20):
	$(MAKE) -C ../drivers/Tecmag-DAC20 DAC20.o

$(DRV_SPC_MI40xx):
	$(MAKE) -C ../drivers/Spectrum-MI40xxSeries Spectrum-MI40xxSeries.a

$(DRV_EUROTHERM2000):
	$(MAKE) -C ../drivers/Eurotherm-2000Series Eurotherm-2000Series.o

$(DRV_TEMPCONT):
	$(MAKE) -C ../drivers tempcont.o

../drivers/dummy/dummy.o:
	$(MAKE) -C ../drivers/dummy dummy.o

../core/core.a:
	$(MAKE) -C ../core core.a

hardware.o: hardware.cpp hardware.h


Mobilecore$(EXEEXT): Mobilecore.o \
		hardware.o \
		$(DRV_SPC_MI40xx) \
		$(DRV_PTS) \
		$(DRV_PB_24BIT) $(DRV_PB) $(DRV_PB_PROG) \
		../core/core.a
	@$(LINK_MACHINE) $^ $(LIBS) -o $@ -lpthread

Mobilecore.o: Mobilecore.cpp \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h ../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.h ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.h \
../core/stopwatch.h ../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h

Mobile_wo_sync_backend$(EXEEXT): Mobile_wo_sync_backend.o \
		hardware.o \
		$(DRV_SPC_MI40xx) \
		$(DRV_PTS) \
		$(DRV_PB_24BIT) $(DRV_PB) $(DRV_PB_PROG) \
		../core/core.a
	@$(LINK_MACHINE) $^ $(LIBS) -o $@ -lpthread

Mobile_wo_sync_backend.o: Mobile_wo_sync_backend.cpp \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h ../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.h ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.h \
../core/stopwatch.h ../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h


bg_backend: bg_backend.o \
		hardware.o \
		$(DRV_SPC_MI40xx) \
		$(DRV_PTS) \
		$(DRV_PB_24BIT) $(DRV_PB) $(DRV_PB_PROG) \
		$(DRV_EUROTHERM2000) $(DRV_TEMPCONT) \
		../core/core.a
	@$(LINK_MACHINE) $^ $(LIBS) -o $@ -lpthread

bg_backend.o: bg_backend.cpp  ../drivers/PTS-Synthesizer/PTS.h \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h \
../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h


magnexgrad_backend_dds: magnexgrad_backend_dds.o \
		hardware.o \
		$(DRV_SPC_MI40xx) \
		$(DRV_PTS) \
		$(DRV_PB_DDSIII) $(DRV_PB) $(DRV_PB_PROG) \
		$(DRV_EUROTHERM2000) $(DRV_TEMPCONT) \
		../core/core.a
	@$(LINK_MACHINE) $^ $(LIBS) -o $@ -lpthread

magnexgrad_backend_dds.o: magnexgrad_backend_dds.cpp \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h \
../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h


magnexgrad_backend: magnexgrad_backend.o \
		hardware.o \
		$(DRV_SPC_MI40xx) \
		$(DRV_PTS) \
		$(DRV_PB_24BIT) $(DRV_PB) $(DRV_PB_PROG) \
		$(DRV_EUROTHERM2000) $(DRV_TEMPCONT) \
		../core/core.a
	@$(LINK_MACHINE) $^ $(LIBS) -o $@ -lpthread

magnexgrad_backend.o: magnexgrad_backend.cpp \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h \
../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h

deuteron_backend.o: deuteron_backend.cpp \
../drivers/SpinCore-PulseBlasterDDSIII/SpinCore-PulseBlasterDDSIII.h \
../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h

deuteron_backend$(EXEEXT): deuteron_backend.o \
		hardware.o \
		$(DRV_SPC_MI40xx) \
		$(DRV_PTS) \
		$(DRV_PB_DDSIII) $(DRV_PB) $(DRV_PB_PROG) \
		../core/core.a
	@$(LINK_MACHINE) $^ $(LIBS) -o $@ -lpthread

dummycore.o: dummycore.cpp ../drivers/dummy/dummy.o

dummycore$(EXEEXT): dummycore.o \
		hardware.o \
		../drivers/dummy/dummy.o \
		$(DRV_TEMPCONT) \
		../core/core.a
	@$(LINK_MACHINE) $^ $(LIBS) -o $@ -lpthread

clean: ../tools/add_endline$(EXEEXT)
	for f in *.cpp *.h; do ../tools/add_endline$(EXEEXT) $$f;done
	rm -f $(MACHINES)
	rm -f *~ *.o *.exe *.stackdump
	rm -f ../tools/add_endline$(EXEEXT)

install: $(MACHINES)
	install $(MACHINES) $(PREFIX)

PFGcore$(EXEEXT): PFGcore.o \
		hardware.o \
		$(DRV_SPC_MI40xx) \
		$(DRV_PTS) \
		$(DRV_DAC20) \
		$(DRV_PB_24BIT) $(DRV_PB) $(DRV_PB_PROG) \
		../core/core.a
	@$(LINK_MACHINE) $^ $(LIBS) -o $@ -lpthread

PFGcore.o: PFGcore.cpp \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h ../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.h ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.h ../drivers/Tecmag-DAC20/DAC20.h\
../core/stopwatch.h ../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h

fc2_backend$(EXEEXT): fc2_backend.o \
		hardware.o \
		$(DRV_SPC_MI40xx) \
		$(DRV_PTS) \
		$(DRV_DAC20) \
		$(DRV_PB_24BIT) $(DRV_PB) $(DRV_PB_PROG) \
		../core/core.a
	@$(LINK_MACHINE) $^ $(LIBS) -o $@ -lpthread

fc2_backend.o: fc2_backend.cpp \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h ../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.h ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.h ../drivers/Tecmag-DAC20/DAC20.h\
../core/stopwatch.h ../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h

berta$(EXEEXT): berta.o \
		hardware.o \
		$(DRV_SPC_MI40xx) \
		$(DRV_PTS) \
		$(DRV_PB_24BIT) $(DRV_PB) $(DRV_PB_PROG) \
		../core/core.a
	@$(LINK_MACHINE) $^ $(LIBS) -o $@ -lpthread

berta.o: berta.cpp \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h ../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.h ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.h \
../core/stopwatch.h ../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h

fc1neu_backend$(EXEEXT): fc1neu_backend.o \
		hardware.o \
		$(DRV_SPC_MI40xx) \
		$(DRV_PTS) \
		$(DRV_DAC20) \
		$(DRV_PB_24BIT) $(DRV_PB) $(DRV_PB_PROG) \
		../core/core.a
	@$(LINK_MACHINE) $^ $(LIBS) -o $@ -lpthread

fc1neu_backend.o: fc1neu_backend.cpp \
../drivers/SpinCore-PulseBlaster24Bit/SpinCore-PulseBlaster24Bit.h ../drivers/SpinCore-PulseBlaster/SpinCore-PulseBlaster.h ../drivers/SpinCore-PulseBlaster/PulseBlasterProgram.h ../drivers/Tecmag-DAC20/DAC20.h\
../core/stopwatch.h ../drivers/Spectrum-MI40xxSeries/Spectrum-MI40xxSeries.h
