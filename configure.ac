# -*-Shell-script-*-
#
# DjVuLibre-3.5
# Copyright (c) 2002  Leon Bottou and Yann Le Cun.
# Copyright (c) 2001  AT&T
#
# This software is subject to, and may be distributed under, the
# GNU General Public License, Version 2. The license should have
# accompanied the software or you may obtain a copy of the license
# from the Free Software Foundation at http://www.fsf.org .
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# DjVuLibre-3.5 is derived from the DjVu(r) Reference Library
# distribued by Lizardtech Software.  On July 19th 2002, Lizardtech 
# Software authorized us to replace the original DjVu(r) Reference 
# Library by the following notice (see etc/PATENT.djvu):
#
# ----------------------------------------
# Initialization
# ----------------------------------------

AC_PREREQ(2.50)
AC_INIT(djvulibre, 3.5.12)
AC_REVISION($Id$)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(INSTALL)
AC_CONFIG_HEADER(config.h:config/config.h.in)
AC_PREFIX_DEFAULT(/usr/local)
AC_CANONICAL_HOST

# ----------------------------------------
# Programs
# ----------------------------------------

# Does make set variable $(MAKE)
AC_PROG_MAKE_SET

# Programs
AC_PROG_RANLIB
AC_PROG_LN_S
AC_PATH_PROG(RM, rm)
AC_PATH_PROG(AR, ar)
AC_PATH_PROG(TOUCH, touch)

# Prefer the shell script
INSTALL='${top_srcdir}/config/install-sh -c' 
AC_SUBST(INSTALL)

# Compilers
AC_PROG_CC
AC_PROG_CXX
AC_LANG(C++)
AC_CXX_OPTIMIZE
AC_CXX_BOOL
AC_CXX_EXCEPTIONS
AC_CXX_TYPENAME
AC_CXX_STDINCLUDES
AC_CXX_NAMESPACES
AC_CXX_MEMBER_TEMPLATES
AC_CXX_RPO

dnl saved_CC="$CC"
dnl CC="$CXX"
dnl AC_LANG_PUSH(C)
dnl AC_DISABLE_STATIC
dnl AC_PROG_LIBTOOL
dnl AC_LANG_POP(C)
dnl CC="$saved_CC"
dnl
dnl This release does not use libtool because
dnl - libtool documentation suggest that it is 
dnl   not ready for general use with C++.

# ----------------------------------------
# Libraries
# ----------------------------------------

AC_CHECK_LIB(m,sqrt)
AC_CHECK_LIB(dl,dlopen)
AC_CHECK_LIB(iconv,libconv)

# ----------------------------------------
# Header Files
# ----------------------------------------

