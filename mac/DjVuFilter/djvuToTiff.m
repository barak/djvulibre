/*
 djvuToTiff.m, DjVu to TIFF converter.
 Author:  Jeff Sickel
 Written for 1.1, May 26, 2004.
 
 Copyright (c) 2004 by Corpus Callosum Corporation, all rights reserved.
 */


#import <Foundation/Foundation.h>

extern int djvu_imageFromPath(const char *from, int page_num);
extern int djvu_width();
extern int djvu_height();

extern const int djvu_isLegalBilevel();
extern const int djvu_getGrays();
extern const int djvu_bm_rowsize(int xmin, int ymin, int w, int h);

extern void
convert(unsigned char *bitmapData);

NSBitmapImageRep *
bitmapImageRepFromDjVu(NSString *djvuPath, int pagenum, int dpi, NSSize size)
{
	NSBitmapImageRep *tiff = nil;

	if (djvu_imageFromPath([djvuPath cString], pagenum)) {
		int bps;				// bits per sample
		int spp;				// samples per pixel
		int bpp=0;				// bits per pixel (default from bps)
		int rowBytes=0;			// bytes per row
		NSString *colorSpace = nil;
		
		if (djvu_isLegalBilevel()) {
			int grays = 2;
			grays = djvu_getGrays();
			rowBytes = djvu_bm_rowsize(0, 0, djvu_width(), djvu_height());
			spp =  1;
			bps = (grays == 2) ? 4 : 8;
		} else {
			bps = 8;
			spp = 3;
		}
		
		colorSpace = ((spp > 2) ? NSCalibratedRGBColorSpace : NSCalibratedWhiteColorSpace);
		tiff = [[NSBitmapImageRep alloc]
						initWithBitmapDataPlanes:NULL
									  pixelsWide:djvu_width()
									  pixelsHigh:djvu_height()
								   bitsPerSample:bps
								 samplesPerPixel:spp
										hasAlpha:NO
										isPlanar:NO
								  colorSpaceName:colorSpace
									 bytesPerRow:rowBytes
									bitsPerPixel:bpp];
		
		NS_DURING
			convert([tiff bitmapData]);
		NS_HANDLER
			[localException raise];
		NS_ENDHANDLER
	}

	return [tiff autorelease];
}
