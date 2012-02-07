dnl Version %I% of %M% by %@%, modified on %G%. -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.
dnl This script uses the macros in config/*.m4
define([AC_CACHE_LOAD], )dnl
define([AC_CACHE_SAVE], )dnl
AC_PREREQ(2.61)

AC_INIT([useoarose], [0.2.0], [adic@mcs.anl.gov])
AC_CONFIG_MACRO_DIR([config])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR([OAWraps/Sage2OA.C])
AC_CANONICAL_TARGET

dnl ---- Automake ----
AM_INIT_AUTOMAKE([foreign 1.10])
AM_MAINTAINER_MODE

dnl AC_CONFIG_HEADERS([OAWraps/config.h])

dnl This is the share directory
SHARE_DIR=$prefix/share
AC_SUBST(SHARE_DIR)

AM_SANITY_CHECK

dnl Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_LIBLTDL_CONVENIENCE
AC_PROG_LIBTOOL

dnl Checks for libraries.
AC_CHECK_LIB(rt, clock_gettime)

dnl Checks for library functions.
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(strstr)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_FUNCS(strchr memcpy abs)
AC_CHECK_HEADERS(unistd.h strings.h)
AC_HEADER_TIME

dnl Checks for typedefs, structures, and compiler characteristics.
dnl AC_C_CONST
dnl AC_C_INLINE
AC_STRUCT_TM

dnl USEOAROSE_OMEGALIBRARY
USEOAROSE_OPENANALYSIS
USEOAROSE_ROSE
dnl USEOAROSE_BOOST
dnl AX_BOOST

dnl Check for doxygen
dnl USEOAROSE_PROG_DOXYGEN

AC_ARG_ENABLE(targetfortran,
[  --enable-targetfortran    Fortran is the input language],
[case "${enableval}" in
  yes) targetfortran=true ;;
  no)  targetfortran=false ;;
  *) AC_MSG_ERROR(bad value ${targetfortran} for --enable-targetfortran) ;;
esac],[targetfortran=false])
AM_CONDITIONAL(TARGET_LANGUAGE_FORTRAN, test x$targetfortran = xtrue)

AC_SUBST(SHELL)

USEOAROSE_SRCDIR=`pwd`

AC_CONFIG_FILES([Makefile OAWraps/Makefile])
AC_OUTPUT

dnl use config.status instead of reconfigure if configure hasn't changed
AC_CONFIG_COMMANDS([outmsg],[chmod +x autogen.sh])

