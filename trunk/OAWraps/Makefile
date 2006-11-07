# Created 8/2002
# Got from Beata 3/2004, took out all excess stuff, MMS
# 9/2/04 - copied from OACVS/UseAliasStart-Beata

# Set the ROSE_INST_ROOT dir in Makefile.in to point to the root of
# your Rose installation.  Make certain you have done a make install.
# Set the OPENANALYSIS_LIB and OPENANALYSIS_INC dirs in Makefile.in
# to point to the library and include installattions of OpenAnalysis.

# Common variable settings (ROSE, etc)
include ../config/Makefile.inc

srcdir = $(USEOAROSEDIR)/OAWraps/

DEFINES = -DHAVE_CONFIG_H -DOA_IRHANDLETYPE_UL $(GLOBAL_DEFINES) #-DOA_OPEN64 
INCLUDES = -I. -I.. $(ROSE_INCLUDES) $(OPENANALYSIS_INC)
LIBS= $(ROSE_LIBS) -lm -lfl -lrt -lpthread $(OPENANALYSIS_LIB)
CXXFLAGS_2 = $(CXXFLAGS) -fPIC -ftemplate-depth-50 
LDFLAGS = -fPIC $(PROFILE_OPTIONS)
AR = ar
CXX_SHARED_LIB_UPDATE = ${CXX} -shared -o 
CXX_STATIC_LIB_UPDATE = ${AR} cru

CXXCOMPILE = $(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS_2)
CXXLINK = $(CXXLD) $(CXXFLAGS_2) $(LDFLAGS) -o $@

#LD_LIBRARY_PATH += /soft/apps/packages/gcc-3.1/lib

bin_PROGRAMS = 

OBJEXT = o
EXEEXT = 

libSage2OASources = Sage2OA.C MemSage2OA.C SageOACallGraph.C common.C


#lib_LIBRARIES = libSage2OA.a
lib_LIBRARIES = libSage2OA.so 
LIBRARIES = $(lib_LIBRARIES)

libSage2OA_so_SOURCES = $(libSage2OASources)
am__objects_1 = Sage2OA.$(OBJEXT) SageOACallGraph.$(OBJEXT) MemSage2OA.$(OBJEXT) common.$(OBJEXT)
am_libSage2OA_so_OBJECTS = $(am__objects_1)
libSage2OA_so_OBJECTS = $(am_libSage2OA_so_OBJECTS)

libSage2OA_a_SOURCES = $(libSage2OASources)
am_libSage2OA_a_OBJECTS = $(am__objects_1)
libSage2OA_a_OBJECTS = $(am_libSage2OA_a_OBJECTS)

all: $(am__objects_1) $(LIBRARIES)

libSage2OA.a: $(libSage2OA_a_OBJECTS)
	rm -f $@
	$(CXX_STATIC_LIB_UPDATE) $@ $(libSage2OA_a_OBJECTS)
	ranlib $@
	test -f $@ || exit 1

libSage2OA.so: $(libSage2OA_so_OBJECTS)
	rm -f $@
	$(CXX_SHARED_LIB_UPDATE) $@ $(libSage2OA_so_OBJECTS)
	test -f $@ || exit 1

.C.o:
	$(CXXCOMPILE) -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<

clean:
	-rm -f *.$(OBJEXT) core *.core $(bin_PROGRAMS) $(LIBRARIES)

.PHONY: all clean

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:

