#!/bin/sh


# Base
DESTDIR=
datadir="/usr/share"
applications="$datadir/applications"
pixmaps="$datadir/pixmaps"
mime_info="$datadir/mime-info"
application_registry="$datadir/application-registry"
applnk="$datadir/applnk"
mimelnk="$datadir/mimelnk"
menutype=applnk
onlyexistingdirs=no
dryrun=no

# Autodetect

desktop_file_install=`which desktop-file-install 2>/dev/null`
if [ -x "$desktop_file_install" ] ; then
    menutype=redhat
elif [ x${XDG_CONFIG_DIRS} != x ] ; then
    menudtype=xdg
elif [ -r /etc/xdg/menus/applications.menu ] ; then
    menutype=xdg
fi

kdeconfig=`which kde-config 2>/dev/null`
if [ -x "$kdeconfig" ]
then
    d=`$kdeconfig --path mime | sed -e 's/^.*://g'`
    test -d "$d" && dir_mimelnk="$d"
    d=`$kdeconfig --path apps | sed -e 's/^.*://g'`
    test -d "$d" && dir_applnk="$d"
fi

# Arguments
for arg
do
  case "$arg" in
      --destdir=*)
	  DESTDIR=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --datadir=*)
	  datadir=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --applications=*)
	  applications=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
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
      --menutype=*)
	  menutype=`echo "$arg" | sed -e 's/^[^=]*=//'` ;;
      --onlyexistingdirs)
	  onlyexistingdirs=yes ;;
      --dryrun|-n)
	  dryrun=yes ;;
      --help)
cat >&2 <<EOF
Usage: installer.sh [..options..]
Valid options are:
    --destdir=DIR (default: /)
    --datadir=DIR (default: $datadir)
    --applications=DIR (default: $applications)
    --pixmaps=DIR (default: $pixmaps)
    --mime_info=DIR (default: $mime_info)
    --application_registry=DIR (default: $application_registry)
    --applnk=DIR (default: $applnk)
    --mimelnk=DIR (default: $mimelnk)
    --menutype=(applnk|redhat|xdg)
    --onlyexistingdirs
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
    test "$dryrun" == "yes" || "$@"
}

makedir()
{
    if [ ! -d $1 -a  $onlyexistingdirs == no ] 
    then
	makedir `dirname $1`
	run mkdir $1
    fi
}

install()
{
    if [ -d $2 ]
    then
	test -r $2/$1 && $run rm -f $2/$1
	run cp $1 $2/$1
	run chmod 644 $2/$1
    fi
}

# Fixup

case "$menutype" in
    redhat) 
	test -x $desktop_file_install || menutype=xdg ;;
    applnk) 
	;;
    xdg)    
	;;
    *)
	echo 1>&2 "$0: incorrect menutype (one of applnk,redhat,xdg)"
	exit 10
	;;
esac

# Go

makedir $DESTDIR$pixmaps
install djvu.png $DESTDIR$pixmaps

makedir $DESTDIR$mime_info
install djvu.mime $DESTDIR$mime_info
install djvu.keys $DESTDIR$mime_info

makedir $DESTDIR$application_registry
install djvu.applications $DESTDIR$application_registry

makedir $DESTDIR$mimelnk/image
install x-djvu.desktop $DESTDIR$mimelnk/image

case "$menutype" in
    redhat)
	makedir $DESTDIR$applications
	run $desktop_file_install --vendor= --dir=$DESTDIR$applications djview.desktop
	;;
    xdg)
	makedir $DESTDIR$applications
	install djview.desktop $DESTDIR$applications
	;;
    applnk)
	makedir $DESTDIR$applnk/Graphics
	install djview.desktop $DESTDIR$applnk/Graphics
esac

