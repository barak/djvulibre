bin_PROGRAMS = bzz c44 cjb2 cpaldjvu csepdjvu ddjvu djvm djvmcvt	\
 djvudump djvups djvuextract djvumake djvused djvutxt djvuserve

jb2cmp_SOURCES = jb2cmp/classify.cpp jb2cmp/cuts.cpp		\
 jb2cmp/frames.cpp jb2cmp/patterns.cpp jb2cmp/classify.h	\
 jb2cmp/mdjvucfg.h jb2cmp/minidjvu.h jb2cmp/patterns.h

AM_CPPFLAGS = -I$(top_srcdir)/libdjvu
AM_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)

bzz_SOURCES = bzz.cpp common.h
bzz_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

c44_SOURCES = c44.cpp common.h
c44_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

cjb2_SOURCES = cjb2.cpp jb2tune.cpp common.h jb2tune.h $(jb2cmp_SOURCES)
cjb2_CPPFLAGS = $(AM_CPPFLAGS) $(TIFF_CFLAGS)
cjb2_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la $(TIFF_LIBS)

cpaldjvu_SOURCES = cpaldjvu.cpp jb2tune.cpp common.h jb2tune.h $(jb2cmp_SOURCES)
cpaldjvu_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

csepdjvu_SOURCES = csepdjvu.cpp jb2tune.cpp common.h jb2tune.h $(jb2cmp_SOURCES)
csepdjvu_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

ddjvu_SOURCES = ddjvu.cpp tiff2pdf.c tiff2pdf.h
ddjvu_CPPFLAGS = -I$(top_srcdir)  $(AM_CPPFLAGS) $(TIFF_CFLAGS)
if HAVE_OS_WIN32
ddjvu_CPPFLAGS += -D__USE_MINGW_ANSI_STDIO
endif
ddjvu_CFLAGS = $(OPTS) $(THREAD_CFLAGS)
ddjvu_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la $(TIFF_LIBS)

djvm_SOURCES = djvm.cpp common.h
djvm_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvmcvt_SOURCES = djvmcvt.cpp common.h
djvmcvt_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvudump_SOURCES = djvudump.cpp common.h
djvudump_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvuextract_SOURCES = djvuextract.cpp common.h
djvuextract_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvumake_SOURCES = djvumake.cpp common.h
djvumake_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvups_SOURCES = djvups.cpp
djvups_CPPFLAGS = -I$(top_srcdir) $(AM_CPPFLAGS)
djvups_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvused_SOURCES = djvused.cpp common.h
djvused_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvuserve_SOURCES = djvuserve.cpp common.h
djvuserve_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvutxt_SOURCES = djvutxt.cpp
djvutxt_CPPFLAGS = -I$(top_srcdir) $(AM_CPPFLAGS)
djvutxt_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

dist_bin_SCRIPTS = any2djvu djvudigital

dist_man1_MANS = any2djvu.1 bzz.1 c44.1 cjb2.1 cpaldjvu.1 csepdjvu.1	\
 ddjvu.1 djvm.1 djvmcvt.1 djvu.1 djvudigital.1 djvudump.1		\
 djvuextract.1 djvumake.1 djvups.1 djvused.1 djvuserve.1 djvutxt.1

EXTRA_DIST = jb2cmp/README tiff2pdf.c.diff
