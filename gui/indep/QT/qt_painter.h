//C-  -*- C++ -*-
//C-
//C- DjVu® Unix Viewer (v. 3.5)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//C-
// 
// $Id$
// $Name$

#ifndef HDR_QT_PAINTER
#define HDR_QT_PAINTER
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "GPixmap.h"
#include "GBitmap.h"
#include "GContainer.h"
#include "GRect.h"

#include <qpainter.h>

class QePainter : public QPainter
{
public:
   class PatchRect : public GPEnabled
   {
   public:
      GRect		rect;
      GP<GPixmap>	pixmap;

      PatchRect(const GRect & _rect, const GP<GPixmap> & _pixmap) :
	    rect(_rect), pixmap(_pixmap) {}
   };
   
   void		drawPixmap(const GRect &rect, GPixmap *pm,
			   int use_shm_extension=0);
   void		drawPixmap(const GRect &rect, int pm_x0, int pm_y0,
			   GPixmap *pm, int use_shm_extension=0);
   void		drawBitmap(const GRect &rect, GBitmap *bm,
			   int use_shm_extension=0);
   void		drawBitmap(const GRect &rect, int bm_x0, int bm_y0,
			   GBitmap *bm, int use_shm_extension=0);
   void		drawPatchedBitmap(const GRect & bm_rect, GBitmap * bm,
				  const GRect & pm_rect, GPixmap * pm,
				  int use_shm_extension=0);
   void		drawPatchedBitmaps(const GRect & bm_rect, GBitmap * bm,
				   const GPList<PatchRect> & pm_list,
				   int use_shm_extension=0);

      // Provide wrappers for standard drawPixmap()
   void drawPixmap(int x, int y, const QPixmap & pix,
		   int sx=0, int sy=0, int sw=-1, int sh=-1)
   {
      QPainter::drawPixmap(x, y, pix, sx, sy, sw, sh);
   }
   void drawPixmap(const QPoint & pnt, const QPixmap & pix, const QRect & sr)
   {
      QPainter::drawPixmap(pnt, pix, sr);
   }
   void drawPixmap(const QPoint & pnt, const QPixmap & pix)
   {
      QPainter::drawPixmap(pnt, pix);
   }
   
   QePainter(void) {}
   QePainter(const QPaintDevice * dev) : QPainter(dev) {}
   QePainter(const QPaintDevice * dev, const QWidget * w) :
	 QPainter(dev, w) {}
   ~QePainter(void) {}
};

#endif
