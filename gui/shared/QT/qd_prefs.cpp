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

#include "djvu_file_cache.h"
#include "qd_prefs.h"
#include "qlib.h"
#include "debug.h"
#include "GRect.h"
#include "GPixmap.h"
#include "qt_painter.h"
#include "qx_imager.h"

#include <qcheckbox.h>
#include <qslider.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <qpaintdevicemetrics.h>
#include <qspinbox.h>
#ifndef QT1
#include <qwhatsthis.h>
#endif

//***************************************************************************
//***************************** QDGammaDispl ********************************
//***************************************************************************

void 
QDGammaDispl::resize( int w, int h )
{
  setGeometry( geometry().x(), geometry().y(), w, h);
}

void 
QDGammaDispl::setGeometry( int x, int y, int w, int h )
{
  int nw, nh;
  nw = nh = QMIN(w,h);
  QFrame::setGeometry(x+(w-nw)/2, y+(h-nh)/2, nw, nh);
}

void
QDGammaDispl::paintEvent(QPaintEvent *)
{
   DEBUG_MSG("QDGammaDispl::paintEvent(): painting...\n");
   DEBUG_MAKE_INDENT(3);

   try
   {
      GRect grect(0, 0, width(), height());
      grect.inflate(-5, -5);
      int size=grect.width()<grect.height() ? grect.width() : grect.height();
      if (size & 1) size--;
      int x=grect.width()-size;
      int y=grect.height()-size;
      grect.xmin+=x/2;
      grect.xmax+=x/2-x;
      grect.ymin+=y/2;
      grect.ymax+=y/2-y;

      GPixel gray={ 128, 128, 128 };
      GP<GPixmap> pm=GPixmap::create(size, size, &gray);

      DEBUG_MSG("pixmap size=" << size << "x" << size << "\n");
      for(int j=0;j<size;j++)
      {
	 GPixel * p=(*pm)[j];
	 const GPixel & c=(j & 1 ? GPixel::BLACK : GPixel::WHITE);
	 if (j>size/2) p+=size/2;
	 for(int i=0;i<size/2;i++) p[i]=c;
      }

      DEBUG_MSG("correcting color for gamma=" << gamma << "\n");
      pm->color_correct(gamma);

      DEBUG_MSG("seeing if dithering is necessary\n");
      if (qxImager)
        qxImager->dither(*pm);
      QePainter p(this);
      p.drawPixmap(grect, pm);
      p.end();
   } catch(const GException & exc)
   {
      showError(this, tr("DjVu Error"), exc);
   }
}

void
QDGammaDispl::setGamma(int _gamma)
{
   gamma=_gamma/10.0;
   repaint(FALSE);
}

QDGammaDispl::QDGammaDispl(QWidget * parent, const char * name) :
      QFrame(parent, name), gamma(0.5)
{
#ifdef QT1
  if (QApplication::style()==WindowsStyle)
    setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
  else
    setFrameStyle(QFrame::Panel | QFrame::Sunken);
#else
  setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
#endif
  setMargin(5);
  setBackgroundColor(white);
  setWFlags(WRepaintNoErase);
}

//***************************************************************************
//***************************** QDGammaPrefs ********************************
//***************************************************************************

double
QDGammaPrefs::displGamma(void) const
{
   return displ_slider->value()/10.0;
}

double
QDGammaPrefs::printGamma(void) const
{
   return print_slider->value()/10.0;
}

bool
QDGammaPrefs::match(void) const
{
   return match_butt->isChecked();
}

void
QDGammaPrefs::matchToggled(bool state)
{
  print_slider->setEnabled(!state);
}

