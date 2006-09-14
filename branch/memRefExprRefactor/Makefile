# Created 8/2002
# Got from Beata 3/2004, took out all excess stuff, MMS
# 9/2/04 - copied from OACVS/UseAliasStart-Beata

# Set the ROSE_INST_ROOT dir in Makefile.in to point to the root of
# your Rose installation.  Make certain you have done a make install.
# Set the OPENANALYSIS_LIB and OPENANALYSIS_INC dirs in Makefile.in
# to point to the library and include installattions of OpenAnalysis.

# Common variable settings (ROSE, etc)
include ./config/Makefile.inc

srcdir = $(topsrcdir)

DEFINES = -DHAVE_CONFIG_H -DOA_IRHANDLETYPE_UL $(GLOBAL_DEFINES) #-DOA_OPEN64 
INCLUDES = -I. -I.. -I./OAWraps/ $(ROSE_INCLUDES) $(OPENANALYSIS_INC)
#LIBS= -L$(srcdir)/OAWraps/ -lSage2OA $(OPENANALYSIS_LIB) $(ROSE_LIBS) -lm -lfl -lrt -lpthread
LIBS= -lSage2OA $(OPENANALYSIS_LIB)
CXXFLAGS = -fPIC -ftemplate-depth-50 $(PROFILE_OPTIONS) #$(DEFINES) $(INCLUDES) $(DEBUGFLAGS)
LDFLAGS = -Wl,-rpath $(srcdir)/OAWraps -L$(srcdir)/OAWraps $(PROFILE_OPTIONS) 

CXXCOMPILE = $(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS)
CXXLINK = $(CXXLD) $(LDFLAGS) -o $@

SUBDIRS = OAWraps

recursive:
	@set amf=$$2; \
	dot_seen=no; \
	target=all; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    dot_seen=yes; \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done; \
	if test "$$dot_seen" = "no"; then \
	  $(MAKE) "$$target-am" || exit 1; \
	fi; test -z "$$fail"

clean-recursive:
	@set amf=$$2; \
	dot_seen=no; \
	target=clean; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    dot_seen=yes; \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $$local_target) \
	   || case "$$amf" in *=*) exit 1;; *k*) fail=yes;; *) exit 1;; esac; \
	done; \
	if test "$$dot_seen" = "no"; then \
	  $(MAKE) "$$target-am" || exit 1; \
	fi; test -z "$$fail"

#LD_LIBRARY_PATH += /soft/apps/packages/gcc-3.1/lib

bin_PROGRAMS = OATest CtoOA staticVirtualMethodResolver astNormalizer nullTranslator CHAandAliasVirtualMethodResolver

all:  recursive

all-am:  $(bin_PROGRAMS)

OBJEXT = o
EXEEXT = 

OPENANALYSIS_LIB_WITH_PATH =$(OPENANALYSIS_DIR)/$(OPENANALYSIS_ARCH)/lib/libOAul.a

OATest_SOURCES = OATest.C
OATest_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH) $(OPENANALYSIS_LIB_WITH_PATH) OAWraps/libSage2OA.so
OATest_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS)

CtoOA_SOURCES = CtoOA.cpp
CtoOA_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH) $(OPENANALYSIS_LIB_WITH_PATH) OAWraps/libSage2OA.so
CtoOA_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS)

astNormalizer_SOURCES = astNormalizer.cpp
astNormalizer_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH) 
astNormalizer_LDADD = $(ROSE_LIBS)

nullTranslator_SOURCES = nullTranslator.cpp
nullTranslator_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH) $(OPENANALYSIS_LIB_WITH_PATH) OAWraps/libSage2OA.so
nullTranslator_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS)

OACFGTest_SOURCES = CFGTest.C
OACFGTest_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH)
OACFGTest_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS)

am_OATest_OBJECTS = OATest.$(OBJEXT)
OATest_OBJECTS = $(am_OATest_OBJECTS)
OATest_LDFLAGS = 

am_CtoOA_OBJECTS = CtoOA.$(OBJEXT)
CtoOA_OBJECTS = $(am_CtoOA_OBJECTS)
CtoOA_LDFLAGS = 

am_astNormalizer_OBJECTS = astNormalizer.$(OBJEXT)
astNormalizer_OBJECTS = $(am_astNormalizer_OBJECTS)
astNormalizer_LDFLAGS = 

am_nullTranslator_OBJECTS = nullTranslator.$(OBJEXT)
nullTranslator_OBJECTS = $(am_nullTranslator_OBJECTS)
nullTranslator_LDFLAGS = 

