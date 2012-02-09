#ifndef USEOA_DEBUG_H_
#define USEOA_DEBUG_H_

/* This macro was taken from the OA_DEBUG_CTRL_MACRO in OA's Utils/Util.hpp header.
   For documentation on how this sucka' works look there.
   SHK: Sucka' or no sucka', added a wrapper to common.h/C to make the macro work with strings
*/
#define OA_GLOBAL_DEBUG_ON

#if defined OA_GLOBAL_DEBUG_ON

#define USEOA_DEBUG_CTRL_MACRO(ModuleNameListDEBUG, DeBugDEBUG)\
    {\
        DeBugDEBUG = false;\
        const char *ept = getenv("USEOA_DEBUG");\
        if( ept != NULL ) {\
            while (*ept == ':') {++ept;}\
            const char *ept1 = ept;\
            const char *mpt1;\
            const char *mpt;\
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

#ifndef USEOA_THROW_MACRO
#define USEOA_THROW_MACRO(StreamArgs) \
   { \
     std::ostringstream aLoNgAnDwEiRdLoCaLnAmeFoRtHiSmAcRoOnLy; \
     aLoNgAnDwEiRdLoCaLnAmeFoRtHiSmAcRoOnLy << __FILE__ << ":" << __LINE__ << ":" << StreamArgs << std::ends; \
     ROSE_ABORT(aLoNgAnDwEiRdLoCaLnAmeFoRtHiSmAcRoOnLy.str().c_str()); \
   }
#else
#error macro name USEOA_THROW_MACRO already in use
#endif

#endif