QDGammaPrefs::QDGammaPrefs(DjVuPrefs * prefs, QWidget * parent, const char * name) 
  : QWidget(parent, name)
{
   DEBUG_MSG("QDGammaPrefs::QDGammaPrefs(): Creating 'Gamma correction' box...\n");
   DEBUG_MAKE_INDENT(3);

   QLabel * label;
   QVBoxLayout * topvlay=new QVBoxLayout(this, 10, 5, "hlink_vlay");
   topvlay->addSpacing(fontMetrics().height());
   
   QGridLayout * glay=new QGridLayout(topvlay, 3, 3);
   topvlay->setStretchFactor(glay, 3);

   glay->addRowSpacing(1, 20);
   glay->setRowStretch(0, 1);
   glay->setRowStretch(1, 1);
   glay->setRowStretch(2, 1);
   glay->addColSpacing(1, 20);
   glay->setColStretch(0, 1);
   glay->setColStretch(2, 1);

   QVBoxLayout * vlay=new QVBoxLayout(5);
   glay->addLayout(vlay, 0, 0);
   vlay->addStretch(1);
   label=new QLabel(tr("Screen color correction.\n") +
                     tr("Adjust slider until\n"
			"gray shades look similar."), this );
   label->setAlignment(AlignCenter);
   vlay->addWidget(label);
   vlay->addSpacing(5);
   displ_slider=new QSlider(5, 50, 1, (int) (prefs->dScreenGamma*10),
			     QSlider::Horizontal, this, "displ_slider");
   displ_slider->setTickmarks(QSlider::Below);
   displ_slider->setTickInterval(5);
   vlay->addWidget(displ_slider);

   QHBoxLayout * hlay=new QHBoxLayout(vlay, 1);
   hlay->addWidget(new QLabel(tr("Darker"), this));
   hlay->addStretch(1);
   hlay->addWidget(new QLabel(tr("Lighter"), this));
   vlay->addStretch(1);
   
      // Preview box
   preview=new QDGammaDispl(this, "preview");
   preview->setGamma(displ_slider->value());
   glay->addWidget(preview, 0, 2);

      // Separator
   QFrame * frame=new QFrame(this, "separator");
   frame->setFrameStyle(QFrame::Sunken | QFrame::HLine);
   glay->addMultiCellWidget(frame, 1, 1, 0, 2);

      // Printer 
   vlay=new QVBoxLayout(5);
   glay->addLayout(vlay, 2, 0);
   vlay->addStretch(1);
   label=new QLabel(tr("Printer color correction."), this);
   label->setAlignment(AlignCenter);
   vlay->addWidget(label);
   print_slider=new QSlider(5, 50, 1, prefs->dPrinterGamma==0 ?
			     (int) (prefs->dScreenGamma*10) :
			     (int) (prefs->dPrinterGamma*10),
			     QSlider::Horizontal, this, "print_slider");
   print_slider->setTickmarks(QSlider::Below);
   print_slider->setTickInterval(5);
   vlay->addWidget(print_slider);

   hlay=new QHBoxLayout(vlay,5);
   hlay->addWidget(new QLabel(tr("Darker"), this));
   hlay->addStretch(1);
   hlay->addWidget(new QLabel(tr("Lighter"), this));
   vlay->addStretch(1);

      // Check Box
   match_butt=new QCheckBox(tr("Automatic color matching."), this, "match_butt");
   match_butt->setChecked(false);
   if (prefs->dPrinterGamma==0)
     {
       match_butt->setChecked(true);
       print_slider->setValue(22);
       print_slider->setEnabled(false);
     }   
   glay->addWidget(match_butt, 2, 2, Qt::AlignCenter);

      // Connecting signals and slots
   connect(displ_slider, SIGNAL(valueChanged(int)), preview, SLOT(setGamma(int)));
   connect(match_butt, SIGNAL(toggled(bool)), this, SLOT(matchToggled(bool)));
    
      // Help
#ifndef QT1
   QWhatsThis::add(this,
                   tr("<b>Screen gamma correction:</b>"
                      "<br>The best color rendition is achieved"
                      " by adjusting the gamma correction slider"
                      " and choosing the position that makes the"
                      " gray square as uniform as possible."
                      "<p><b>Printer color correction:</b>"
                      "<br>The <i>automatic color matching</i> option"
                      " works best with PostScript printers"
                      " and ICC profiled printers."
                      " The slider might be useful in other cases."));
#endif
}

//***************************************************************************
//******************************* QDLensPrefs *******************************
//***************************************************************************

