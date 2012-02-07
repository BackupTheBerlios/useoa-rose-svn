lib_LTLIBRARIES = liboarose.la
liboarose_la_SOURCES =  \
    MemSage2OA.h Sage2OA.C SageOACallGraph.C common.C \
    MemSage2OA.C Sage2OA.h SageOACallGraph.h common.h

liboarose_la_CPPFLAGS = -ftemplate-depth-50 -DHAVE_CONFIG_H -DOA_OPEN64 -DOA_IRHANDLETYPE_UL \
    @ROSE_INCLUDES@ -I@OPENANALYSIS_DIR@/include 
 
#Commenting out for now. Jean Utke should uncomment when appropriate.     
#liboarose_la_CPPFLAGS = -ftemplate-depth-50 -DHAVE_CONFIG_H -DOA_OPEN64 -DOA_IRHANDLETYPE_UL \
#    @ROSE_INCLUDES@ @BOOST_CPPFLAGS@ -I@OPENANALYSIS_DIR@/include 

#if TARGET_LANGUAGE_FORTRAN    
#liboarose_la_CPPFLAGS += -DAD_FORTRAN_SPECIFIC
#endif

liboarose_la_LIBADD = -L@OPENANALYSIS_DIR@/lib -lOAul -L@ROSE_LIB_DIR@ -lrose -lm -lpthread 



