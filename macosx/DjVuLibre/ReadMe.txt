DjVu.mdimporter is a Spotlight metadata importer for DjVu files.  

To install:

    copy DjVu.mdimporter to /Library/Spotlight or ~/Library/Spotlight
    
    run mdimport to for the initial index import (not completely necessary, but
    good to start things off):
    
    /usr/bin/mdimport -r /Library/Spotlight/DjVu.mdimporter

Note:
    As of version 1.2, DjVu.mdimported and djvu.qlgenerator are included
    with DjView.app.

Further information on DjVu and DjVuLibre, including source, can be found at
http://djvulibre.djvuzone.org/.

DjVu.mdimporter and DjVuLibre.framework are maintained by
Jeff Sickel <jas@corpus-callosum.com>.


History:

16-Mar-2009 version 1.3
    file system representation update (discovered by Nicholas Kulikov)
    
01-Sep-2008 version 1.2
    bundled with DjView.app
    
10-Dec-2007 version 1.1
    built with djvulibre 3.5.19
    added org.djvu.DjView for eventually roll in with DjView.app

version 1.0
    The importer is based on the latest stable build of djvulibre--3.5.17 at the
    time of compilation.  It is deployed as a universal binary for ppc and i386
    machines.

    metadata imported:
        kMDItemTextContent
        kMDItemNumberOfPages
        kMDItemPixelHeight
        kMDItemPixelWidth
        kMDItemPageHeight
        kMDItemPageWidth
        kMDItemVersion
        kMDItemOrientation
