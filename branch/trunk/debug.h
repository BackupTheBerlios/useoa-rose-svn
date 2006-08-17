#ifndef USEOA_DEBUG_H_
#define USEOA_DEBUG_H_

/* This macro was taken from the OA_DEBUG_CTRL_MACRO in OA's Utils/Util.hpp header.
   For documentation on how this sucka' works look there.
*/
#define OA_GLOBAL_DEBUG_ON

#if defined OA_GLOBAL_DEBUG_ON

#define USEOA_DEBUG_CTRL_MACRO(ModuleNameListDEBUG, DeBugDEBUG)\
    {\
        DeBugDEBUG = false;\
        char *ept = getenv("USEOA_DEBUG");\
        if( ept != NULL ) {\
            while (*ept == ':') {++ept;}\
            char *ept1 = ept;\
            char *mpt1;\
            char *mpt;\
            mpt1 = ModuleNameListDEBUG;\
            while (*mpt1 == ':') {++mpt1;}\
            mpt = mpt1;\
            while(*mpt1 != '\0') {\
                while(*ept != '\0'){\
                    if( *ept == *mpt ){\
                        *ept++;\
                        *mpt++;\
                        if ((*ept == '\0' || *ept == ':' ) && \
                          (*mpt == '\0' || *mpt == ':')) {\
                            DeBugDEBUG = true;\
                            break;\
                        }\
                    }\
                    else {\
                        mpt = mpt1;\
                        while ((*ept != ':') && (*ept != '\0')){++ept;}\
                        while (*ept == ':') {++ept;}\
                    }\
                }\
                if (DeBugDEBUG) {break;}\
                while( (*mpt1 != ':') && (*mpt1 != '\0') ){++mpt1;}\
                while (*mpt1 == ':') {++mpt1;}\
                mpt = mpt1; ept = ept1;\
            }\
        }\
    }\

#else
/* 
*    If OA_GLOBAL_DEBUG_ON is not set, we still need to define DeBugDEBUG  
*/ 
#define USEOA_DEBUG_CTRL_MACRO(ModuleNameListDEBUG, DeBugDEBUG)\
    DeBugDEBUG = false;\

#endif


#endif
