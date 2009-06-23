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
// 
// $Id: qt_painter.h,v 1.7 2007/03/25 20:48:23 leonb Exp $
// $Name: release_3_5_22 $

#ifndef HDR_QT_PAINTER
#define HDR_QT_PAINTER
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
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
