
ACLOCAL_AMFLAGS = -I config

MAINTAINERCLEANFILES = \
Makefile.in \
aclocal.m4 \
configure \
config.log \
config/ar-lib \
config/compile \
config/config.guess \
config/config.h.in \
config/config.h.in~ \
config/config.sub \
config/depcomp \
config/install-sh \
config/libtool.m4 \
config/ltmain.sh \
config/ltoptions.m4 \
config/ltsugar.m4 \
config/ltversion.m4 \
config/lt~obsolete.m4 \
config/missing \
$(DIST_ARCHIVES)

SUBDIRS = libdjvu tools

if WANT_XMLTOOLS
SUBDIRS += xmltools share
endif

# what about I18N subdir ?

SUBDIRS += desktopfiles

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

EXTRA_DIST = \
COPYRIGHT \
README_macosx \
README_win32 \
autogen.sh \
djvulibre.spec \
doc/ \
debian/ \
macosx/ \
win32/