static const QString ctrl_hkey=QT_TRANSLATE_NOOP("QDLensPrefs","Control key");
static const QString shift_hkey=QT_TRANSLATE_NOOP("QDLensPrefs","Shift key");
static const QString alt_hkey=QT_TRANSLATE_NOOP("QDLensPrefs","Alt key");
static const QString mid_hkey=QT_TRANSLATE_NOOP("QDLensPrefs","Middle mouse button");

int
QDLensPrefs::size(void) const
{
   return DjVuPrefs::legal_mag_size[size_menu->currentItem() %
				    DjVuPrefs::legal_mag_size_num];
}

int
QDLensPrefs::scale(void) const
{
   return DjVuPrefs::legal_mag_scale[scale_menu->currentItem() %
				     DjVuPrefs::legal_mag_scale_num];
}

DjVuPrefs::MagButtType
QDLensPrefs::hotKey(void) const
{
   DjVuPrefs::MagButtType key=DjVuPrefs::MAG_CTRL;

   QString text=hkey_menu->currentText();
   if (text==ctrl_hkey)
      key=DjVuPrefs::MAG_CTRL;
   else if (text==shift_hkey)
      key=DjVuPrefs::MAG_SHIFT;
   else if (text==alt_hkey)
      key=DjVuPrefs::MAG_ALT;
   else if (text==mid_hkey)
      key=DjVuPrefs::MAG_MID;
   
   return key;
}

void
QDLensPrefs::slotHotKeyChanged(const QString & )
{
   emit sigHotKeyChanged(hotKey());
}

void
QDLensPrefs::slotHlHotKeyChanged(DjVuPrefs::HLButtType key)
{
      // Make sure that we don't have the same button selected
   QString cur_text=hkey_menu->currentText();
   if ((key==DjVuPrefs::HLB_CTRL && cur_text==ctrl_hkey) ||
       (key==DjVuPrefs::HLB_SHIFT && cur_text==shift_hkey) ||
       (key==DjVuPrefs::HLB_ALT && cur_text==alt_hkey))
     setComboBoxCurrentItem(hkey_menu, mid_hkey);
}

QDLensPrefs::QDLensPrefs(DjVuPrefs * prefs, QWidget * parent, const char * name)
  : QWidget(parent, name)
{
   DEBUG_MSG("QDLensPrefs::QDLensPrefs(): Creating 'Magnifying Glass' box...\n");
   DEBUG_MAKE_INDENT(3);

   QGridLayout * glay=new QGridLayout(this, 2, 5, 10, 5, "lens_glay");
   glay->addRowSpacing(0,fontMetrics().height());
   glay->setRowStretch(0, 1);
   glay->setRowStretch(4, 2);
   glay->setColStretch(0, 1);
   glay->setColStretch(1, 1);

   QLabel * size_label=new QLabel(tr("Size: "), this);
   glay->addWidget(size_label,1,0);
   size_menu=new QComboBox(FALSE, this, "mag_size_menu");
   for(int i=0;i<DjVuPrefs::legal_mag_size_num;i++)
   {
      int size=DjVuPrefs::legal_mag_size[i];
      {
        QString mesg=QString::number(size)+tr(" pixels");
        size_menu->insertItem(mesg);
      }
      if (size==prefs->magnifierSize)
	 size_menu->setCurrentItem(i);
   }
   glay->addWidget(size_menu,1,1);

   QLabel * mag_label=new QLabel(tr("Magnification: "), this);
   glay->addWidget(mag_label,2,0);
   scale_menu=new QComboBox(FALSE, this, "mag_scale_menu");
   for(int i=0;i<DjVuPrefs::legal_mag_scale_num;i++)
   {
      int scale=DjVuPrefs::legal_mag_scale[i];
      char buffer[64];
      sprintf(buffer, "%1.1fx", scale/10.0);
      scale_menu->insertItem(buffer);
      if (scale==prefs->magnifierScale)
	 scale_menu->setCurrentItem(i);
   }
   glay->addWidget(scale_menu,2,1);

   QLabel * label=new QLabel(tr("To activate press: "), this);
   glay->addWidget(label,3,0);
   hkey_menu=new QComboBox(FALSE, this, "hkey_menu");
   hkey_menu->insertItem(tr(ctrl_hkey));
   hkey_menu->insertItem(tr(shift_hkey));
   hkey_menu->insertItem(tr(alt_hkey));
   hkey_menu->insertItem(tr(mid_hkey));
   QString cur_hkey=ctrl_hkey;
   int item = 0;
   if (prefs->magnifierHotKey==DjVuPrefs::MAG_CTRL)
     { cur_hkey=ctrl_hkey; item=0; }
   else if (prefs->magnifierHotKey==DjVuPrefs::MAG_SHIFT)
      cur_hkey=shift_hkey;
   else if (prefs->magnifierHotKey==DjVuPrefs::MAG_ALT)
      cur_hkey=alt_hkey;
   else if (prefs->magnifierHotKey==DjVuPrefs::MAG_MID)
      cur_hkey=mid_hkey;
   setComboBoxCurrentItem(hkey_menu, cur_hkey);
   glay->addWidget(hkey_menu,3,1);
   connect(hkey_menu, SIGNAL(activated(const QString &)),
	   this, SLOT(slotHotKeyChanged(const QString &)));

      // Help
#ifndef QT1
   QWhatsThis::add(this,
                   tr("<b>Magnification lens:</b>"
                      "<br>The <i>magnification lens</i>"
                      " appears under the mouse pointer when you press"
                      " the selected key."));
#endif
}

