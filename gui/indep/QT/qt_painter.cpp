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

#include "qt_painter.h"
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
   if (qxImager)
     qxImager->displayPixmap((u_long) hd, gc,
                             ph_rect, pm, use_shm_extension);
#else
   G_THROW("Implement QePainter for windows\n");
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
   if (qxImager)
     qxImager->displayPixmap((u_long) hd, gc,
                             ph_rect, pm_x0, pm_y0, pm, use_shm_extension);
#else
   G_THROW("Implement QePainter for windows\n");
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
   if (qxImager)
     qxImager->displayBitmap((u_long) hd, gc,
                             ph_rect, bm, use_shm_extension);
#else
   G_THROW("Implement QePainter for windows\n");
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
   if (qxImager)
     qxImager->displayBitmap((u_long) hd, gc,
                             ph_rect, bm_x0, bm_y0, bm, use_shm_extension);
#else
   G_THROW("Implement QePainter for windows\n");
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
  if (qxImager)
    {
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
    }
#else
  G_THROW("Implement QePainter for windows\n");
#endif
}

