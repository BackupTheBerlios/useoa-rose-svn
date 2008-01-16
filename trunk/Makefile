# Created 8/2002
# Got from Beata 3/2004, took out all excess stuff, MMS
# 9/2/04 - copied from OACVS/UseAliasStart-Beata

# AIS - 6/26/06
#  Added code for build.log, the log file will show the configuration options
#  set last time Make was run.  This information may be useful to aid in
#  debugging, also the benchmark scripts poll information from it.

# Set the ROSE_INST_ROOT dir in Makefile.in to point to the root of
# your Rose installation.  Make certain you have done a make install.
# Set the OPENANALYSIS_LIB and OPENANALYSIS_INC dirs in Makefile.in
# to point to the library and include installattions of OpenAnalysis.


#############################################################################
# Common variable settings (ROSE, etc)
include ./config/Makefile.inc

DEFINES = -DHAVE_CONFIG_H -DOA_IRHANDLETYPE_UL $(GLOBAL_DEFINES) #-DROSE_PRE_0_9_0B #-DOA_OPEN64 
INCLUDES = -I. -I.. -I./OAWraps/ $(ROSE_INCLUDES) $(OPENANALYSIS_INC) -I/fusion/research2/bwhite/cpp-annotation-language
INCLUDES = -I. -I.. -I./OAWraps/ $(ROSE_INCLUDES) $(OPENANALYSIS_INC) 
#LIBS= -L$(srcdir)/OAWraps/ -lSage2OA $(OPENANALYSIS_LIB) $(ROSE_LIBS) -lm -lfl -lrt -lpthread
LIBS= -lfl -lSage2OA $(OPENANALYSIS_LIB) -lm 
LDFLAGS = $(CXXFLAGS) -Wl,-rpath $(USEOAROSEDIR)/OAWraps -L$(USEOAROSEDIR)/OAWraps 

CXXCOMPILE = $(CXX) $(DEFINES) $(INCLUDES) $(CXXFLAGS) -fPIC -ftemplate-depth-50
CXXLINK = $(CXXLD) $(LDFLAGS) -o $@

SUBDIRS = OAWraps


all:  build_log recursive mark_compile_successful

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

#bin_PROGRAMS = OATest CtoOA CHAandAliasVirtualMethodResolver
#bin_PROGRAMS = OATest CtoOA genPairs
bin_PROGRAMS = OATest CtoOA

# recursive

all-am:  $(bin_PROGRAMS)

OBJEXT = o
EXEEXT = 

OPENANALYSIS_LIB_WITH_PATH =$(OADIR)/$(ARCH)/lib/libOAul.a

OATest_SOURCES = OATest.C
OATest_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH) $(OPENANALYSIS_LIB_WITH_PATH)
OATest_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS) 

CtoOA_SOURCES = CtoOA.cpp
CtoOA_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH) $(OPENANALYSIS_LIB_WITH_PATH)
CtoOA_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS)

genPairs_SOURCES = genPairs.cpp
genPairs_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH) $(OPENANALYSIS_LIB_WITH_PATH)
genPairs_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS)

OACFGTest_SOURCES = CFGTest.C
OACFGTest_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH)
OACFGTest_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS)

CHAandAliasVirtualMethodResolver_SOURCES = CHAandAliasVirtualMethodResolver.cpp
CHAandAliasVirtualMethodResolver_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH) $(OPENANALYSIS_LIB_WITH_PATH) OAWraps/libSage2OA.so
CHAandAliasVirtualMethodResolver_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS)

am_OATest_OBJECTS = OATest.$(OBJEXT)
OATest_OBJECTS = $(am_OATest_OBJECTS)
OATest_LDFLAGS = 

am_CtoOA_OBJECTS = CtoOA.$(OBJEXT)
CtoOA_OBJECTS = $(am_CtoOA_OBJECTS)
CtoOA_LDFLAGS = 

am_genPairs_OBJECTS = genPairs.$(OBJEXT)
genPairs_OBJECTS = $(am_genPairs_OBJECTS)
genPairs_LDFLAGS = 

am_staticVirtualMethodResolver_OBJECTS = staticVirtualMethodResolver.$(OBJEXT)
staticVirtualMethodResolver_OBJECTS = $(am_staticVirtualMethodResolver_OBJECTS)
staticVirtualMethodResolver_LDFLAGS = 
staticVirtualMethodResolver_DEPENDENCIES = $(ROSE_LIBS_WITH_PATH) $(OPENANALYSIS_LIB_WITH_PATH)
staticVirtualMethodResolver_LDADD = $(LIBS_WITH_RPATH) $(LIBS) $(ROSE_LIBS)

