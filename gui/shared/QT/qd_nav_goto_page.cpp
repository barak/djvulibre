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

#include <stdio.h>

#include "DjVuDocument.h"
#include "DjVuImage.h"

#include "qd_nav_goto_page.h"

#include <qlabel.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "qt_fix.h"

class QDPageNumVal : public QValidator
{
public:
   virtual void		fixup(QString &);
#ifdef QT1
   virtual State	validate(QString &, int &);
#else
   virtual State	validate(QString &, int &) const;
#endif

   QDPageNumVal(QComboBox * parent, const char * name=0) :
   QValidator(parent, name) {};
};

void
QDPageNumVal::fixup(QString & str)
{
#ifdef QT1
   str.detach();
#else
   str.truncate(0);
#endif

   QComboBox * menu=(QComboBox *) parent();
   menu->setEditText(str=menu->text(menu->currentItem()));
}

QDPageNumVal::State
QDPageNumVal::validate(QString & input, int & pos)
#ifndef QT1
const
#endif
{
   if (!input.length()) return Valid;
   
   bool status;
   int page=input.toInt(&status)-1;
   if (!status) return Invalid;

   if (page<0) return Invalid;

   QComboBox * menu=(QComboBox *) parent();
   if (page>=menu->count()) return Invalid;
   
   return Acceptable;
}

int
QDNavGotoPage::getPageNum(void) const
{
      // We can't use menu->currentItem() here because it won't work when
      // user presses "Enter" in the menu
   return atoi(menu->currentText())-1;
}

QDNavGotoPage::QDNavGotoPage(GP<DjVuDocument> &doc,
			     DjVuImage * dimg,
			     QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   setCaption(tr("DjVu: Goto Page"));
   QWidget * start=startWidget();

      // Create the menu
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);
   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   QLabel * label=new QLabel(tr("Goto page"), start, "goto_label");
   hlay->addWidget(label);
   menu=new QComboBox(TRUE, start, "goto_menu");
   menu->setInsertionPolicy(QComboBox::NoInsertion);
   menu->setValidator(new QDPageNumVal(menu));
   hlay->addWidget(menu);

      // Create the buttons
   hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   hlay->addStretch(1);
   QPushButton * ok_butt=new QPushButton(tr("&OK"), start, "ok_butt");
   ok_butt->setDefault(TRUE);
   hlay->addWidget(ok_butt);
   QPushButton * cancel_butt=new QPushButton(tr("&Cancel"), start, "cancel_butt");
   hlay->addWidget(cancel_butt);

      // Set menu contents
   int cur_page=doc->url_to_page(dimg->get_djvu_file()->get_url());
   int pages=doc->get_pages_num();
   for(int i=0;i<pages;i++)
   {
      char buffer[64];
      sprintf(buffer, "%d", i+1);
      menu->insertItem(buffer);
   };
   menu->setCurrentItem(cur_page);
   
      // Connect signals
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
}
