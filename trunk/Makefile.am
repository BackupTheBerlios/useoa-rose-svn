SUBDIRS = OAWraps/ 

ACLOCAL_AMFLAGS = -I config

bin_PROGRAMS = OATest CtoOA

OATest_SOURCES = OATest.C
OATest_CPPFLAGS =  -ftemplate-depth-50 -DHAVE_CONFIG_H -DOA_OPEN64 -DOA_IRHANDLETYPE_UL \
    -IOAWraps/ @ROSE_INCLUDES@  -I@OPENANALYSIS_DIR@/include
OATest_LDADD = OAWraps/liboarose.la -L@ROSE_LIB_DIR@ -lrose -L@OPENANALYSIS_DIR@/lib -lOAul \
    -lm -lpthread

CtoOA_SOURCES = CtoOA.cpp
CtoOA_CPPFLAGS =  -ftemplate-depth-50 -DHAVE_CONFIG_H -DOA_OPEN64 -DOA_IRHANDLETYPE_UL \
    -IOAWraps/ @ROSE_INCLUDES@  -I@OPENANALYSIS_DIR@/include
CtoOA_LDADD = OAWraps/liboarose.la -L@ROSE_LIB_DIR@ -lrose -L@OPENANALYSIS_DIR@/lib -lOAul \
    -lm -lpthread

