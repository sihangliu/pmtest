#!/bin/bash
#make distclean
#make clean
#./src/cleanup.sh

#make NVML_DEBUG=yes USE_NVML=yes STD=-std=gnu99 EXTRA_CFLAGS="-D_ENABLE_UTRACE -Wno-error" 
make NVML_DEBUG=yes USE_NVML=yes STD=-std=gnu99 EXTRA_CFLAGS="-Wno-error" 
