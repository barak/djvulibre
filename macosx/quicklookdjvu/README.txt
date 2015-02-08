djvu.qlgenerator

QuickLook generator for DjVu documents.

To install, copy djvu.qlgenerator to:

	DjView.app/Contents/Library/QuickLook  (version 1.0+)
	/Library/QuickLook
	~/Library/QuickLook

QuickLook is a Leopard only feature.  The DjVu generator depends on the
DjVu.mdimporter also being installed in order to properly render multi-page
DjVu documents when using the Finder, Time Machine, or QuickLook previews.
Multi-paged documents will default to previewing five pages with a "..->" image
to indicate more pages are available when viewing the documents with DjView or
other DjVu viewers.

Additional settings can be modified trough the defaults system.  Settings are
for defining the maximum number of pages to preview, enabling thumbnailing,
and debugging messages to the console.  Settings can be modified as follows:

	defaults write org.djvu.qlgenerator previewpages 2
	defaults write org.djvu.qlgenerator debug YES
	defaults write org.djvu.qlgenerator thumbnail NO

Modifications or deleting the org.djvu.qlgenerator entry from defaults will
reset the qlgenerator to default values built into the generator, e.g.:

	defaults delete org.djvu.qlgenerator thumbnail
	defaults delete org.djvu.qlgenerator


Occasionally, prior DjVu viewers may have installed a default icon into the
HFS+ resource fork of a .djvu file.  This will prevent the first page from being
previewed in CoverFlow views and thumbnails in other Finder views.  Corrections
can usually be made by resetting the "Open With" segment of the file's
inspector view from the Finder.

Further information about DjVu can be found at http://www.djvu.org/.

djvu.qlgenerator is maintained by Jeff Sickel <jas@corpus-callosum.com>.


History:

15-Mar-2009 version 1.1
    handle localized file names (UTF-8)
    bug/patch submitted by Nicholas Kulikov
    
13-Dec-2007 version 1.0
    able to be bundled inside DjView.app 
    added defaults for org.djvu.qlgenerator previewpages settings

10-Dec-2007 version 0.9
    original release
    and embedded statically linked version of ddjvu is included