am_OACFGTest_OBJECTS = OACFGTest.$(OBJEXT)
OACFGTest_OBJECTS = $(am_OACFGTest_OBJECTS)
OACFGTest_LDFLAGS =

am_CHAandAliasVirtualMethodResolver_OBJECTS = CHAandAliasVirtualMethodResolver.$(OBJEXT)
CHAandAliasVirtualMethodResolver_OBJECTS = $(am_CHAandAliasVirtualMethodResolver_OBJECTS)
CHAandAliasVirtualMethodResolver_LDFLAGS = 

OATest$(EXEEXT): $(OATest_OBJECTS) $(OATest_DEPENDENCIES) 
	@rm -f OATest$(EXEEXT)
	$(CXXLINK) $(OATest_LDFLAGS) $(OATest_OBJECTS) $(OATest_LDADD)

CtoOA$(EXEEXT): $(CtoOA_OBJECTS) $(CtoOA_DEPENDENCIES) 
	@rm -f CtoOA$(EXEEXT)
	$(CXXLINK) $(CtoOA_LDFLAGS) $(CtoOA_OBJECTS) $(CtoOA_LDADD)

genPairs$(EXEEXT): $(genPairs_OBJECTS) $(genPairs_DEPENDENCIES) 
	@rm -f genPairs$(EXEEXT)
	$(CXXLINK) $(genPairs_LDFLAGS) $(genPairs_OBJECTS) $(genPairs_LDADD)


staticVirtualMethodResolver$(EXEEXT): $(staticVirtualMethodResolver_OBJECTS) $(staticVirtualMethodResolver_DEPENDENCIES) 
	@rm -f staticVirtualMethodResolver$(EXEEXT)
	$(CXXLINK) $(staticVirtualMethodResolver_LDFLAGS) $(staticVirtualMethodResolver_OBJECTS) $(staticVirtualMethodResolver_LDADD)

OACFGTest$(EXEEXT): $(OACFGTest_OBJECTS) $(OACFGTest_DEPENDENCIES) 
	@rm -f OACFGTest$(EXEEXT)
	$(CXXLINK) $(OACFGTest_LDFLAGS) $(OACFGTest_OBJECTS) $(OACFGTest_LDADD)


CHAandAliasVirtualMethodResolver$(EXEEXT): $(CHAandAliasVirtualMethodResolver_OBJECTS) $(CHAandAliasVirtualMethodResolver_DEPENDENCIES) 
	@rm -f CHAandAliasVirtualMethodResolver$(EXEEXT)
	$(CXXLINK) $(CHAandAliasVirtualMethodResolver_LDFLAGS) $(CHAandAliasVirtualMethodResolver_OBJECTS) $(CHAandAliasVirtualMethodResolver_LDADD)

.C.o: 
	$(CXXCOMPILE) -c -o $@ `test -f '$<' || echo '$(USEOAROSEDIR)/'`$<

.cpp.o: 
	$(CXXCOMPILE) -c -o $@ `test -f '$<' || echo '$(USEOAROSEDIR)/'`$<

clean-am:
	-rm -f *.$(OBJEXT) core *.core $(bin_PROGRAMS)

clean: clean-recursive clean_log

.PHONY: recursive all all-am clean-recursive clean clean-am


.PHONY : build_log mark_compile_successful clean_log
# added by AIS (06/25/06)
build_log:
	@echo -e "\033[32mOutputing flags to build.log\033[0m" 
	@date +"Compiling UseOA-ROSE on %D at %r %Z" > build.log 
	@echo "Compile started by" `whoami` >> build.log 
	@echo "OADIR:='$(OADIR)'"  >> build.log 
	@echo "ROSEDIR:='$(ROSEDIR)'"  >> build.log 
	@echo "USEOAROSEDIR:='$(USEOAROSEDIR)'"  >> build.log 
	@echo "CXXFLAGS:='$(CXXFLAGS)'"  >> build.log 
	@echo "CCFLAGS:='$(CCFLAGS)'"  >> build.log 
	@echo "ARCH:='$(ARCH)'"  >> build.log 
	@echo "CXX:='$(CXX)'"  >> build.log 
	@echo "CC:='$(CC)'"  >> build.log 
	@echo "CXXLD:='$(CXXLD)'"  >> build.log 
	@echo "CCLD:='$(CCLD)'"  >> build.log               
	@echo "CXX version => '$(shell $(CXX) --version)'"  >> build.log               

mark_compile_successful:
	@echo "COMPILE_SUCCESFUL='yes'" >> build.log
	@date +"Compilation finished on %D at %r %Z" >> build.log

clean_log:
	@echo "Project cleaned" > build.log

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
