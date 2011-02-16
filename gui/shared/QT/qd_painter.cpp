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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qd_painter.h"
#include "debug.h"
#include "qlib.h"
#include "cin_data.h"

#include <qpixmap.h>

void
QDPainter::drawDjVuWPaper(const GRect &rect, int x0, int y0)
{
   DEBUG_MSG("QDPainter::drawDjVuWPaper(): Displaying DejaVu wallpaper\n");
   DEBUG_MAKE_INDENT(3);

   if (!bmp || !bmp->rows()) bmp=GBitmap::create(*CINData::get("ppm_djvu_logo"));

   QPixmap qpix(bmp->columns(), bmp->rows());
   QePainter painter(&qpix);
   painter.drawBitmap(GRect(0, 0, qpix.width(), qpix.height()), bmp);
   painter.end();

   save();
   try
   {
      QPoint pnt=xForm(QPoint(0, 0));
      GRect ph_rect=rect; ph_rect.translate(pnt.x(), pnt.y());
      setClipRect(G2Q(ph_rect));
      setClipping(TRUE);
      fillRect(G2Q(rect), white);
      
	 // Display bitmap
      int  m = 40;
      int  xs = x0 + m - 1;
      for (int y=y0; y<rect.ymax; y+=qpix.height()+m)
      {
	 for (int x=xs; x<rect.xmax; x+=qpix.width()+m)
	    if (x+qpix.width()>rect.xmin && y+qpix.height()>=rect.ymin)
	       drawPixmap(x, y, qpix);
	 xs += qpix.width() * 3 / 5;
	 while (xs - m >= x0)
	    xs = xs - qpix.width() - m;
      }
   } catch(...)
   {
      restore();
      throw;
   }
   restore();
}
