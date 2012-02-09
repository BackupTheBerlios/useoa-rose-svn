lib_LTLIBRARIES = liboarose.la
liboarose_la_SOURCES =  \
    MemSage2OA.C Sage2OA.C SageOACallGraph.C common.C \
    MemSage2OA.h Sage2OA.h SageOACallGraph.h common.h 

liboarose_la_CPPFLAGS = -ftemplate-depth-50 -DHAVE_CONFIG_H -DOA_OPEN64 -DOA_IRHANDLETYPE_UL \
    @ROSE_INCLUDES@ -I@OPENANALYSIS_DIR@/include 

liboarose_la_LIBADD = -L@OPENANALYSIS_DIR@/lib -lOAul -L@ROSE_LIB_DIR@ -lrose -lm -lpthread 

include_HEADERS = Sage2OA.h MemSage2OA.h SageOACallGraph.h common.h rose2OAconfig.h rose2OAunconfig.h

