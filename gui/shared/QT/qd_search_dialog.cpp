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
// $Id: qd_search_dialog.cpp,v 1.15 2008/08/05 20:52:26 bpearlmutter Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "DjVuAnno.h"
#include "DjVuFile.h"
#include "DataPool.h"
#include "ByteStream.h"

#include "qd_search_dialog.h"

#include "debug.h"
#include "qlib.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qapplication.h>

bool	QDSearchDialog::all_pages=true;
bool	QDSearchDialog::case_sensitive=false;
bool	QDSearchDialog::whole_word=false;
bool	QDSearchDialog::search_backwards=false;


static GList<DjVuTXT::Zone*>
search_string(DjVuTXT *txt, QString dat, int &pfrom, 
              bool fwd, bool casep, bool wordp)
{
  GList<DjVuTXT::Zone*> res;
  // Access utf8 string
  GUTF8String utext = txt->textUTF8;
  QString qtext = QStringFromGString(utext);
  // Normalize dat
  dat = dat.stripWhiteSpace();
  if (! dat.length())
    return res;
  // Normalize from
  int from = pfrom;
  if (fwd)
    {
      if (from < 0)
        from = 0;
      else
        from += 1;
    }
  else
    {
      if (from < 0 || from >= (int)qtext.length())
        from = qtext.length() - 1;
      else
        from -= 1;
    }
  // Search
  int found;
  int foundx;
  for(;;)
    {
      if (fwd)
        found = qtext.find(dat, from, casep);
      else
        found = qtext.findRev(dat, from, casep);
      if (found < 0)
        return res;
      foundx = found + dat.length();
      if (!wordp)
        break;
      if (found==0 || !qtext[found-1].isLetterOrNumber())
        if (foundx==(int)qtext.length() || !qtext[foundx].isLetterOrNumber())
          break;
      if (fwd) {
        if (++pfrom >= (int)qtext.length())
          return res;
      } else {
        if (--pfrom < 0)
          return res;
      }
    }
  // Compute indices into utf8 string
  int ufound = -1;
  int ufoundx = -1;
  const char *ctext = (const char*)utext;
  int qi = 0;
  int ui = 0;
  while (ctext[ui] && ufoundx<0)
    {
      if (ufound<0 && qi == found)
        ufound = ui;
      if (ufoundx<0 && qi == foundx)
        ufoundx = ui;
      ui += 1;
      if ( (ctext[ui] & 0xc0) != 0x80)
        qi += 1;
    }
  // Retrieve zones
  if (ufoundx > ufound && ufound >= 0) 
    {
      txt->page_zone.find_zones(res, ufound, ufoundx);
      pfrom = found;
    }
  return res;
}


