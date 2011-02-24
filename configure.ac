# -*-Shell-script-*-
#
# DjVuLibre-3.5
# Copyright (c) 2002  Leon Bottou and Yann Le Cun.
# Copyright (c) 2001  AT&T
#
# This software is subject to, and may be distributed under, the
# GNU General Public License, either Version 2 of the license,
# or (at your option) any later version. The license should have
# accompanied the software or you may obtain a copy of the license
# from the Free Software Foundation at http://www.fsf.org .
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# DjVuLibre-3.5 is derived from the DjVu(r) Reference Library
# distribued by Lizardtech Software.
#
# ----------------------------------------
# Initialization
# ----------------------------------------

AC_PREREQ([2.65])
AC_INIT([djvulibre],[3.5.24])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config)
AC_CONFIG_SRCDIR(INSTALL)
AC_CONFIG_HEADER(config.h:config/config.h.in)
AC_PREFIX_DEFAULT(/usr/local)
AC_CANONICAL_HOST

# Library version
LIBDJVU_VER_COMPAT=21
LIBDJVU_VER=`echo $PACKAGE_VERSION | sed -e 's/^3\.5\.//'`
LIBDJVU_VER=$LIBDJVU_VER:0:`expr $LIBDJVU_VER - $LIBDJVU_VER_COMPAT`
AC_SUBST(LIBDJVU_VER)

# Additional libraries
AC_ARG_WITH(extra-includes,
            AS_HELP_STRING([--with-extra-includes=DIR],[Define an additional directory for include files]),
        [ if test -d "$withval" ; then
            CPPFLAGS="$CPPFLAGS -I$withval" 
          else
            AC_MSG_ERROR([Cannot stat directory $withval])
          fi ] )

AC_ARG_WITH(extra-libraries,
            AS_HELP_STRING([--with-extra-libraries=DIR],[Define an additional directory for library files]),
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

# Extra flags to make dll
DLLFLAGS=
AC_SUBST(DLLFLAGS)

# Special cases
case "$host" in
  *-mingw32)
     DLLFLAGS="$DLLFLAGS -Wl,--export-all-symbols"
     LIBS=-lmsvcp60
  ;;
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
AC_PATH_PROG(AR, ar)
AC_PATH_PROG(TOUCH, touch)
AC_PATH_PROG(MKDIR, mkdir)
AC_PATH_PROG(RSVG, rsvg, false)
AC_PATH_PROG(CONVERT, convert, false)
AC_PROG_INSTALL

# tweak rm to make libtool happy
AC_PATH_PROG(RM, rm) 
RM="$RM -f"

# Libtool & Compilers
AC_DISABLE_STATIC
AC_PROG_RANLIB
LT_INIT
AC_PROG_CC
AC_PROG_CXX
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
AC_CXX_INTEL_ATOMIC_BUILTINS
AC_CXX_RPO

# ----------------------------------------
# Libraries
# ----------------------------------------

AC_CHECK_LIB(m,sqrt)

# AC_CHECK_LIB(iconv,libiconv_open) # not really used.


# ----------------------------------------
# Header Files
# ----------------------------------------

AC_HEADER_STDC
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_HEADER_SYS_WAIT     
AC_CHECK_HEADERS(wchar.h wctype.h sys/mman.h iconv.h)
AC_CHECK_HEADERS(stdint.h inttypes.h sys/ipc.h sys/shm.h)

# ----------------------------------------
# Types
# ----------------------------------------

AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_CHECK_TYPES(wchar_t)
AC_CHECK_TYPES(long long int)
AC_CHECK_TYPES(mbstate_t,,,[#include "wchar.h"])
AC_SYS_LARGEFILE

# ----------------------------------------
# Functions
# ----------------------------------------

AC_FUNC_MMAP
AC_FUNC_FORK
AC_FUNC_FSEEKO
AC_CHECK_FUNCS(wcrtomb iswspace setenv)
AC_CHECK_FUNCS(putc_unlocked strerror vsnprintf)
AC_CHECK_FUNCS(gethostname strftime getpwuid)
AC_CHECK_FUNCS(sigaction mkstemp)

# iconv function might be defined as libiconv in iconv.h
AC_MSG_CHECKING([for iconv])
ac_has_iconv=no
AC_LANG_PUSH(C)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include "iconv.h"]], [[iconv((iconv_t)0, (char**)0, (size_t*)0, (char**)0, (size_t*)0);]])],[ac_has_iconv=yes],[])
AC_MSG_RESULT($ac_has_iconv)
AC_LANG_POP(C)
if test x$ac_has_iconv = xyes ; then
    AC_DEFINE(HAVE_ICONV,1,
        [Define to 1 if you have the iconv function.])
fi


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

# ----------------------------------------
# What to compile
# ----------------------------------------


# Check compilation of xmltools
ac_enable_xmltools=yes
AC_ARG_ENABLE(xmltools,
   AS_HELP_STRING([--disable-xmltools],[Do not compile the djvu xml tools.]),
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
   AS_HELP_STRING([--enable-djview],[Enable the compilation of djview3.]),
   [ac_enable_djview=$enableval], 
   [ac_enable_djview=no] ) 
