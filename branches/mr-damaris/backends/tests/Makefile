#############################################################################
#
# Author: Achim Gaedke
# Created: June 2004
#
#############################################################################


CXX=g++
CXXFLAGS=-g -O0 -I..
LDFLAGS=
LIBS=-lexpat

.PHONY: ../core/core.a all clean install

all: state_read_test.exe state_iterate_test.exe iterator_insert_test.exe stopwatch_test.exe

../core/core.a:
	$(MAKE) -C ../core core.a

iterator_insert_test.exe: iterator_insert_test.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

state_read_test.exe: state_read_test.o ../core/core.a
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

state_read_test.o: state_read_test.cpp ../core/states.h ../core/xml_states.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

state_iterate_test.exe: state_iterate_test.o ../core/core.a
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

state_iterate_test.o: state_iterate_test.cpp ../core/states.h ../core/xml_states.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

stopwatch_test.exe: stopwatch_test.cpp ../core/stopwatch.h
	$(CXX) $(CXXFLAGS) -o $@ $<

xml_result_test.exe: xml_result_test.cpp ../core/xml_result.h ../core/result.h ../core/core.a
	$(CXX) $(CXXFLAGS) -o $@ $<  $(LDFLAGS) $(LIBS)  ../core/core.a

clean:
	rm -f iterator_insert_test.exe state_read_test.exe state_iterate_test.exe stopwatch_test.exe *.o *~ *.stackdump core.{1,2,3,4,5,6,7,8,9}{0,1,2,3,4,5,6,7,8,9}*

install:
