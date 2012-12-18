CPPFLAGS=-Iinclude -I../..
CXXFLAGS=-Wshadow -Wall -O0 -g
AR=ar
LIBS+= -lspcm_linux -lpthread -lm -lxerces-c -lexpat

SPC_HEADERS = include/spcerr.h include/regs.h include/dlltyp.h include/spcioctl.inc
SPC_ZIP = drv_spcm_linux_drv_v214b5633.zip
# SPC_ZIP = drv_new.zip

all: $(SPC_HEADERS) Spectrum-M2i40xxSeries.a

$(SPC_HEADERS): $(SPC_ZIP)
	unzip -u $< "*.h" "*.txt" "*.inc" -d include/

Spectrum-M2i40xxSeries.a: Spectrum-M2i40xxSeries.o GatedData.o
	$(AR) r $@ $^

Spectrum-M2i40xxSeries.o: Spectrum-M2i40xxSeries.cpp Spectrum-M2i40xxSeries.h GatedData.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

GatedData.o: GatedData.cpp GatedData.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

test.o: test.cpp Spectrum-M2i40xxSeries.h include/spcerr.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

test: test.o Spectrum-M2i40xxSeries.a ../../core/core.a
#	$(CXX) -o $@ -I../.. ../ADC.h $^ $(LIBS)
	$(CXX) -o $@ $(LIBS) -I../ADC.h $^
	

../../core/core.a:
	$(MAKE) -C ../../core core.a

clean:
	rm -f *.o *.a *~ test *.core
	rm -rf include
