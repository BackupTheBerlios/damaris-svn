CXXFLAGS=-g -O0 -Wshadow -Wall -pedantic
CXXCPPFLAGS=-I../..
CXX=g++

all: SpinCore-PulseBlaster24Bit.o

test: test.cpp SpinCore-PulseBlaster24Bit.o ../SpinCore-PulseBlaster/SpinCore-PulseBlaster.o ../SpinCore-PulseBlaster/PulseBlasterProgram.o ../../core/core.a
	$(CXX) $(CXXFLAGS) $(CXXCPPFLAGS) -o $@ $^ -lm -lexpat -lxerces-c

../SpinCore-PulseBlaster/SpinCore-PulseBlaster.o: ../SpinCore-PulseBlaster/SpinCore-PulseBlaster.cpp ../SpinCore-PulseBlaster/SpinCore-PulseBlaster.h
	$(MAKE) -C ../SpinCore-PulseBlaster SpinCore-PulseBlaster.o

../SpinCore-PulseBlaster/PulseBlasterProgram.o: ../SpinCore-PulseBlaster/PulseBlasterProgram.cpp ../SpinCore-PulseBlaster/PulseBlasterProgram.h
	$(MAKE) -C ../SpinCore-PulseBlaster PulseBlasterProgram.o

../../core/core.a:
	$(MAKE) -C ../../core core.a

SpinCore-PulseBlaster24Bit.o: SpinCore-PulseBlaster24Bit.cpp SpinCore-PulseBlaster24Bit.h
	$(CXX) $(CXXFLAGS) $(CXXCPPFLAGS) -c -o $@ $<

clean:
	rm -f *.o *~ test
