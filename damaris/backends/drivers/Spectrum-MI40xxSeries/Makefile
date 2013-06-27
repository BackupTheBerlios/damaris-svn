CPPFLAGS=-Iinclude -I../..
CXXFLAGS=-Wshadow -Wall -pedantic -O0
AR=ar


SPC_HEADERS = include/spcerr.h include/regs.h include/dlltyp.h include/spcioctl.inc
SPC_ZIP = ../Spectrum-M2i40xxSeries/drv_spcm_linux_drv_v214b5633.zip

all: clean $(SPC_HEADERS) patch Spectrum-MI40xxSeries.a hw_test_int hw_test_ext

$(SPC_HEADERS): $(SPC_ZIP)
	unzip -u $< "*.h" "*.txt" "*.inc" -d include/

patch:
	patch -N -p0 < ftbfs_patch.diff

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
hw_test_int: hw_test_intclock.cpp 
	$(CXX) -o $@ -Iinclude $^
hw_test_ext: hw_test_extclock.cpp 
	$(CXX) -o $@ -Iinclude $^


../../core/core.a:
	$(MAKE) -C ../../core core.a

clean:
	rm -f *.o *.a *~ test *.core hw_test_ext hw_test_int
	rm -rf include
