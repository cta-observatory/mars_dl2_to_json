# Convert MARS DL2 files to json and hdf5

A C++ program to read MARS DL2 root files and output
to json.

Build using `make`, requires ROOT and MARS build with that ROOT.


```
$ source /path/to/root/bin/thisroot.sh
$ export MARSSYS=/path/to/MARS
$ make
$ ./build/read_mars_cta input.root output.json
```

The binary is linked using rpath, so it is not required to
set the `LD_LIBRARY_PATH` for ROOT and MARS to use it.
