bin_PROGRAMS = \
bzz \
c44 \
cjb2 \
cpaldjvu \
csepdjvu \
ddjvu \
djvm \
djvmcvt \
djvudump \
djvups \
djvuextract \
djvumake \
djvused \
djvutxt \
djvuserve

jb2cmp_SOURCES = \
jb2cmp/classify.cpp \
jb2cmp/cuts.cpp \
jb2cmp/frames.cpp \
jb2cmp/patterns.cpp \
jb2cmp/classify.h \
jb2cmp/mdjvucfg.h \
jb2cmp/minidjvu.h \
jb2cmp/patterns.h

bzz_SOURCES = bzz.cpp common.h
bzz_CPPFLAGS = -I$(top_srcdir)/libdjvu
bzz_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
bzz_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

c44_SOURCES = c44.cpp common.h
c44_CPPFLAGS = -I$(top_srcdir)/libdjvu
c44_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
c44_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

cjb2_SOURCES = cjb2.cpp jb2tune.cpp common.h jb2tune.h $(jb2cmp_SOURCES)
cjb2_CPPFLAGS = -I$(top_srcdir)/libdjvu $(TIFF_CFLAGS)
cjb2_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
cjb2_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la $(TIFF_LIBS)

cpaldjvu_SOURCES = cpaldjvu.cpp jb2tune.cpp common.h jb2tune.h $(jb2cmp_SOURCES)
cpaldjvu_CPPFLAGS = -I$(top_srcdir)/libdjvu
cpaldjvu_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
cpaldjvu_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

csepdjvu_SOURCES = csepdjvu.cpp jb2tune.cpp common.h jb2tune.h $(jb2cmp_SOURCES)
csepdjvu_CPPFLAGS = -I$(top_srcdir)/libdjvu
csepdjvu_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
csepdjvu_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

ddjvu_SOURCES = ddjvu.cpp tiff2pdf.c tiff2pdf.h
ddjvu_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libdjvu $(TIFF_CFLAGS)
if HAVE_OS_WIN32
ddjvu_CPPFLAGS += -D__USE_MINGW_ANSI_STDIO
endif
ddjvu_CFLAGS = $(OPTS) $(THREAD_CFLAGS)
ddjvu_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
ddjvu_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la $(TIFF_LIBS)

djvm_SOURCES = djvm.cpp common.h
djvm_CPPFLAGS = -I$(top_srcdir)/libdjvu
djvm_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
djvm_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvmcvt_SOURCES = djvmcvt.cpp common.h
djvmcvt_CPPFLAGS = -I$(top_srcdir)/libdjvu
djvmcvt_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
djvmcvt_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvudump_SOURCES = djvudump.cpp common.h
djvudump_CPPFLAGS = -I$(top_srcdir)/libdjvu
djvudump_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
djvudump_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvuextract_SOURCES = djvuextract.cpp common.h
djvuextract_CPPFLAGS = -I$(top_srcdir)/libdjvu
djvuextract_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
djvuextract_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvumake_SOURCES = djvumake.cpp common.h
djvumake_CPPFLAGS = -I$(top_srcdir)/libdjvu
djvumake_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
djvumake_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvups_SOURCES = djvups.cpp
djvups_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libdjvu
djvups_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
djvups_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvused_SOURCES = djvused.cpp common.h
djvused_CPPFLAGS = -I$(top_srcdir)/libdjvu
djvused_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
djvused_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvuserve_SOURCES = djvuserve.cpp common.h
djvuserve_CPPFLAGS = -I$(top_srcdir)/libdjvu
djvuserve_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
djvuserve_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

djvutxt_SOURCES = djvutxt.cpp
djvutxt_CPPFLAGS = -I$(top_srcdir) -I$(top_srcdir)/libdjvu
djvutxt_CXXFLAGS = $(OPTS) $(CXXRPOFLAGS) $(THREAD_CFLAGS)
djvutxt_LDADD = $(top_builddir)/libdjvu/libdjvulibre.la

dist_bin_SCRIPTS = any2djvu djvudigital

dist_man1_MANS = \
any2djvu.1 \
bzz.1 \
c44.1 \
cjb2.1 \
cpaldjvu.1 \
csepdjvu.1 \
ddjvu.1 \
djvm.1 \
djvmcvt.1 \
djvu.1 \
djvudigital.1 \
djvudump.1 \
djvuextract.1 \
djvumake.1 \
djvups.1 \
djvused.1 \
djvuserve.1 \
djvutxt.1

EXTRA_DIST = jb2cmp/README tiff2pdf.c.diff