void
QDSearchDialog::slotSearch(void)
{
   if (in_search)
   {
      stop=true;
      return;
   }
   
   if (text->text()=="") return;
   
   try
   {
      bool fwd=!back_butt->isChecked();
      in_search=true;
      stop=false;
      search_butt->setText(tr("&Stop"));
      text->setEnabled(FALSE);
      clear_butt->setEnabled(FALSE);

	 // 'asked_once' is useful for one-page searches
      bool asked_once=false;
	 // 'start_page_num' is for the document search
      int start_page_num=page_num;
	 // Will be true if no TXT chunks have been found
      bool no_txt=true;

	 // We want to place 'anno' here because to keep it alive as long
	 // as zone_list is alive.
      GP<DjVuText> anno;
      GList<DjVuTXT::Zone *> zone_list;
      while(true)
      {
	 GP<DjVuFile> page_file=doc->get_djvu_file(page_num);

	 if (!page_file->is_all_data_present())
	 {
	    GP<DataPool> pool=page_file->get_init_data_pool();
	    int last_done=-1;
	       // Wait until we have data for this file
	    while(!stop && !page_file->is_all_data_present())
	    {
	       int done=5*(20*pool->get_size()/pool->get_length());
	       if (done!=last_done)
	       {
                 QString buffer=tr("Loading page %1: %2%...")
                   .arg(page_num+1)
                   .arg(done);
                 status_label->setText(buffer);
                 last_done=done;
	       }
	       qApp->processEvents(100);
	    }
	 }
	 if (stop)
	 {
	    page_num=seen_page_num;
	    break;
	 }
	 
	 GP<ByteStream> str=page_file->get_text();
	 if (str)
	 {
	    str->seek(0);
	    anno=DjVuText::create();
	    anno->decode(str);
	    GP<DjVuTXT> txt=anno->txt;
	    if (txt)
              {
	       no_txt = false;
	       zone_list = search_string(txt, text->text(), page_pos,
                                         fwd, case_butt->isChecked(),
                                         whole_word_butt->isChecked());
	       if (zone_list.size()) 
                 break;
	    }
	 }
	    // Didn't find anything. Switch to the next page or give up
	 int pages_num=doc->get_pages_num();
	 if (!all_pages_butt->isChecked() ||
	     (fwd && page_num>=pages_num-1) ||
	     (!fwd && page_num==0))
	 {
	    if (asked_once)
	    {
	       page_num=seen_page_num;
	       break;
	    } 
            else
	    {
	       asked_once=true;
	       QString msg;
               if (all_pages_butt->isChecked())
                 {
                   if (fwd)
                     msg=tr("End of document reached. "
                            "Continue from the beginning?");
                   else
                     msg=tr("Beginning of document reached. "
                            "Continue from the end?");
                 }
               else
                 {
                   if (fwd)
                     msg=tr("End of page reached. "
                            "Continue from the beginning?");
                   else
                     msg=tr("Beginning of page reached. "
                            "Continue from the end?");
                 }

	       if (QMessageBox::information(this, "DjVu", msg, 
                                            tr("&Yes"), tr("&No"))==0)
               {
		  if (all_pages_butt->isChecked())
		  {
		     if (fwd) 
                       page_num=-1;
		     else 
                       page_num=doc->get_pages_num();
		  } 
                  else
		  {
		     if (fwd) 
                       page_num--;
		     else 
                       page_num++;
		  }
	       } 
               else
	       {
		  page_num=seen_page_num;
		  break;
	       }
	    }
	 }
	    // Going to the next page (or rewinding to the other side of
	    // this page if asked to)
	 if (fwd)
	 {
	    page_num++;
	    page_pos = -1;
	 } 
         else
	 {
	    page_num--;
	    page_pos = -1;
	 }

         {
           QString mesg=tr("Page ")+QString::number(page_num+1);
	   status_label->setText(mesg);
         }

	    // Wrapped back and returned
	 if (all_pages_butt->isChecked() && page_num==start_page_num)
           {
             page_num=seen_page_num;
             break;
           }
      } // while(true)

      emit sigDisplaySearchResults(seen_page_num=page_num, zone_list);
      if (zone_list.size()==0)
      {
	 if (no_txt)
	 {
           if (all_pages_butt->isChecked())
             showMessage(this, tr("DjVu Search Failed"),
                         tr("After looking through every page of this document,\n"
                            "we have found that none of them contain\n"
                            "textual information. This means that either the document\n"
                            "creator did not run an OCR engine on this document,\n"
			    "or the OCR engine did not recognize any text.\n"
                            "\n"
                            "The search is impossible."), 
                         true, false, true);
           else
             showMessage(this, tr("DjVu Search Failed"),
                         tr("This page does not contain textual information,\n"
                            "which means that either the creator of this document\n" 
                            "did not run an OCR engine on it, or the OCR engine\n"
                            "did not recognize any text on this page.\n"
                            "\n"
                            "The search is impossible."), 
                         true, false, true);
	 } else 
           showInfo(this, "DjVu", tr("Search string not found"));
      }
      
      in_search=false;
      search_butt->setText(tr("&Find"));
      {
        QString mesg=tr("Page ")+QString::number(page_num+1);
        status_label->setText(mesg);
      }
      text->setEnabled(TRUE);
      clear_butt->setEnabled(TRUE);
   } 
   catch(const GException & exc)
   {
      status_label->hide();
      in_search=false;
      search_butt->setText(tr("&Find"));
      {
        QString mesg=tr("Page ")+QString::number(page_num+1);
        status_label->setText(mesg);
      }
      text->setEnabled(TRUE);
      clear_butt->setEnabled(TRUE);
      showError(this, exc);
   }
}

