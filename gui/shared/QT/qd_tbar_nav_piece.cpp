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

#include "DjVuDocument.h"
#include "qd_tbar_nav_piece.h"
#include "debug.h"
#include "qlib.h"
#include "qd_toolbutt.h"
#include "djvu_base_res.h"
#include "cin_data.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <qtooltip.h>

#include "qt_fix.h"

//****************************************************************************
//****************************** QDValidator *********************************
//****************************************************************************

class QDValidator : public QValidator
{
public:
   virtual void		fixup(QString &);
#ifdef QT1
   virtual State	validate(QString &, int &);
#else
   virtual State	validate(QString &, int &) const;
#endif
   
   QDValidator(QComboBox * parent, const char * name=0) :
	 QValidator(parent, name) {};
};

void
QDValidator::fixup(QString & str)
{
#ifdef QT1
   str.detach();
#else
   str.truncate(0);
#endif
   QComboBox * menu=(QComboBox *) parent();
   menu->setEditText(str=menu->text(menu->currentItem()));
}

QValidator::State
QDValidator::validate(QString & input, int & pos)
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

//****************************************************************************
//****************************** QDTBarNavPiece ******************************
//****************************************************************************

void
QDTBarNavPiece::setEnabled(bool en)
{
   if (created)
   {
      page_menu->setEnabled(en);
      npage_butt->setEnabled(en);
      ppage_butt->setEnabled(en);
      nnpage_butt->setEnabled(en);
      pppage_butt->setEnabled(en);
      fpage_butt->setEnabled(en);
      lpage_butt->setEnabled(en);
   }
}

void
QDTBarNavPiece::create(void)
{
   separator=new QFrame(toolbar, "separator");
   separator->setFrameStyle(QFrame::VLine | QFrame::Sunken);
   separator->setMinimumWidth(10);
   if ( qdtoolbar_child ) 
      ((QDToolBar *)toolbar)->addLeftWidget(separator);
   
   label=new QeLabel(tr("Page"), toolbar, "page_label");
   QToolTip::add(label, tr("Page"));
   
   page_menu=new QeComboBox(TRUE, toolbar, "page_menu");
   page_menu->setInsertionPolicy(QComboBox::NoInsertion);
   page_menu->setValidator(new QDValidator(page_menu));
   connect(page_menu, SIGNAL(activated(const QString &)),
	   this, SLOT(slotPage(const QString &)));
   QToolTip::add(page_menu, tr("Page"));
   if ( qdtoolbar_child ) 
      ((QDToolBar *)toolbar)->addLeftWidgets(label, page_menu);

   fpage_butt=new QDToolButton(*CINData::get("ppm_vfpage"), true,
			       IDC_NAV_FIRST_PAGE, toolbar, tr("First page"));
   connect(fpage_butt, SIGNAL(clicked(void)), this, SLOT(slotPage(void)));
   
   pppage_butt=new QDToolButton(*CINData::get("ppm_vpppage"), true,
				IDC_NAV_PREV_PAGE10, toolbar, tr("-10 pages"));
   connect(pppage_butt, SIGNAL(clicked(void)), this, SLOT(slotPage(void)));
   
   ppage_butt=new QDToolButton(*CINData::get("ppm_vppage"), true,
			       IDC_NAV_PREV_PAGE, toolbar, tr("Previous Page"));
   connect(ppage_butt, SIGNAL(clicked(void)), this, SLOT(slotPage(void)));
   
   npage_butt=new QDToolButton(*CINData::get("ppm_vnpage"), true,
			       IDC_NAV_NEXT_PAGE, toolbar, tr("Next Page"));
   connect(npage_butt, SIGNAL(clicked(void)), this, SLOT(slotPage(void)));

   nnpage_butt=new QDToolButton(*CINData::get("ppm_vnnpage"), true,
				IDC_NAV_NEXT_PAGE10, toolbar, tr("+10 pages"));
   connect(nnpage_butt, SIGNAL(clicked(void)), this, SLOT(slotPage(void)));

   lpage_butt=new QDToolButton(*CINData::get("ppm_vlpage"), true,
			       IDC_NAV_LAST_PAGE, toolbar, tr("Last page"));
   connect(lpage_butt, SIGNAL(clicked(void)), this, SLOT(slotPage(void)));

   if ( qdtoolbar_child ) 
      ((QDToolBar *)toolbar)->addLeftWidgets(fpage_butt, pppage_butt, ppage_butt,
					     npage_butt, nnpage_butt, lpage_butt);
   
   created=true;
}

void
QDTBarNavPiece::destroy(void)
{
   if (created)
   {
      delete separator; separator=0;
      delete label; label=0;
      delete page_menu; page_menu=0;
      delete npage_butt; npage_butt=0;
      delete ppage_butt; ppage_butt=0;
      delete nnpage_butt; nnpage_butt=0;
      delete pppage_butt; pppage_butt=0;
      delete fpage_butt; fpage_butt=0;
      delete lpage_butt; lpage_butt=0;
      created=false;
   }
}

QDTBarNavPiece::QDTBarNavPiece(QWidget * toolbar) : QDTBarPiece(toolbar)
{
   if ( dynamic_cast<QDToolBar *>(toolbar) )
      qdtoolbar_child=TRUE;
   else
      qdtoolbar_child=FALSE;
      
   created=false;
   separator=0;
   label=0;
   page_menu=0;
   npage_butt=0;
   ppage_butt=0;
   nnpage_butt=0;
   pppage_butt=0;
   fpage_butt=0;
   lpage_butt=0;

   // if parent is not QDToolBar, then it is most likely a
   // QToolBar. by convention QToolBar content should NOT
   // change dynamically. so create toolbar at the beginning
   if ( qdtoolbar_child )
      ((QDToolBar *)toolbar)->addPiece(this);
   else
      create();
}

void
QDTBarNavPiece::update(int page_num, int pages_num)
{
   
   if (!qdtoolbar_child || pages_num>1)
   {
      if (!created) create();
      
      if (page_menu->count()!=pages_num) page_menu->clear();
      if (!page_menu->count())
      {
	 for(int i=0;i<pages_num;i++)
	 {
	    char buffer[128];
	    sprintf(buffer, "%d", i+1);
	    page_menu->insertItem(buffer);
	 }
      }
      page_menu->setCurrentItem(page_num);
      page_menu->setEnabled(pages_num>1);
      page_menu->setFixedSize(page_menu->sizeHint());

      npage_butt->setEnabled(page_num+1<pages_num);
      ppage_butt->setEnabled(page_num>0);
      nnpage_butt->setEnabled(page_num+10<pages_num);
      pppage_butt->setEnabled(page_num>=10);
      fpage_butt->setEnabled(page_num>0);
      lpage_butt->setEnabled(page_num+1<pages_num);
   }
   else if ( qdtoolbar_child )
   {
      destroy();
   }

      // Keep everything disabled if the toolbar is disabled.
   if (!toolbar->isEnabled()) setEnabled(false);
   
}

void
QDTBarNavPiece::slotPage( const QString & qpage_str)
{
   const char * const page_str=qpage_str;
      // Validator will make sure, that page_str is valid
   emit sigGotoPage(atoi(page_str)-1);
}

void
QDTBarNavPiece::slotPage(void)
{
   const QObject * obj=sender();
   if (obj && obj->isWidgetType() && obj->inherits("QDToolButton"))
   {
      const QDToolButton * butt=(QDToolButton *) obj;
      emit sigDoCmd(butt->cmd);
   }
}

// END OF FILE
