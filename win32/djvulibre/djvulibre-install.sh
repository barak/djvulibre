#!/bin/sh

c=/cygdrive/c
qtdir=$c/QtSDK/Desktop/Qt/4.8.1/msvc2010
djdir=$HOME/djvulibre-3.5
dwdir=$djdir/win32/djvulibre/Release
djsrc=$HOME/djvulibre-djview/src
msvc="$c/Program Files/Microsoft Visual Studio 10.0/VC"
msredist="$msvc/redist/x86/Microsoft.VC100.CRT"

target=$HOME/DjVuLibre

function run() {
    echo "$@"
    "$@"
    if test $? -ne 0 ; then
      echo "FAILED: " "$@"
    fi
}

## Djvulibre tools
echo ---- DjVuLibre tools

djexe="bzz.exe c44.exe cjb2.exe cpaldjvu.exe csepdjvu.exe
       ddjvu.exe djvm.exe djvmcvt.exe djvudump.exe djvuextract.exe 
       djvumake.exe djvups.exe djvused.exe djvutoxml.exe
       djvutxt.exe djvuxmlparser.exe"
djdll="libdjvulibre.dll libjpeg.dll libtiff.dll libz.dll"
for n in $djdll $djexe ; do 
    run cp $dwdir/$n $target ; done

## Qt libs
echo ---- Qt libs

qtdll="QtCore4.dll QtGui4.dll QtNetwork4.dll QtOpenGL4.dll"
qtssl="ssleay32.dll libeay32.dll libssl32.dll"
qtplug="accessible codecs imageformats"
for n in $qtdll ; do 
    run cp $qtdir/lib/$n $target ; done
for n in $qtssl ; do 
    test -r $qtdir/bin/$n && run cp $qtdir/bin/$n $target ; done
test -d $target/plugins || run mkdir $target/plugins
for n in $qtplug ; do 
    test -d $target/plugins/$n || run mkdir $target/plugins/$n 
    for m in $qtdir/plugins/$n/*.dll ; do
        run cp $m $target/plugins/$n ; done
    run chmod 0755 $target/plugins/$n/* ; done
    run rm $target/plugins/*/*d4.dll
    run rm $target/plugins/imageformats/qsvg*
run find $target -name '*.dll' -exec chmod 0755 {} \;

echo '[Paths]' > $target/qt.conf

## MS libs
echo ---- MS libs

for n in "$msredist"/* ; do
    run cp "$n" $target; done

## DjVuLibre shared files
echo ---- DjVuLibre shared files

test -d $target/share || mkdir $target/share
run cp -r $djdir/share/djvu $target/share
run find $target/share -name CVS -exec rm -rf {} \; -prune

## DjVuLibre dev files
echo ---- DjVuLibre dev files

run cp $dwdir/libdjvulibre.lib $target
test -d $target/include || run mkdir $target/include
test -d $target/include/libdjvu || run mkdir $target/include/libdjvu
run cp $djdir/libdjvu/miniexp.h $target/include/libdjvu
run cp $djdir/libdjvu/ddjvuapi.h $target/include/libdjvu

## DjView exe
echo ---- DjView exe

if test -r $dwdir/djview.exe ; then
  run cp $dwdir/djview.exe $target
else
  run cp $djsrc/release/djview.exe $target
fi
( cd $djsrc; run $qtdir/bin/lrelease djview.pro )
test -d $target/share/djvu/djview4 || run mkdir $target/share/djvu/djview4
run cp $djsrc/*.qm $target/share/djvu/djview4
run cp $qtdir/translations/qt_*.qm  $target/share/djvu/djview4
run chmod 0644 $target/share/djvu/djview4/qt*.qm
run rm -f $target/share/djvu/djview4/qt_help_*.qm


## Doc
echo ---- Doc

run cp $djdir/win32/djvulibre/djvulibre.nsi $target
run cp $djdir/win32/djvulibre/djvulibre*.nsh $target

test -d $target/man || run mkdir $target/man
test -d $target/doc || run mkdir $target/doc
run cp $djdir/doc/*.djvu $target/doc
run cp $djdir/doc/*.txt $target/doc
run unix2dos $target/doc/*.txt
run cp $djdir/COPYING $target/COPYING.txt
run unix2dos $target/COPYING.txt
