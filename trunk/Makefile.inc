# Created 6/2002

SHELL=/bin/sh

# ROSE variables
ROSE_INST_ROOT=/s/parsons/c/fac/mstrout/rose-0.8.6a/LINUX-3.3.4/
ROSE_INCLUDES=-I$(ROSE_INST_ROOT)/include
ROSE_LIBS=-Wl,-rpath,$(ROSE_INST_ROOT)/lib -L$(ROSE_INST_ROOT)/lib -lrose -ledg -lrt


OPENANALYSIS_DIR=/s/parsons/c/fac/mstrout/OA/trunk/OpenAnalysis


CXX = g++33

# ================ end of package-specific settings ================

.SUFFIXES: .cpp
