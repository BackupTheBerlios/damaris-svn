#############################################################################
#
# Author: Achim Gaedke
# Created: June 2004
#
#############################################################################

CXX=g++
CXXFLAGS=-g -O0 -Wshadow -Wall -pedantic
CXXCPPFLAGS= -I. -I.. -I/usr/include 
LIBS=-lexpat -L/usr/lib
AR=ar

.PHONY: clean all install

CORE_CLASSES=job.o job_receiver.o result.o core_config.o core.o states.o xml_states.o xml_result.o

all: core.a

../tools/add_endline.exe: ../tools/add_endline.cpp
	$(CXX) $< -o $@

core.a:	$(CORE_CLASSES)
	$(AR) rs $@ $^

core.o: core.cpp core.h core_exception.h job.h

result.o: result.cpp result.h

job.o: job.cpp job.h

job_receiver.o: job_receiver.cpp job_receiver.h job.h

core_config.o: core_config.cpp core_config.h core_exception.h

states.o: states.cpp states.h

xml_states.o: xml_states.cpp xml_states.h states.h

xml_result.o: xml_result.cpp xml_result.h result.h

clean:	../tools/add_endline.exe
	for f in *.cpp *.h; do ../tools/add_endline.exe $$f; done; \
rm -f *.o *~ core.a

DLLS=/bin/cygexpat-0.dll /bin/cygwin1.dll

install:
	for dll in $(DLLS);do test -f $$dll && install $$dll $(PREFIX); done

.cpp.o:
	@echo "  Compiling  $<"
	@$(CXX) -c $(CXXFLAGS) $(CXXCPPFLAGS) $< -o $@