//***************************************************************************
//******************************* QDOptimPrefs ******************************
//***************************************************************************

bool
QDOptimPrefs::fastZoom(void) const
{
   return fastzoom_butt->isChecked();
}

bool
QDOptimPrefs::fastThumb(void) const
{
   return fastthumb_butt->isChecked();
}

bool
QDOptimPrefs::optimizeLCD(void) const
{
   return lcd_butt->isChecked();
}

QDOptimPrefs::QDOptimPrefs(DjVuPrefs * prefs, QWidget * parent, const char * name) 
  : QWidget(parent, name)
{
   DEBUG_MSG("QDOptimPrefs::QDOptimPrefs(): Creating 'Optimization' box...\n");
   DEBUG_MAKE_INDENT(3);

   QVBoxLayout * vlay=new QVBoxLayout(this, 10, 5, "optim_vlay");
   vlay->addSpacing(fontMetrics().height());
   vlay->addStretch(1);

   fastzoom_butt=new QCheckBox(tr("Favor fast magnifications "
                                  "for \"Fit\" resolutions."),
				this, "fastzoom_butt");
   fastzoom_butt->setChecked(prefs->fastZoom);
   vlay->addWidget(fastzoom_butt);

   fastthumb_butt=new QCheckBox(tr("Enable fast thumbnails."),
				 this, "fastthumb_butt");
   fastthumb_butt->setChecked(prefs->fastThumb);
   vlay->addWidget(fastthumb_butt);

   lcd_butt=new QCheckBox(tr("Optimize images for LCD displays."),
			   this, "lcd_butt");
   lcd_butt->setChecked(prefs->optimizeLCD);
   vlay->addWidget(lcd_butt);

   vlay->addStretch(2);

      // Help
#ifndef QT1
   QWhatsThis::add(this,
                   tr("<b>Optimizations:</b>"
                      "<ul>"
                      "<li>The <i>fast magnification</i> option"
                      " selects integer magnifications that require"
                      " less computing resources.</li>"
                      "<li>The <i>fast thumbnails</i> option"
                      " never compute thumbnails unless the page data"
                      " is already cached. This cuts network traffic that"
                      " might be otherwise unnecessary.</li>"
                      "<li>The <i>LCD display</i> option might improve"
                      " the image quality on certain LCD displays.</li>"
                      "</ul>"));
#endif


}

//***************************************************************************
//******************************* QDCachePrefs ******************************
//***************************************************************************

int
QDCachePrefs::mcacheSize(void) const
{
   int mcache_size;
   sscanf(mcache_menu->currentText(), "%d", &mcache_size);
   return mcache_size;
}

