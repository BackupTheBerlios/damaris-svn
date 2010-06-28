CPPFLAGS=-Iinclude -I../..
CXXFLAGS=-Wshadow -Wall -pedantic -O0 -g
AR=ar

all: Spectrum-MI40xxSeries.a

#include/spcerr.h include/regs.h include/dlltyp.h include/spcioctl.inc: drv_header_v323b1742.zip
#	unzip -u $< "*.h" "*.txt" "*.inc" -d include/

include/spcerr.h include/regs.h include/dlltyp.h include/spcioctl.inc: drv_new.zip
	unzip -u $< "*.h" "*.txt" "*.inc" -d include/

Spectrum-MI40xxSeries.a: Spectrum-MI40xxSeries.o GatedData.o
	$(AR) r $@ $^

Spectrum-MI40xxSeries.o: Spectrum-MI40xxSeries.cpp Spectrum-MI40xxSeries.h GatedData.h include/spcerr.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

GatedData.o: GatedData.cpp GatedData.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

test.o: test.cpp Spectrum-MI40xxSeries.h include/spcerr.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

test: test.o Spectrum-MI40xxSeries.o ../../core/core.a
	$(CXX) -o $@ -I../.. ../ADC.h $^ -lpthread -lm

../../core/core.a:
	$(MAKE) -C ../../core core.a

clean:
	rm -f *.o *.a *~ test *.core
	rm -rf include
