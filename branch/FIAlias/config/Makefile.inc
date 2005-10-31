# Created 6/2002

SHELL=/bin/sh

# Top-level UseOA source directory
topsrcdir=/usr/casc/overture/bwhite/UseOA-ROSE-FIalias/

# Root of ROSE installation
ROSE_INST_ROOT=/usr/casc/overture/bwhite/rose-0.8.6a-latest/LINUX-3.3.2/

# OpenAnalysis variables
OPENANALYSIS_LIB =-L/usr/casc/overture/bwhite/rose-0.8.6a-latest/LINUX-3.3.2/src/midend/programAnalysis/OpenAnalysis -lOAul
OPENANALYSIS_INC =-I/usr/casc/overture/bwhite/rose-0.8.6a-latest/ROSE/src/midend/programAnalysis/ 
  
OPENANALYSIS_DIR=/usr/casc/overture/bwhite/OpenAnalysis-FIAlias-10-21-05
OPENANALYSIS_ARCH=i686-Linux
OPENANALYSIS_LIB =-L$(OPENANALYSIS_DIR)/$(OPENANALYSIS_ARCH)/lib -lOAul
OPENANALYSIS_INC =-I$(OPENANALYSIS_DIR)/$(OPENANALYSIS_ARCH)/include 

# Compiler/linker
CXX = g++
CXXLD = g++ 


ROSE_INCLUDES=-I$(ROSE_INST_ROOT)/include
ROSE_LIBS=-Wl,-rpath,$(ROSE_INST_ROOT)/lib -L$(ROSE_INST_ROOT)/lib -lrose -ledg -lm -lfl -lrt -lpthread


# ================ end of package-specific settings ================

.SUFFIXES: .cpp
