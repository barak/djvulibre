
dnl -------------------------------------------------------
dnl @synopsis AC_DEFINE_INSTALL_PATHS
dnl Define various installation paths
dnl -------------------------------------------------------
AC_DEFUN([AC_DEFINE_INSTALL_PATHS],[
  save_prefix="${prefix}"
  save_exec_prefix="${prefix}"
  test "x$prefix" = xNONE && prefix="$ac_default_prefix"
  test "x$exec_prefix" = xNONE && exec_prefix="$prefix"
  DIR_PREFIX="`eval echo \"$prefix\"`"
  AC_DEFINE_UNQUOTED(DIR_PREFIX,["${DIR_PREFIX}"],[directory "prefix"])
  DIR_EXEC_PREFIX="`eval echo \"$exec_prefix\"`"
  AC_DEFINE_UNQUOTED(DIR_EXEC_PREFIX,["${DIR_EXEC_PREFIX}"],[directory "exec_prefix"])
  DIR_BINDIR="`eval echo \"$bindir\"`"
  AC_DEFINE_UNQUOTED(DIR_BINDIR,["${DIR_BINDIR}"],[directory "bindir"])
  DIR_LIBDIR="`eval echo \"$libdir\"`"
  AC_DEFINE_UNQUOTED(DIR_LIBDIR,["${DIR_LIBDIR}"],[directory "libdir"])
  DIR_DATADIR="`eval echo \"$datadir\"`"
  AC_DEFINE_UNQUOTED(DIR_DATADIR,["${DIR_DATADIR}"],[directory "datadir"])
  DIR_MANDIR="`eval echo \"$mandir\"`"
  AC_DEFINE_UNQUOTED(DIR_MANDIR,["${DIR_MANDIR}"],[directory "mandir"])
  prefix="${save_prefix}"
  exec_prefix="${save_exec_prefix}"
])

dnl -------------------------------------------------------
dnl @synopsis AC_CHECK_CXX_OPT(OPTION,
dnl               ACTION-IF-OKAY,ACTION-IF-NOT-OKAY)
dnl Check if compiler accepts option OPTION.
dnl -------------------------------------------------------
AC_DEFUN(AC_CHECK_CXX_OPT,[
 opt="$1"
 AC_MSG_CHECKING([if $CXX accepts $opt])
 echo 'void f(){}' > conftest.cc
 if test -z "`${CXX} ${CXXFLAGS} ${OPTS} $opt -c conftest.cc 2>&1`"; then
    AC_MSG_RESULT(yes)
    rm conftest.* 
    $2
 else
    AC_MSG_RESULT(no)
    rm conftest.*
    $3
 fi
])

dnl -------------------------------------------------------
dnl @synopsis AC_CXX_OPTIMIZE
dnl Setup option --enable-debug
dnl Collects optimization/debug option in variable OPTS
dnl Filter options from CFLAGS and CXXFLAGS
dnl -------------------------------------------------------
AC_DEFUN(AC_CXX_OPTIMIZE,[
   AC_REQUIRE([AC_CANONICAL_HOST])
   AC_ARG_ENABLE(debug,
        AC_HELP_STRING([--enable-debug],
                       [Compile with debugging options (default: no)]),
        [ac_debug=$enableval],[ac_debug=no])
   OPTS=
   AC_SUBST(OPTS)
   saved_CXXFLAGS="$CXXFLAGS"
   saved_CFLAGS="$CFLAGS"
   CXXFLAGS=
   CFLAGS=
   for opt in $saved_CXXFLAGS ; do
     case $opt in
       -g*) test $ac_debug != no && OPTS="$OPTS $opt" ;;
       -O*) ;;
       *) CXXFLAGS="$CXXFLAGS $opt" ;;
     esac
   done
   for opt in $saved_CFLAGS ; do
     case $opt in
       -O*|-g*) ;;
       *) CFLAGS="$CFLAGS $opt" ;;
     esac
   done
   if test $ac_debug = no ; then
     OPTS=-DNO_DEBUG
     AC_CHECK_CXX_OPT([-Wall],[OPTS="$OPTS -Wall"])
     AC_CHECK_CXX_OPT([-O3],[OPTS="$OPTS -O3"],
        [ AC_CHECK_CXX_OPT([-O2], [OPTS="$OPTS -O2"] ) ] )
     AC_CHECK_CXX_OPT([-funroll-loops], [OPTS="$OPTS -funroll-loops"])
     case "${host_cpu}" in
        i?86)
           opt="-mcpu=${host_cpu}"
           AC_CHECK_CXX_OPT([$opt], [OPTS="$OPTS $opt"])
           ;;
      esac
   fi
   case x"$ac_debug" in
     x[0-9])  OPTS="$OPTS -DDEBUGLVL=$ac_debug" ;;
     xr*)   OPTS="$OPTS -DRUNTIME_DEBUG_ONLY" ;;
   esac
])

