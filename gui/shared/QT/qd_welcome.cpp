//C-  -*- C++ -*-
//C- -----------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- DjVuLibre-3.5 is derived from the DjVu (r) Reference Library.
//C- 
//C- DjVu (r) Reference Library (v. 3.5)
//C- Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
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
//C- -----------------------------------------------------------
// 
// $Id$
// $Name$

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
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
