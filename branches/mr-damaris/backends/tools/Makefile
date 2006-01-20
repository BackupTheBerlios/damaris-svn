#############################################################################
#
# Author: Achim Gaedke
# Created: June 2004
#
#############################################################################


CXX=g++
CXXFLAGS=-g -O0 -I. -I.. -Wshadow -Wall
LIBS=-lexpat -lxerces-c

.PHONY: all clean install

all: restart_core.exe ../core/core_config.o add_endline.exe

restart_core.o:  restart_core.cpp ../core/core_config.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

restart_core.exe: restart_core.o ../core/core_config.o
	$(CXX) $^ -o $@ $(LIBS)

add_endline.exe: add_endline.cpp
	$(CXX) $^ -o $@

../core/core_config.o:
	$(MAKE) -C ../core core_config.o

clean:
	rm -f *.exe *.o *~

install: restart_core.exe
	install restart_core.exe $(PREFIX)
