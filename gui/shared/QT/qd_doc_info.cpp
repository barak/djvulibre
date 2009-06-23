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
// $Id: qd_doc_info.cpp,v 1.13 2007/03/25 20:48:25 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qd_doc_info.h"
#include "qlib.h"
#include "debug.h"
#include "IFFByteStream.h"
#include "DjVuDumpHelper.h"
#include "DjVmDir.h"
#include "DjVmDir0.h"
#include "DjVuNavDir.h"
#include "DataPool.h"

#include <qheader.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qpopupmenu.h>
#include <qtabbar.h>
#include <qtabdialog.h>
#include <qapplication.h>



#define PAGE_PAGE_COL 0
#define PAGE_FILE_COL 1
#define PAGE_SIZE_COL 2

#define THUMB_NUM_COL   0
#define THUMB_FILE_COL  1
#define THUMB_PAGES_COL 2
#define THUMB_SIZE_COL  3

#define FILE_NUM_COL  0
#define FILE_FILE_COL 1
#define FILE_TYPE_COL 2
#define FILE_SIZE_COL 3


//**************************** QDPageListItem *******************************

// class QDDocInfoItem : public QListViewItem
class QDPageListItem : public QListViewItem
{
public:
   GP<DjVuFile>                djvu_file;

#ifdef QT1
   virtual const char *        key(int column, bool) const
#else
   virtual QString         key(int column, bool) const
#endif
   {
     if (column==PAGE_PAGE_COL || column==PAGE_SIZE_COL)
     {
#ifdef QT1
       static char buffer[16];
#else
       char buffer[16];
#endif
       sprintf(buffer, "%09d", atoi(text(column)));
       return buffer;
     } else
     {
       return text(column);
     }
   }
   
   QDPageListItem(
     QListView * parent,
     const char * name0,
     const char * name1=0,
     const char * name2=0,
     const char * name3=0,
     const char * name4=0,
     const char * name5=0,
     const char * name6=0,
     const char * name7=0
   ) : QListViewItem(parent, name0, name1, name2, name3,
                name4, name5, name6, name7) {}
   virtual ~QDPageListItem(void) {}
};

//**************************** QDThumbListItem *******************************

class QDThumbListItem : public QListViewItem
{
public:
   GP<DjVuFile>               djvu_file;

#ifdef QT1
   virtual const char *        key(int column, bool) const
#else
   virtual QString         key(int column, bool) const
#endif
   {
     if (column==THUMB_NUM_COL || column==THUMB_SIZE_COL ||
           column==THUMB_PAGES_COL)
     {
#ifdef QT1
       static char buffer[16];
#else
       char buffer[16];
#endif
       sprintf(buffer, "%09d", atoi(text(column)));
       return buffer;
     } else
     {
       return text(column);
     }
   }

   QDThumbListItem(
     QListView * parent,
     const char * name0,
     const char * name1=0,
     const char * name2=0,
     const char * name3=0,
     const char * name4=0,
     const char * name5=0,
     const char * name6=0,
     const char * name7=0
   ) : QListViewItem(parent, name0, name1, name2, name3,
                name4, name5, name6, name7) {}
   virtual ~QDThumbListItem(void) {}
};

//**************************** QDFileListItem *******************************

class QDFileListItem : public QListViewItem
{
public:
#ifdef QT1
   virtual const char *        key(int column, bool) const
#else
   virtual QString         key(int column, bool) const
#endif
   {
     if (column==FILE_NUM_COL || column==FILE_SIZE_COL)
     {
#ifdef QT1
       static char buffer[16];
#else
       char buffer[16];
#endif
       sprintf(buffer, "%09d", atoi(text(column)));
       return buffer;
     } else
     {
       return text(column);
     }
   }

   QDFileListItem(
     QListView * parent,
     const char * name0,
     const char * name1=0,
     const char * name2=0,
     const char * name3=0,
     const char * name4=0,
     const char * name5=0,
     const char * name6=0,
     const char * name7=0
   ) : QListViewItem(parent, name0, name1, name2, name3,
                       name4, name5, name6, name7) {}
   virtual ~QDFileListItem(void) {}
};

//************************** QDPort connections ******************************

void
QDDocInfo::slotNotifyError(const GP<DjVuPort> &, const GUTF8String &qmsg)
      // Will be called for obsolete documents only
{
   ::showError(this, tr("DjVu Error"), QStringFromGString(qmsg));
}

