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
// $Id: qd_viewer_prefs.cpp,v 1.13 2007/03/25 20:48:23 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "djvu_file_cache.h"
#include "qd_viewer_prefs.h"
#include "qlib.h"
#include "debug.h"

#include <qlayout.h>
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#ifndef QT1
#include <qwhatsthis.h>
#endif
#include <stdio.h>

//***************************************************************************
//******************************* QDHlinkPrefs ******************************
//***************************************************************************

bool
QDHlinkPrefs::disablePopup(void) const
{
   return popup_butt->isChecked();
}

bool
QDHlinkPrefs::simpleBorder(void) const
{
   return border_butt->isChecked();
}

DjVuPrefs::HLButtType
QDHlinkPrefs::hlinkKey(void) const
{
   return (DjVuPrefs::HLButtType) key_menu->currentItem();
}

void
QDHlinkPrefs::slotHotKeyChanged(int cur_item)
{
   emit sigHotKeyChanged((DjVuPrefs::HLButtType) cur_item);
}

void
QDHlinkPrefs::slotMagHotKeyChanged(DjVuPrefs::MagButtType key)
{
      // Make sure that we don't have this key selected

      // Translate it to HLButtType
   if (key!=DjVuPrefs::MAG_MID)
   {
      int our_key=DjVuPrefs::HLB_SHIFT;
      if (key==DjVuPrefs::MAG_CTRL)
	 our_key=DjVuPrefs::HLB_CTRL;
      else if (key==DjVuPrefs::MAG_SHIFT)
	 our_key=DjVuPrefs::HLB_SHIFT;
      else if (key==DjVuPrefs::MAG_ALT)
	 our_key=DjVuPrefs::HLB_ALT;

	 // See if we have it currently selected
      if (key_menu->currentItem()==our_key)
	 key_menu->setCurrentItem((our_key+1) % DjVuPrefs::HLB_ITEMS);
   }
}

QDHlinkPrefs::QDHlinkPrefs(DjVuPrefs * prefs, QWidget * parent, const char * name) :
      QWidget(parent, name)
{
   DEBUG_MSG("QDHlinkPrefs::QDHlinkOptimPrefs(): ...\n");
   DEBUG_MAKE_INDENT(3);

   QVBoxLayout * vlay=new QVBoxLayout(this, 10, 5, "hlink_vlay");
   vlay->addSpacing(fontMetrics().height());
   vlay->addStretch(1);
   popup_butt=new QCheckBox(tr("Disable hyperlink popup messages."),
			     this, "popup_butt");
   popup_butt->setChecked(!prefs->hlinksPopup);
   vlay->addWidget(popup_butt);
   
   border_butt=new QCheckBox(tr("Draw hyperlinks using simple border."), 
			      this, "border_butt");
   border_butt->setChecked(prefs->hlinksBorder);
   vlay->addWidget(border_butt);
   vlay->addStretch(1);
   QHBoxLayout * hlay=new QHBoxLayout(vlay, 5);
   QLabel * key_label=new QLabel(tr("\"Show all hyperlinks\" key:"), this);
   hlay->addWidget(key_label);
   key_menu=new QComboBox(this, "key_menu");
   for(int i=0;i<DjVuPrefs::HLB_ITEMS;i++)
      key_menu->insertItem(DjVuPrefs::hlb_names[i], i);
   key_menu->setCurrentItem(prefs->hlb_num);
   hlay->addWidget(key_menu);
   vlay->addStretch(2);

   connect(key_menu, SIGNAL(activated(int)),
	   this, SLOT(slotHotKeyChanged(int)));

#ifndef QT1
   QWhatsThis::add(this,
                   tr("<b>Hyperlinks:</b><ul>"
                      "<li>Hyperlinks are hilited when the mouse hovers"
                      " above the hyperlink region or when the"
                      " the key combination specified"
                      " in the <i>show all hyperlinks</i> box"
                      " is depressed.</li>"
                      "<li>The <i>disable hyperlink popups</i> option"
                      " hides the popup messages optionally associated with"
                      " the hyperlinks.</li>"
                      "<li>The <i>draw using simple border</i> option"
                      " selects simpler hiliting effects.</li>"
                      "</ul>"));
#endif

}

//***************************************************************************
//******************************* QDViewerPrefs *****************************
//***************************************************************************