am_staticVirtualMethodResolver_OBJECTS = staticVirtualMethodResolver.$(OBJEXT)
staticVirtualMethodResolver_OBJECTS = $(am_staticVirtualMethodResolver_OBJECTS)
staticVirtualMethodResolver_LDFLAGS = 
staticVirtualMethodResolver_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH) $(OPENANALYSIS_LIB_WITH_PATH)
staticVirtualMethodResolver_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS)

am_CHAandAliasVirtualMethodResolver_OBJECTS = CHAandAliasVirtualMethodResolver.$(OBJEXT)
CHAandAliasVirtualMethodResolver_OBJECTS = $(am_CHAandAliasVirtualMethodResolver_OBJECTS)
CHAandAliasVirtualMethodResolver_LDFLAGS = 
CHAandAliasVirtualMethodResolver_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH) $(OPENANALYSIS_LIB_WITH_PATH)
CHAandAliasVirtualMethodResolver_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS)

am_OACFGTest_OBJECTS = OACFGTest.$(OBJEXT)
OACFGTest_OBJECTS = $(am_OACFGTest_OBJECTS)
OACFGTest_LDFLAGS =

OATest$(EXEEXT): $(OATest_OBJECTS) $(OATest_DEPENDENCIES) 
	@rm -f OATest$(EXEEXT)
	$(CXXLINK) $(OATest_LDFLAGS) $(OATest_OBJECTS) $(OATest_LDADD)

CtoOA$(EXEEXT): $(CtoOA_OBJECTS) $(CtoOA_DEPENDENCIES) 
	@rm -f CtoOA$(EXEEXT)
	$(CXXLINK) $(CtoOA_LDFLAGS) $(CtoOA_OBJECTS) $(CtoOA_LDADD)

astNormalizer$(EXEEXT): $(astNormalizer_OBJECTS) $(astNormalizer_DEPENDENCIES) 
	@rm -f astNormalizer$(EXEEXT)
	$(CXXLINK) $(astNormalizer_LDFLAGS) $(astNormalizer_OBJECTS) $(astNormalizer_LDADD)

nullTranslator$(EXEEXT): $(nullTranslator_OBJECTS) $(nullTranslator_DEPENDENCIES) 
	@rm -f nullTranslator$(EXEEXT)
	$(CXXLINK) $(nullTranslator_LDFLAGS) $(nullTranslator_OBJECTS) $(nullTranslator_LDADD)

staticVirtualMethodResolver$(EXEEXT): $(staticVirtualMethodResolver_OBJECTS) $(staticVirtualMethodResolver_DEPENDENCIES) 
	@rm -f staticVirtualMethodResolver$(EXEEXT)
	$(CXXLINK) $(staticVirtualMethodResolver_LDFLAGS) $(staticVirtualMethodResolver_OBJECTS) $(staticVirtualMethodResolver_LDADD)

OACFGTest$(EXEEXT): $(OACFGTest_OBJECTS) $(OACFGTest_DEPENDENCIES) 
	@rm -f OACFGTest$(EXEEXT)
	$(CXXLINK) $(OACFGTest_LDFLAGS) $(OACFGTest_OBJECTS) $(OACFGTest_LDADD)

CHAandAliasVirtualMethodResolver$(EXEEXT): $(CHAandAliasVirtualMethodResolver_OBJECTS) $(CHAandAliasVirtualMethodResolver_DEPENDENCIES) 
	@rm -f CHAandAliasVirtualMethodResolver$(EXEEXT)
	$(CXXLINK) $(CHAandAliasVirtualMethodResolver_LDFLAGS) $(CHAandAliasVirtualMethodResolver_OBJECTS) $(CHAandAliasVirtualMethodResolver_LDADD)

OACFGTest$(EXEEXT): $(OACFGTest_OBJECTS) $(OACFGTest_DEPENDENCIES) 
	@rm -f OACFGTest$(EXEEXT)
	$(CXXLINK) $(OACFGTest_LDFLAGS) $(OACFGTest_OBJECTS) $(OACFGTest_LDADD)


.C.o:
	$(CXXCOMPILE) -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<

.cpp.o:
	$(CXXCOMPILE) -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<

clean-am:
	-rm -f *.$(OBJEXT) core *.core $(bin_PROGRAMS)

clean: clean-recursive

.PHONY: recursive all all-am clean-recursive clean clean-am

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
