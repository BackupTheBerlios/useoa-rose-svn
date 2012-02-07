# Version 1.10 of anl-doxygen.m4 by norris@cutie.local, modified on 04/09/05. -*- Autoconf -*-
#------------------------------------------------------------------------------
# DOXYGEN_PATH: See if the user gave us the location of doxygen
# then search $anl_dox_dirs_dirs.
# One of these directories must be
# present and contain the correct files, or else the user
# must give the directory that contains the correct files.
#------------------------------------------------------------------------------
AC_DEFUN([USEOAROSE_PROG_DOXYGEN],
[
AC_PREREQ(2.59)
anl_dox_dirs="
    $prefix/bin/doxygen
    /usr/bin/doxygen
    /usr/local/bin/doxygen
    /opt/local/bin/doxygen
    /sw/bin/doxygen
    $ANL_HOME/doxygen
    $TOP/doxygen
    $HOME/doxygen/bin/doxygen
    $HOME/bin/doxygen
"
AC_ARG_WITH(doxygen,
    [Location of doxygen:
   --with-doxygen=DoxygenLocation
                          The location of doxygen. Give the full path:
                              --with-doxygen='/usr/local/bin/doxygen'],
    , [with_doxygen=_searching])

case "$with_doxygen" in
    no)
        AC_MSG_WARN([Option '--without-doxygen'
        makes it impossible to produce source-based documentation.])
        ;;
    yes)
        #  User didn't give the option or didn't give useful
        #  information, search for it ourselves
        with_doxygen=_searching
        ;;
    *)
        DOXYGEN_PATH="$with_doxygen"
        ;;
esac

if test "$with_doxygen" = _searching ; then
    AC_MSG_CHECKING([for OPTIONAL doxygen])
    for dox in $anl_dox_dirs ; do
        if test -x $dox ; then
            with_doxygen=$dox
            AC_MSG_RESULT([found $dox.])
            break
        fi
    done
fi

if test "$with_doxygen" = _searching ; then
    AC_MSG_RESULT([Cannot find doxygen.])
    AC_MSG_WARN([Without doxygen, it is impossible to produce source-based
        documentation.  If you wish to be able to generate this documentation,
        please re-run configure with a --with-doxygen=doxygen_path option.])
    with_doxygen="no"
fi

if test "$with_doxygen" = no ; then
   with_doxygen="@echo WARNING: doxygen not configured!"
fi

DOXYGEN_PATH="$with_doxygen"
AC_SUBST(DOXYGEN_PATH)
]
)
