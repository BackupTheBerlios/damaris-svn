CXX=g++
CXXFLAGS=-g -O0 -I. -I../.. -I/opt/local/include
LDFLAGS=-lexpat -lxerces-c -L/opt/local/lib

.PHONY: all clean install

all: DAC20.o DAC_test.exe pfg_test.exe

../../tools/add_endline.exe: ../../tools/add_endline.cpp
	$(CXX) $< -o $@

../../core/core.a:
	$(MAKE) -C ../../core core.a

clean: ../../tools/add_endline.exe
	for f in DAC20.cpp DAC20.h DAC_test.cpp pfg_test.cpp; do $< $$f; done
	rm -f *.exe *.o *~ core.*

DAC20.o: DAC20.cpp DAC20.h
	$(CXX) -D TIMING=9e-8  -c $(CXXFLAGS) -o $@ $<

DAC_test.exe: DAC_test.o DAC20.o ../../core/core.a
	$(CXX) $(LDFLAGS) -o $@ $^
pfg_test.exe: pfg_test.o DAC20.o ../../core/core.a
	$(CXX) $(LDFLAGS) -o $@ $^