void
QDDocInfo::slotNotifyFileFlagsChanged(
  const GP<DjVuFile> & file, long set_mask, long )
{
   int doc_type=doc->get_doc_type();
   if (set_mask & DjVuFile::ALL_DATA_PRESENT)
   {
      if(doc_type == DjVuDocument::OLD_INDEXED)
      {
        int page_num=doc->url_to_page(file->get_url());
        if (page_num>=0)
        {
           QDPageListItem * item=(QDPageListItem *) page_items[page_num];
           if (!item->djvu_file) item->djvu_file=(DjVuFile *) file;
           preloadNextPage();
           updatePage((DjVuFile *) file);
        }
      } else if (doc_type==DjVuDocument::BUNDLED ||
                 doc_type==DjVuDocument::INDIRECT)
      {
         GP<DjVmDir> djvm_dir=doc->get_djvm_dir();
         GP<DjVmDir::File> frec=djvm_dir->name_to_file(file->get_url().fname());
         if (frec->is_thumbnails())
         {
            preloadNextThumb();
            updateThumb(file);
         }
      }
   }
}

//***************************** QDDocInfo ********************************

void
QDDocInfo::preloadNextPage(void)
      // This function is only called for SINGLE_PAGE and OLD_INDEXED
      // when we don't have separate file sizes.
{
   DEBUG_MSG("QDDocInfo::preloadNextPage(): searching next page to preload...\n");
   DEBUG_MAKE_INDENT(3);

   for(int page=0;page<page_items.size();page++)
   {
      QDPageListItem * item=(QDPageListItem *) page_items[page];
      if (!item->djvu_file)
      {
         item->djvu_file=doc->get_djvu_file(page);
         if (item->djvu_file->is_data_present())
            port.notify_file_flags_changed(item->djvu_file, DjVuFile::ALL_DATA_PRESENT, 0);
         return;
      }
   }
}

void
QDDocInfo::preloadNextThumb(void)
      // It's used to get obtain the list of pages covered by
      // the thumbnail file. Ideally all the info is in the DjVmDir,
      // and doesn't require downloading. But it may be wrong,
      // and we want to be sure...
{
   DEBUG_MSG("QDDocInfo::preloadNextThumb(): searching next thumbnail to preload...\n");
   DEBUG_MAKE_INDENT(3);

   for(int thumb_num=0;thumb_num<thumb_items.size();thumb_num++)
   {
      QDThumbListItem * item=(QDThumbListItem *) thumb_items[thumb_num];
      if (!item->djvu_file)
      {
         item->djvu_file=doc->get_djvu_file(GStringFromQString(item->text(THUMB_FILE_COL)));
         if (item->djvu_file->is_data_present())
            port.notify_file_flags_changed(item->djvu_file, DjVuFile::ALL_DATA_PRESENT, 0);
         return;
      }
   }
}

void
QDDocInfo::updatePage(const GP<DjVuFile> & file)
      // If file!=0 then this function will attempt to update entry for
      // page with corresponding to this file. Otherwise it will just
      // recalculate total size of the document basing on information in
      // widgets
{
   if (file)
   {
      int page=doc->url_to_page(file->get_url());
      if (page<0) return;
      if (file->is_data_present())
      {
         GUTF8String mesg=GUTF8String(file->file_size);
         ((QListViewItem *) page_items[page])->setText(PAGE_SIZE_COL, QStringFromGString(mesg));
      }
   }

   int total_size=0;
   if (doc->is_bundled())
   {
         // Get the size of the archive. DataPool is smart enough to interpret
         // IFF structure, so the result will be OK even if not all data
         // is there yet.
      GP<DataPool> pool=doc->get_init_data_pool();
      total_size=pool->get_length();
      if (total_size<0) total_size=pool->get_size();
   } else
   {
      for(int page=0;page<page_items.size();page++)
         total_size+=atoi(((QListViewItem *) page_items[page])->text(PAGE_SIZE_COL));
   }
   if (total_size>0)
   {
     GUTF8String mesg=total_size;
     size_label->setText(QStringFromGString(mesg));
   }
}

