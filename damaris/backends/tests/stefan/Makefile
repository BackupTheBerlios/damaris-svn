#############################################################################
#
# Author: Stefan Reutter
# Created: August 2013
#
#############################################################################


CXX=g++
CXXFLAGS=-g -O0 -W -Wall -Wextra -Wshadow -pedantic -std=c++11 
CXXCPPFLAGS=-I../..
LDFLAGS=
LIBS=-lexpat -lxerces-c

.PHONY: ../../core/core.a all clean

all: test

clean:
	rm -f test *.o

../../core/core.a:
	$(MAKE) -C ../../core core.a
	
../../machines/hardware.o:
	$(MAKE) -C ../../machines hardware.o
	
test: testmain.o tjob.o tcore.o ../../machines/hardware.o ../../core/core.a
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)
	
testmain.o: testmain.cpp
	$(CXX) $(CXXFLAGS) $(CXXCPPFLAGS) -c -o $@ $<
	
tjob.o: tjob.cpp tjob.h
	$(CXX) $(CXXFLAGS) $(CXXCPPFLAGS) -c -o $@ $<
	
result.o: result.hpp 

tcore.o: tcore.cpp tcore.h
	$(CXX) $(CXXFLAGS) $(CXXCPPFLAGS) -c -o $@ $<