int
QDCachePrefs::pcacheSize(void) const
{
   int pcache_size=0;
   if (pcache_menu)
      sscanf(pcache_menu->currentText(), "%d", &pcache_size);
   return pcache_size;
}

void
QDCachePrefs::slotClearCache(void)
{
   get_file_cache()->clear();
}

void
QDCachePrefs::enablePCache(bool en)
{
   if (pcache_label) pcache_label->setEnabled(en);
   if (pcache_menu) pcache_menu->setEnabled(en);
   if (pcache_butt) pcache_butt->setEnabled(en);
}

void
QDCachePrefs::resetPCache(void)
{
   if (pcache_menu)
      pcache_menu->setCurrentItem(0);
}

QDCachePrefs::QDCachePrefs(DjVuPrefs * prefs, bool pcache_on,
			   QWidget * parent, const char * name) 
  : QWidget(parent, name)
{
   DEBUG_MSG("QDCachePrefs::QDCachePrefs(): Creating 'Cache Preferences' box...\n");
   DEBUG_MAKE_INDENT(3);

   QGridLayout * glay=new QGridLayout(this, 3, 3 + pcache_on, 10, 5, "cache_glay");
   glay->addRowSpacing(0, fontMetrics().height());
   glay->setRowStretch(0, 1);
   glay->setRowStretch(2+pcache_on, 2);
   glay->setColStretch(0, 1);
   glay->setColStretch(1, 1);
   glay->setColStretch(2, 1);

   QLabel * mcache_label=new QLabel(tr("Off screen cache size:"), this);
   glay->addWidget(mcache_label, 1, 0);
   mcache_menu=new QComboBox(this, "mcache_menu");
   int mcache_item=-1;
   mcache_menu->insertItem("0 Mb"); mcache_item+=(prefs->mcacheSize>=0);
   mcache_menu->insertItem("1 Mb"); mcache_item+=(prefs->mcacheSize>=1);
   mcache_menu->insertItem("2 Mb"); mcache_item+=(prefs->mcacheSize>=2);
   mcache_menu->insertItem("4 Mb"); mcache_item+=(prefs->mcacheSize>=4);
   mcache_menu->insertItem("6 Mb"); mcache_item+=(prefs->mcacheSize>=6);
   mcache_menu->setCurrentItem(mcache_item);
   glay->addWidget(mcache_menu, 1, 1);

   if (pcache_on)
   {
      pcache_label=new QLabel(tr("Decoded pages cache:"), this);
      pcache_label->setEnabled(get_file_cache()->is_enabled());
      glay->addWidget(pcache_label, 2, 0);
      pcache_menu=new QComboBox(this, "pcache_menu");
      pcache_menu->setEnabled(get_file_cache()->is_enabled());
      int pcache_item=-1;
      pcache_menu->insertItem("0 Mb"); pcache_item+=(prefs->pcacheSize>=0);
      pcache_menu->insertItem("2 Mb"); pcache_item+=(prefs->pcacheSize>=2);
      pcache_menu->insertItem("4 Mb"); pcache_item+=(prefs->pcacheSize>=4);
      pcache_menu->insertItem("6 Mb"); pcache_item+=(prefs->pcacheSize>=6);
      pcache_menu->insertItem("10 Mb"); pcache_item+=(prefs->pcacheSize>=10);
      pcache_menu->insertItem("15 Mb"); pcache_item+=(prefs->pcacheSize>=15);
      pcache_menu->insertItem("20 Mb"); pcache_item+=(prefs->pcacheSize>=20);
      pcache_menu->insertItem("30 Mb"); pcache_item+=(prefs->pcacheSize>=30);
      pcache_menu->setCurrentItem(pcache_item);
      glay->addWidget(pcache_menu, 2, 1);

      pcache_butt=new QPushButton(tr("Clear"), this, "pcache_butt");
      pcache_butt->setEnabled(get_file_cache()->is_enabled());
      glay->addWidget(pcache_butt, 2, 2);
   } 
   else
   {
      pcache_label=0;
      pcache_menu=0;
      pcache_butt=0;
   }
   if (pcache_butt)
     {
       connect(pcache_butt, SIGNAL(clicked(void)), this, SLOT(slotClearCache(void)));
#ifndef QT1
       QWhatsThis::add(this,
                   tr("<b>Caches:</b>"
                      "<br>There are several caches in <i>djview</i>."
                      "<ul>"
                      "<li>The <i>off screen cache</i> stores image data"
                      " located outside the visible area."
                      " This cache makes panning smoother.</li>"
                      "<li>The <i>decoded page cache</i> contains partially"
                      " decoded pages. It provides faster response times"
                      " when navigating a multipage document or when returning"
                      " to a previously viewed page. Clearing this cache"
                      " might be useful to reflect a change in the page"
                      " data without restarting the DjVu viewer.</li>"
                      "</ul>"));
#endif
     }
}

