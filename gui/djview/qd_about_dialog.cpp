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
// $Id: qd_about_dialog.cpp,v 1.10 2007/03/25 20:48:23 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
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


QDAboutDialog::QDAboutDialog(QWidget *parent, const char *name, bool modal)
  : QeDialog(parent, name, modal)
{
      setCaption(tr("About DjView"));
      QWidget * start=startWidget();
      QLabel * label;
      QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);
      QFrame * frame=new QFrame(start);
      frame->setFrameStyle(QFrame::Box | QFrame::Sunken);
      vlay->addWidget(frame);
      QVBoxLayout * frame_vlay=new QVBoxLayout(frame, 20, 10);
      label=new QLabel(tr("DjVu standalone viewer\nand browser plugin."), frame);
      QFont font=label->font();
      font.setBold(TRUE);
      font.setPointSize(font.pointSize()+3);
      label->setFont(font);
      label->setAlignment(AlignCenter);
      frame_vlay->addWidget(label);
      QString ver=tr("Version DjVuLibre-")+DJVIEW_VERSION_STR;
      label=new QLabel(ver, frame);
      label->setAlignment(AlignCenter);
      frame_vlay->addWidget(label);
      QHBoxLayout * hlay=new QHBoxLayout(vlay, 10);
      hlay->addStretch(1);
      QPushButton * butt=new QPushButton(tr("&Close"), start);
      butt->setDefault(TRUE);
      hlay->addWidget(butt);
      hlay->addStretch(1);
      connect(butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
}

QDAboutDialog::~QDAboutDialog()
{
}