void
QDDocInfo::updateThumb(const GP<DjVuFile> & file)
      // Here we need to determine how many icons the file contains,
      // and update the corresponding list item
{
  int icons_num=0;

  GP<ByteStream> str=file->get_djvu_bytestream(false, true);
  GP<IFFByteStream> iff=IFFByteStream::create(str);
  GUTF8String chkname;
  if (iff->get_chunk(chkname))
  {
    while(iff->get_chunk(chkname))
    {
      icons_num++;
      iff->close_chunk();
    }
  }

      // We just got the number of images in the file with thumbnails.
      // Now get the start page, and we're done.
  GP<DjVmDir> djvm_dir=doc->get_djvm_dir();
  if (djvm_dir)
  {
    GUTF8String name=file->get_url().fname();
    GP<DjVmDir::File> frec=djvm_dir->name_to_file(name);
    if (frec)
    {
      GPList<DjVmDir::File> files_list=djvm_dir->get_files_list();
      for(GPosition pos=files_list;pos;++pos)
      {
        if (files_list[pos]==frec)
        {
          while(pos && !files_list[pos]->is_page())
             ++pos;
          if (pos)
          {
            int start_page_num=files_list[pos]->get_page_num();
               // Now count the number of pages until the next
               // file with thumbnails to detect problems
            int pages_until_next_thumb=0;
            while(pos && !files_list[pos]->is_thumbnails())
            {
               if (files_list[pos]->is_page())
                  pages_until_next_thumb++;
               ++pos;
            }

            for(int i=0;i<thumb_items.size();i++)
            {
              QDThumbListItem * item=(QDThumbListItem *) thumb_items[i];
              if (GStringFromQString(item->text(THUMB_FILE_COL))==name)
              {
                GUTF8String buffer;
                if (icons_num==1)
                   buffer.format("%d", start_page_num+1);
                else
                   buffer.format("%d-%d", start_page_num+1,
                           start_page_num+icons_num);
                if (pages_until_next_thumb!=icons_num)
                   buffer+=" (WRONG!)";
                item->setText(THUMB_PAGES_COL, QStringFromGString(buffer));
                break;
              }
            }
          }
          break;
        }
      }
    }
  }
}

void
QDDocInfo::show(void)
{
   DEBUG_MSG("QDDocInfo::show(): The QDDocInfo dialog shown\n");
   DEBUG_MAKE_INDENT(3);

   try
   {
         // The only reason to start preloading is to get those
         // damn file sizes. We need it in OLD_INDEXED and
         // SINGLE_PAGE formats only.
      int doc_type=doc->get_doc_type();
      if (doc_type!=DjVuDocument::BUNDLED &&
          doc_type!=DjVuDocument::INDIRECT &&
          doc_type!=DjVuDocument::OLD_BUNDLED)
      {
            // Start up to 6 preloading threads. Before each of them dies,
            // it will spawn another one. Thus there will always be up to 6
            // threads preloading the files.
         for(int i=0;i<6;i++) preloadNextPage();
      } // Otherwise all information is readily available from DjVmDir

      if (doc_type==DjVuDocument::BUNDLED ||
          doc_type==DjVuDocument::INDIRECT)
      {
         preloadNextThumb();
         preloadNextThumb();
      }
   } catch(const GException & exc)
   {
      ::showError(this, exc);
   }
   QTabDialog::show();
}

void
QDDocInfo::slotShowInfo(void)
{
  DEBUG_MSG("QDDocInfo::slotShowInfo(): showing info on current item...\n");
  DEBUG_MAKE_INDENT(3);

  const QObject * s=sender();

  GP<DjVuFile> file=0;
  QString title;

  int doc_type=doc->get_doc_type();

  if (s==page_info_butt)
  {
    QDPageListItem * item=(QDPageListItem *) page_list->currentItem();

    file=doc->get_djvu_file(atoi(item->text(PAGE_PAGE_COL))-1);
    title=tr("Information on page ")+item->text(PAGE_PAGE_COL)+
        " ("+item->text(PAGE_FILE_COL)+")";
  } else if (s==file_info_butt)
  {
    QDFileListItem * item=(QDFileListItem *) file_list->currentItem();

    file=doc->get_djvu_file(GStringFromQString(item->text(FILE_FILE_COL)));
    title=tr("Information on file ")+item->text(FILE_FILE_COL);
  } else if (s==thumb_info_butt)
  {
    QDThumbListItem * item=(QDThumbListItem *) thumb_list->currentItem();

    file=doc->get_djvu_file(GStringFromQString(item->text(THUMB_FILE_COL)));
    title=tr("Information on file ")+item->text(THUMB_FILE_COL);
  }

  if (file)
  {
         // First wait until we have all data for this file
    if (!file->is_data_present())
    {
      QeDialog * d=new QeDialog(this);
      d->setCaption(tr("Please wait..."));
      QWidget * start=d->startWidget();
      QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);
      QLabel * l;
      l=new QLabel(tr("Please wait while I'm downloading the data..."), start);
      vlay->addWidget(l);
      QHBoxLayout * hlay=new QHBoxLayout(10);
      vlay->addLayout(hlay);
      hlay->addStretch(1);
      QPushButton * cancel_butt=new QPushButton(tr("&Cancel"), start);
      cancel_butt->setDefault(TRUE);
      hlay->addWidget(cancel_butt);
      connect(cancel_butt, SIGNAL(clicked(void)), d, SLOT(reject(void)));
      d->show();
      while(d->isVisible() && !file->is_data_present())
         qApp->processEvents(100);
      delete d;
      if (!file->is_data_present())
         return;
    }
    DjVuDumpHelper dumper;
    GP<DataPool> pool=file->get_init_data_pool();
    GP<ByteStream> str=dumper.dump(pool);
    TArray<char> data=str->get_data();
    GUTF8String info=(const char *) data;
       // If we dump contents of a file with thumbnails
       // we can expand it, because know all informatoin about
       // the whole DjVu document
    if (doc_type==DjVuDocument::BUNDLED ||
      doc_type==DjVuDocument::INDIRECT)
    {
      GUTF8String name=file->get_url().fname();
      GP<DjVmDir> djvm_dir=doc->get_djvm_dir();
      GP<DjVmDir::File> frec=djvm_dir->name_to_file(name);
      if (frec->is_thumbnails())
      {
        int start_page_num=-1;
        GPList<DjVmDir::File> files_list=djvm_dir->get_files_list();
        for(GPosition pos=files_list;pos;++pos)
        {
          if (files_list[pos]->get_save_name()==name)
          {
            while(pos && !files_list[pos]->is_page())
              ++pos;
            if (pos)
              start_page_num=files_list[pos]->get_page_num();
            break;
          }
        }
        if (start_page_num>=0)
        {
          start_page_num++;
          const char * ptr;
          while((ptr=strstr(info, "icon\n")))
          {
            GUTF8String tmp((const char *)info, ptr-(const char*)info);
            tmp+="icon for page "+GUTF8String(start_page_num++);
            tmp+=ptr+4;
            info=tmp;
          }
        }
      }
    }
    showMessage(this, title, QStringFromGString(info), 1, 1);
  }
}

