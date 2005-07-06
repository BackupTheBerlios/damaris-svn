#############################################################################
#
# Author: Achim Gaedke
# Created: June 2004
#
#############################################################################
CXX=g++
CXXFLAGS=-g -O0 -Wall -Wshadow -I../.. -I.

.PHONY: all clean intall

all: SpinCore-PulseBlasterDDSIII.o

../../tools/add_endline.exe: ../../tools/add_endline.cpp
	$(CXX) $< -o $@

../SpinCore-PulseBlaster/SpinCore-PulseBlaster.o: ../SpinCore-PulseBlaster/SpinCore-PulseBlaster.cpp ../SpinCore-PulseBlaster/SpinCore-PulseBlaster.h
	$(MAKE) -C ../SpinCore-PulseBlaster SpinCore-PulseBlaster.o

SpinCore-PulseBlasterDDSIII.o: SpinCore-PulseBlasterDDSIII.cpp SpinCore-PulseBlasterDDSIII.h ../SpinCore-PulseBlaster/SpinCore-PulseBlaster.h ../pulsegen.h ../frequgen.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

../../core/core.a:
	make -C ../../core core.a

test.exe:       test.cpp SpinCore-PulseBlasterDDSIII.o ../SpinCore-PulseBlaster/SpinCore-PulseBlaster.o ../SpinCore-PulseBlaster/PulseBlasterProgram.o ../../core/core.a
	$(CXX) $(CXXFLAGS) -o $@ $^ -lexpat

clean: ../../tools/add_endline.exe
	for f in *.cpp *.h; do ../../tools/add_endline.exe $$f; done; \
rm -f *.stackdump *.o *.exe \#* *~

install:
	install PBD03PC.dll $(PREFIX)
