# Makefile.inc

SHELL = /bin/sh

# Set variables from environment variables
ifndef USEOA_ROSE_OADIR
  ifndef OADIR
    $(error "Need OADIR env var set to OpenAnalysis path")
  else
    OADIR:=$(OADIR)
  endif
else
  OADIR:=$(USEOA_ROSE_OADIR)
endif

ifndef USEOA_ROSE_ROSEDIR
  ifndef ROSEDIR
    $(error "Need ROSEDIR env var set to a ROSE build directory path")
  else
    ROSEDIR:=$(ROSEDIR)
  endif
else
  ROSEDIR:=$(USEOA_ROSE_ROSEDIR)
endif

# get current directory
WD := $(shell pwd)
ifndef USEOA_ROSE_USEOAROSEDIR
  USEOAROSEDIR:=$(WD)
else
  USEOAROSEDIR:=$(USEOA_ROSE_USEOAROSEDIR)
endif

ifndef USEOA_ROSE_CXXFLAGS
  CXXFLAGS:=$(CXXFLAGS)
else
  CXXFLAGS:=$(USEOA_ROSE_CXXFLAGS)
endif

ifndef USEOA_ROSE_CCFLAGS
  CCFLAGS:=$(CCFLAGS)
else
  CCFLAGS:=$(USEOA_ROSE_CCFLAGS)
endif

ifndef USEOA_ROSE_ARCH
  PLATFORM := $(shell cd $(OADIR)/config && ./hpcplatform)
  ARCH:=$(PLATFORM)
  ifeq ($(PLATFORM),)
    $(error "Unknown/unsupported platform") # unavailable in older gmakes
  endif
else
  ARCH:=$(USEOA_ROSE_ARCH)
endif

ifndef USEOA_ROSE_CXX
  CXX:=$(CXX)
else
  CXX:=$(USEOA_ROSE_CXX)
endif

ifndef USEOA_ROSE_CC
  CC:=$(CC)
else
  CC:=$(USEOA_ROSE_CC)
endif

ifndef USEOA_ROSE_CXXLD
  ifndef CXXLD
    CXXLD:=$(CXX)
  else
    CXXLD:=$(CXXLD)
  endif
else
  CXXLD:=$(USEOA_ROSE_CXXLD)
endif

ifndef USEOA_ROSE_CCLD
  ifndef CCLD
    CCLD:=$(CC)
  else
    CCLD:=$(CCLD)
  endif
  CCLD:=$(CCLD)
else
  CCLD:=$(USEOA_ROSE_CCLD)
endif

# ----------------- Set up OpenAnalysis and ROSE lib and inc dirs

OPENANALYSIS_LIB =-L$(OADIR)/$(ARCH)/lib -lOAul
OPENANALYSIS_INC =-I$(OADIR)/$(ARCH)/include 

ROSE_INCLUDES=-I$(ROSEDIR)/include
ROSE_LIBS=-Wl,-rpath,$(ROSEDIR)/lib -L$(ROSEDIR)/lib -lrose -ledg -lm -lfl -lrt -lpthread


# ================ end of package-specific settings ================

.SUFFIXES: .cpp
