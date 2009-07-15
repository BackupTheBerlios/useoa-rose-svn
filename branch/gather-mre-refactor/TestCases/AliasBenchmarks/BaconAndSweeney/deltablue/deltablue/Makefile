#CC = CC 
CC = gcc
#CC = /home/kahuna/local/bin/NEW/g++ 
#STATIC = #-Bstatic 
STATIC = #-static
#CFLAGS = -g  -fexternal-templates -O2 # NB: -fall-virtuals doesn't work
CFLAGS = -O4 -msupersparc
PROF = #-pg
OBJS1 = Constraints.o DeltaBlue.o TList.o UsefulConstraints.o DBBench.o List.o
OBJS2 = Constraints.o DeltaBlue.o TList.o UsefulConstraints.o UrsBench.o List.o
VPROFLIBS = # ../../vprof/bin/vprof-runtime/standard/vprof-runtime.o
LINKER = oopLinker

%.o: %.cc
	${CC} ${CFLAGS} -c $< 

default: deltabench

clean:
	rm *.o

deltabench: 	${OBJS1}
	${LINKER} ${STATIC} ${OBJS1} -o deltablue ${VPROFLIBS}

deltabench_p: 	${OBJS1}
	${LINKER} ${PROF} -static ${OBJS1} -o deltablue ${VPROFLIBS}

ursbench: 	${OBJS2}
	${LINKER} ${STATIC} ${OBJS2} -o ursbench ${VPROFLIBS}

ursbench_p: 	${OBJS2}
	${LINK} ${PROF} ${STATIC} ${OBJS2} -o ursbench_p ${VPROFLIBS}

List.o:	List.cc List.h top.h 

DeltaBlue.o:   DeltaBlue.cc DeltaBlue.h List.h Constraints.h top.h

Constraints.o:	 Constraints.h Constraints.cc List.h DeltaBlue.h top.h

UsefulConstraints.o: UsefulConstraints.h UsefulConstraints.cc Constraints.h List.h DeltaBlue.h top.h

DBBench.o: DBBench.cc UsefulConstraints.h Constraints.h List.h DeltaBlue.h top.h

UrsBench.o: UrsBench.cc UsefulConstraints.h Constraints.h List.h DeltaBlue.h top.h

TList.o: List.o
