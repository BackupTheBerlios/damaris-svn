#############################################################################
#
# Author: Achim Gaedke
# Created: October 2004
#
#############################################################################

CXX=g++
CXXFLAGS=-g -O0 -I. -I../..
LDFLAGS=-lexpat

.PHONY: all clean install

all: PTS.o PTS_test.exe

../../tools/add_endline.exe: ../../tools/add_endline.cpp
	$(CXX) $< -o $@

../../core/core.a:
	$(MAKE) -C ../../core core.a

PTS.o: PTS.cpp ../frequgen.h
	$(CXX) -c $(CXXFLAGS) -o $@ $<

PTS_test.o: PTS_test.cpp PTS.h
	$(CXX) -c $(CXXFLAGS) -o $@ $<

PTS_test.exe: PTS_test.o PTS.o ../../core/core.a
	$(CXX) $(LDFLAGS) -o $@ $^

clean: ../../tools/add_endline.exe
	for f in PTS.cpp PTS.h PTS_test.cpp; do $< $$f; done
	rm -f *.exe *.o *~ core.*
