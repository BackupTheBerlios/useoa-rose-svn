# Created 6/2002

# Set variables from the configure script
OADIR:=$(USEOA_ROSE_OADIR)
ROSEDIR:=$(USEOA_ROSE_ROSEDIR)
CXXFLAGS:=$(USEOA_ROSE_CXXFLAGS)
ARCH:=$(USEOA_ROSE_ARCH)
srcdir=$(USEOA_ROSE_USEOAROSEDIR)

SHELL=/bin/sh

# Define UNRELEASED_ROSE if using ROSE-January18a-2006 or some derivative.
#GLOBAL_DEFINES = 
GLOBAL_DEFINES = -DUNRELEASED_ROSE

OPENANALYSIS_LIB =-L$(OADIR)/$(ARCH)/lib -lOAul
OPENANALYSIS_INC =-I$(OADIR)/$(ARCH)/include 

ROSE_INCLUDES=-I$(ROSEDIR)/include
ROSE_LIBS=-Wl,-rpath,$(ROSEDIR)/lib -L$(ROSEDIR)/lib -lrose -ledg -lm -lfl -lrt -lpthread


# ================ end of package-specific settings ================

.SUFFIXES: .cpp
