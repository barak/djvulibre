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

#include "DjVuDocument.h"
#include "DjVmDir.h"

#include "qd_tbar_nav_piece.h"
#include "debug.h"
#include "qlib.h"
#include "qd_base.h"
#include "qd_toolbutt.h"
#include "djvu_base_res.h"
#include "cin_data.h"

#include <qcombobox.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <qtooltip.h>



//****************************************************************************
//****************************** QDTBarNavPiece ******************************
//****************************************************************************

void
QDTBarNavPiece::setEnabled(bool en)
{
  page_menu->setEnabled(en);
  npage_butt->setEnabled(en);
  ppage_butt->setEnabled(en);
  fpage_butt->setEnabled(en);
  lpage_butt->setEnabled(en);
}

QDTBarNavPiece::QDTBarNavPiece(QWidget * toolbar) : QDTBarPiece(toolbar)
{
   if ( dynamic_cast<QDToolBar *>(toolbar) )
     qdtoolbar_child=TRUE;
   else
     qdtoolbar_child=FALSE;
   
   page_menu=new QComboBox(TRUE, toolbar, "page_menu");
   page_menu->setInsertionPolicy(QComboBox::NoInsertion);
   page_menu->setAutoCompletion(true);
 
   connect(page_menu, SIGNAL(activated(const QString&)), 
           this, SLOT(slotPage(const QString&)));
   QToolTip::add(page_menu, tr("Page"));

   fpage_butt=new QDToolButton(*CINData::get("ppm_vfpage"), true,
			       IDC_NAV_FIRST_PAGE, toolbar, tr("First page"));
   connect(fpage_butt, SIGNAL(clicked(void)), this, SLOT(slotPage(void)));
   
   ppage_butt=new QDToolButton(*CINData::get("ppm_vppage"), true,
			       IDC_NAV_PREV_PAGE, toolbar, tr("Previous Page"));
   connect(ppage_butt, SIGNAL(clicked(void)), this, SLOT(slotPage(void)));
   
   npage_butt=new QDToolButton(*CINData::get("ppm_vnpage"), true,
			       IDC_NAV_NEXT_PAGE, toolbar, tr("Next Page"));
   connect(npage_butt, SIGNAL(clicked(void)), this, SLOT(slotPage(void)));
   
   lpage_butt=new QDToolButton(*CINData::get("ppm_vlpage"), true,
			       IDC_NAV_LAST_PAGE, toolbar, tr("Last page"));
   connect(lpage_butt, SIGNAL(clicked(void)), this, SLOT(slotPage(void)));
   
   back_butt=new QDToolButton(*CINData::get("ppm_back"), true,
			       IDC_HISTORY_BACK, toolbar, tr("Back"));
   connect(back_butt, SIGNAL(clicked(void)), this, SLOT(slotPage(void)));
   
   forw_butt=new QDToolButton(*CINData::get("ppm_forw"), true,
			       IDC_HISTORY_FORW, toolbar, tr("Forward"));
   connect(forw_butt, SIGNAL(clicked(void)), this, SLOT(slotPage(void)));
   
   if ( qdtoolbar_child ) 
     ((QDToolBar *)toolbar)->addLeftWidgets(back_butt, forw_butt);
   if ( qdtoolbar_child ) 
     ((QDToolBar *)toolbar)->addLeftWidget(page_menu);
   if ( qdtoolbar_child ) 
     ((QDToolBar *)toolbar)->addLeftWidgets(fpage_butt, ppage_butt,
                                            npage_butt, lpage_butt);
   options = 0;
   active = false;
   if ( qdtoolbar_child )
     ((QDToolBar *)toolbar)->addPiece(this);
}

void
QDTBarNavPiece::setOptions(int opts)
{
  bool b;
  options = opts;
  b = active && !(opts & QDBase::OverrideFlags::TOOLBAR_NO_PAGECOMBO);
  showOrHide(page_menu, b);
  b = active && !(opts & QDBase::OverrideFlags::TOOLBAR_NO_FIRSTLAST);
  showOrHide(fpage_butt, b);
  showOrHide(lpage_butt, b);
  b = active && !(opts & QDBase::OverrideFlags::TOOLBAR_NO_PREVNEXT);
  showOrHide(ppage_butt, b);
  showOrHide(npage_butt, b);
  b = active && !(opts & QDBase::OverrideFlags::TOOLBAR_NO_BACKFORW);
  showOrHide(back_butt, b);
  showOrHide(forw_butt, b);
}

void
QDTBarNavPiece::update(int page_num, GP<DjVuDocument> doc, 
                       bool back, bool forw)
{
  int pages_num = (doc) ? doc->get_pages_num() : 0;
  
  if (!qdtoolbar_child || pages_num>1)
    {
      if (! active)
        {
          active = true;
          setOptions(options);
        }
      // Generate or regenerate page title/number menu as needed.
      if (page_menu->count()!=pages_num) 
        page_menu->clear();
      if (!page_menu->count())
        {
          // Set menu contents
          int pagenum = 0;
          GPList<DjVmDir::File> lst = doc->get_djvm_dir()->get_files_list();
          for (GPosition p=lst; p; ++p) 
            {
              char buffer[64];
              GP<DjVmDir::File> f = lst[p];
              if (!f->is_page())
                continue;
              ++pagenum;
              GNativeString id = f->get_load_name();
              GNativeString title = f->get_title();
              if (title != id) 
                {
                  page_menu->insertItem(QStringFromGString(title));
                } 
              else 
                {
                  sprintf(buffer, "%d", pagenum);
                  page_menu->insertItem(buffer);               
                }
            }
        }
      page_menu->setCurrentItem(page_num);
      page_menu->setEnabled(pages_num>1);
      page_menu->setFixedSize(page_menu->sizeHint());
      npage_butt->setEnabled(page_num+1<pages_num);
      ppage_butt->setEnabled(page_num>0);
      fpage_butt->setEnabled(page_num>0);
      lpage_butt->setEnabled(page_num+1<pages_num);
      if (back_butt)
        back_butt->setEnabled(back);
      if (forw_butt)
        forw_butt->setEnabled(forw);
    }
  else
    {
      if (active)
        {
          active = false;
          setOptions(options);
        }
    }
  // Keep everything disabled if the toolbar is disabled.
  if (!toolbar->isEnabled()) 
    setEnabled(false);
}

void
QDTBarNavPiece::slotPage(const QString &text)
{
  int index = page_menu->currentItem();
  if (index < 0 || index >= page_menu->count() ||
      page_menu->text(index) != text)
    {
      int i;
      for (i = 0; i<page_menu->count(); i++)
        if (page_menu->text(i) == text)
          break;
      if (i>=0 && i<page_menu->count())
        index = i;
    }
  emit sigGotoPage(index);
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
