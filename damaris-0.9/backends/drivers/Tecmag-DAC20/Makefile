CXX=g++
CXXFLAGS=-g -O0 -Wall -Wshadow -pedantic
CXXCPPFLAGS=-I. -I../..
LDFLAGS=-lexpat -lxerces-c

.PHONY: all clean install

all: DAC20.o DAC_test.exe

../../tools/add_endline.exe: ../../tools/add_endline.cpp
	$(CXX) $< -o $@

../../core/core.a:
	$(MAKE) -C ../../core core.a

clean: ../../tools/add_endline.exe
	for f in DAC20.cpp DAC20.h DAC_test.cpp; do $< $$f; done
	rm -f *.exe *.o *~ core.*

DAC20.o: DAC20.cpp DAC20.h
	$(CXX) -c $(CXXFLAGS) $(CXXCPPFLAGS) -o $@ $<

DAC_test.o: DAC_test.cpp
	$(CXX) -c $(CXXFLAGS) $(CXXCPPFLAGS) -o $@ $<

DAC_test.exe: DAC_test.o DAC20.o ../../core/core.a
	$(CXX)  -o $@ $^ $(LDFLAGS)
