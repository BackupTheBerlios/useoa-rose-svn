# Version 1.10 of anl-arch.m4 by norris@cutie.local, modified on 04/09/05. -*- Autoconf -*-
AC_DEFUN([ROSE2OA_GETARCH],
[
AC_PREREQ(2.59)

RPATH_LDFLAG=""
LDINSTALL_NAME=""

dnl AC_DEFINE([ROSE2OA_ARCH], [], [Description])
AC_DEFINE([ROSE2OA_ARCH], ["LINUX"], [The ROSE2OA architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) 
ROSE2OA_ARCH=LINUX

case "$HOSTTYPE" in
*-aix*)
  arch=rs6000
  machine=rs6000
  ROSE2OA_ARCH=AIX
  AC_DEFINE([ROSE2OA_ARCH], ["AIX"], [The ROSE2OA architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
*-solaris2*)
  arch=solaris
  machine=solaris2
  if test "$CC" != "gcc"; then
      CFLAGS=-w -
  fi 
  ROSE2OA_ARCH=SOLARIS
  AC_DEFINE([ROSE2OA_ARCH], ["SOLARIS"], [The ROSE2OA architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
*-linux*)
  arch=linux
  machine=i386_linux
  RPATH_LDFLAG="-Wl,-rpath,"
  ROSE2OA_ARCH=LINUX
  AC_DEFINE([ROSE2OA_ARCH], ["LINUX"], [The ROSE2OA architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
*powermac*)
  os=`uname -s`
  arch=$os
  machine=powermac_$os
  RPATH_LDFLAG="-L"
  LDINSTALL_NAME="-install_name $PREFIX/"
  ROSE2OA_ARCH=MAC
  AC_DEFINE([ROSE2OA_ARCH], ["MAC"], [The ROSE2OA architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
*intel-pc*)
  os=`uname -s`
  arch=$os
  machine=intel_$os
  RPATH_LDFLAG="-L"
  LDINSTALL_NAME="-install_name $PREFIX/"
  ROSE2OA_ARCH=MAC
  AC_DEFINE([ROSE2OA_ARCH], ["MAC"], [The ROSE2OA architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
*win32*)
  arch=win32
  machine=win32
  ROSE2OA_ARCH=WIN32
  AC_DEFINE([ROSE2OA_ARCH], ["WIN32"], [The ROSE2OA architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
*)
  ROSE2OA_ARCH=UNKNOWN
  AC_DEFINE([ROSE2OA_ARCH], ["UNKNOWN"], [The ROSE2OA architecture name, e.g., LINUX, MAC, SOLARIS, AIX]) ;;
esac

AC_SUBST(machine)
AC_SUBST(arch)
AC_SUBST(RPATH_LDFLAG)
AC_SUBST(ROSE2OA_ARCH)

])