void
QDSearchDialog::slotSetPageNum(int page_num_)
{
   seen_page_num=page_num_;

   if (!in_search)
   {
      if (page_num!=page_num_)
	 page_pos = -1;
      page_num = seen_page_num;
   }
   
   QString mesg=tr("Page ")+QString::number(page_num+1);
   status_label->setText(mesg);
}

void
QDSearchDialog::slotTextChanged(const QString & qtext)
{
   const char * const str=qtext;
   search_butt->setEnabled(str && strlen(str));
}

void
QDSearchDialog::done(int rc)
{
   stop=true;
   all_pages=all_pages_butt->isChecked();
   case_sensitive=case_butt->isChecked();
   whole_word=whole_word_butt->isChecked();
   search_backwards=back_butt->isChecked();
   emit sigDone();
   QDialog::done(rc);
}

QDSearchDialog::QDSearchDialog(int page_num_, const GP<DjVuDocument> & doc_,
			       QWidget * parent, const char * name, bool modal) :
      QeDialog(parent, name, modal), page_num(page_num_), doc(doc_)
{
   seen_page_num = page_num;
   page_pos = -1;
   in_search=false;
   
   setCaption(tr("DjVu: Find"));
   
   QWidget * start=startWidget();
   QLabel * label;
   
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);

   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   label=new QLabel(tr("Find: "), start);
   hlay->addWidget(label);
   text=new QLineEdit(start, "search_text");
   label->setBuddy(text);
   hlay->addWidget(text, 1);

   hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);

   all_pages_butt=new QCheckBox(tr("Search &all pages"), start, "all_pages_butt");
   if (doc->get_pages_num()>1)
      all_pages_butt->setChecked(all_pages);
   else
   {
      all_pages_butt->setChecked(TRUE);
      all_pages_butt->setEnabled(FALSE);
   }
   hlay->addWidget(all_pages_butt);
   hlay->addStretch(1);
   
   case_butt=new QCheckBox(tr("&Case sensitive"), start, "case_butt");
   case_butt->setChecked(case_sensitive);
   hlay->addWidget(case_butt);
   hlay->addStretch(1);

   whole_word_butt=new QCheckBox(tr("&Whole word"), start, "whole_word_butt");
   whole_word_butt->setChecked(whole_word);
   hlay->addWidget(whole_word_butt);
   hlay->addStretch(1);

   back_butt=new QCheckBox(tr("Search &backwards"), start, "back_butt");
   back_butt->setChecked(search_backwards);
   hlay->addWidget(back_butt);
   
   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   status_label=new QLabel(tr("Loading page WWWWWWW"), start);
   status_label->setMinimumSize(status_label->sizeHint());
   {
     QString mesg=tr("Page ")+QString::number(page_num+1);
     status_label->setText(mesg);
   }
   butt_lay->addWidget(status_label);
   butt_lay->addStretch(1);
   search_butt=new QPushButton(tr("&Find"), start, "search_butt");
   search_butt->setEnabled(FALSE);
   butt_lay->addWidget(search_butt);
   clear_butt=new QPushButton(tr("&Clear"), start, "clear_butt");
   butt_lay->addWidget(clear_butt);
   QPushButton * close_butt=new QPushButton(tr("C&lose"), start, "close_butt");
   butt_lay->addWidget(close_butt);
   search_butt->setDefault(TRUE);

   connect(search_butt, SIGNAL(clicked(void)), this, SLOT(slotSearch(void)));
   connect(clear_butt, SIGNAL(clicked(void)), text, SLOT(clear(void)));
   connect(close_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));

   connect(text, SIGNAL(textChanged(const QString &)),
	   this, SLOT(slotTextChanged(const QString &)));
}
