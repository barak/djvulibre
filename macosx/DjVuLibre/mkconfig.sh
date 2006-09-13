#!/bin/sh

# Make sure that the config.h file is generated properly enough for gov work

withjpeg=""
withtiff=""
if [ -e /usr/local/lib/libjpeg* ]; then
    withjpeg="--with-jpeg=/usr/local"
fi
if [ -e /usr/local/lib/libtiff* ]; then
    withtiff="--with-tiff=/usr/local"
fi

if [ ! -e ../../config.h ]; then
    cd ../..; \
    ./configure --enable-xmltools --enable-static $withjpeg $withtiff
fi

(
    ljpeg=""
    if [ "x$withjpeg" != "x" ]; then
        ljpeg="-ljpeg"
    fi
    ltiff=""
    if [ "x$withtiff" != "x" ]; then
        ltiff="-ltiff"
    fi
    if [ "x$SOURCE_ROOT" != "x" ]; then
        cd $SOURCE_ROOT
    fi
    if [ -e VersionConfig ]; then
        mv VersionConfig _VersionConfig
    fi

    ver=`grep DJVULIBRE_VERSION ../../config.h | awk '{print $3}'`
    ver=`echo $ver | sed 's/"//g'`
    compver=`echo $ver | awk -F. '{printf "%s.%s", $1, $2}'`

    echo "DYLIB_CURRENT_VERSION = $ver" > VersionConfig
    echo "DYLIB_COMPATIBILITY_VERSION = $compver" >> VersionConfig
    
    a=""
    if [ "x$ljpeg" != "x" ]; then
        a="$ljpeg"
    fi
    if [ "x$ltiff" != "x" ]; then
        a="$a $ltiff"
    fi
    echo "VersionLDFLAGS = $a" >> $SOURCE_ROOT/VersionConfig
)