AC_HEADER_STDC
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_HEADER_SYS_WAIT     
AC_CHECK_HEADERS(wchar.h wctype.h sys/mman.h dlfcn.h iconv.h)
AC_CHECK_HEADERS(sys/ipc.h sys/shm.h)
AC_CHECK_HEADERS(X11/extensions/Xext.h,,,[#include <X11/Xlib.h>])

# ----------------------------------------
# Types
# ----------------------------------------

AC_CHECK_TYPES(wchar_t)
AC_CHECK_TYPES(long long int)
AC_CHECK_TYPES(mbstate_t,,,[#include "wchar.h"])

# ----------------------------------------
# Functions
# ----------------------------------------

AC_FUNC_MMAP
AC_FUNC_FORK
AC_CHECK_FUNCS(wcrtomb iswspace strerror vsnprintf)
AC_CHECK_FUNCS(gethostname dlopen iconv strftime)

# ----------------------------------------
# Test auxilliary packages
# ----------------------------------------

# Search JPEG library
AC_PATH_JPEG(,
[ no_jpeg=yes
  AC_MSG_WARN([JPEG support is disabled]) ])

# Search MULTITHREADING library
AC_PATH_THREADS(,
[ no_threads=yes
  AC_MSG_WARN([Thread support is disabled]) ])

# Search QT library
AC_PATH_QT(,
[ no_qt=yes
  AC_MSG_WARN([Qt support is disabled]) ])


# ----------------------------------------
# What to compile
# ----------------------------------------


# Check compilation of xmltools
ac_enable_xmltools=no
case "${host_os}" in
   *cygwin*) ac_enable_xmltools=yes ;;
esac
AC_ARG_ENABLE(xmltools,
   AC_HELP_STRING([--enable-xmltools],
                  [enable compilation of the djvu xml tools.]),
   [ac_enable_xmltools=$enableval])
if test x$ac_enable_xmltools != xno ; then
   XML_YES=''
   XML_NO='#'
else
   XML_YES='#'
   XML_NO=''
fi
AC_SUBST(XML_YES)
AC_SUBST(XML_NO)


# Check compilation of djview
AC_ARG_ENABLE(djview,
   AC_HELP_STRING([--enable-djview],
                  [enable compilation of djview (default).]),
   [ac_enable_djview=$enableval], 
   [ac_enable_djview=yes] ) 
if test x$ac_enable_djview != xno ; then
  if test x$no_threads = xyes || test x$no_qt = xyes ; then
     AC_MSG_WARN([Disabling compilation of djview.
--------------------------------------------------------
*** The GUI portion of this package will not be compiled.
*** This is probably not what you want.  Chech the output
*** of configure for warning messages.  Read file INSTALL. 
*** Investigate options --with-qt and --enable-threads.
--------------------------------------------------------])
     ac_enable_djview=no;
  fi
fi
if test x$ac_enable_djview != xno ; then
   GUI_YES=''
   GUI_NO='#'
else
   GUI_YES='#'
   GUI_NO=''
fi
AC_SUBST(GUI_YES)
AC_SUBST(GUI_NO)   


# ----------------------------------------
# Extra
# ----------------------------------------

# Should libdjvu be a shared library ?
ac_shared_libdjvu=no
if test x$ac_debug = xno ; then
  case "${host_os}" in
    *linux*) ac_shared_libdjvu=yes ;;
  esac
fi
AC_ARG_ENABLE([shared],
  AC_HELP_STRING([--enable-shared],
                 [Enable shared libdjvu (default: linux only)]),
  [ac_shared_libdjvu=$enableval] )
if test x$ac_shared_libdjvu != xno ; then
  SHARED_YES=''
  SHARED_NO='#'
else
  SHARED_YES='#'
  SHARED_NO=''
fi
AC_SUBST(SHARED_YES)
AC_SUBST(SHARED_NO)

# Work around broken compilers with the famous LPR0 bug
if test x$ac_shared_libdjvu != xno && test x$GXX = xyes; then
    AC_MSG_CHECKING([if $CXX has the LPR0 bug])
    echo 'extern "C" { inline int sqr(int x)throw(){return x*x;}}int main(){}' \
        > conftest.cc
    if ${CXX} -fPIC ${OPTS} conftest.cc -o conftest >/dev/null 2>/dev/null ; then
        AC_MSG_RESULT([no])
    else
        AC_MSG_RESULT([yes])
        OPTS=`echo $OPTS | sed -e 's/-mcpu=i686/-mpentium/'`
    fi
fi

# What compiler are we using ?
if test x$GXX != xyes ; then
  AC_MSG_WARN([The recommended c++ compiler is g++.])
  if test $ac_cv_cxx_exceptions = no ; then
    AC_MSG_ERROR([Exception support in c++ compiler is mandatory.]) 
  fi
else
  cxxver="`$CXX --version | sed -e 's/^gcc-//' -e 's/^egcs-//'`"
  action=none
  case "$cxxver" in
changequote(<<, >>)dnl                                                                              
    1.*)            action=error  ;;
    2.[0-8]*)       action=error  ;;
    2.90*)          action=error  ;;
    2.91.[6-9]*)    ;;
    2.91*)          action=warn2  ;;
    2.96*)          action=redhat ;;
    2.9*)           ;;
    3.0)            action=warn3  ;;
changequote([, ])dnl 
  esac
  case $action in
    error)
        AC_MSG_ERROR([Your c++ compiler ($cxxver) is outdated.])
        ;;
    warn2)
        AC_MSG_WARN([Your c++ compiler ($cxxver) has known problems.
-------------------------------------------
*** Please upgrade to gcc-2.95.3 or better.
-------------------------------------------])
        ;;
    warn3)
        AC_MSG_WARN([Your c++ compiler ($cxxver) has known problems.
-------------------------------------------
*** Please upgrade to gcc-3.0.2 or better.
-------------------------------------------])
        ;;
    redhat)
        # Check for infamous early 2.96 compiler
        for n in `g++ -v 2>&1 | grep version` ; do rhversion=$n ; done 
        case "$rhversion" in
changequote(<<, >>)dnl
          2.96-1[0-9][0-9]*) ;;
          2.96-8[5-9]*) ;;
          2.96-9*) ;;
          *)
changequote([, ])dnl 
             AC_MSG_WARN([Your c++ compiler ($cxxver) has known problems.
-------------------------------------------------------
*** You have an early version of the gcc-2.96 compiler.
*** The good news is that the new version works nicely.
*** Check <http://www.redhat.com/support/errata/RHBA-2001-082.html>.
*** Please upgrade.
-------------------------------------------------------])
            ;;
        esac
  esac
fi

