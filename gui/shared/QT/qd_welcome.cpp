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
// $Id: qd_welcome.cpp,v 1.13 2007/03/25 20:48:27 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qd_welcome.h"
#include "qlib.h"
#include "debug.h"

#include <qmessagebox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qpixmap.h>



void	
QDWelcome::done(int rc)
{
  emit closed();
  QDialog::done(rc);
}


QDWelcome::QDWelcome(QWidget * parent, const char * name, bool modal) :
      QeDialog(parent, name, modal)
{
   DEBUG_MSG("QDWelcome::QDWelcome(): Creating Welcome dialog...\n");
   DEBUG_MAKE_INDENT(3);

   setCaption(tr("Welcome to the DjVu Plug-in"));
   const int minh = 31;

   QLabel * label;
   QWidget * start=startWidget();
   QGridLayout * glay=new QGridLayout(start, 5, 2, 10, 10, "glay");

   QPushButton * prefs_butt=new QPushButton(tr("&Preferences"), start, "prefs_butt");
   glay->addWidget(prefs_butt, 0, 0);
   prefs_butt->setMinimumHeight(minh);
   label=new QLabel(tr("Adjust gamma correction, caches, etc."), start, "prefs_label");
   glay->addWidget(label, 0, 1);

   QPushButton * help_butt=new QPushButton(tr("&Help"), start, "help_butt");
   glay->addWidget(help_butt, 1, 0);
   help_butt->setMinimumHeight(minh);
   label=new QLabel(tr("Learn how to use DjVu"), start, "help_label");
   glay->addWidget(label, 1, 1);

   QPushButton * about_butt=new QPushButton(tr("&About"), start, "about_butt");
   glay->addWidget(about_butt, 2, 0);
   about_butt->setMinimumHeight(minh);
   label=new QLabel(tr("Credit and links"), start, "about_label");
   glay->addWidget(label, 2, 1);

   QWidget *message = new QWidget(start);
   glay->addMultiCellWidget(message, 3, 3, 0, 1);
   QHBoxLayout * hlay=new QHBoxLayout(message, 0, 10);
   label=new QLabel(message, "icon");
#if defined(QT1) || defined(QT2)
   QPixmap iconpm = QMessageBox::standardIcon(QMessageBox::Information,
					      QApplication::style() );
#else
   QPixmap iconpm = QMessageBox::standardIcon(QMessageBox::Information);
#endif
   label->setPixmap(iconpm);
   hlay->addWidget(label,0);
   label=new QLabel(tr("These functions and many others can be\n"
			"accessed from within a DjVu document\n"
			"by clicking the right mouse button."), message, "info");
   label->setAlignment(AlignCenter);
   hlay->addWidget(label,1);

   close_butt=new QPushButton(tr("&Close"), start, "close_butt");
   glay->addWidget(close_butt, 4, 0);
   close_butt->setMinimumHeight(minh);
   never_butt=new QCheckBox(tr("&Never show this window again"), start, "never_butt");
   never_butt->setChecked(FALSE);
   glay->addWidget(never_butt, 4, 1);
   close_butt->setDefault(TRUE);

      // Connecting signals and slots
   connect(close_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(close_butt, SIGNAL(clicked(void)), this, SIGNAL(closed(void)));
   connect(prefs_butt, SIGNAL(clicked(void)), this, SIGNAL(preferences(void)));
   connect(help_butt, SIGNAL(clicked(void)), this, SIGNAL(help(void)));
   connect(about_butt, SIGNAL(clicked(void)), this, SIGNAL(about(void)));
}
