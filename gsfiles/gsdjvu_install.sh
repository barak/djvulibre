#!/bin/sh


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

echo Installing library files
for n in lib/*.ps lib/Fontmap* lib/CIDFnmap* ; do
  test -r $n && $INSTALL -m 644 $n $DESTDIR$gsdir/lib
done

echo Done
