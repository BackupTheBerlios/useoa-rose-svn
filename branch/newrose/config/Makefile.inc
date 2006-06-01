# Created 6/2002

SHELL=/bin/sh

# Define UNRELEASED_ROSE if using ROSE-January18a-2006 or some derivative.
#GLOBAL_DEFINES = 
GLOBAL_DEFINES = -DUNRELEASED_ROSE

# Top-level UseOA source directory
#topsrcdir=/usr/casc/overture/bwhite/UseOA-ROSE-trunk/
#topsrcdir=/s/parsons/c/fac/mstrout/OA/UseOA-ROSE-FIAlias/
#topsrcdir=/s/parsons/c/fac/mstrout/OA/armstrong/UseOA-ROSE
#topsrcdir=/s/bach/l/under/stonea/berlios/UseOA-ROSE/trunk
topsrcdir=/home/stone48/UseOA-ROSE

# Root of ROSE installation
#ROSE_INST_ROOT=/s/parsons/c/fac/mstrout/rose-0.8.6a/LINUX-3.3.4/
#ROSE_INST_ROOT=/s/bach/e/proj/oadev/ROSE/rose-for-4.0.2-temp/LINUX-CORE4-gcc4.0.2
#ROSE_INST_ROOT=/s/bach/e/proj/oadev/ROSE/rose-for-64bit-temp/armstrong
#ROSE_INST_ROOT=/s/bach/e/proj/oadev/ROSE/rose-for-4.0.2-temp/armstrong
ROSE_INST_ROOT=/home/stone48/ROSE-CVS/gcc3.3.3

# OpenAnalysis variables
#OPENANALYSIS_DIR=/s/parsons/c/fac/mstrout/OA/armstrong/OpenAnalysis
#OPENANALYSIS_DIR=/s/bach/l/under/stonea/berlios/openanalysis/OpenAnalysis/trunk
#OPENANALYSIS_DIR=/s/bach/l/under/stonea/berlios/openanalysis/OpenAnalysis/branch/rmIters
OPENANALYSIS_DIR=/home/stone48/OACompile
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
