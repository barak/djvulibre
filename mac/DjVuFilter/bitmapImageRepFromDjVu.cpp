/*
 *  bitmapImageRepFromDjVu.cpp
 *  DjVuFilter
 *
 *  Created by Jeff Sickel on Thu Jan 24 2002.
 *  Copyright (c) 2001, 2003 Corpus Callosum Corporation. All rights reserved.
 *
 *  2004-05-24 migrated to DjVuLibre
 */


#include "GException.h"
#include "GSmartPointer.h"
#include "GRect.h"
#include "GPixmap.h"
#include "GBitmap.h"
#include "DjVuImage.h"
#include "DjVuDocument.h"
#include "GOS.h"
#include "ByteStream.h"
#include "GURL.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif
    
	static double flag_scale = -1;
	static int    flag_size = -1;
	static int    flag_subsample = -1;
	static int    flag_segment = -1;
	static int    flag_verbose = 1;
	static int    flag_mode = 0;
	
	GRect fullrect;
	GRect segmentrect;
	
	static GP<DjVuImage> dimg;
	GP<GPixmap> pm;
	GP<GBitmap> bm;
	
	unsigned long start, stop;
	
	int
		djvu_imageFromPath(const char *from, int page_num)
	{
			unsigned long start, stop;
			
			// Create DjVuDocument
			const GURL from_url = GURL::Filename::UTF8(from);
			GP<DjVuDocument> doc=DjVuDocument::create_wait(from_url);
			if (! doc->wait_for_complete_init())
				G_THROW( ERR_MSG("Decoding failed") );
			
			// Create DjVuImage
			start=GOS::ticks();
			// GP<DjVuImage> dimg=doc->get_page(page_num);
			dimg=doc->get_page(page_num);
			if (! dimg->wait_for_complete_decode() )
				G_THROW("Decoding failed. Nothing can be done.");
			stop=GOS::ticks();
			
			// Verbose
			if (flag_verbose)
			{
				fprintf(stderr,"%s", (const char*)dimg->get_long_description());
				fprintf(stderr,"Decoding time:    %lu ms\n", stop - start);
			}
			
			// Check
			DjVuInfo *info = dimg->get_info();
			int colorp = dimg->is_legal_photo();
			int blackp = dimg->is_legal_bilevel();
			int compoundp = dimg->is_legal_compound();
			if (flag_verbose)
			{
				if (compoundp)
					fprintf(stderr, "This is a legal Compound DjVu image\n");
				else if (colorp)
					fprintf(stderr, "This is a legal Photo DjVu image\n");
				else if (blackp)
					fprintf(stderr, "This is a legal Bilevel DjVu image\n");
				// Without included files
				fprintf(stderr, "Direct memory usage is %4.1f Kb\n",
						(double)(dimg->get_djvu_file()->get_memory_usage())/1024 );
			}
			if (!compoundp && !colorp && !blackp)
			{
				fprintf(stderr,"Warning: This is not a well formed DjVu image\n");
				if (!info)
					G_THROW("Cannot find INFO chunk. Aborting.");
			}
			
			// Setup rectangles
			if (flag_size<0 && flag_scale<0 && flag_subsample<0)
				flag_scale = 100;
			if (flag_subsample>0)
				flag_scale = (double) info->dpi / flag_subsample;
			if (flag_scale>0)
			{
				int w = (int)(dimg->get_width() * flag_scale / info->dpi);
				int h = (int)(dimg->get_height() * flag_scale / info->dpi);
				if (w<1) w=1;
				if (h<1) h=1;
				fullrect = GRect(0,0, w, h);
			}
			if (flag_segment < 0)
				segmentrect = fullrect;
			
			// Render
			start = GOS::ticks();
			switch(flag_mode)
			{
				case 's':
					bm = dimg->get_bitmap(segmentrect, fullrect);
					break;
				case 'f':
					pm = dimg->get_fg_pixmap(segmentrect, fullrect);
					break;
				case 'b':
					pm = dimg->get_bg_pixmap(segmentrect, fullrect);
					break;
				default:
					fprintf(stderr,"getting pixmap...");
					pm = dimg->get_pixmap(segmentrect, fullrect);
					if (! pm)
						bm = dimg->get_bitmap(segmentrect, fullrect);
						break;
			}
			stop = GOS::ticks();
			if (flag_verbose)
			{
				fprintf(stderr,"Rendering time:   %lu ms\n", stop - start);
			}
			
			return 1;
	}
	
	int djvu_width() {
		int w = (int)(dimg->get_width() * flag_scale / dimg->get_dpi());
		if (w<1) w=1;
		return w;
	}
	
	int djvu_height() {
		int h = (int)(dimg->get_height() * flag_scale / dimg->get_info()->dpi);
		if (h<1) h=1;
		return h;
	}
	
	const int djvu_isLegalBilevel() {
		return dimg->is_legal_bilevel();
	}
	
	const int djvu_getGrays() {
		return bm->get_grays();
	}
	
	const int djvu_bm_rowsize(int xmin, int ymin, int w, int h)
	{
		int result;
		if (pm)
			result = pm->rowsize();
		else
			result = bm->rowsize();
		
		return result;
	}
	
	void
		convert(unsigned char *bitmapData)
	{
			// Save image
			if (pm)
			{
				int rows, columns, y, x;
				// GP<ByteStream> out=ByteStream::create(to,"wb");
				// pm->save_ppm(*out);
				
				rows = pm->rows();
				columns = pm->columns();
				for (y=rows-1; y >= 0; y--) {
					GPixel *p = (*pm)[y];
					for (x = 0; x < columns; x++) {
						*bitmapData++ = p[x].r;
						*bitmapData++ = p[x].g;
						*bitmapData++ = p[x].b;
					}
				}
			}
			else if (bm)
			{
				int nrows, ncolumns, y, x;
				int grays = bm->get_grays();
				
				// GP<ByteStream> out=ByteStream::create(to,"wb");
				
				nrows = bm->rows();
				ncolumns = bm->columns();
				
				if (grays == 2) {
					if (flag_verbose) {
						fprintf(stderr, "Using BiLevel rendering with grays == %i\n", grays);
					}
					for (y=nrows-1; y >= 0; y--) {
						unsigned char *p = (*bm)[y];	// get the pixels of the row
						unsigned char acc = 0;
						unsigned char mask = 0;
						for (x = 0; x < ncolumns; x++) {
							if (mask == 0)
								mask = 0x80;
							if (p[x])
								acc |= mask;
							mask >>= 1;
							if (mask==0) {
								*bitmapData++ = acc;
								acc = mask = 0;
							}
						}
						if (mask != 0)
							*bitmapData++ = acc;
					}
				} else {
					if (flag_verbose) {
						fprintf(stderr, "Using BiLevel rendering with grays == %i\n", grays);
					}
					for (y=nrows-1; y >= 0; y--) {
						unsigned char *p = (*bm)[y];
						for (x = 0; x < ncolumns; x++) {
							// packs byte based on number of grays
							*bitmapData++ =  abs((p[x] * 255)/(grays) - 255);
						}
					}
				}
			}
			else
			{
				G_THROW("Cannot render the requested image");
			}
	}
	
#ifdef __cplusplus
}
#endif