dnl -------------------------------------------------------
dnl @synopsis AC_CXX_MEMBER_TEMPLATES
dnl If the compiler supports member templates, 
dnl define HAVE_MEMBER_TEMPLATES.
dnl -------------------------------------------------------
AC_DEFUN([AC_CXX_MEMBER_TEMPLATES],
[AC_CACHE_CHECK(whether the compiler supports member templates,
ac_cv_cxx_member_templates,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
template<class T, int N> class A
{ public:
  template<int N2> A<T,N> operator=(const A<T,N2>& z) { return A<T,N>(); }
};],[A<double,4> x; A<double,7> y; x = y; return 0;],
 ac_cv_cxx_member_templates=yes, ac_cv_cxx_member_templates=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_member_templates" = yes; then
  AC_DEFINE(HAVE_MEMBER_TEMPLATES,,
        [define if the compiler supports member templates])
fi
])

dnl -------------------------------------------------------
dnl @synopsis AC_CXX_BOOL
dnl If the compiler recognizes bool as a separate built-in type,
dnl define HAVE_BOOL. Note that a typedef is not a separate
dnl type since you cannot overload a function such that it 
dnl accepts either the basic type or the typedef.
dnl -------------------------------------------------------
AC_DEFUN([AC_CXX_BOOL],
[AC_CACHE_CHECK(whether the compiler recognizes bool as a built-in type,
ac_cv_cxx_bool,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE([
int f(int  x){return 1;}
int f(char x){return 1;}
int f(bool x){return 1;}
],[bool b = true; return f(b);],
 ac_cv_cxx_bool=yes, ac_cv_cxx_bool=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_bool" = yes; then
  AC_DEFINE(HAVE_BOOL,,[define if bool is a built-in type])
fi
])

dnl -------------------------------------------------------
dnl @synopsis AC_CXX_EXCEPTIONS
dnl If the C++ compiler supports exceptions handling (try,
dnl throw and catch), define HAVE_EXCEPTIONS.
dnl -------------------------------------------------------
AC_DEFUN([AC_CXX_EXCEPTIONS],
[AC_CACHE_CHECK(whether the compiler supports exceptions,
ac_cv_cxx_exceptions,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 AC_TRY_COMPILE(,[try { throw  1; } catch (int i) { return i; }],
 ac_cv_cxx_exceptions=yes, ac_cv_cxx_exceptions=no)
 AC_LANG_RESTORE
])
if test "$ac_cv_cxx_exceptions" = yes; then
  AC_DEFINE(HAVE_EXCEPTIONS,,[define if the compiler supports exceptions])
fi
])


dnl -------------------------------------------------------
dnl @synopsis AC_CXX_RPO
dnl Defines option --enable-rpo and searches program RPO.
dnl Set output variables CXXRPOFLAGS and RPO. 
dnl -------------------------------------------------------
AC_DEFUN([AC_CXX_RPO],
[ AC_ARG_VAR(RPO,[Location of the RPO program.])
  if test x$GXX = xyes ; then
    AC_ARG_ENABLE([rpo],
    AC_HELP_STRING([--enable-rpo],
                   [Enable compilation with option -frepo]),
    [ac_rpo=$enableval], [ac_rpo=no] )
  if test x$ac_rpo != xno ; then
    CXXRPOFLAGS=-frepo
    AC_PATH_PROG(RPO, rpo, [unknown], $PATH)
    if ! test -x ${RPO} ; then
        AC_MSG_ERROR([Cannot find rpo program])
    fi
  else
    CXXRPOFLAGS=
    RPO=':'
  fi
  AC_SUBST(CXXRPOFLAGS)
 fi 
])


dnl ------------------------------------------------------------------
dnl @synopsis AC_PATH_PTHREAD([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl This macro figures out how to build C programs using POSIX
dnl threads.  It sets the PTHREAD_LIBS output variable to the threads
dnl library and linker flags, and the PTHREAD_CFLAGS output variable
dnl to any special C compiler flags that are needed.  (The user can also
dnl force certain compiler flags/libs to be tested by setting these
dnl environment variables.).  
dnl ------------------------------------------------------------------
AC_DEFUN([AC_PATH_PTHREAD], [
AC_REQUIRE([AC_CANONICAL_HOST])
acx_pthread_ok=no
# First, check if the POSIX threads header, pthread.h, is available.
# If it isn't, don't bother looking for the threads libraries.
AC_CHECK_HEADER(pthread.h, , acx_pthread_ok=noheader)
# We must check for the threads library under a number of different
# names; the ordering is very important because some systems
# (e.g. DEC) have both -lpthread and -lpthreads, where one of the
# libraries is broken (non-POSIX).
# First of all, check if the user has set any of the PTHREAD_LIBS,
# etcetera environment variables, and if threads linking works.
if test x${PTHREAD_LIBS+set} = xset ||
   test x${PTHREAD_CFLAGS+set} = xset ; then
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
        save_CXXFLAGS="$CXXFLAGS"
        CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        AC_MSG_CHECKING([provided PTHREAD_LIBS/PTHREAD_CFLAGS.])
        AC_TRY_LINK_FUNC(pthread_join, acx_pthread_ok=yes)
        AC_MSG_RESULT($acx_pthread_ok)
        if test x"$acx_pthread_ok" = xno; then
                PTHREAD_LIBS=""
                PTHREAD_CFLAGS=""
        fi
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
        CXXFLAGS="$save_CXXFLAGS"
fi
# Create a list of thread flags to try.  Items starting with a "-" are
# C compiler flags, and other items are library names, except for "none"
# which indicates that we try without any flags at all.
acx_pthread_flags="pthreads none -Kthread -kthread lthread 
                   -pthread -pthreads -mthreads pthread
                   --thread-safe -mt"
# The ordering *is* (sometimes) important.  
# Some notes on the individual items follow:
# pthreads: AIX (must check this before -lpthread)
# none: in case threads are in libc; should be tried before -Kthread and
#       other compiler flags to prevent continual compiler warnings
# -Kthread: Sequent (threads in libc, but -Kthread needed for pthread.h)
# -kthread: FreeBSD kernel threads (preferred to -pthread since SMP-able)
# lthread: LinuxThreads port on FreeBSD (also preferred to -pthread)
# -pthread: Linux/gcc (kernel threads), BSD/gcc (userland threads)
# -pthreads: Solaris/gcc
# -mthreads: Mingw32/gcc, Lynx/gcc
# -mt: Sun Workshop C (may only link SunOS threads [-lthread], but it
#      doesn't hurt to check since this sometimes defines pthreads too;
#      also defines -D_REENTRANT)
# pthread: Linux, etcetera
# --thread-safe: KAI C++
case "${host_cpu}-${host_os}" in
        *solaris*)
        # On Solaris (at least, for some versions), libc contains stubbed
        # (non-functional) versions of the pthreads routines, so link-based
        # tests will erroneously succeed.  (We need to link with -pthread or
        # -lpthread.)  (The stubs are missing pthread_cleanup_push, or rather
        # a function called by this macro, so we could check for that, but
        # who knows whether they'll stub that too in a future libc.)  So,
        # we'll just look for -pthreads and -lpthread first:
        acx_pthread_flags="-pthread -pthreads pthread -mt $acx_pthread_flags"
        ;;
esac
if test x"$acx_pthread_ok" = xno; then
for flag in $acx_pthread_flags; do
        case $flag in
                none)
                AC_MSG_CHECKING([whether pthreads work without any flags])
                ;;
                -*)
                AC_MSG_CHECKING([whether pthreads work with $flag])
                PTHREAD_CFLAGS="$flag"
                ;;
                *)
                AC_MSG_CHECKING([for the pthreads library -l$flag])
                PTHREAD_LIBS="-l$flag"
                ;;
        esac
        save_LIBS="$LIBS"
        save_CFLAGS="$CFLAGS"
        save_CXXFLAGS="$CXXFLAGS"
        LIBS="$PTHREAD_LIBS $LIBS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
        CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
        # Check for various functions.  We must include pthread.h,
        # since some functions may be macros.  (On the Sequent, we
        # need a special flag -Kthread to make this header compile.)
        # We check for pthread_join because it is in -lpthread on IRIX
        # while pthread_create is in libc.  We check for pthread_attr_init
        # due to DEC craziness with -lpthreads.  We check for
        # pthread_cleanup_push because it is one of the few pthread
        # functions on Solaris that doesn't have a non-functional libc stub.
        # We try pthread_create on general principles.
        AC_TRY_LINK([#include <pthread.h>],
                    [pthread_t th; pthread_join(th, 0);
                     pthread_attr_init(0); pthread_cleanup_push(0, 0);
                     pthread_create(0,0,0,0); pthread_cleanup_pop(0); ],
                    [acx_pthread_ok=yes])
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
        CXXFLAGS="$save_CXXFLAGS"
        AC_MSG_RESULT($acx_pthread_ok)
        if test "x$acx_pthread_ok" = xyes; then
                break;
        fi
        PTHREAD_LIBS=""
        PTHREAD_CFLAGS=""
done
fi
# Various other checks:
if test "x$acx_pthread_ok" = xyes; then
        save_LIBS="$LIBS"
        LIBS="$PTHREAD_LIBS $LIBS"
        save_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
        save_CXXFLAGS="$CXXFLAGS"
        CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
        AC_MSG_CHECKING([if more special flags are required for pthreads])
        flag=no
        case "${host_cpu}-${host_os}" in
                *-aix* | *-freebsd*)     flag="-D_THREAD_SAFE";;
                *solaris* | alpha*-osf*) flag="-D_REENTRANT";;
        esac
        AC_MSG_RESULT(${flag})
        if test "x$flag" != xno; then
                PTHREAD_CFLAGS="$flag $PTHREAD_CFLAGS"
        fi
        LIBS="$save_LIBS"
        CFLAGS="$save_CFLAGS"
        CXXFLAGS="$save_CXXFLAGS"