void
QDViewerPrefs::done(int rc)
{
   DEBUG_MSG("QDViewerPrefs::done() called\n");
   DEBUG_MAKE_INDENT(3);
   
   if (rc==Accepted)
   {
      DEBUG_MSG("OK pressed. Updating preferences structure\n");

      DjVuPrefs disk_prefs;
      disk_prefs.dScreenGamma=prefs->dScreenGamma
        =gamma_prefs->displGamma();
      disk_prefs.dPrinterGamma=prefs->dPrinterGamma
        = ((gamma_prefs->match()) ? 0 : gamma_prefs->printGamma());
      disk_prefs.fastZoom=prefs->fastZoom
        =optim_prefs->fastZoom();
      disk_prefs.fastThumb=prefs->fastThumb
        =optim_prefs->fastThumb();
      disk_prefs.optimizeLCD=prefs->optimizeLCD
        =optim_prefs->optimizeLCD();
      disk_prefs.hlinksPopup=prefs->hlinksPopup
        =!hlink_prefs->disablePopup();
      disk_prefs.hlinksBorder=prefs->hlinksBorder
        =hlink_prefs->simpleBorder();
      disk_prefs.hlb_num=prefs->hlb_num
        =hlink_prefs->hlinkKey();
      disk_prefs.magnifierSize=prefs->magnifierSize
        =lens_prefs->size();
      disk_prefs.magnifierScale=prefs->magnifierScale
        =lens_prefs->scale();
      disk_prefs.magnifierHotKey=prefs->magnifierHotKey
        =lens_prefs->hotKey();
      disk_prefs.mcacheSize=prefs->mcacheSize
        =cache_prefs->mcacheSize();
      disk_prefs.pcacheSize=prefs->pcacheSize
        =cache_prefs->pcacheSize();
      disk_prefs.toolBarOn=prefs->toolBarOn
        =tbar_prefs->enabled();
      disk_prefs.toolBarAlwaysVisible=prefs->toolBarAlwaysVisible
        =tbar_prefs->visible();
      disk_prefs.toolBarDelay=prefs->toolBarDelay
        =tbar_prefs->delay();

      if (global_butt->isChecked()) disk_prefs.save();
   } else
   {
      DEBUG_MSG("Cancel pressed.\n");
   }
   QeDialog::done(rc);
}

void
QDViewerPrefs::slotGlobalToggled(bool state)
{
   bool en=get_file_cache()->is_enabled() && state && !cache_disabled;
   cache_prefs->enablePCache(en);
}

void
QDViewerPrefs::slotHelp(void)
{
#ifndef QT1
  QWhatsThis::enterWhatsThisMode();
#endif
}

void
QDViewerPrefs::disableCache(void)
{
   cache_disabled=true;
   cache_prefs->enablePCache(false);
   cache_prefs->resetPCache();
}

QDViewerPrefs::QDViewerPrefs(DjVuPrefs * _prefs, QWidget * parent,
			     const char * name, bool modal) :
      QeDialog(parent, name, modal), cache_disabled(false), prefs(_prefs)
{
   DEBUG_MSG("QDViewerPrefs::QDViewerPrefs(): Creating Viewer Prefs dialog...\n");
   DEBUG_MAKE_INDENT(3);

   setCaption(tr("DjVu: Viewer Preferences"));
   QWidget *start = startWidget();
   QVBoxLayout * top_vlay=new QVBoxLayout(start, 10, 5, "top_vlay");
   QTabWidget *tabwidget = new QTabWidget(start, "tabwidget");
   top_vlay->addWidget(tabwidget);
   
      // *** Tabs
   gamma_prefs = new QDGammaPrefs(_prefs, tabwidget, "gamma_prefs");
   tabwidget->addTab(gamma_prefs, tr("Gamma"));
   tbar_prefs=new QDTbarPrefs(_prefs, tabwidget, "tbar_prefs");
   tabwidget->addTab(tbar_prefs, tr("Toolbar"));
   hlink_prefs=new QDHlinkPrefs(_prefs, tabwidget, "hlink_prefs");
   tabwidget->addTab(hlink_prefs, tr("Links"));
   lens_prefs=new QDLensPrefs(_prefs, tabwidget, "lens_prefs");
   tabwidget->addTab(lens_prefs, tr("Lens"));
   cache_prefs=new QDCachePrefs(_prefs, true, tabwidget, "cache_prefs");
   tabwidget->addTab(cache_prefs, tr("Cache"));
   optim_prefs=new QDOptimPrefs(_prefs, tabwidget, "optim_prefs");
   tabwidget->addTab(optim_prefs, tr("Optimization"));

      // ** Bottom Row
   QHBoxLayout * butt_lay=new QHBoxLayout(top_vlay, 2);
   global_butt=new QCheckBox(tr("Save preferences."), start, "global_butt");
   global_butt->setChecked(TRUE);
   butt_lay->addWidget(global_butt);
   butt_lay->addStretch(1);
#ifndef QT1
   QPushButton * help_butt=new QPushButton(tr("Help"), start, "ok_butt");
   butt_lay->addWidget(help_butt);
#endif
   QPushButton * ok_butt=new QPushButton(tr("&OK"), start, "ok_butt");
   ok_butt->setDefault(TRUE);
   butt_lay->addWidget(ok_butt);
   QPushButton * cancel_butt=new QPushButton(tr("&Cancel"), start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);

      // Connecting signals and slots
#ifndef QT1
   connect(help_butt, SIGNAL(clicked(void)), this, SLOT(slotHelp(void)));
#endif
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
   connect(global_butt, SIGNAL(toggled(bool)), this, SLOT(slotGlobalToggled(bool)));
   connect(hlink_prefs, SIGNAL(sigHotKeyChanged(DjVuPrefs::HLButtType)),
	   lens_prefs, SLOT(slotHlHotKeyChanged(DjVuPrefs::HLButtType)));
   connect(lens_prefs, SIGNAL(sigHotKeyChanged(DjVuPrefs::MagButtType)),
	   hlink_prefs, SLOT(slotMagHotKeyChanged(DjVuPrefs::MagButtType)));
}
