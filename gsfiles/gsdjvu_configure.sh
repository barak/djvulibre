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


###### Patches

rm -f 2>/dev/null gsdjvu
cp $gsfiles/gsdjvu gsdjvu

rm -f 2>/dev/null src/gdevdjvu.c
cp $gsfiles/gdevdjvu.c src/gdevdjvu.c

rm -f 2>/dev/null lib/ps2utf8.ps
cp $gsfiles/ps2utf8.ps lib/ps2utf8.ps

test -r src/contrib.mak.gsdjvu && \
  mv src/contrib.mak.gsdjvu src/contrib.mak
cp src/contrib.mak src/contrib.mak.gsdjvu
grep -q djvusep src/contrib.mak || \
  cat $gsfiles/contrib.mak.add >> src/contrib.mak


###### Configure

./configure \
  --without-x \
  --without-ijs \
  --without-gimp-print

cp Makefile Makefile.gsdjvu

sed < Makefile.gsdjvu > Makefile.tmp1 \
    -e 's!$(DD)[a-z0-9]*jet[a-z0-9]*\.dev!!g' \
    -e 's!$(DD)cdj[a-z0-9]*\.dev!!g' \
    -e 's!$(DD)bj[a-z0-9]*\.dev!!g' \
    -e 's!$(DD)pj[a-z0-9]*\.dev!!g' \
    -e 's!$(DD)lj[a-z0-9]*\.dev!!g' \
    -e 's!$(DD)pxl[a-z0-9]*\.dev!!g' \
    -e 's!$(DD)uniprint\.dev!!g' \
    -e 's!$(DD)x[a-z0-9]*\.dev!!g' \
    -e 's!$(DD)ps[a-z0-9]*\.dev!!g' \
    -e 's!$(DD)pdb[a-z0-9]*\.dev!!g' \
    -e 's!$(DD)pdb[a-z0-9]*\.dev!!g' \
    -e 's!^\(GS_LIB_DEFAULT=\).*$!\1/usr/lib/gsdjvu/lib:/usr/lib/gsdjvu/fonts!'

if grep -q djvusep Makefile.tmp1
then
   mv Makefile.tmp1 Makefile
else
   sed < Makefile.tmp1 > Makefile \
     -e 's!$(DD)bbox.dev!\0 $(DD)djvumask.dev $(DD)djvusep.dev!g' 
   rm Makefile.tmp1
fi

###### Ready

echo "Compile with 'make'"
echo "Install with 'gsdjvu_install.sh'"