fi
AC_ARG_VAR(PTHREAD_LIBS, [Flags for linking pthread programs.])
AC_ARG_VAR(PTHREAD_CFLAGS, [Flags for compiling pthread programs.])
# execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_pthread_ok" = xyes; then
        AC_DEFINE(HAVE_PTHREAD,1,[Define if pthreads are available])
        ifelse([$1],,:,[$1])
else
        ifelse([$2],,:,[$2])
fi
])


dnl ------------------------------------------------------------------
dnl @synopsis AC_PATH_COTHREADS([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl Define HAVE_COTHREAD if cothreads can be used.
dnl Define HAVE_COTHREAD_PATCH if cothread libgcc patch is available
dnl ------------------------------------------------------------------

AC_DEFUN([AC_PATH_COTHREAD], [
AC_REQUIRE([AC_CANONICAL_HOST])
acx_cothread=no
if test x$GXX = xyes ; then
  AC_MSG_CHECKING([whether cothreads work with ${host_cpu}])
  case ${host_cpu} in
    i?86|powerpc*|mips*|alpha*|hppa*)
      acx_cothread=yes
      ;;   
  esac
  AC_MSG_RESULT($acx_cothread)
fi
if test x$acx_cothread != xno ; then
  AC_MSG_CHECKING([whether libgcc contains the cothread patch])
  AC_LANG_PUSH(C++)
  AC_TRY_LINK([extern "C" { void *(*__get_eh_context_ptr)();
                            void *__new_eh_context(void); }],
              [ __get_eh_context_ptr = &__new_eh_context;],
              [acx_cothread_patch=yes], [acx_cothread_patch=no])
  AC_LANG_POP(C++)
  AC_MSG_RESULT($acx_cothread_patch)
  if test x$acx_cothread_patch = xno ; then
    AC_MSG_CHECKING([if the cothread patch is critical]) 
    echo 'void foo() { throw "Hello"; }' > conftest.cc
    compile="$CXX $CXXFLAGS -c conftest.cc"
    check="nm conftest.o | grep sjthrow | cat > conftest.out"
    acx_cothread_patch=yes
    if AC_TRY_EVAL(compile) && AC_TRY_EVAL(check) ; then
      if test -z "`cat conftest.out`" ; then
        acx_cothread_patch=no
      fi
    fi
    AC_MSG_RESULT($acx_cothread_patch)
    rm conftest.*
    if test x$acx_cothread_patch = xyes ; then
        acx_cothread=no
        AC_MSG_WARN([Cothread cannot work without the patch])
    else
        AC_MSG_WARN([Applying the patch is recommended anyway])
    fi
    AC_MSG_WARN([See the INSTALL file for more information])
  fi
fi
# Must do.
if test x$acx_cothread = xyes ; then
   AC_DEFINE(HAVE_COTHREAD,1,
                [Define if cothreads are available.])
   if test x$acx_cothread_patch = xyes ; then
      AC_DEFINE(HAVE_COTHREAD_PATCH,1,
                [Define if libgcc contains the cothread patch.])
   fi
   ifelse([$1],,:,[$1])
else
   ifelse([$2],,:,[$2])
fi
])

dnl ------------------------------------------------------------------
dnl @synopsis AC_PATH_THREADS([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl Process optional option --enable-threads
dnl Check availability of pthreads or cothreads
dnl using AC_PATH_PTHREAD and AC_PATH_COTHREAD.
dnl Set output variable THREADS_LIBS and THREADS_CFLAGS
dnl ------------------------------------------------------------------

AC_DEFUN([AC_PATH_THREADS], [
AC_ARG_ENABLE(threads,
            AC_HELP_STRING([--enable-threads],
                           [select threading model (default is auto)]),
            ac_use_threads=$enableval, ac_use_threads=auto)
ac_threads=no
if test x$ac_use_threads != xno ; then
  case x$ac_use_threads in
  x|xyes|xauto|xposix|xpthread)
        AC_PATH_PTHREAD(
                [ ac_threads=pthread
                  ac_use_threads=pthread
                  THREAD_LIBS="$PTHREAD_LIBS"
                  THREAD_CFLAGS="$PTHREAD_CFLAGS -DTHREADMODEL=POSIXTHREADS"
                ] )
        ;;
  esac
  case x$ac_use_threads in
  xposix|xpthread)
        ;;
  x|xyes|xauto|xcothread)
        AC_PATH_COTHREAD(
                [ ac_threads=cothread
                  THREAD_CFLAGS="-DTHREADMODEL=COTHREADS"
                ] )
        ;;
  *)
        AC_MSG_ERROR(
[Invalid argument for --enable-threads
Valid arguments are: yes, no, posix, pthread, cothread, auto.])
        ;;
  esac
fi
AC_SUBST(THREAD_LIBS)
AC_SUBST(THREAD_CFLAGS)
AC_MSG_CHECKING([threading model])
AC_MSG_RESULT($ac_threads)
if test $ac_threads != no ; then
   ifelse([$1],,:,[$1])
else
   ifelse([$2],,:,[$2])
fi
])

