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
AC_INIT(djvulibre, 3.5.14)
AC_REVISION($Id$)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_SRCDIR(INSTALL)
AC_CONFIG_HEADER(config.h:config/config.h.in)
AC_PREFIX_DEFAULT(/usr/local)
AC_CANONICAL_HOST

AC_ARG_WITH(extra-includes,
            AC_HELP_STRING([--with-extra-includes=DIR],
                       [Define an additional directory for include files]),
        [ if test -d "$withval" ; then
            CPPFLAGS="$CPPFLAGS -I$withval" 
          else
            AC_MSG_ERROR([Cannot stat directory $withval])
          fi ] )

AC_ARG_WITH(extra-libraries,
            AC_HELP_STRING([--with-extra-libraries=DIR],
                       [Define an additional directory for library files]),
        [ if test -d "$withval" ; then
           LDFLAGS="$LDFLAGS -L$withval"
          else
            AC_MSG_ERROR([Cannot stat directory $withval])
          fi ] )

# Always look into a "gnu" directory.
curwd=`pwd`
if test -d $curwd/gnu/include ; then
   CPPFLAGS="$CPPFLAGS -I$curwd/gnu/include"
fi
if test -d $curwd/gnu/lib ; then
   LDFLAGS="$LDFLAGS -L$curwd/gnu/lib"
fi

# Special cases
case "$host" in
  *-darwin* | *-macos10*)
     if test -d /opt/local ; then
       CPPFLAGS="$CPPFLAGS -I/opt/local/include"
       LDFLAGS="$LDFLAGS -L/opt/local/lib"
     elif test -d /sw ; then
       CPPFLAGS="$CPPFLAGS -I/sw/include"
       LDFLAGS="$LDFLAGS -L/sw/lib"
     fi
  ;;
esac



# ----------------------------------------
# Programs
# ----------------------------------------

# Does make set variable $(MAKE)
AC_PROG_MAKE_SET

# Programs
AC_PROG_LN_S
AC_PATH_PROG(RM, rm)
AC_PATH_PROG(AR, ar)
AC_PATH_PROG(TOUCH, touch)
AC_PATH_PROG(MKDIR, mkdir)

# Prefer the shell script
INSTALL='${top_srcdir}/config/install-sh -c' 
AC_SUBST(INSTALL)

# Compilers
AC_PROG_CC
AC_PROG_CXX

# Libtool
AC_DISABLE_STATIC
AC_PROG_LIBTOOL
AC_SUBST(LIBTOOL_DEPS)

# C++
AC_LANG(C++)
AC_CXX_OPTIMIZE
AC_CXX_BOOL
AC_CXX_EXCEPTIONS
AC_CXX_TYPENAME
AC_CXX_STDINCLUDES
AC_CXX_NAMESPACES
AC_CXX_MEMBER_TEMPLATES
AC_CXX_RPO

# ----------------------------------------
# Libraries
# ----------------------------------------

AC_CHECK_LIB(m,sqrt)
AC_CHECK_LIB(iconv,libiconv)

# ----------------------------------------
# Header Files
# ----------------------------------------

AC_HEADER_STDC
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_HEADER_SYS_WAIT     
AC_CHECK_HEADERS(wchar.h wctype.h sys/mman.h iconv.h)
AC_CHECK_HEADERS(stdint.h sys/ipc.h sys/shm.h)
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
AC_CHECK_FUNCS(wcrtomb iswspace)
AC_CHECK_FUNCS(putc_unlocked strerror vsnprintf)
AC_CHECK_FUNCS(gethostname iconv strftime getpwuid)

# ----------------------------------------
# Test auxilliary packages
# ----------------------------------------

# Search JPEG library
AC_PATH_JPEG(,
[ no_jpeg=yes
  AC_MSG_WARN([JPEG support is disabled]) ])

# Search TIFF library
AC_PATH_TIFF(,
[ no_tiff=yes
  AC_MSG_WARN([Tiff support is disabled]) ])

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
ac_enable_xmltools=yes
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


# Check compilation of i18n files.
AC_ARG_ENABLE(i18n,
   AC_HELP_STRING([--enable-i18n],
                  [enable compilation of locales (default:all).]),
   [ac_enable_i18n=$enableval], 
   [ac_enable_i18n=all] ) 
if test "$ac_enable_i18n" != xno ; then
   I18N_YES=''
   I18N_NO='#'
   LINGUAS=
   if test "$ac_enable_i18n" = all ; then
     for n in $srcdir/i18n/* ; do
         if test -r $n/Makefile.in ; then 
             LINGUAS="$LINGUAS `basename $n`"
         fi
     done
   else
     for n in $ac_enable_i18n ; do
         if test -r $srcdir/i18n/$n/Makefile.in ; then
             LINGUAS="$LINGUAS $n"
         fi
     done
   fi
else
   I18N_YES='#'
   I18N_NO=''
fi
AC_SUBST(LINGUAS)
AC_SUBST(I18N_YES)
AC_SUBST(I18N_NO)   


# Locate desktop files (gui/desktop/acdesktop.m4)
if test $ac_enable_djview != no 
then
    AC_FIND_DESKTOP_DIRS
fi



# ----------------------------------------
# Extra
# ----------------------------------------

# Work around broken compilers with the famous LPR0 bug
if test x$ac_shared_libdjvu != xno && test x$GXX = xyes; then
    AC_MSG_CHECKING([if $CXX has the LPR0 bug])
    echo 'extern "C"{inline int sqr(int x)throw(){return x*x;}}int main(){}' \
        > conftest.cc
    if ${CXX} -fPIC ${OPTS} conftest.cc -o conftest \
        > /dev/null 2> /dev/null ; then
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
  AC_MSG_CHECKING([g++ version]) 
changequote(<<, >>)dnl
  cxxver="`$CXX --version | head -1 | 
             sed -e 's/^.*[^0-9.]\([2-4]\.[0-9.]*\).*$/\1/'`"
changequote([, ])dnl 
  AC_MSG_RESULT($cxxver)
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
  # Do we want the pragmas interface/implementation
  case "$cxxver" in
changequote(<<, >>)dnl
    2*) cxxpragmas=yes ;;
    3.0*) cxxpragmas=yes ;;
    *) cxxpragmas=no ;;
changequote([, ])dnl
  esac
  if test x$cxxpragmas = xyes ; then 
    AC_DEFINE(NEED_GNUG_PRAGMAS,1,
             [Define to use g++ pragmas interface/implementation])
  fi
fi

# What version of QT are we using
if test x${qt_version+set} = xset ; then
  if test "$qt_version" -lt 200 ; then
     AC_MSG_ERROR([We no longer support Qt-1x.])
  elif test "$qt_version" -lt 220 ; then
     AC_MSG_WARN([We suggest using QT-2.2.0 or better.])
  fi
fi


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
AC_CONFIG_FILES(gui/desktop/Makefile)
AC_CONFIG_FILES(gui/bin2cpp/Makefile)
AC_CONFIG_FILES(gui/djview/Makefile)
AC_CONFIG_FILES(gui/nsdejavu/Makefile)
AC_CONFIG_FILES(gsfiles/Makefile)
AC_CONFIG_FILES(i18n/Makefile)
AC_CONFIG_FILES(i18n/ja/Makefile)
AC_CONFIG_FILES(i18n/de/Makefile)
AC_CONFIG_FILES(i18n/fr/Makefile)
# Go
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




