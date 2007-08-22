#############################################################################
#
# Author: Achim Gaedke
# Created: June 2004
#
#############################################################################

CXX=g++
CXXFLAGS=-g -O0 -I. -I../..
LDFLAGS=

.PHONY: all clean install

all: TiePie-HS3.a

../../tools/add_endline.exe: ../../tools/add_endline.cpp
	$(CXX) $< -o $@

TiePie-HS3.o: TiePie-HS3.cpp TiePie-HS3.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

tiepie.o: tiepie.cpp tiepie.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

TiePie-HS3.a: TiePie-HS3.o tiepie.o
	$(AR) r $@ $^

HS3test.exe: HS3test.cpp TiePie-HS3.a ../../core/core.a
	$(CXX) $(CXXFLAGS) $^ -o $@

../../core/core.a:
	$(MAKE) -C ../../core core.a

clean:
	for f in *.cpp *.h; do ../../tools/add_endline.exe $$f; done; \
rm -f *.o *~ \#* *.stackdump *.a *.exe

install:
	install *.hex *.dll $(PREFIX)