dnl ------------------------------------------------------------------
dnl @synopsis AC_PATH_JPEG([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl Process option --with-jpeg.
dnl Search JPEG along the extra
dnl Define HAVE_JPEG.
dnl Set output variable JPEG_CFLAGS and JPEG_LIBS
dnl ------------------------------------------------------------------

AC_DEFUN([AC_PATH_JPEG],
[
  AC_ARG_VAR(JPEG_LIBS)
  AC_ARG_VAR(JPEG_CFLAGS)
  ac_jpeg=no
  AC_ARG_WITH(jpeg,
     AC_HELP_STRING([--with-jpeg=DIR],
                    [where the IJG jpeg library is located]),
     [ac_jpeg=$withval], [ac_jpeg=yes] )
  # Process specification
  if test x$ac_jpeg = xyes ; then
     test x${JPEG_LIBS+set} != xset && JPEG_LIBS="-ljpeg"
  elif test x$ac_jpeg != xno ; then
     test x${JPEG_LIBS+set} != xset && JPEG_LIBS="-L$ac_jpeg -ljpeg"
     test x${JPEG_CFLAGS+set} != xset && JPEG_CFLAGS="-I$ac_jpeg"
  fi
  # Try linking
  if test x$ac_jpeg != xno ; then
     AC_MSG_CHECKING([for jpeg library])
     save_CFLAGS="$CFLAGS"
     save_CXXFLAGS="$CXXFLAGS"
     save_LIBS="$LIBS"
     CFLAGS="$CFLAGS $JPEG_CFLAGS"
     CXXFLAGS="$CXXFLAGS $JPEG_CFLAGS"
     LIBS="$LIBS $JPEG_LIBS"
     AC_TRY_LINK([
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h> 
#include <jpeglib.h>
#ifdef __cplusplus
}
#endif ],[
jpeg_CreateDecompress(0,0,0);],
       [ac_jpeg=yes], [ac_jpeg=no] )
     CFLAGS="$save_CFLAGS"
     CXXFLAGS="$save_CXXFLAGS"
     LIBS="$save_LIBS"
     AC_MSG_RESULT($ac_jpeg)
   fi
   # Finish
   if test x$ac_jpeg = xno; then
      JPEG_CFLAGS= ; JPEG_LIBS=
      ifelse([$2],,:,[$2])
   else
      AC_DEFINE(HAVE_JPEG,1,[Define if you have the IJG JPEG library.])
      ifelse([$1],,:,[$1])
   fi
])

dnl ------------------------------------------------------------------
dnl @synopsis AC_PATH_QT([ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]])
dnl Process option --with-qt=DIR
dnl Define HAVE_QT and set it to QT_VERSION
dnl Set output variables MOC, UIC, QT_LIBS and QT_CFLAGS.
dnl ------------------------------------------------------------------

