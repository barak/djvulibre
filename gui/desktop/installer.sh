#!/bin/sh
#
# DjVuLibre-3.5
# Copyright (c) 2002  Leon Bottou and Yann Le Cun.
# Copyright (c) 2001  AT&T
#
# This software is subject to, and may be distributed under, the
# GNU General Public License, Version 2. The license should have
# accompanied the software or you may obtain a copy of the license
# from the Free Software Foundation at http://www.fsf.org .
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# DjVuLibre-3.5 is derived from the DjVu(r) Reference Library
# distribued by Lizardtech Software.  On July 19th 2002, Lizardtech 
# Software authorized us to replace the original DjVu(r) Reference 
# Library by the following notice (see etc/PATENT.djvu):

DESTDIR=
srcdir=.
datadir=/usr/share
for arg ; do
  case "$arg" in
      --datadir=*)
	  datadir=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
  esac
done

# ------------------------------------------
# It is likely that this script will need
# many adjustement to work reliably on a
# large number of platforms. Please submit
# patches to http://djvu.sourceforge.net.
# ------------------------------------------

applications="$datadir/applications"
icons="$datadir/icons"
pixmaps="$datadir/pixmaps"
mime_info="$datadir/mime-info"
application_registry="$datadir/application-registry"
applnk="$datadir/applnk"
mimelnk="$datadir/mimelnk"
menus=applnk
dryrun=no

# Autodetect
if [ -n "${XDG_CONFIG_DIRS}" ] ; then
    menus=xdg
elif [ -r /etc/xdg/menus/applications.menu ] ; then
    menus=xdg
elif [ -r /etc/X11/desktop-menus/applications.menu ] ; then
    menus=xdg-old # redhat8 and redhat9 style. 
fi

kdeconfig=`which kde-config 2>/dev/null`
if [ -x "$kdeconfig" ] ; then
    d=`$kdeconfig --path mime | sed -e 's/^.*://g'`
    test -d "$d" && mimelnk="$d"
    d=`$kdeconfig --path apps | sed -e 's/^.*://g'`
    test -d "$d" && applnk="$d"
    d=`$kdeconfig --path icon | sed -e 's/^.*://g'`
    test -d "$d" && icons="$d"
fi

test -d "$applications"         || applications=no
test -d "$icons"                || icons=no
test -d "$pixmaps"              || pixmaps=no
test -d "$mime_info"            || mime_info=no
test -d "$application_registry" || application_registry=no
test -d "$applnk"               || applnk=no
test -d "$mimelnk"              || mimelnk=no

# Arguments

for arg ; do
  case "$arg" in
      --destdir=*)
	  DESTDIR=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --srcdir=*)
	  srcdir=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --datadir=*)
	  datadir=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --applications=*)
	  applications=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --icons=*)
	  icons=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --pixmaps=*)
	  pixmaps=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --mime=*)
	  mime=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --application=*)
	  application=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --applnk=*)
	  applnk=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --mimelnk=*)
	  mimelnk=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --menus=*)
	  menus=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --dryrun|-n)
	  dryrun=yes ;;
      --help)
cat >&2 <<EOF
Usage: installer.sh [..options..]
Valid options are:
    --destdir=DIR              (/)
    --srcdir=DIR               (.)
    --datadir=DIR              ($datadir)
    --applications=DIR         ($applications)
    --icons=DIR                ($icons)
    --pixmaps=DIR              ($pixmaps)
    --mime_info=DIR            ($mime_info)
    --application_registry=DIR ($application_registry)
    --applnk=DIR               ($applnk)
    --mimelnk=DIR              ($mimelnk)
    --menus=MODEL              ($menus)
EOF
          exit 0 
	  ;;
      *)
	  echo 1>&2 "$0: Unrecognized argument '$arg'."
	  echo 1>&2 "Type '$0 --help' for more information."
	  exit 10
	  ;;
  esac
done

# Utilities

run()
{
    echo "+ $*"
    test "$dryrun" = "yes" || "$@"
}

makedir()
{
    if [ ! -d $1 ] ; then
	makedir `dirname $1`
	run mkdir $1
    fi
}

install()
{
    if [ -d `dirname $2` -a ! -d "$2" ] ; then
	test -f $2 && run rm -f $2
	run cp $srcdir/$1 $2
	run chmod 644 $2
    fi
}

# Go

if [ "$icons" != no ] ; then
  makedir $DESTDIR$icons/hicolor/48x48/mimetypes
  makedir $DESTDIR$icons/hicolor/32x32/mimetypes
  makedir $DESTDIR$icons/hicolor/22x22/mimetypes
  install hi48-mimetype-djvu.png $DESTDIR$icons/hicolor/48x48/mimetypes/djvu.png
  install hi48-mimetype-djvu.png $DESTDIR$icons/hicolor/32x32/mimetypes/djvu.png
  install hi48-mimetype-djvu.png $DESTDIR$icons/hicolor/22x22/mimetypes/djvu.png
fi

if [ "$pixmaps" != no ] ; then
  makedir $DESTDIR$pixmaps
  install hi48-mimetype-djvu.png $DESTDIR$pixmaps/djvu.png
fi

if [ "$mime_info" != no ] ; then
  makedir $DESTDIR$mime_info
  install djvu.mime $DESTDIR$mime_info/djvu.mime
  install djvu.keys $DESTDIR$mime_info/djvu.keys
fi

if [ "$application_registry" != no ] ; then
  makedir $DESTDIR$application_registry
  install djvu.applications $DESTDIR$application_registry/djvu.applications
fi

if [ "$mimelnk" != no ] ; then
  makedir $DESTDIR$mimelnk/image
  install x-djvu.desktop $DESTDIR$mimelnk/image/x-djvu.desktop
fi

case "$menus" in
    xdg*)
	if [ "$applications" != no ] ; then
	  makedir $DESTDIR$applications
	  install djview.desktop $DESTDIR$applications/djview.desktop
	fi
	;;
    *)
	if [ "$applnk" != no ] ; then
	  makedir $DESTDIR$applnk/Graphics
	  install djview.desktop $DESTDIR$applnk/Graphics/djview.desktop
	fi
	;;
esac

