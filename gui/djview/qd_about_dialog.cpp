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

#include "qd_about_dialog.h"
#include "version.h"

#include "prefs.h"
#include "debug.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qfont.h>
#include <qpushbutton.h>


#include "qt_fix.h"


QDAboutDialog::QDAboutDialog(QWidget *parent, const char *name, bool modal)
  : QeDialog(parent, name, modal)
{
      setCaption(tr("About DjView"));
      QWidget * start=startWidget();
      QeLabel * label;
      QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);
      QFrame * frame=new QFrame(start);
      frame->setFrameStyle(QFrame::Box | QFrame::Sunken);
      vlay->addWidget(frame);
      QVBoxLayout * frame_vlay=new QVBoxLayout(frame, 20, 10);
      label=new QeLabel(tr("DjVu standalone viewer\nand browser plugin."), frame);
      QFont font=label->font();
      font.setBold(TRUE);
      font.setPointSize(font.pointSize()+3);
      label->setFont(font);
      label->setAlignment(AlignCenter);
      frame_vlay->addWidget(label);
      QString ver=tr("Version DjVuLibre-")+DJVIEW_VERSION_STR;
      label=new QeLabel(ver, frame);
      label->setAlignment(AlignCenter);
      frame_vlay->addWidget(label);
      frame_vlay->activate();
      QHBoxLayout * hlay=new QHBoxLayout(10);
      vlay->addLayout(hlay);
      hlay->addStretch(1);
      QePushButton * butt=new QePushButton(tr("&Close"), start);
      butt->setDefault(TRUE);
      hlay->addWidget(butt);
      hlay->addStretch(1);
      vlay->activate();
      connect(butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
}

QDAboutDialog::~QDAboutDialog()
{
}
