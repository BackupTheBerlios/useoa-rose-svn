# Created 6/2002

SHELL=/bin/sh

# Define UNRELEASED_ROSE if using ROSE-January18a-2006 or some derivative.
GLOBAL_DEFINES = 
#GLOBAL_DEFINES = -DUNRELEASED_ROSE

# Top-level UseOA source directory
topsrcdir=/usr/casc/overture/bwhite/UseOA-ROSE-trunk/
#topsrcdir=/s/parsons/c/fac/mstrout/OA/UseOA-ROSE-FIAlias/
#topsrcdir=/s/parsons/c/fac/mstrout/OA/LINUX-x86-32bit-gcc4.0.2/UseOA-ROSE
#topsrcdir=/s/bach/l/under/stonea/berlios/UseOA-ROSE/trunk

# Root of ROSE installation
ROSE_INST_ROOT=/usr/casc/overture/bwhite/rose-0.8.6a-latest/LINUX-3.3.2/
#ROSE_INST_ROOT=/usr/casc/overture/bwhite/ROSE-January-18a-2006/LINUX-3.3.3/
#ROSE_INST_ROOT=/usr/casc/overture/bwhite/ROSE-January-27a-2006/LINUX-3.3.3/
#ROSE_INST_ROOT=/s/parsons/c/fac/mstrout/rose-0.8.6a/LINUX-3.3.4/
#ROSE_INST_ROOT=/s/bach/e/proj/oadev/ROSE/rose-for-4.0.2-temp/LINUX-CORE4-gcc4.0.2
#ROSE_INST_ROOT=/s/bach/e/proj/oadev/ROSE/rose-for-4.0.2-temp/armstrong
#ROSE_INST_ROOT=/s/bach/e/proj/oadev/ROSE/rose-for-64bit-temp/grace

# OpenAnalysis variables
#OPENANALYSIS_DIR=/s/parsons/c/fac/mstrout/OA/LINUX-x86-32bit-gcc4.0.2/OpenAnalysis
#OPENANALYSIS_DIR=/s/bach/l/under/stonea/berlios/openanalysis/OpenAnalysis/trunk
#OPENANALYSIS_DIR=/s/bach/l/under/stonea/berlios/openanalysis/OpenAnalysis/branch/rmIters
#OPENANALYSIS_DIR=/usr/casc/overture/bwhite/OpenAnalysis-svn/openanalysis/OpenAnalysis/trunk/
#OPENANALYSIS_DIR=/usr/casc/overture/bwhite/OpenAnalysis-rev29/
#OPENANALYSIS_DIR=/usr/casc/overture/bwhite/OpenAnalysis-rev30/
#OPENANALYSIS_DIR=/usr/casc/overture/bwhite/OpenAnalysis-rev31/
OPENANALYSIS_DIR=/usr/casc/overture/bwhite/OpenAnalysis/
OPENANALYSIS_ARCH=i686-Linux
#OPENANALYSIS_ARCH=opteron-Linux
OPENANALYSIS_LIB =-L$(OPENANALYSIS_DIR)/$(OPENANALYSIS_ARCH)/lib -lOAul
OPENANALYSIS_INC =-I$(OPENANALYSIS_DIR)/$(OPENANALYSIS_ARCH)/include 

# Compiler/linker
CXX = g++
CXXLD = g++

ROSE_INCLUDES=-I$(ROSE_INST_ROOT)/include
ROSE_LIBS=-Wl,-rpath,$(ROSE_INST_ROOT)/lib -L$(ROSE_INST_ROOT)/lib -lrose -ledg -lm -lfl -lrt -lpthread


# ================ end of package-specific settings ================

.SUFFIXES: .cpp