void
QDDocInfo::slotGotoPage(void)
{
   DEBUG_MSG("QDDocInfo::slotGotoPage(): going to the page " <<
           page_list->currentItem()->text(PAGE_PAGE_COL) << "...\n");
   DEBUG_MAKE_INDENT(3);

   emit sigGotoPage(atoi(page_list->currentItem()->text(PAGE_PAGE_COL))-1);
}

void
QDDocInfo::slotItemDblClicked(QListViewItem * item)
{
   DEBUG_MSG("QDDocInfo:slotItemDblClicked(): page " << item->text(PAGE_PAGE_COL) << "\n");
   DEBUG_MAKE_INDENT(3);

   slotGotoPage();
}

void
QDDocInfo::slotItemSelected(QListViewItem * item)
{
   DEBUG_MSG("QDDocInfo:slotItemSelected(): page " << item->text(PAGE_PAGE_COL) << "\n");
   DEBUG_MAKE_INDENT(3);

   const QObject * s=sender();
   if (s==page_list)
   {
      page_info_butt->setEnabled(TRUE);
      page_goto_butt->setEnabled(TRUE);
   } else if (s==file_list)
   {
      file_info_butt->setEnabled(TRUE);
   }else if (s==thumb_list) 
   {
      thumb_info_butt->setEnabled(TRUE);
   }
}

void
QDDocInfo::slotRightButtonPressed(QListViewItem * item,
                                  const QPoint & pos, int)
{
   DEBUG_MSG("QDDocInfo:slotRightButtonPressed(): showing popup menu\n");
   DEBUG_MAKE_INDENT(3);

   if (item)
   {
      page_list->setSelected(item, TRUE);

      QPopupMenu popup_menu;
      popup_menu.insertItem(tr("Page &Info"), (int) 0);
      popup_menu.insertItem(tr("&Goto to page"), (int) 1);

      int rc=popup_menu.exec(pos);

      if (rc==1) slotGotoPage();
      else if (rc==0) slotShowInfo();
   } else
   {
      DEBUG_MSG("but it's outside of any item...\n");
   }
}

