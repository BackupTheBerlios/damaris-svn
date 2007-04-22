#############################################################################
#
# Author: Achim Gaedke
# Created: June 2004
#
#############################################################################


CXX=g++
CXXFLAGS=-Wall -Wshadow -pedantic -g -O0
CXXCPPFLAGS=-I. -I../..
LIBS=-lpthread

.PHONY: clean install all

all:	dummy.o

../tempcont.o: ../tempcont.cpp ../tempcont.h
	$(MAKE) -C .. tempcont.o

../../core/core.a:
	$(MAKE) -C ../core core.a

dummy.o: dummy.h dummy.cpp ../tempcont.h ../ADC.h ../pulsegen.h ../frequgen.h
	$(CXX) $(CXXFLAGS) $(CXXCPPFLAGS) -c -o dummy.o dummy.cpp

temperature_dummy.exe: temperature_dummy.cpp dummy.o ../tempcont.o ../../core/core.a
	$(CXX) $(CXXFLAGS) -o temperature_dummy.exe temperature_dummy.cpp dummy.o ../tempcont.o ../../core/core.a $(LIBS)

../../tools/add_endline.exe: ../../tools/add_endline.cpp
	$(CXX) $< -o $@

clean: ../../tools/add_endline.exe
	for f in *.cpp *.h; do ../../tools/add_endline.exe $$f; done; \
rm -f *~ *.o

install:

