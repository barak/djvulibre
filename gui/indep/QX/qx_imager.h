//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either Version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C- 
//C- DjVuLibre-3.5 is derived from the DjVu(r) Reference Library from
//C- Lizardtech Software.  Lizardtech Software has authorized us to
//C- replace the original DjVu(r) Reference Library notice by the following
//C- text (see doc/lizard2002.djvu and doc/lizardtech2007.djvu):
//C-
//C-  ------------------------------------------------------------------
//C- | DjVu (r) Reference Library (v. 3.5)
//C- | Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- | The DjVu Reference Library is protected by U.S. Pat. No.
//C- | 6,058,214 and patents pending.
//C- |
//C- | This software is subject to, and may be distributed under, the
//C- | GNU General Public License, either Version 2 of the license,
//C- | or (at your option) any later version. The license should have
//C- | accompanied the software or you may obtain a copy of the license
//C- | from the Free Software Foundation at http://www.fsf.org .
//C- |
//C- | The computer code originally released by LizardTech under this
//C- | license and unmodified by other parties is deemed "the LIZARDTECH
//C- | ORIGINAL CODE."  Subject to any third party intellectual property
//C- | claims, LizardTech grants recipient a worldwide, royalty-free, 
//C- | non-exclusive license to make, use, sell, or otherwise dispose of 
//C- | the LIZARDTECH ORIGINAL CODE or of programs derived from the 
//C- | LIZARDTECH ORIGINAL CODE in compliance with the terms of the GNU 
//C- | General Public License.   This grant only confers the right to 
//C- | infringe patent claims underlying the LIZARDTECH ORIGINAL CODE to 
//C- | the extent such infringement is reasonably necessary to enable 
//C- | recipient to make, have made, practice, sell, or otherwise dispose 
//C- | of the LIZARDTECH ORIGINAL CODE (or portions thereof) and not to 
//C- | any greater extent that may be necessary to utilize further 
//C- | modifications or combinations.
//C- |
//C- | The LIZARDTECH ORIGINAL CODE is provided "AS IS" WITHOUT WARRANTY
//C- | OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- | TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- | MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//C- +------------------------------------------------------------------

#ifndef HDR_QX_IMAGER
#define HDR_QX_IMAGER
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "qt_imager.h"
#include "GRect.h"
#include "GPixmap.h"
#include "GBitmap.h"
#include "GContainer.h"

// This class provives implementation of the interface defined by QeImager
// in qt_imager.* Please note, that we avoid including X11 headers in
// this header. The point is that they conflict a lot with QT stuff, and
// it's difficult to resolve these conflicts in each and every file,
// which is going to include qx_imager.h
// That's why types of arguments are funny sometimes: like 'u_long drawable'
// instead of normal 'Drawable drawable'

class QXImager : public QeImager
{
protected:
   int		  colormap_warned;
   int		  in_netscape;

   Display	 *displ;
   void		 *visual;
   unsigned long  colormap;
   int		  depth;
   bool		  is_color;
   u_int32	  red_mask, green_mask, blue_mask;

   bool		optimizeLCD;
   int		machine_byte_order;
   u_int32	table24_red[256];
   u_int32	table24_green[256];
   u_int32	table24_blue[256];
   u_int32	table24l_red[256];
   u_int32	table24l_green[256];
   u_int32	table24l_blue[256];
   u_int32	table16_red[256];
   u_int32	table16_green[256];
   u_int32	table16_blue[256];
   u_int32	table8[256];	// In color mode only 6*6*6 elements are used
   int		table8_idx_red[256];
   int		table8_idx_green[256];
   int		table8_idx_blue[256];
   u_long	allocated_color[256];
   int		allocated_colors;

      // These three are used by allocateCell() to save data between
      // calls. Don't interfere!
   bool		colormap_full;
   void		* colormap_cells;	// XColor * colormap_cells;
   int		max_distance;
   
