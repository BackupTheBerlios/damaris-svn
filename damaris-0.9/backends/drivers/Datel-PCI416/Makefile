#############################################################################
#
# Author: Achim Gaedke
# Created: June 2004
#
#############################################################################


# für den ersten Anlauf
CPP=g++
CPPFLAGS=-g -I. -I../..
LDFLAGS=

.PHONY: all clean install

../../tools/add_endline.exe: ../../tools/add_endline.cpp
	$(CXX) $< -o $@

all: Datel-PCI416.o

Datel-PCI416.o: Datel-PCI416.cpp Datel-PCI416.h PCI416_func_import.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

PCI416_func_import.cpp: 416inc/41632DLL.H
	grep "^TFP_" $< |sed 's/.* \(.*\);/pci41632dll_getproc(\1)/' >$@

clean:
	for f in *.cpp *.h; do ../../tools/add_endline.exe $$f; done; \
rm -f *.o *~ *.stackdump PCI416_func_import.cpp

install:

