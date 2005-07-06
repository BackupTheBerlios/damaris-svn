CPPFLAGS=-Iinclude -I../..
CXXFLAGS=-Wshadow -Wall -pedantic -O0 -g

all: Spectrum-MI40xxSeries.o

Spectrum-MI40xxSeries.o: Spectrum-MI40xxSeries.cpp Spectrum-MI40xxSeries.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

test.o: test.cpp Spectrum-MI40xxSeries.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

test: test.o Spectrum-MI40xxSeries.o ../../core/core.a
	$(CXX) -o $@ -I../.. ../ADC.h $^ -lpthread -lm

../../core/core.a:
	$(MAKE) -C ../../core core.a

clean:
	rm -f *.o *~ test *.core