AC_DEFUN([AC_PATH_QT],
[
  AC_REQUIRE([AC_PATH_X])
  AC_REQUIRE([AC_PATH_XTRA])
  if test ${no_x-no} != yes ; then
    X_LIBS="$X_LIBS $X_PRE_LIBS -lXext -lX11 $X_EXTRA_LIBS"
  fi
  # Variables
  AC_ARG_VAR(QTDIR,[Location of the Qt package.])
  AC_ARG_VAR(QT_CFLAGS,[Flags for compiling Qt programs.])
  AC_ARG_VAR(QT_LIBS,[Flags for linking Qt programs.])
  AC_ARG_VAR(MOC,[Location of the MOC program.])
  AC_ARG_VAR(UIC,[Location of the UIC program.])
  # Arguments
  AC_ARG_WITH(qt, 
      AC_HELP_STRING([--with-qt=DIR],
                     [where the Qt root is installed.]),
      [ test x$withval != xyes && QTDIR=$withval ])
  test x$no_x = yes && QTDIR=no
  # Resolve variables
  if test x$QTDIR != xno ; then
    AC_MSG_CHECKING([for Qt root directory])
    if test $x{QT_CFLAGS+set} != set -a $x{QT_LIBS+set} != set ; then
       ac_qt_dirs="$QTDIR /usr/lib/qt2 /usr/lib/qt /usr/local /usr/X11R6 /usr"
       QTDIR=no
       for dir in $ac_qt_dirs ; do
          if test -r $dir/include/qwidget.h ; then
            QTDIR=$dir
            break
          fi
       done
    fi
    AC_MSG_RESULT($QTDIR)
  fi
  # Programs
  if test x$QTDIR != xno ; then
    test x{$QT_CFLAGS+set} != set && QT_CFLAGS="-I$QTDIR/include"
    test x{$QT_LIBS+set} != set && QT_LIBS="-L$QTDIR/lib -lqt"
    AC_PATH_PROG(MOC, moc, [unknown], "$QTDIR/bin:$PATH")
    AC_PATH_PROG(UIC, uic, [unknown], "$QTDIR/bin:$PATH")
    test -x "$MOC" || QTDIR=no
  fi
  # Execute
  if test x$QTDIR != xno ; then
    AC_MSG_CHECKING([if a small Qt program runs])
    AC_LANG_PUSH(C++)
    save_CXXFLAGS="$CXXFLAGS"
    save_LIBS="$LIBS"
    CXXFLAGS="$CXXFLAGS $CFLAGS $QT_CFLAGS $X_CFLAGS"
    LIBS="$QT_LIBS $X_LIBS $LIBS"
    AC_TRY_RUN([
#include <qfile.h>
#include <qtextstream.h>
#include <qglobal.h>
void main() { 
QFile qf("confout"); if (!qf.open(IO_WriteOnly)) exit(1);
QTextStream ts(&qf); ts << QT_VERSION; exit(0);
}],[okay=yes],[okay=no; QTDIR=no]) 
    CXXFLAGS="$save_CXXFLAGS"
    LIBS="$save_LIBS"
    AC_LANG_POP(C++)
    AC_MSG_RESULT($okay)
  fi
  # Version
  if test x$QTDIR != xno ; then
     AC_MSG_CHECKING([Qt version])
     qt_version=`cat < confout`
     AC_MSG_RESULT($qt_version)
     AC_DEFINE_UNQUOTED(HAVE_QT,$qt_version,
                        [Define to Qt version if available])
     rm confout 2>/dev/null
  fi
  # Execute
  if test x$QTDIR = xno ; then
    QT_CFLAGS= ; QT_LIBS= ; QTDIR=
    ifelse([$2],,:,[$2])        
  else
    ifelse([$1],,:,[$1])        
  fi
])

