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

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qt_painter.h"
#include "exc_msg.h"

#ifdef UNIX
#include "qx_imager.h"
#else
// "You got to include smth like qw_imager.h here"
#endif

void
QePainter::drawPixmap(const GRect & irect, GPixmap *pm,
		      int use_shm_extension)
{
   GRect ph_rect=irect;
   QPoint pnt=xForm(QPoint(0, 0));
   ph_rect.translate(pnt.x(), pnt.y());

#ifdef UNIX
   if (!qxImager)
      throw ERROR_MESSAGE("QePainter::drawPixmap",
			  "Can't draw pixmap since QXImager has not been initialized yet.");

   qxImager->displayPixmap((u_long) hd, gc,
			   ph_rect, pm, use_shm_extension);
#else
   throw ERROR_MESSAGE("QePainter::drawPixmap",
                       "Implement QePainter for windows\n");
#endif
}

void
QePainter::drawPixmap(const GRect &irect, int pm_x0, int pm_y0,
		      GPixmap *pm, int use_shm_extension)
{
   GRect ph_rect=irect;
   QPoint pnt=xForm(QPoint(0, 0));
   ph_rect.translate(pnt.x(), pnt.y());
   
#ifdef UNIX
   if (!qxImager)
      throw ERROR_MESSAGE("QePainter::drawPixmap",
			  "Can't draw pixmap since QXImager has not been initialized yet.");
   
   qxImager->displayPixmap((u_long) hd, gc,
			   ph_rect, pm_x0, pm_y0, pm, use_shm_extension);
#else
   throw ERROR_MESSAGE("QePainter::drawPixmap",
                       "Implement QePainter for windows\n");
#endif
}

void
QePainter::drawBitmap(const GRect &irect, GBitmap *bm,
		      int use_shm_extension)
{
   GRect ph_rect=irect;
   QPoint pnt=xForm(QPoint(0, 0));
   ph_rect.translate(pnt.x(), pnt.y());
   
#ifdef UNIX
   if (!qxImager)
      throw ERROR_MESSAGE("QePainter::drawBitmap",
			  "Can't draw bitmap since QXImager has not been initialized yet.");
   
   qxImager->displayBitmap((u_long) hd, gc,
			   ph_rect, bm, use_shm_extension);
#else
   throw ERROR_MESSAGE("QePainter::drawBitmap",
                       "Implement QePainter for windows\n");
#endif
}

void
QePainter::drawBitmap(const GRect &irect, int bm_x0, int bm_y0,
		      GBitmap *bm, int use_shm_extension)
{
   GRect ph_rect=irect;
   QPoint pnt=xForm(QPoint(0, 0));
   ph_rect.translate(pnt.x(), pnt.y());
   
#ifdef UNIX
   if (!qxImager)
      throw ERROR_MESSAGE("QePainter::drawBitmap",
			  "Can't draw bitmap since QXImager has not been initialized yet.");
   
   qxImager->displayBitmap((u_long) hd, gc,
			   ph_rect, bm_x0, bm_y0, bm, use_shm_extension);
#else
   throw ERROR_MESSAGE("QePainter::drawBitmap",
                       "Implement QePainter for windows\n");
#endif
}

void
QePainter::drawPatchedBitmap(const GRect & bm_rect, GBitmap * bm,
			     const GRect & pm_rect, GPixmap * pm,
			     int use_shm_extension)
{
   GPList<PatchRect> list;
   list.append(new PatchRect(pm_rect, pm));
   drawPatchedBitmaps(bm_rect, bm, list, use_shm_extension);
}

void
QePainter::drawPatchedBitmaps(const GRect & bm_rect, GBitmap * bm,
			      const GPList<PatchRect> & pm_list,
			      int use_shm_extension)
{
#ifdef UNIX
   if (!qxImager)
      throw ERROR_MESSAGE("QePainter::drawPatchedBitmap",
			  "Can't draw bitmap since QXImager has not been initialized yet.");

   QPoint pnt=xForm(QPoint(0, 0));

   GRect bmh_rect=bm_rect;
   bmh_rect.translate(pnt.x(), pnt.y());
   
   GPList<QXImager::PatchRect> list;
   for(GPosition pos=pm_list;pos;++pos)
   {
      PatchRect & p=*pm_list[pos];
      GP<QXImager::PatchRect> prect=new QXImager::PatchRect(p.rect, p.pixmap);
      prect->rect.translate(pnt.x(), pnt.y());
      list.append(prect);
   }
   
   qxImager->displayPatchedBitmaps((u_long) hd, gc, bmh_rect, bm,
				   list, use_shm_extension);
#else
   throw ERROR_MESSAGE("QePainter::drawPatchedBitmap",
                       "Implement QePainter for windows\n");
#endif
}

