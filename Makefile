all: build/read_mars_cta


build/read_mars_cta: read_mars_cta.cxx | build
# check if root and MARS are available
ifndef ROOTSYS
  $(error ROOTSYS is undefined, source /path/to/root/bin/thisroot.sh)
endif
ifndef MARSSYS
  $(error MARSSYS is undefined, export MARSSYS=/path/to/mars)
endif

	$(CXX) read_mars_cta.cxx \
		-O2 --std=c++11 \
		`root-config --cflags --libs` \
		-o build/read_mars_cta \
		-I$$MARSSYS -lmars -L$$MARSSYS \
		-Wl,-rpath,$$MARSSYS \
		-Wl,-rpath,$$ROOTSYS/lib



build: 
	mkdir -p build

clean: 
	rm -rf build

.PHONY: clean all
