ACLOCAL_AMFLAGS = -I config

SUBDIRS = libdjvu tools share

if WANT_XMLTOOLS
SUBDIRS += xmltools
endif

# what about I18N subdir ?

SUBDIRS += desktopfiles

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

EXTRA_DIST = COPYRIGHT autogen.sh djvulibre.spec

# Distribute some entire subdirectories.
# No trailing slashes, as they put foo/* in foo/foo/*.

EXTRA_DIST += doc debian macosx win32
