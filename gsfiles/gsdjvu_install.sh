#!/bin/sh
#C--------------------------------------------------------------------
#C- DjVuLibre-3.5
#C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
#C- Copyright (c) 2001  AT&T
#C-
#C- This software is subject to, and may be distributed under, the
#C- GNU General Public License, Version 2. The license should have
#C- accompanied the software or you may obtain a copy of the license
#C- from the Free Software Foundation at http://www.fsf.org .
#C-
#C- This program is distributed in the hope that it will be useful,
#C- but WITHOUT ANY WARRANTY; without even the implied warranty of
#C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#C- GNU General Public License for more details.
#C-
#C- DjVuLibre-3.5 is derived from the DjVu(r) Reference Library
#C- distributed by Lizardtech Software.  On July 19th 2002, Lizardtech
#C- Software authorized us to replace the original DjVu(r) Reference
#C- Library notice by the following text (see doc/lizard2002.djvu):
#C--------------------------------------------------------------------


###### Checks

gsfiles=`dirname $0`
if [ ! -d $gsfiles -o ! -r $gsfiles/contrib.mak.add ]
then
    echo 1>&2 "Please run this script with its full pathname."
    echo 1>&2 "This is used to locate related files."
    exit 10
fi

if [ ! -d $gsfiles -o ! -r $gsfiles/gdevdjvu.c ]
then
    echo 1>&2 "File 'gdevdjvu.c' is absent."
    echo 1>&2 "Please see <http://djvu.sourceforge.net/gsdjvu>"
    echo 1>&2 "for an explanation."
    exit 10
fi

if [ ! -r src/gserror.h -o ! -r lib/gs_init.ps ]
then
    echo 1>&2 "Please run this script"
    echo 1>&2 "from the ghostscript source directory"
    exit 10
fi

if [ ! -d fonts -o ! -r fonts/n021003l.pfb ]
then
    echo 1>&2 "Please unpack recent ghostscript fonts"
    echo 1>&2 "in the ghostscript source directory"
    exit 10
fi

if [ ! -x configure ]
then
    echo 1>&2 "Please use ghostscript-7.07 or better"
    exit 10
fi


###### Install

DESTDIR="${DESTDIR-}"
INSTALL="${INSTALL-./src/instcopy}"
MKDIRP="${MKDIRP-mkdir -p}"

prefix=${prefix-/usr}
bindir=$prefix/bin
gsdir=$prefix/lib/gsdjvu

$MKDIRP 2>/dev/null $DESTDIR$gsdir
$MKDIRP 2>/dev/null $DESTDIR$bindir
$MKDIRP 2>/dev/null $DESTDIR$gsdir/bin
$MKDIRP 2>/dev/null $DESTDIR$gsdir/fonts
$MKDIRP 2>/dev/null $DESTDIR$gsdir/lib

echo Installing programs
echo > gsdjvu.sh '#!/bin/sh' 
echo >>gsdjvu.sh 'GS_LIB="'"$gsdir/lib:$gsdir/fonts"'" exec "'"$gsdir/bin/gs"'" "$@"'
$INSTALL -m 755  gsdjvu $DESTDIR$gsdir/
$INSTALL -m 755  bin/gs $DESTDIR$gsdir/bin
$INSTALL -m 755  gsdjvu.sh $DESTDIR$bindir/gsdjvu 

echo Installing fonts
for n in fonts/* ; do 
  test -r $n && $INSTALL -m 644 $n $DESTDIR$gsdir/fonts 
done

echo Installing resources
if test -d Resource ; then
  for n in `find Resource -type d -print` ; do
      $MKDIRP $DESTDIR$gsdir/$n
  done
  for n in `find Resource -type f -print` ; do
      $INSTALL -m 644 $n $DESTDIR$gsdir/$n
  done
  cp lib/gs_res.ps lib/gs_res.ps.orig
  sed -e "s:(/Resource/:($gsdir/Resource/:g" lib/gs_res.ps.orig > lib/gs_res.ps
fi

echo Installing library files
for n in lib/*.ps lib/Fontmap* lib/CIDFnmap* lib/cidfmap lib/FAPI* ; do
  test -r $n && $INSTALL -m 644 $n $DESTDIR$gsdir/lib
done


echo Done
