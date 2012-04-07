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
AC_INIT([djvulibre],[3.5.25])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config)
AC_CONFIG_SRCDIR(libdjvu/ddjvuapi.cpp)
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
AC_PATH_PROG(RSVG, rsvg-convert, false)
AC_PATH_PROG(CONVERT, convert, false)
AC_PATH_PROG(INKSCAPE, inkscape, false)
AC_PROG_INSTALL

# tweak rm to make libtool happy
AC_PATH_PROG(RM, rm) 
RM="$RM -f"

# Libtool & Compilers
AC_DISABLE_STATIC
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

/* config.h: end */
#endif
])