QDDocInfo::QDDocInfo(const GP<DjVuDocument> & _doc, QWidget * parent,
                     const char * name, bool modal) :
      QTabDialog(parent, name, modal), doc(_doc), port(1, 0)
{
   DEBUG_MSG("QDDocInfo::QDDocInfo(): Creating 'Doc Info' dialog...\n");
   DEBUG_MAKE_INDENT(3);

   int doc_type=doc->get_doc_type();

   setCaption(tr("DjVu multipage document info"));

   page_list=thumb_list=file_list=0;
   page_info_butt=thumb_info_butt=file_info_butt=0;
   
   QWidget *summary_widget=new QWidget(this, "summary_widget");
   QVBoxLayout * summary_vlay=new QVBoxLayout(summary_widget, 10, 10, "summary_vlay");
   {
     QLabel * label;
     QFont font;

     label=new QLabel(tr("Document Information"), summary_widget, "title_label");
     label->setAlignment(AlignCenter);
     font=label->font();
     font.setWeight(QFont::Bold);
     label->setFont(font);
     summary_vlay->addWidget(label);
     summary_vlay->addSpacing(15);

     int rows=3;
     switch(doc_type)
     {
       case DjVuDocument::OLD_BUNDLED:
       case DjVuDocument::BUNDLED:
       case DjVuDocument::INDIRECT:
         rows=5;
         break;
       case DjVuDocument::OLD_INDEXED:
         rows=3;
         break;
       default:
         rows=4;
         break;
     }
     QGridLayout *summary_glay = new QGridLayout(summary_vlay, rows, 2, 5);
     int row=0;
     if (doc_type!=DjVuDocument::OLD_INDEXED)
     {
         //**** "Name" field
	label=new QLabel(tr("Document Name:"), summary_widget);
        summary_glay->addWidget(label, row, 0);
        GUTF8String fname_str=doc->get_init_url().fname();
        label=new QLabel(QStringFromGString(fname_str), summary_widget);
        font=label->font();
        font.setFamily("Courier");
        label->setFont(font);
        label->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
        summary_glay->addWidget(label, row++, 1);
     }
   
        //**** "Format" field
     label=new QLabel(tr("Document Format:"), summary_widget);
     summary_glay->addWidget(label, row, 0);
     label=new QLabel(doc_type==DjVuDocument::BUNDLED ? tr("BUNDLED") :
                     doc_type==DjVuDocument::OLD_BUNDLED ? tr("Obsolete BUNDLED") :
                     doc_type==DjVuDocument::INDIRECT ? tr("INDIRECT") :
                     doc_type==DjVuDocument::OLD_INDEXED ? tr("Obsolete INDEXED") :
                     doc_type==DjVuDocument::SINGLE_PAGE ? tr("SINGLE_PAGE") :
                     tr("UNKNOWN"), summary_widget);
     font=label->font();
     font.setFamily("Courier");
     label->setFont(font);
     label->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
     summary_glay->addWidget(label, row++, 1);

      //**** "Total size" field
     label=new QLabel(tr("Total size:"), summary_widget);
     summary_glay->addWidget(label, row, 0);
     size_label=label=new QLabel(tr("unknown"), summary_widget);
     font=label->font();
     font.setFamily("Courier");
     label->setFont(font);
     label->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
     summary_glay->addWidget(label, row++, 1);

      //**** "Pages Number" field
     label=new QLabel(tr("Pages:"), summary_widget);
     summary_glay->addWidget(label, row, 0);
     {
       GUTF8String mesg=GUTF8String(doc->get_pages_num());
       label=new QLabel(QStringFromGString(mesg), summary_widget);
     }
     font=label->font();
     font.setFamily("Courier");
     label->setFont(font);
     label->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
     summary_glay->addWidget(label, row++, 1);

     if (doc_type==DjVuDocument::OLD_BUNDLED ||
         doc_type==DjVuDocument::BUNDLED ||
         doc_type==DjVuDocument::INDIRECT)
     {
       //**** "Files Number" field
       label=new QLabel(tr("Files:"), summary_widget);
       summary_glay->addWidget(label, row, 0);
       if (doc_type==DjVuDocument::BUNDLED ||
           doc_type==DjVuDocument::INDIRECT)
       {
         GUTF8String mesg=doc->get_djvm_dir()->get_files_num();
         label=new QLabel(QStringFromGString(mesg), summary_widget);

       }else if (doc_type==DjVuDocument::OLD_BUNDLED)
       {
         GUTF8String mesg=doc->get_djvm_dir0()->get_files_num();
         label=new QLabel(QStringFromGString(mesg), summary_widget);
       }
       font=label->font();
       font.setFamily("Courier");
       label->setFont(font);
       label->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
       summary_glay->addWidget(label, row++, 1);
     }
   }
   QWidget *page_widget=0, * thumb_widget=0, * file_widget=0;

   GP<DjVmDir> djvm_dir;
   GP<DjVmDir0> djvm_dir0;
   switch(doc_type) 
   {
     case DjVuDocument::BUNDLED:
     case DjVuDocument::INDIRECT:
       djvm_dir=doc->get_djvm_dir();
       break;
     case DjVuDocument::OLD_BUNDLED:
       djvm_dir0=doc->get_djvm_dir0();
       break;
     default:
       break;
   }
     //********************************************************************
     //************************* page_list ********************************
     //********************************************************************
   {
     page_widget=new QWidget(this, "page_widget");
     QVBoxLayout * page_vlay=new QVBoxLayout(page_widget, 10, 5);

     page_list=new QListView(page_widget, "page_list_view");
     page_list->addColumn(tr("Page"));
     page_list->setColumnWidthMode(PAGE_PAGE_COL, QListView::Maximum);
     page_list->addColumn(tr("File Name"));
     page_list->setColumnWidthMode(PAGE_FILE_COL, QListView::Maximum);
     page_list->setColumnWidth(PAGE_FILE_COL, page_list->header()->cellSize(PAGE_FILE_COL)*2);
     page_list->addColumn(tr("File Size"));
     page_list->setColumnWidthMode(PAGE_SIZE_COL, QListView::Maximum);
     page_list->setAllColumnsShowFocus(TRUE);
     QToolTip::add(page_list,
		   tr("Double click any page from the list to view it.\n")+
		   tr("Press the right mouse button to see popup menu."));
     page_vlay->addWidget(page_list, 1);

     QHBoxLayout * page_hlay=new QHBoxLayout(page_vlay,5);
     page_hlay->addStretch(1);
     page_info_butt=new QPushButton(tr("&Info"), page_widget, "page_info_butt");
     page_info_butt->setEnabled(FALSE);
     page_hlay->addWidget(page_info_butt);
     page_goto_butt=new QPushButton(tr("&Goto"), page_widget, "page_goto_butt");
     page_goto_butt->setEnabled(FALSE);
     page_hlay->addWidget(page_goto_butt);
   }

      //********************************************************************
      //************************* thumb_list *******************************
      //********************************************************************
   if(djvm_dir)
   {
     GPList<DjVmDir::File> files_list=djvm_dir->get_files_list();
     int count=0;
     for(GPosition pos=files_list;pos;++pos)
     {
       GP<DjVmDir::File> frec=files_list[pos];
       if (frec && frec->is_thumbnails())
       {
         if(!thumb_widget)
         {
           thumb_widget=new QWidget(this, "thumb_widget");
           QVBoxLayout * thumb_vlay=new QVBoxLayout(thumb_widget, 10, 5);

           thumb_list=new QListView(thumb_widget, "thumb_list_view");
           thumb_list->addColumn(tr("Order #"));
           thumb_list->setColumnWidthMode(THUMB_NUM_COL, QListView::Maximum);
           thumb_list->addColumn(tr("File Name"));
           thumb_list->setColumnWidthMode(THUMB_FILE_COL, QListView::Maximum);
           thumb_list->addColumn(tr("Pages Covered"));
           thumb_list->setColumnWidthMode(THUMB_PAGES_COL, QListView::Maximum);
           thumb_list->addColumn(tr("File Size"));
           thumb_list->setColumnWidthMode(THUMB_SIZE_COL, QListView::Maximum);
           thumb_list->setAllColumnsShowFocus(TRUE);
           thumb_vlay->addWidget(thumb_list, 1);

           QHBoxLayout * thumb_hlay=new QHBoxLayout(thumb_vlay);
           thumb_hlay->addStretch(1);
           thumb_info_butt=new QPushButton(tr("&Info"), thumb_widget, "thumb_info_butt");
           thumb_info_butt->setEnabled(FALSE);
           thumb_hlay->addWidget(thumb_info_butt);
         }
         thumb_items.resize(count);
         GUTF8String file_size="unknown";
         if (frec->size>0)
           file_size=GUTF8String(frec->size);
         GUTF8String count_str=GUTF8String(count+1);
         thumb_items[count]=new QDThumbListItem(thumb_list, count_str, 
						frec->get_save_name(), "unknown",
						file_size);
         count++;
       }
     }
     if (thumb_list)
     {
       thumb_list->setVScrollBarMode(QScrollView::AlwaysOn);
       const int min_thumb_width=thumb_list->sizeHint().width()+
	 thumb_list->verticalScrollBar()->width();
       thumb_list->setMinimumWidth(min_thumb_width);
       const int min_list_height=
	 ((QListViewItem *) thumb_items[0])->height()*10;
       thumb_list->setMinimumHeight(min_list_height);
     }
   }

     //********************************************************************
     //************************* file_list ********************************
     //********************************************************************
   if (djvm_dir || djvm_dir0)
   {
     file_widget=new QWidget(this, "file_widget");
     QVBoxLayout * file_vlay=new QVBoxLayout(file_widget, 10, 5);

     file_list=new QListView(file_widget, "file_list_view");
     file_list->addColumn(tr("Order #"));
     file_list->setColumnWidthMode(FILE_NUM_COL, QListView::Maximum);
     file_list->addColumn(tr("File Name"));
     file_list->setColumnWidthMode(FILE_FILE_COL, QListView::Maximum);
     file_list->setColumnWidth(FILE_FILE_COL, 
			       file_list->header()->cellSize(FILE_FILE_COL)*3/2);
     file_list->addColumn(tr("File Type"));
     file_list->setColumnWidthMode(FILE_TYPE_COL, QListView::Maximum);
     file_list->setColumnWidth(FILE_TYPE_COL, 
			       file_list->header()->cellSize(FILE_TYPE_COL)*3/2);
     file_list->addColumn(tr("File Size"));
     file_list->setColumnWidthMode(FILE_SIZE_COL, QListView::Maximum);
     file_list->setAllColumnsShowFocus(TRUE);
     file_vlay->addWidget(file_list, 1);

     QHBoxLayout * file_hlay=new QHBoxLayout(file_vlay);
     file_hlay->addStretch(1);
     file_info_butt=new QPushButton(tr("&Info"), file_widget, "file_info_butt");
     file_info_butt->setEnabled(FALSE);
     file_hlay->addWidget(file_info_butt);
   }

      //********************************************************************
      //************************ Filling the lists *************************
      //********************************************************************

   int total_size=0;          // Used for SINGLE_PAGE and OLD_INDEXED
                                      // documents only because for others
                              // we can get the size from DjVmDir and
                              // DjVmDir0
   GP<DjVuNavDir> nav_dir=doc->get_nav_dir();

      //*************************** page_list ******************************
   int pages=doc->get_pages_num();
   if(page_list)
   {
     page_items.resize(pages-1);
   }
   for(int page=0;page<pages;page++)
   {
     GURL url=doc->page_to_url(page);
     GUTF8String page_str=GUTF8String(page+1);
     GUTF8String fname_str=url.fname();
     if(page_list)
       page_items[page]=new QDPageListItem(page_list, page_str, 
					   fname_str, "unknown");

     if (doc_type==DjVuDocument::OLD_BUNDLED ||
	 doc_type==DjVuDocument::BUNDLED     ||
	 doc_type==DjVuDocument::INDIRECT     )
     {
          // Can fill out data right here. No download required.
          // Thanks to complete information in DjVmDir[0]
       int page_size=0;
       if (doc_type==DjVuDocument::OLD_BUNDLED)
       {
          GP<DjVmDir0::FileRec> frec=djvm_dir0->get_file(url.fname());
          if (frec && frec->size>0)
             page_size=frec->size;
       } else
       {
          GP<DjVmDir::File> f=djvm_dir->page_to_file(page);
          if (f && f->size>0)
             page_size=f->size;
       }
       GUTF8String size_str="unknown";
       if (page_size>0)
       {
          size_str=GUTF8String(page_size);
       }
       if(page_list)
       {
	 ((QDPageListItem *)page_items[page]) ->
	     setText(PAGE_SIZE_COL, QStringFromGString(size_str));
       }
       if (page_size>0 && total_size>=0)
       {
         total_size+=page_size;
       } else
       {
         total_size=-1;
       }
     }
   }
      //*************************** file_list ******************************
   if (file_list)
   {
     if (djvm_dir0)
     {
       int files_num=djvm_dir0->get_files_num();
       file_items.resize(files_num-1);
       for(int file_num=0;file_num<files_num;file_num++)
       {
          GP<DjVmDir0::FileRec> frec=djvm_dir0->get_file(file_num);
          if (frec)
          {
             GUTF8String file_size="unknown";
             if (frec->size>0)
                file_size=GUTF8String(frec->size);
             GUTF8String file_type="unknown";
             if (nav_dir)
             {
                file_type="INCLUDE";
                if (nav_dir->name_to_page(frec->name)>=0)
                   file_type="PAGE";
             }
             GUTF8String file_num_str=GUTF8String(file_num+1);
             file_items[file_num]=new QDFileListItem(file_list,
                                                     file_num_str,
                                                     frec->name,
                                                     file_type, file_size);
          }
       }
     } else if(djvm_dir)
     {
       GPList<DjVmDir::File> files_list=djvm_dir->get_files_list();
       file_items.resize(files_list.size()-1);
       int count=0;
       for(GPosition pos=files_list;pos;++pos)
       {
          GP<DjVmDir::File> frec=files_list[pos];
          if (frec)
          {
             GUTF8String file_size="unknown";
             if (frec->size>0)
                file_size=GUTF8String(frec->size);
             GUTF8String count_str=GUTF8String(count+1);
             file_items[count]=new QDFileListItem(file_list,
                                                  count_str,
                                                  frec->get_save_name(),
                                                  frec->get_str_type(),
                                                  file_size);
             count++;
          }
       }
     }
   }

   if (doc->is_bundled())
   {
         // Get the size of the archive. DataPool is smart enough to interpret
         // IFF structure, so the result will be OK even if not all data
         // is there yet.
      GP<DataPool> pool=doc->get_init_data_pool();
      int total_size=pool->get_length();
      if (total_size<0) total_size=pool->get_size();
      GUTF8String mesg=total_size;
      size_label->setText(QStringFromGString(mesg));
   } else if (doc_type==DjVuDocument::INDIRECT)
   {
      GUTF8String mesg=(total_size>0 ? GUTF8String(total_size) : GUTF8String("unknown"));
      size_label->setText(QStringFromGString(mesg));
   }

   if(page_list)
   {
      // Final adjustments of the lists
     page_list->setVScrollBarMode(QScrollView::AlwaysOn);
     const int min_list_width = page_list->sizeHint().width()
                              + page_list->verticalScrollBar()->width();
     page_list->setMinimumWidth(min_list_width);
     const int min_list_height = ((QListViewItem *) page_items[0])->height()*10;
     page_list->setMinimumHeight(min_list_height);
   }
   if (file_list)
   {
      file_list->setVScrollBarMode(QScrollView::AlwaysOn);
      const int min_file_width = file_list->sizeHint().width()
			       + file_list->verticalScrollBar()->width();
      file_list->setMinimumWidth(min_file_width);
      const int min_list_height=((QListViewItem *) file_items[0])->height()*10;
      file_list->setMinimumHeight(min_list_height);
   }
   
   addTab(summary_widget, tr("&Summary"));
   if(page_widget)
     addTab(page_widget, tr("&Pages"));
   if (thumb_widget)
     addTab(thumb_widget, tr("&Thumbnails"));
   if(file_widget)
     addTab(file_widget,tr("&All Files"));

   setOKButton(tr("&Close"));

   if(page_widget)
   {
      // Connecting signals and slots
     connect(page_list, SIGNAL(currentChanged(QListViewItem *)),
	   this, SLOT(slotItemSelected(QListViewItem *)));
     connect(page_list, SIGNAL(selectionChanged(QListViewItem *)),
	   this, SLOT(slotItemSelected(QListViewItem *)));
     connect(page_list, SIGNAL(doubleClicked(QListViewItem *)),
	   this, SLOT(slotItemDblClicked(QListViewItem *)));
     connect(page_list, SIGNAL(rightButtonPressed(QListViewItem *, const QPoint &, int)),
	   this, SLOT(slotRightButtonPressed(QListViewItem *, const QPoint &, int)));
     connect(page_info_butt, SIGNAL(clicked(void)), this, SLOT(slotShowInfo(void)));
     connect(page_goto_butt, SIGNAL(clicked(void)), this, SLOT(slotGotoPage(void)));
   }
   if (thumb_list)
   {
      connect(thumb_list, SIGNAL(currentChanged(QListViewItem *)),
	      this, SLOT(slotItemSelected(QListViewItem *)));
      connect(thumb_list, SIGNAL(selectionChanged(QListViewItem *)),
	      this, SLOT(slotItemSelected(QListViewItem *)));
      connect(thumb_list, SIGNAL(doubleClicked(QListViewItem *)),
	      this, SLOT(slotItemSelected(QListViewItem *)));
      connect(thumb_info_butt, SIGNAL(clicked(void)), this, SLOT(slotShowInfo(void)));
   }
   if (file_list)
   {
      connect(file_list, SIGNAL(currentChanged(QListViewItem *)),
	      this, SLOT(slotItemSelected(QListViewItem *)));
      connect(file_list, SIGNAL(selectionChanged(QListViewItem *)),
	      this, SLOT(slotItemSelected(QListViewItem *)));
      connect(file_list, SIGNAL(doubleClicked(QListViewItem *)),
	      this, SLOT(slotItemSelected(QListViewItem *)));
      connect(file_info_butt, SIGNAL(clicked(void)), this, SLOT(slotShowInfo(void)));
   }
   connect(&port, SIGNAL(sigNotifyError(const GP<DjVuPort> &, const GUTF8String &)),
	   this, SLOT(slotNotifyError(const GP<DjVuPort> &, const GUTF8String &)));
   connect(&port, SIGNAL(sigNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)),
	   this, SLOT(slotNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)));

   DjVuPort::get_portcaster()->add_route(doc, port.getPort());

   QeDialog::makeTransient(this, parent);
}