if test x$ac_enable_djview != xno ; then
    AC_MSG_WARN([Enabling compilation of djview3.
--------------------------------------------------------
*** The Qt2 or Qt3 based viewer, djview3, is considered obsolete.
*** The recommended viewer is now the Qt4 based viewer, djview4.
*** Djview4 comes in a separate source package that should be
*** compiled after installing the djvulibre library.
--------------------------------------------------------])
    # Search QT library
    AC_PATH_QT(,
        [ no_qt=yes
            AC_MSG_WARN([Qt3 support is disabled]) ])
    
    # Search X11 and Xt headers
    if test "$no_x" != "yes" ; then
        savedCFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS ${X_CFLAGS}"
        AC_CHECK_HEADERS(X11/extensions/Xext.h,,,[include <X11/Xlib.h>])
        AC_CHECK_HEADER(X11/Shell.h, [], [no_xt=yes])
        AC_CHECK_HEADER(X11/IntrinsicP.h, [], [no_xt=yes])
        CFLAGS="$savedCFLAGS"
        if test "$no_xt" = "yes" ; then
            AC_MSG_WARN([Xt include files not found])
        else
            AC_DEFINE(HAVE_XT,1,[Define if you have the Xt include files.])
        fi
    fi

    if test x$no_threads = xyes || test x$no_qt = xyes || test x$no_xt = xyes 
    then
       AC_MSG_WARN([Disabling compilation of djview.
--------------------------------------------------------
*** The GUI portion of this package will not be compiled.
*** This is probably not what you want.  Chech the output
*** of configure for warning messages.  Read file INSTALL. 
*** Investigate options --with-qt and --enable-threads.
*** Make sure the Xt include files are present.
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
   AS_HELP_STRING([--disable-i18n],[Do not prepare the localization files.]),
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



# ----------------------------------------
# Desktop files
# ----------------------------------------

ac_desktopfiles=yes
AC_ARG_ENABLE(desktopfiles,
    AS_HELP_STRING([--disable-desktopfiles],[Do not install scripts to register icons and menus.]),
    [ac_desktopfiles=$enableval])

# Check for xdg tools
AC_PATH_PROG(XDG_MIME, xdg-mime)
AC_PATH_PROG(XDG_ICON_RESOURCE, xdg-icon-resource)
AC_PATH_PROG(XDG_DESKTOP_MENU, xdg-desktop-menu)
if test -x ${XDG_MIME} && \
   test -x ${XDG_ICON_RESOURCE} && \
   test -x ${XDG_DESKTOP_MENU}
then 
  true
elif test "$ac_desktopfiles" != no ; then
  ac_desktopfiles=no
  AC_MSG_WARN([Cannot find the Xdg Portland tools.
--------------------------------------------------------
*** The scripts to register icons and menus 
*** ${datadir}/djvu/osi/desktop/register-djvu-mimetype, and
*** ${datadir}/djvu/djview3/desktop/register-djview3-menu
*** will not be installed.
--------------------------------------------------------])
fi

if test ${ac_desktopfiles} != no ; then
  DESKTOP_YES=''
  DESKTOP_NO='#'
else
  DESKTOP_YES='#'
  DESKTOP_NO=''
fi
AC_SUBST(DESKTOP_YES)
AC_SUBST(DESKTOP_NO)


# ----------------------------------------
# Flags for NSDEJAVU.SO
# ----------------------------------------

NSDEJAVU_CFLAGS=
NSDEJAVU_LIBS=
if test x$ac_enable_djview != xno 
then
    AC_PATH_GLIB
    NSDEJAVU_CFLAGS=${X_CFLAGS}
    case "$host" in
        *-cygwin* | *-mingw*)
            NSDEJAVU_LIBS="`echo ${X_LIBS} | sed -e 's/-lXext/-lXt -lXext/'`"
            NSDEJAVU_LIBS="-no-undefined $NSDEJAVU_LIBS"
            ;;
    esac
fi
AC_SUBST(NSDEJAVU_CFLAGS)
AC_SUBST(NSDEJAVU_LIBS)

# -----------
# How to install shared libraries
if test -z "$INSTALL_SHLIB" ; then
  INSTALL_SHLIB="${INSTALL_DATA}"
  case "$host" in
    *-cygwin* | *-mingw*)
        INSTALL_SHLIB="cp"
        ;;
  esac
fi
AC_SUBST(INSTALL_SHLIB)



# ----------------------------------------
# Compiler version check
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



# ----------------------------------------
# Qt version check
# ----------------------------------------

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
AC_CONFIG_FILES(libdjvu/ddjvuapi.pc)
AC_CONFIG_FILES(tools/Makefile)
AC_CONFIG_FILES(tools/jb2cmp/Makefile)
AC_CONFIG_FILES(xmltools/Makefile)
AC_CONFIG_FILES(gui/Makefile)
AC_CONFIG_FILES(gui/bin2cpp/Makefile)
AC_CONFIG_FILES(gui/djview/Makefile)
AC_CONFIG_FILES(gui/nsdejavu/Makefile)
AC_CONFIG_FILES(i18n/Makefile)
AC_CONFIG_FILES(i18n/ja/Makefile)
AC_CONFIG_FILES(i18n/de/Makefile)
AC_CONFIG_FILES(i18n/fr/Makefile)
AC_CONFIG_FILES(i18n/cs/Makefile)
AC_CONFIG_FILES(desktopfiles/Makefile)

# Go
AC_OUTPUT

# Final message
echo ""
echo "Configuration is done."
echo "Compile with command:"
test x$GXX = xyes && \
echo "    $ make depend     (optional!)"
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
#if defined(__CYGWIN32__) || !defined(WIN32)
# define UNIX 1
#endif

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
#define GCONTAINER_NO_MEMBER_TEMPLATES 1
#endif
#ifndef HAVE_TYPENAME
#define GCONTAINER_NO_TYPENAME 1
#endif

/* - COTHREAD */
#ifdef HAVE_COTHREAD
#ifndef HAVE_COTHREAD_PATCH
#define NO_LIBGCC_HOOKS 1
#endif
#endif

/* - JPEG */
#ifdef HAVE_JPEG
#define NEED_JPEG_DECODER 1
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




