# Version 1.10 of anl-arch.m4 by norris@cutie.local, modified on 04/09/05. -*- Autoconf -*-
AC_DEFUN([USEOAROSE_GETARCH],
[
AC_PREREQ(2.59)

RPATH_LDFLAG=""
LDINSTALL_NAME=""

dnl AC_DEFINE([USEOAROSE_ARCH], [], [Description])
AC_DEFINE([USEOAROSE_ARCH], ["LINUX"], [The USEOAROSE architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) 
USEOAROSE_ARCH=LINUX

case "$HOSTTYPE" in
*-aix*)
  arch=rs6000
  machine=rs6000
  USEOAROSE_ARCH=AIX
  AC_DEFINE([USEOAROSE_ARCH], ["AIX"], [The USEOAROSE architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
*-solaris2*)
  arch=solaris
  machine=solaris2
  if test "$CC" != "gcc"; then
      CFLAGS=-w -
  fi 
  USEOAROSE_ARCH=SOLARIS
  AC_DEFINE([USEOAROSE_ARCH], ["SOLARIS"], [The USEOAROSE architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
*-linux*)
  arch=linux
  machine=i386_linux
  RPATH_LDFLAG="-Wl,-rpath,"
  USEOAROSE_ARCH=LINUX
  AC_DEFINE([USEOAROSE_ARCH], ["LINUX"], [The USEOAROSE architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
*powermac*)
  os=`uname -s`
  arch=$os
  machine=powermac_$os
  RPATH_LDFLAG="-L"
  LDINSTALL_NAME="-install_name $PREFIX/"
  USEOAROSE_ARCH=MAC
  AC_DEFINE([USEOAROSE_ARCH], ["MAC"], [The USEOAROSE architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
*intel-pc*)
  os=`uname -s`
  arch=$os
  machine=intel_$os
  RPATH_LDFLAG="-L"
  LDINSTALL_NAME="-install_name $PREFIX/"
  USEOAROSE_ARCH=MAC
  AC_DEFINE([USEOAROSE_ARCH], ["MAC"], [The USEOAROSE architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
*win32*)
  arch=win32
  machine=win32
  USEOAROSE_ARCH=WIN32
  AC_DEFINE([USEOAROSE_ARCH], ["WIN32"], [The USEOAROSE architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
*)
  USEOAROSE_ARCH=UNKNOWN
  AC_DEFINE([USEOAROSE_ARCH], ["UNKNOWN"], [The USEOAROSE architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
esac

AC_SUBST(machine)
AC_SUBST(arch)
AC_SUBST(RPATH_LDFLAG)
AC_SUBST(USEOAROSE_ARCH)

])