//***************************************************************************
//******************************** QDTbarPrefs ******************************
//***************************************************************************

bool
QDTbarPrefs::enabled(void) const
{
   return on_butt->isChecked();
}

bool
QDTbarPrefs::visible(void) const
{
   return vis_butt->isChecked();
}

int
QDTbarPrefs::delay(void) const
{
   return delay_spin->value();
}

void
QDTbarPrefs::slotTBToggled(bool)
{
   vis_butt->setEnabled(on_butt->isChecked());
   delay_label->setEnabled(on_butt->isChecked() &&
			   !vis_butt->isChecked());
   delay_spin->setEnabled(delay_label->isEnabled());
}

QDTbarPrefs::QDTbarPrefs(DjVuPrefs * prefs, QWidget * parent, const char * name) 
  : QWidget(parent, name)
{
   DEBUG_MSG("QDTbarPrefs::QDTbarOptimPrefs(): "
             "Creating 'Toolbar Preferences' box...\n");
   DEBUG_MAKE_INDENT(3);

   QVBoxLayout * vlay=new QVBoxLayout(this, 10, 5, "tbar_vlay");
   vlay->addSpacing(fontMetrics().height());
   vlay->addStretch(1);
   on_butt=new QCheckBox(tr("Enable toolbar"), this, "on_butt");
   on_butt->setChecked(prefs->toolBarOn);
   vlay->addWidget(on_butt);
   vis_butt=new QCheckBox(tr("Toolbar is always visible"), this, "vis_butt");
   vis_butt->setChecked(prefs->toolBarAlwaysVisible);
   vis_butt->setEnabled(prefs->toolBarOn);
   vlay->addWidget(vis_butt);
   vlay->addStretch(1);
   QHBoxLayout * spin_lay=new QHBoxLayout(vlay,5);
   delay_label=new QLabel(tr("Toolbar hide delay:"), this, "delay_label");
   delay_label->setEnabled(prefs->toolBarOn && !prefs->toolBarAlwaysVisible);
   spin_lay->addWidget(delay_label);
   delay_spin=new QSpinBox(0, 5000, 250, this, "delay_spin");
   delay_spin->setSuffix(" msec");
   delay_spin->setValue(prefs->toolBarDelay);
   delay_spin->setEnabled(prefs->toolBarOn && !prefs->toolBarAlwaysVisible);
   spin_lay->addWidget(delay_spin);
   vlay->addStretch(2);

   connect(on_butt, SIGNAL(toggled(bool)), this, SLOT(slotTBToggled(bool)));
   connect(vis_butt, SIGNAL(toggled(bool)), this, SLOT(slotTBToggled(bool)));

#ifndef QT1
   QWhatsThis::add(this,
                   tr("<b>Toolbar:</b>"
                      "<br>Checking the <i>enable toolbar</i> box"
                      " adds a toolbar at the bottom of each DjVu"
                      " image. Checking the <i>always visible</i> box"
                      " makes the toolbar visible at all times."
                      " Otherwise the toolbar appears when the mouse"
                      " pointer comes close to the bottom of the image"
                      " and remains visible until the specified delay"
                      " expires."));
#endif
}
