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

#include "GPixmap.h"

#include "qd_toolbutt.h"
#include "qt_painter.h"
#include "qx_imager.h"
#include "debug.h"


#include <qpaintdevicemetrics.h>
#include <qtooltip.h>
#include <qimage.h>
#include <qbitmap.h>
#include <qlabel.h>
#include <qapplication.h>



QDToolButton::QDToolButton(ByteStream & str, bool _shadow, int _cmd,
			   QWidget * parent, const QString & name) :
      QToolButton(parent, name), shadow(_shadow), cmd(_cmd)
{
      // Make a copy of the data (I will have to rewind the stream and
      // I am not sure, that input ByteStream supports it)
   GP<ByteStream> gmstr=ByteStream::create();
   ByteStream &mstr = *gmstr;
   mstr.copy(str);
   mstr.seek(0);
   
   set_on=set_off=set_armed=0;
   status_bar=0;
   if (!(set_off=createSet(mstr, shadow ? 1 : 0)))
      G_THROW("Failed to create pixmap for QDToolButton");
   if (shadow)
   {
      mstr.seek(0);
      if (!(set_armed=createSet(mstr, 2)))
        G_THROW("Failed to create pixmap for QDToolButton");
      set_armed->setPixmap(set_armed->pixmap(QIconSet::Small, QIconSet::Normal),
			   QIconSet::Small, QIconSet::Active);
   }
   
   setIconSet(*set_off);
   setUsesBigPixmap(FALSE);
   
   setMinimumWidth(set_off->pixmap().width()+4);
   setMinimumHeight(set_off->pixmap().height()+4);
      
   if (!name.isEmpty()) QToolTip::add(this, name);

   connect(this, SIGNAL(toggled(bool)), this, SLOT(slotToggled(bool)));
   connect(this, SIGNAL(pressed(void)), this, SLOT(slotPressed(void)));
   connect(this, SIGNAL(released(void)), this, SLOT(slotReleased(void)));
}

QDToolButton::~QDToolButton(void)
{
   delete set_off;
   delete set_on;
   delete set_armed;
}

void
QDToolButton::enterEvent(QEvent * ev)
{
   if (status_bar) status_bar->setText(name());
   QToolButton::enterEvent(ev);
}

void
QDToolButton::leaveEvent(QEvent * ev)
{
   if (status_bar) status_bar->setText(" ");
   QToolButton::leaveEvent(ev);
}

QIconSet *
QDToolButton::createSet(ByteStream & str, int shadow_width)
{
   GP<GPixmap> ggpix=GPixmap::create(str);
   GPixmap &gpix=*ggpix;

   int width=gpix.columns();
   int height=gpix.rows();

      // Create mask by hands (heuristic mask offered by QT is ugly)
   QImage qimg(width, height, 1, 2, QImage::LittleEndian);
   qimg.fill(0);
   GPixel trans_pix=gpix[0][0];
   for(int i=0;i<height;i++)
      for(int j=0;j<width;j++)
      {
	 GPixel & pixel=gpix[i][j];
	 if (pixel!=trans_pix) qimg.setPixel(j, height-i-1, 1);
      }
   QBitmap qbmp1(width, height);
   qbmp1.convertFromImage(qimg, MonoOnly);

   if (qxImager)
     qxImager->dither(gpix);

      // Create Normal pixmap
   QPixmap qpix1(width, height);
   QePainter p1(&qpix1);
   p1.drawPixmap(GRect(0, 0, width, height), &gpix);
   p1.end();

      // Create Disabled pixmap. Actually I wouldn't have to create the
      // disabled pixmap myself were it not for QT-1.41 bug.
   QPixmap qpix2(width+1, height+1);
#ifdef QT1
   QColorGroup dis(QApplication::palette()->disabled());
#else
   QColorGroup dis(QApplication::palette().disabled());
#endif
   qpix2.fill(dis.background());
   QePainter p2(&qpix2);
   p2.setPen(dis.base());
   p2.drawPixmap(1, 1, qbmp1);
   p2.setPen(dis.foreground());
   p2.drawPixmap(0, 0, qbmp1);
   p2.end();

      // Create the 2nd mask
   QBitmap qbmp2(width+1, height+1);
   qbmp2.fill(color0);
   qbmp1.setMask(qbmp1);
   QePainter p3(&qbmp2);
   p3.drawPixmap(0, 0, qbmp1);
   p3.drawPixmap(1, 1, qbmp1);
   p3.end();

      // Set Masks
   qpix1.setMask(qbmp1);
   qpix2.setMask(qbmp2);

      // Now let's see if we need to create shadows.
   QPixmap qpix3=qpix1;
   if (shadow_width)
   {
      QPixmap qpix(qpix1.width(), qpix1.height());
      QColorGroup normal_group=palette().normal();
      qpix.fill(normal_group.background());
      QePainter p(&qpix);
      p.setPen(normal_group.dark());
      p.drawPixmap(shadow_width, shadow_width, qbmp1);
      p.drawPixmap(0, 0, qpix1);
      p.end();
      qpix1=qpix;
   }

      // Create QIconSet
   QIconSet * set=new QIconSet(qpix1, QIconSet::Small);
   set->setPixmap(qpix2, QIconSet::Small, QIconSet::Disabled);
   set->setPixmap(qpix3, QIconSet::Small, QIconSet::Active);
   return set;
}

void
QDToolButton::setOnPixmap(ByteStream & str)
{
   if (!(set_on=createSet(str, shadow ? 1 : 0)))
     G_THROW("Failed to assign an 'ON' pixmap to QDToolButton");
}

void
QDToolButton::slotToggled(bool on)
{
   if (isToggleButton())
      if (set_off && set_on)
      {
	 setIconSet(on ? *set_on : *set_off);
	 repaint();
      }
}

void
QDToolButton::slotReleased(void)
{
   if (!isToggleButton())
      if (set_off)
      {
	 setIconSet(*set_off);
	 repaint();
      }
}

void
QDToolButton::slotPressed(void)
{
   if (!isToggleButton())
      if (set_armed)
      {
	 setIconSet(*set_armed);
	 repaint();
      }
}
