ACLOCAL_AMFLAGS = -I config

SUBDIRS = libdjvu tools

if WANT_XMLTOOLS
SUBDIRS += xmltools share
endif

# what about I18N subdir ?

SUBDIRS += desktopfiles

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

EXTRA_DIST = COPYRIGHT autogen.sh djvulibre.spec \
 doc/ debian/ macosx/ win32/
