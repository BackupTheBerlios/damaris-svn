#############################################################################
#
# Author: Achim Gaedke
# Created: January 2005
#
#############################################################################

CXX=g++
CXXFLAGS=-O0 -g -I. -I../..
LIBS=-lpthread

.PHONY: all clean

all:	Eurotherm-2000Series.o

../../tools/add_endline.exe: ../../tools/add_endline.cpp
	$(CXX) $< -o $@

../tempcont.o: ../tempcont.h ../tempcont.cpp
	$(MAKE) -C .. tempcont.o

Eurotherm-2000Series.o: Eurotherm-2000Series.cpp Eurotherm-2000Series.h ../tempcont.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

test.exe: test.cpp Eurotherm-2000Series.o Eurotherm-2000Series.h  ../tempcont.o
	$(CXX) $(CXXFLAGS) -o $@ $< Eurotherm-2000Series.o ../tempcont.o $(LIBS)

gnuplot_output.exe: gnuplot_output.cpp Eurotherm-2000Series.o  ../tempcont.o
	$(CXX) $(CXXFLAGS) -o $@ $< Eurotherm-2000Series.o ../tempcont.o $(LIBS)

clean: ../../tools/add_endline.exe
	for f in Eurotherm-2000Series.cpp Eurotherm-2000Series.h test.cpp Makefile; do ../../tools/add_endline.exe $$f; done; \
rm -f *~ *.o *.exe *.stackdump core.{0,1,2,3,4,5,6,7,8,9}*;
