# Created 8/2002
# Got from Beata 3/2004, took out all excess stuff, MMS
# 9/2/04 - copied from OACVS/UseAliasStart-Beata

# Common variable settings (ROSE, etc)
include ./Makefile.inc

### NewOA
#OPENANALYSIS_DIR =/home/derivs2/mstrout/OA/OAclones/NewOA
OPENANALYSIS_ARCH=i686-Linux
#OPENANALYSIS_LIB =-L/usr/casc/overture/bwhite/rose-0.8.4c//LINUX-3.3.2/src/midend/programAnalysis/OpenAnalysis -lOAul
#OPENANALYSIS_INC =-I/usr/casc/overture/bwhite/rose-0.8.4c//LINUX-3.3.2/src/midend/programAnalysis/OpenAnalysis/include/
OPENANALYSIS_LIB =-L$(OPENANALYSIS_DIR)/$(OPENANALYSIS_ARCH)/lib -lOAul
OPENANALYSIS_INC =-I$(OPENANALYSIS_DIR)/$(OPENANALYSIS_ARCH)/include 
# ================ end of package-specific settings ================

SRC=OAWraps/Sage2OA.C testAll.cpp  OAWraps/SageOACallGraph.C OAWraps/MemSage2OA.C
OBJ=Sage2OA.o testAll.o SageOACallGraph.o MemSage2OA.o

DEFINES = -DHAVE_CONFIG_H -DOA_IRHANDLETYPE_UL #-DOA_OPEN64 
INCLUDES = -I. -I./OAWraps $(ROSE_INCLUDES) $(OPENANALYSIS_INC)
LIBS= $(ROSE_LIBS) -lm -lpthread $(OPENANALYSIS_LIB)
CXXFLAGS = -fPIC -ftemplate-depth-50 -g $(DEFINES) $(INCLUDES) $(DEBUGFLAGS)

#LD_LIBRARY_PATH += /soft/apps/packages/gcc-3.1/lib

all:  check


check: $(OBJ) 
	$(CXX) -o testAll.exe $(OBJ) $(LIBS)


.cpp.o: $(SRC) #$(DEP)
	$(CXX) -c $(CXXFLAGS) $<

.C.o: $(SRC) #$(DEP)
	$(CXX) -c $(CXXFLAGS) $<

Sage2OA.o: OAWraps/Sage2OA.C OAWraps/Sage2OA.h
	$(CXX) -c $(CXXFLAGS) OAWraps/Sage2OA.C

MemSage2OA.o: OAWraps/MemSage2OA.C OAWraps/MemSage2OA.h
	$(CXX) -c $(CXXFLAGS) OAWraps/MemSage2OA.C

SageOACallGraph.o: OAWraps/SageOACallGraph.C OAWraps/SageOACallGraph.h
	$(CXX) -c $(CXXFLAGS) OAWraps/SageOACallGraph.C

clean:
	$(RM) -f *.a *.o *.so *.exe

#%.d: %.cpp
#	$(SHELL) -ec '$(CXX) $(CXXFLAGS) -MM $(CPPFLAGS) $< \
#		 | sed "s/$*\\.o[ :]*/& $@ /g" > $@'

## Dependencies
#-include $(SRC:.cpp=.d)


