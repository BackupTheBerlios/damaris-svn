# author: Achim Gaedke
# created: February 2005

ifdef KERNELRELEASE

obj-m += pulseblaster.o

else

ifeq ($(shell uname -o), GNU/Linux)
KERNELSRC=/lib/modules/$(shell uname -r)/build
endif

THISDIR=$(shell cd . >/dev/null; pwd)
LIBS=-lm
CXXFLAGS=-g -O0 -Wall -Wshadow -pedantic
CPPFLAGS=-I../..

all: PulseBlasterProgram.o SpinCore-PulseBlaster.o

pulseblaster_test: pulseblaster_test.cpp SpinCore-PulseBlaster.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o$@ $^ $(LIBS)

PulseBlasterProgram.o: PulseBlasterProgram.cpp PulseBlasterProgram.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o$@ $<

SpinCore-PulseBlaster.o: SpinCore-PulseBlaster.cpp SpinCore-PulseBlaster.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o$@ $<

../../core/core.a:
	$(MAKE) -C ../../core core.a

pulseblaster.ko:
	$(MAKE) -C $(KERNELSRC) SUBDIRS=$(THISDIR)

clean:
	rm -f *.o *~ pulseblaster_test
ifeq ($(shell uname -o), GNU/Linux)
	$(MAKE) -C $(KERNELSRC) SUBDIRS=$(THISDIR) clean
endif

endif
