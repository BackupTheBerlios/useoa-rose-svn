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
CXXFLAGS = -fPIC -ftemplate-depth-50 -g #$(DEFINES) $(INCLUDES) $(DEBUGFLAGS)
LDFLAGS = -Wl,-rpath $(srcdir)/OAWraps/ -L$(srcdir)/OAWraps 

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

bin_PROGRAMS = OATest OACFGTest

all:  recursive

all-am:  $(bin_PROGRAMS)

OBJEXT = o
EXEEXT = 

OPENANALYSIS_LIB_WITH_PATH =$(OPENANALYSIS_DIR)/$(OPENANALYSIS_ARCH)/lib/libOAul.a
OATest_SOURCES = testAll.C
OATest_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH) $(OPENANALYSIS_LIB_WITH_PATH)
OATest_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS)

OACFGTest_SOURCES = CFGTest.C
OACFGTest_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH)
OACFGTest_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS)

am_OATest_OBJECTS = OATest.$(OBJEXT)
OATest_OBJECTS = $(am_OATest_OBJECTS)
OATest_LDFLAGS = 

am_OACFGTest_OBJECTS = OACFGTest.$(OBJEXT)
OACFGTest_OBJECTS = $(am_OACFGTest_OBJECTS)
OACFGTest_LDFLAGS =

OATest$(EXEEXT): $(OATest_OBJECTS) $(OATest_DEPENDENCIES) 
	@rm -f OATest$(EXEEXT)
	$(CXXLINK) $(OATest_LDFLAGS) $(OATest_OBJECTS) $(OATest_LDADD)

OACFGTest$(EXEEXT): $(OACFGTest_OBJECTS) $(OACFGTest_DEPENDENCIES) 
	@rm -f OACFGTest$(EXEEXT)
	$(CXXLINK) $(OACFGTest_LDFLAGS) $(OACFGTest_OBJECTS) $(OACFGTest_LDADD)


.C.o:
	$(CXXCOMPILE) -c -o $@ `test -f '$<' || echo '$(srcdir)/'`$<

clean-am:
	-rm -f *.$(OBJEXT) core *.core $(bin_PROGRAMS)

clean: clean-recursive

.PHONY: recursive all all-am clean-recursive clean clean-am

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