   void		copyPixmap(class DXImage * image, const GRect & im_rect,
			   GPixmap *pm, int pm_x0, int pm_y0,
			   int use_shm_extension);
   void		copyBitmap(class DXImage * image, const GRect & im_rect,
			   GBitmap * bm, int bm_x0, int bm_y0,
			   int use_shm_extension);
   u_int32	allocateCell(int r, int g, int b);	// r=[0, 0xffff]
public:
   int		getDepth(void) const { return depth; }
   bool		isColor(void) const { return is_color; }
   
   class PatchRect : public GPEnabled
   {
   public:
     GRect		rect;
     GP<GPixmap>	pixmap;
     int		pm_x0, pm_y0;
     PatchRect(const GRect & _rect, const GP<GPixmap> & _pixmap,
	       int _pm_x0=0, int _pm_y0=0) 
       : rect(_rect), pixmap(_pixmap), pm_x0(_pm_x0), pm_y0(_pm_y0) {}
   };
  
   virtual u_int32	getGrayXColor(float level);
   virtual u_int32	getXColor(u_char r, u_char g, u_char b);
   virtual u_int32	getXColor(u_int32 color);
   virtual u_int32	getXColor(const char * name);

   virtual QColor	getGrayColor(float level);
   virtual QColor	getColor(u_char r, u_char g, u_char b);
   virtual QColor	getColor(u_int32 color);
   virtual QColor	getColor(const char * name);
   virtual QColor	getColor(const QColor & col);
   
   virtual QPixmap	getColorPixmap(int width, int height,
				       u_char r, u_char g, u_char b);
   virtual QPixmap	getColorPixmap(int width, int height,
				       u_int32 color);
   
   void		displayPixmap(u_long drawable, GC gc,
			      const GRect &rect, int pm_x0, int pm_y0,
			      GPixmap *pm, int use_shm_extension=0);
   void		displayBitmap(u_long drawable, GC gc,
			      const GRect &rect, int bm_x0, int bm_y0,
			      GBitmap *bm, int use_shm_extension=0);
   void		displayPatchedBitmap(u_long drawable, GC gc,
				     const GRect & bm_rect, int bm_x0,
				     int bm_y0, GBitmap * bm,
				     const GRect & pm_rect, int pm_x0,
				     int pm_y0, GPixmap * pm,
				     int use_shm_extension=0);
   void		displayPatchedBitmaps(u_long drawable, GC gc,
				      const GRect & bm_rect, int bm_x0,
				      int bm_y0, GBitmap * bm,
				      const GPList<PatchRect> & pm_list,
				      int use_shm_extension=0);

   void		displayPixmap(u_long drawable, GC gc,
			      const GRect &rect, GPixmap *pm,
			      int use_shm_extension=0)
   {
      displayPixmap(drawable, gc, rect, 0, 0, pm, use_shm_extension);
   }
   void		displayBitmap(u_long drawable, GC gc,
			      const GRect &rect, GBitmap *bm,
			      int use_shm_extension=0)
   {
      displayBitmap(drawable, gc, rect, 0, 0, bm, use_shm_extension);
   }
   void		displayPatchedBitmap(u_long drawable, GC gc,
				     const GRect & bm_rect, GBitmap * bm,
				     const GRect & pm_rect, GPixmap * pm,
				     int use_shm_extension=0)
   {
      displayPatchedBitmap(drawable, gc, bm_rect, 0, 0, bm,
			   pm_rect, 0, 0, pm, use_shm_extension);
   }
   void		displayPatchedBitmaps(u_long drawable, GC gc,
				      const GRect & bm_rect, GBitmap * bm,
				      const GPList<PatchRect> & pm_list,
				      int use_shm_extension=0)
   {
      displayPatchedBitmaps(drawable, gc, bm_rect, 0, 0, bm,
			    pm_list, use_shm_extension);
   }

   void		setOptimizeLCD(bool _optimizeLCD);

   void		dither(GPixmap & gpix, int x0=0, int y0=0);
   
   QXImager(Display * _displ, void * _visual,
	    unsigned long _colormap, int _depth,
	    bool _in_netscape, bool _optimizeLCD);
   ~QXImager(void);
};

extern QXImager	* qxImager;

#endif
