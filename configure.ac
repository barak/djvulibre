# -*-Shell-script-*-
# ----------------------------------------
# Initialization
# ----------------------------------------

AC_PREREQ(2.50)
AC_INIT(djvu, 3.5)
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
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_LN_S
AC_PATH_PROG(RM, rm)
AC_PATH_PROG(AR, ar)
AC_PATH_PROG(TOUCH, touch)

# Compilers
AC_PROG_CC
AC_PROG_CXX
AC_LANG(C++)
AC_CXX_BOOL
AC_CXX_EXCEPTIONS
AC_CXX_MEMBER_TEMPLATES
AC_CXX_OPTIMIZE
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
dnl - libdjvu has not been sufficiently tested 
dnl   with compilers other than g++ >= 2.95


# ----------------------------------------
# Libraries
# ----------------------------------------

AC_CHECK_LIB(m,sqrt)
AC_CHECK_LIB(dl,dlopen)

# ----------------------------------------
# Header Files
# ----------------------------------------

AC_HEADER_STDC
AC_HEADER_DIRENT
AC_HEADER_TIME
AC_CHECK_HEADERS(wchar.h sys/mman.h dlfcn.h iconv.h)
AC_CHECK_HEADERS(sys/ipc.h sys/shm.h)

# ----------------------------------------
# Types
# ----------------------------------------

AC_CHECK_TYPES(wchar_t)
AC_CHECK_TYPES(mbstate_t,,,[#include "wchar.h"])

# ----------------------------------------
# Functions
# ----------------------------------------

AC_FUNC_MMAP
AC_CHECK_FUNCS(wcrtomb strerror vsnprintf)
AC_CHECK_FUNCS(gethostname dlopen iconv)

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

# Check compilation of djview
AC_ARG_ENABLE(djview,
   AC_HELP_STRING([--enable-djview],
                  [enable compilation of djview (default).]),
   [ac_enable_djview=$enableval], 
   [ac_enable_djview=no] ) 
if test x$no_threads = xyes ||
   test x$no_qt = xyes ; then
   AC_MSG_WARN([Disabling compilation of djview])
   GUI_YES='#'
   GUI_NO=''
else
   GUI_YES=''
   GUI_NO='#'
fi
AC_SUBST(GUI_YES)
AC_SUBST(GUI_NO)   


# ----------------------------------------
# Extra
# ----------------------------------------

# What compiler are we using ?
if test x$GXX != xyes ; then
  AC_MSG_WARN([g++ is the recommended compiler: expect trouble.])
  if test $ac_cv_cxx_exceptions = no ; then
    AC_MSG_ERROR([exception support in c++ compiler is mandatory.]) 
  fi
else
  case "`$CXX --version`" in
    *2.95.*) ;;
    *2.96*) ;;
    *3.*) AC_MSG_WARN([no serious tests have been made with g++ 3.x.]) ;;
    *2.91.*) AC_MSG_WARN([strongly suggesting g++ 2.95.3 or better]) ;;
    *) AC_MSG_ERROR([use a better compiler such as g++ 2.95.3 or better]) ;;
  esac
fi

# What version of QT are we using
if test x${qt_version+set} = xset ; then
  if test "$qt_version" -lt 220 ; then
     AC_MSG_WARN([We suggest using QT-2.2.0 or better.])
  elif test "$qt_version" -ge 300 ; then
     AC_MSG_ERROR([We do not yet support QT-3.])
  fi
fi

# How to build the netscape plugin ?
case ${host_os} in
  hpux*)
    NSDEJAVU_PROGRAM="nsdejavu.sl"
    NSDEJAVU_LDFLAGS="-shared -Wl,-o,nsdejavu.sl,+I,plugin_init"
    NSDEJAVU_LIBS="-Wl,-a,archive_shared -lgcc"
    ;;
  *)
    NSDEJAVU_PROGRAM="nsdejavu.so"
    NSDEJAVU_LDFLAGS="-shared"
    if test -z "$x_libraries" ; then
       NSDEJAVU_LIBS="-lXt -lX11 -lgcc -lm -lc"
    else
       NSDEJAVU_LIBS="-L$x_libraries -lXt -lX11 -lgcc -lm -lc"
    fi
    ;;
esac
AC_SUBST(NSDEJAVU_PROGRAM)
AC_SUBST(NSDEJAVU_LDFLAGS)
AC_SUBST(NSDEJAVU_LIBS)

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
    echo 'extern "C" { inline int sqr(int x) throw() { return x * x ; } } int main() {} ' > conftest.cc
    if ${CXX} -fPIC ${OPTS} conftest.cc -o conftest >/dev/null 2>/dev/null ; then
        AC_MSG_RESULT([no])
    else
        AC_MSG_RESULT([yes])
        OPTS=`echo $OPTS | sed -e 's/-mcpu=i686/-mpentium/'`
    fi
fi


# ----------------------------------------
# End
# ----------------------------------------

# Define installation paths
AC_DEFINE_INSTALL_PATHS

# Output files
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(libdjvu/Makefile)
AC_CONFIG_FILES(tools/Makefile)
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
#define bool char
#endif

/* - WCHAR etc.*/
#if ! defined(HAVE_WCHAR_T)
#define HAS_WCHAR 0
#define HAS_MBSTATE 0
#else
#define HAS_WCHAR 1
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
#endif
#if HAVE_QT < 300
#define QT2
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