# What version of QT are we using
if test x${qt_version+set} = xset ; then
  if test "$qt_version" -lt 200 ; then
     AC_MSG_ERROR([We no longer support Qt-1x.])
  elif test "$qt_version" -lt 220 ; then
     AC_MSG_WARN([We suggest using QT-2.2.0 or better.])
  fi
fi

# Rules for building the netscape plugin.
NSDEJAVU='nsdejavu.so'
NSDEJAVU_FLAGS="-fpic -DPIC $X_CFLAGS"
NSDEJAVU_LIBS="$X_LIBS"
NSDEJAVU_LINK='${CC} -shared -o ${NSDEJAVU} ${CFLAGS} -shared ${OBJS} ${LIBS}'
case ${host_os} in 
  hpux*) NSDEJAVU="nsdejavu.sl" ;; 
esac
if test x${GXX} != xyes ; then
    AC_MSG_WARN([Please verify that gui/nsdejavu/Makefile works with this compiler.])
fi
AC_SUBST(NSDEJAVU)
AC_SUBST(NSDEJAVU_FLAGS)
AC_SUBST(NSDEJAVU_LIBS)
AC_SUBST(NSDEJAVU_LINK)




# ----------------------------------------
# End
# ----------------------------------------

# Define installation paths and version string
AC_DEFINE_INSTALL_PATHS
AC_DEFINE_UNQUOTED(DJVULIBRE_VERSION,["${PACKAGE_VERSION}"],[version string])

# Output files
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(libdjvu/Makefile)
AC_CONFIG_FILES(tools/Makefile)
AC_CONFIG_FILES(xmltools/Makefile)
AC_CONFIG_FILES(gui/Makefile)
AC_CONFIG_FILES(gui/bin2cpp/Makefile)
AC_CONFIG_FILES(gui/djview/Makefile)
AC_CONFIG_FILES(gui/nsdejavu/Makefile)
AC_OUTPUT

# Final message
echo ""
echo "Configuration is done."
echo "Compile with command:"
test x$GXX = xyes && \
echo "    $ make depend"
echo "    $ make"
echo "Then install the binaries in $prefix with command:"
echo "    # make install"

# ----------------------------------------
# Stuff added to config.h
# ----------------------------------------

# Fence
AH_TOP([
#ifndef CONFIG_H
#define CONFIG_H
/* config.h: begin */
])

# L18N Macros
AH_BOTTOM([
/* - Miscellaneous */
#define AUTOCONF 1
#define UNIX 1

/* - BOOL */
#if !defined(HAVE_BOOL) && !defined(bool)
#define bool  char
#define true  1
#define false 0
#endif

/* - WCHAR etc.*/
#if ! defined(HAVE_WCHAR_T)
#define HAS_WCHAR 0
#define HAS_WCTYPE 0
#define HAS_MBSTATE 0
#else
#define HAS_WCHAR 1
#if defined(HAVE_WCTYPE_H) && defined(HAVE_ISWSPACE)
#define HAS_WCTYPE 1
#endif
#if defined(HAVE_MBSTATE_T) && defined(HAVE_WCRTOMB)
#define HAS_MBSTATE 1
#endif
#endif
#if defined(HAVE_ICONV_H) && defined(HAVE_ICONV)
#define HAS_ICONV 1
#else
#define HAS_ICONV 0
#endif

/* - I18N MESSAGES HELL */
#define HAS_CTRL_C_IN_ERR_MSG 1

/* - CONTAINERS */
#ifndef HAVE_MEMBER_TEMPLATES
#define GCONTAINER_NO_MEMBER_TEMPLATES
#endif
#ifndef HAVE_TYPENAME
#define GCONTAINER_NO_TYPENAME
#endif

/* - COTHREAD */
#ifdef HAVE_COTHREAD
#ifndef HAVE_COTHREAD_PATCH
#define NO_LIBGCC_HOOKS
#endif
#endif

/* - JPEG */
#ifdef HAVE_JPEG
#define NEED_JPEG_DECODER
#endif

/* - MMAP */
#if defined(HAVE_MMAP) && defined(HAVE_SYS_MMAN_H)
#define HAS_MEMMAP 1
#else
#define HAS_MEMMAP 0
#endif

/* - DLOPEN */
#if defined(HAVE_DLFCN_H) && defined(HAVE_DLOPEN)
#define HAS_DLOPEN 1
#else
#define HAS_DLOPEN 0
#endif

/* - QT */
#ifdef HAVE_QT
#if HAVE_QT < 200
#define QT1
#elif HAVE_QT < 300
#define QT2
#else
#define QT3
#endif
#endif

/* - X STUFF */
#ifdef HAVE_SYS_IPC_H
#ifdef HAVE_SYS_SHM_H
#define USE_XSHM 1
#endif
#endif

/* config.h: end */
#endif
])




