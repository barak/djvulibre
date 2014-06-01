#! /bin/sh

# First invoke libtoolize manually to install goo in config/.

# (Invoking aclocal manually would install the .m4 files but
# config.{sub,guess} requires libtoolize, which also handles the .m4
# files.)

# This would not be necessary if automake were in use.
# But as is, autoreconf will barf on missing files, even when
# instructed to install missing files.

# Why is this?  Ask Dr Seuss.

libtoolize --install

autoreconf --install $*
