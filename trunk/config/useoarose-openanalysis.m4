# Version 1.10 of anl-newoa.m4 by norris@cutie.local, modified on 04/09/05. -*- Autoconf -*-
AC_DEFUN([USEOAROSE_OPENANALYSIS],
[
AC_PREREQ(2.59)
openanalysis_dirs="
    $OPENANALYSIS_HOME
    /usr/local/OpenAnalysis
    $HOME/OA/OpenAnalysis
    $HOME/OpenAnalysis
    $HOME/OpenAD/OpenAnalysis
    ../../OA/OpenAnalysis
"
AC_ARG_WITH(openanalysis,
    [Location of OpenAnalysis sources:   
  --with-openanalysis=OPENANALYSIS_DIR
                          The location of OpenAnalysis sources
                          Give the full path: e.g.
                              --with-openanalysis=/usr/local/OpenAnalysis ] ,
    , [with_openanalysis=yes])

case "$with_openanalysis" in
    no)
        # lUser input error
        with_openanalysis=_searching
        ;;
    yes)
        #  User didn't give the option or didn't give useful
        #  information, search for it ourselves
        with_openanalysis=_searching
        ;;
    *)
        OPENANALYSIS_DIR="$with_openanalysis"
        ;;
esac

if test "$with_openanalysis" = _searching ; then
    AC_MSG_CHECKING([for REQUIRED openanalysis distribution])
    for chome in $openanalysis_dirs ; do
        if test -d $chome ; then
            with_openanalysis=$chome
            AC_MSG_RESULT([found $chome.])
            break
        fi
    done
fi
if test "$with_openanalysis" = _searching ; then
    AC_MSG_RESULT([Cannot find OpenAnalysis sources directory.])
    AC_MSG_ERROR([Please install it and/or use --with-openanalysis=OPENANALYSIS_DIR.])
fi
if test ! -f "$with_openanalysis/include/OpenAnalysis/IRInterface/CFGIRInterface.hpp"  ; then
    AC_MSG_RESULT([No include/OpenAnalysis/IRInterface/CFGIRInterface.hpp in $with_openanalysis.])
    AC_MSG_ERROR([Please install OpenAnalysis.])
fi
OPENANALYSIS_DIR="$with_openanalysis"
AC_SUBST(OPENANALYSIS_DIR)

])
