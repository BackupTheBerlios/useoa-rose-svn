# Version 1.11 of anl-rose.m4 by norris@cutie.local, modified on 04/09/05. -*- Autoconf -*-
AC_DEFUN([USEOAROSE_ROSE],
[
AC_PREREQ(2.59)
AC_ARG_WITH(rose,
    [Location of ROSE installation:   
  --with-rose=ROSE_INST_ROOT   
                          The location of ROSE installation
                          Give the full path: e.g.
                              --with-rose=/usr/local/rose ] ,
    , [with_rose=yes])

case "$with_rose" in
    no)
        # lUser input error
        with_rose=_searching
        ;;
    yes)
        #  User didn't give the option or didn't give useful
        #  information, search for it ourselves
        with_rose=_searching
        ;;
    *)
        ROSE_INST_ROOT="$with_rose"
        ;;
esac

if test "$with_rose" = _searching ; then
    AC_MSG_CHECKING([for optional rose  header distribution])
    for chome in $rose_dirs ; do
        if test -d $chome ; then
            with_rose=$chome
            AC_MSG_RESULT([found $chome.])
            break
        fi
    done
fi
if test "$with_rose" = _searching ; then
    AC_MSG_RESULT([Cannot find ROSE sources directory.])
    AC_MSG_ERROR([Please install it and/or use --with-rose=ROSE_INST_ROOT.])
fi
if test ! -f "$with_rose/include/Cxx_Grammar.h"  ; then
    AC_MSG_RESULT([No include/Cxx_Grammar.h in $with_rose.])
    AC_MSG_ERROR([Please install ROSE with SAGE3 support or later.])
fi
ROSE_INST_ROOT="$with_rose"
ROSE_INCLUDES="-I$with_rose/include"
ROSE_LIB_DIR="$with_rose/lib"
AC_SUBST(ROSE_INST_ROOT)
AC_SUBST(ROSE_INCLUDES)
AC_SUBST(ROSE_LIB_DIR)
])
