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
// $Id: qd_doc_saver.cpp,v 1.14 2007/03/25 20:48:25 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qd_doc_saver.h"
#include "qlib.h"
#include "debug.h"
#include "GOS.h"
#include "ByteStream.h"
#include "DataPool.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qlistbox.h>
#include <qlineedit.h>



//***************************************************************************
//*************************** QDDocNameDialog dialog ************************
//***************************************************************************

class QDDocNameDialog : public QeDialog
{
   Q_OBJECT
private:
   QLineEdit	* text;
protected:
   virtual void	done(int);
protected slots:
   void		slotBrowse(void);
public:
   GUTF8String	docName(void) const { return GStringFromQString(text->text()); }
   QDDocNameDialog(QString message, GUTF8String doc_name,
		   QWidget * parent=0, const char * name=0);
   ~QDDocNameDialog(void) {};
};

void
QDDocNameDialog::slotBrowse(void)
{
   static const char * filters[]={ "*.djvu", "*.djv", 0, 0 };
   QString filter2 = tr("All files (*)");
   filters[2] = filter2;

   GUTF8String doc_full_name=GStringFromQString(text->text());
   QFileInfo fi=QFileInfo(QStringFromGString(doc_full_name));
   QString doc_dir=fi.dirPath();
   if (!QFileInfo(doc_dir).isDir()) doc_dir=QeFileDialog::lastSaveDir;
   if (!QFileInfo(doc_dir).isDir()) doc_dir=QDir::currentDirPath();
   doc_full_name=GURL::expand_name(GOS::basename(doc_full_name), 
                                   GStringFromQString(doc_dir));
   QeFileDialog fd(doc_dir, filters[0], this, "djvu_fd", TRUE);
   fd.setFilters((const char **) filters);
   fd.setCaption(tr("Select DjVu document file name..."));
   fd.setSelection(QStringFromGString(doc_full_name));
      
   if (fd.exec()==QDialog::Accepted) text->setText(fd.selectedFile());
}

void
QDDocNameDialog::done(int rc)
{
   if (rc==Accepted)
   {
	 // See if the output file exists.
      try
      {
	 GUTF8String fname=GStringFromQString(text->text());
	 GP<ByteStream> str=ByteStream::create(GURL::Filename::UTF8(fname),"rb");
	 char ch;
	 str->read(&ch, 1);
	 if (QMessageBox::warning(this, "File already exists",
                                  tr("File '%1' already exists.\n"
                                     "Are you sure you want to overwrite it?\n")
                                  .arg( QStringFromGString(fname)),
				  tr("&Yes"), tr("&No"), 0, 0, 1)==1)
         {
           return;
         }
      } catch(...) {}

	 // Try to create the named file.
      try
      {
	 GUTF8String fname=GStringFromQString(text->text());
	 GP<ByteStream> str=ByteStream::create(GURL::Filename::UTF8(fname),"a");
      } 
      catch(const GException & exc)
      {
         QString qmesg(exc.get_cause());
	 showError(this, "DjVu Error", qmesg);
	 return;
      }
   }
   QeDialog::done(rc);
}

QDDocNameDialog::QDDocNameDialog(QString message, GUTF8String doc_name,
				 QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   QWidget * start=startWidget();

   setCaption(tr("DjVu document file name"));

   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 5, "vlay");

   QLabel * label=new QLabel( message, start, "message_label");
   vlay->addWidget(label);
   label->setMaximumHeight(label->sizeHint().height());

   QHBoxLayout * hlay=new QHBoxLayout(vlay);

   label=new QLabel(tr("File name: "), start, "fname_label");
   hlay->addWidget(label);
   label->setMaximumHeight(label->sizeHint().height());

   text=new QLineEdit(start, "text");
   text->setText(QStringFromGString(doc_name));
   hlay->addWidget(text, 1);

   QPushButton * browse_butt=new QPushButton(tr("&Browse"), start, "browse_butt");
   hlay->addWidget(browse_butt);
   browse_butt->setMaximumHeight(browse_butt->sizeHint().height());
   
   QHBoxLayout * butt_lay=new QHBoxLayout(vlay);
   butt_lay->addStretch(1);
   QPushButton * ok_butt=new QPushButton(tr("&OK"), start, "ok_butt");
   butt_lay->addWidget(ok_butt);
   QPushButton * cancel_butt=new QPushButton(tr("&Cancel"), start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);
   ok_butt->setDefault(TRUE);
   ok_butt->setMaximumHeight(ok_butt->sizeHint().height());
   cancel_butt->setMaximumHeight(cancel_butt->sizeHint().height());
   
      // Connecting signals and slots
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
   connect(browse_butt, SIGNAL(clicked(void)), this, SLOT(slotBrowse(void)));
}

//***************************************************************************
//************************ QDSavedFilesDialog dialog ************************
//***************************************************************************

class QDSavedFilesDialog : public QeDialog
{
   Q_OBJECT
public:
   QDSavedFilesDialog(const GP<DjVuDocument> & doc, 
                      GUTF8String dir_name, GUTF8String doc_name, 
                      QWidget * parent=0, const char * name=0);
   ~QDSavedFilesDialog(void) {};
};

QDSavedFilesDialog::QDSavedFilesDialog(const GP<DjVuDocument> & doc,
                                       GUTF8String dir_name, 
                                       GUTF8String doc_name, 
				       QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   DEBUG_MSG("QDSavedFilesDialog::QDSavedFilesDialog(): Creating files list...\n");
   DEBUG_MAKE_INDENT(3);

   setCaption("Files list to save");

   QWidget * start=startWidget();

   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 0, "vlay");
   QString message;
   if (doc->get_doc_type()==DjVuDocument::OLD_BUNDLED ||
       doc->get_doc_type()==DjVuDocument::OLD_INDEXED   )
     message = tr("This will create the following files "
                  "(plus additional included files) "
                  "into the directory '%1'.");
   else
     message = tr("This will create the following files "
                  "into the directory '%1'.");

   message = message.arg(QStringFromGString(dir_name));
   QLabel *label = new QLabel(message, start);
   label->setAlignment(AlignLeft | WordBreak);
   vlay->addWidget(label);
   vlay->addStrut(300);

   QListBox * rc = new QListBox(start, "files_list");
   rc->setSelectionMode(QListBox::NoSelection);
   rc->setColumnMode(QListBox::FitToWidth);
   if (doc->get_doc_type()==DjVuDocument::OLD_BUNDLED ||
       doc->get_doc_type()==DjVuDocument::OLD_INDEXED)
     {
       GURL dir_url=GURL::Filename::UTF8(dir_name);
       for(int page=0;page<doc->get_pages_num();page++)
	 {
	   GURL url=GURL::UTF8(doc->page_to_url(page).name(),dir_url);
	   GUTF8String gfname=url.fname();
	   rc->insertItem(QStringFromGString(gfname));
	 }
     } 
   else
     {
       GP<DjVmDir> dir=doc->get_djvm_dir();
       GPList<DjVmDir::File> flist=dir->get_files_list();
       for(GPosition pos=flist;pos;++pos)
	 {
	   GUTF8String gfname=flist[pos]->get_save_name();
	   rc->insertItem(QStringFromGString(gfname));
	 }
     }
   vlay->addSpacing(10);
   vlay->addWidget(rc);

   label=new QLabel(tr("Do you want to proceed?"), start);
   vlay->addSpacing(10);
   vlay->addWidget(label);
   vlay->addSpacing(10);

   QHBoxLayout * butt_lay=new QHBoxLayout(vlay, 5);
   butt_lay->addStretch(1);
   QPushButton * yes_butt=new QPushButton(tr("&Yes"), start, "yes_butt");
   butt_lay->addWidget(yes_butt);
   QPushButton * no_butt=new QPushButton(tr("&No"), start, "no_butt");
   butt_lay->addWidget(no_butt);
   yes_butt->setDefault(TRUE);
   
      // Connecting signals and slots
   connect(yes_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(no_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
}

//************************** QDPort connections ******************************

void
QDDocSaver::slotNotifyError(const GP<DjVuPort> &, const GUTF8String &msg)
{
   ::showError(parent, tr("DjVu Error"), QStringFromGString(msg));
}

void
QDDocSaver::slotNotifyFileFlagsChanged(const GP<DjVuFile> & file, long set_mask, long)
{
  if (set_mask & DjVuFile::ALL_DATA_PRESENT)
    {
      for(int comp=0;comp<comp_files.size();comp++) {
        if (comp_files[comp] == file && !comp_done[comp]) {
          comp_done[comp] = true;
          if (++done_comps < comp_files.size())  
            preloadNextPage();
          progress_dialog->setProgress(done_comps*100/comp_files.size());
          break;
        }
      }
    }
}

//***************************** QDDocSaver ********************************

void
QDDocSaver::preloadNextPage(void)
{
   DEBUG_MSG("QDDocSaver::preloadNextPage(): searching next page to preload...\n");
   DEBUG_MAKE_INDENT(3);

   if (!progress_dialog || !progress_dialog->wasCancelled())
      for(int comp=0;comp<comp_files.size();comp++)
	 if (!comp_files[comp])
	 {
	    GP<DjVuFile> file=doc->get_djvu_file(comp_ids[comp]);
	    comp_files[comp]=file;
	    if (file->is_all_data_present())
	       port.notify_file_flags_changed(file, DjVuFile::ALL_DATA_PRESENT, 0);
	    return;
	 }
}

void
QDDocSaver::save(void)
{
   DEBUG_MSG("QDDocSaver::save(): saving the stuff\n");
   DEBUG_MAKE_INDENT(3);

   try
   {
      if (doc->get_pages_num()>1)
      {
	 switch(QMessageBox::information(parent, tr("File format"),
		     tr( "Would you like to save the document in\n"
                         "BUNDLED format (one file) or in INDIRECT\n"
                         "format (many files: ideal for web browsing)?\n"),
		     tr("&Bundled"), 
                     tr("&Indirect"), 
                     tr("&Cancel"), 0, 2))
           {
	    case 0: saveas_bundled=true; break;
	    case 1: saveas_bundled=false; break;
	    default: return;
	 }
      } else saveas_bundled=true;

      QString doc_dir;
      GUTF8String doc_name, doc_full_name;
      GURL doc_url=doc->get_init_url();
      doc_dir=QeFileDialog::lastSaveDir;
      if (!QFileInfo(doc_dir).isDir() && doc_url.is_local_file_url())
	 doc_dir=QFileInfo(QStringFromGString(doc_url.UTF8Filename())).dirPath();
      if (!QFileInfo(doc_dir).isDir()) doc_dir=QDir::currentDirPath();
      if (doc->get_doc_type()==DjVuDocument::OLD_INDEXED)
      {
        doc_name="document.djvu";
      }else 
      {
        doc_name=doc_url.fname();
        if(doc->needs_rename())
        {
          int i=doc_name.rsearch('.');
          if(i<1)
          {
            i=doc_name.length();
          }
          doc_name=doc_name.substr(0,i)+".djvu";
        }
      }
      doc_full_name=GURL::expand_name(doc_name, GStringFromQString(doc_dir));

      QString message;
      if (saveas_bundled) 
        message=tr("Please enter the document file name.");
      else 
        message=tr("Please enter the name of the main document file.\n"
                   "All other files will be saved in the same directory.");
      bool conflict=false;
      do
      {
	 QDDocNameDialog dnd(message, doc_full_name, parent, "doc_name_dialog");
	 if (dnd.exec()!=QDialog::Accepted) return;
	 doc_full_name=dnd.docName();
	 doc_dir=QFileInfo(QStringFromGString(doc_full_name)).dirPath();
	 doc_name=GOS::basename(doc_full_name);
	 QeFileDialog::lastSaveDir=doc_dir;
	 if (!saveas_bundled)
	 {
           GP<DjVmDir> dir = doc->get_djvm_dir();
           if (dir)
           {
             GPList<DjVmDir::File> flist = dir->get_files_list();
             for(GPosition pos=flist; pos && !conflict; ++pos)
             {
               conflict=(flist[pos]->get_save_name() == doc_name);
             }
           } else
           {
             int pages_num=doc->get_pages_num();
             for(int page_num=0;page_num<pages_num&&!conflict;)
             {
               GUTF8String name=doc->page_to_url(page_num++).fname();
               conflict = (name==doc_name);
             }
           }
         }
         if (conflict)
         {
           showError(parent, tr("DjVu Error"),
                     tr("The chosen name coincides with the name of\n"
                        "one of the files composing this multipage document.\n"
                        "Please choose another name."));
         }
      } while(conflict); // IS THIS REALLY WHAT IS NEEDED HERE????

      bool save_doc=false;
      if (doc->is_bundled())
      {
	 DEBUG_MSG("the document is bundled => wait till download finishes\n");

	    // We will use the capability of DataPool to interpret
	    // IFF structure and estimate the size of the data chunk.
	    // Any bundled DjVu file is supposed to be IFF so we can use it
	    // to estimate %% completed
	 GP<DataPool> data_pool=doc->get_init_data_pool();
	 
	 progress_dialog=new QProgressDialog(tr("Please wait while the file "
                                                "is being downloaded...        "),
					     tr("&Cancel"), 100, parent, 
                                             "progress_dialog", FALSE);
	 progress_dialog->setCaption(tr("Please wait"));
	 progress_dialog->setProgress(0);
	 progress_dialog->setMinimumDuration(0);
	 progress_dialog->show();
	 QeDialog::makeTransient(progress_dialog, parent);

	    // Wait until either all data is here or "Cancel" is pressed
	 while(!progress_dialog->wasCancelled())
	 {
	    qApp->processEvents(100);
	    int progress=100*data_pool->get_size()/data_pool->get_length();
	    if (progress==100) break;
	    progress_dialog->setProgress(progress);

	    if (data_pool->is_eof())
	    {
	       ::showInfo(parent, "DjVu",
			  tr("Data stream from Netscape has been stopped."));
	       break;
	    }
	 }
	 progress_dialog->setProgress(100);
	 
	 save_doc=(data_pool->get_size()>=data_pool->get_length());
      } else
      {
	 done_comps=0;
	 for(int comp=0;comp<comp_files.size();comp++) {
            comp_files[comp]=0;
	    comp_done[comp]=false;
	 }

	 DjVuPort::get_portcaster()->add_route(doc, port.getPort());
	 for(int i=0;i<4;i++) preloadNextPage();

	 progress_dialog=new QProgressDialog(tr("Please wait while all pages "
                                                "are being loaded...        "),
					     tr("&Cancel"), 100, parent, 
                                             "progress_dialog", FALSE);
	 progress_dialog->setCaption(tr("Please wait"));
	 progress_dialog->setMinimumDuration(0);
	 progress_dialog->setProgress(0);
	 QeDialog::makeTransient(progress_dialog, parent);

	 while(!progress_dialog->wasCancelled() &&
	       done_comps<comp_files.size())
	    qApp->processOneEvent();

	 progress_dialog->setProgress(100);
	    
	 DjVuPort::get_portcaster()->del_route(doc, port.getPort());

	 save_doc=(done_comps==comp_files.size());
      }
      delete progress_dialog; progress_dialog=0;

      if (save_doc)
      {
	    // Finally - saving the document
	 if (!saveas_bundled)
	 {
	    // Last question before we overwrite everything...
	    QDSavedFilesDialog sd(doc, GStringFromQString(doc_dir), 
                                  doc_name, parent, "saved_files_dialog");
	    if (sd.exec()!=QDialog::Accepted) return;
	 }
	 doc->save_as(GURL::Filename::UTF8(doc_full_name), saveas_bundled);
      }
   } catch(const GException & exc)
   {
      ::showError(parent, exc);
      delete progress_dialog; progress_dialog=0;
   }
}

QDDocSaver::QDDocSaver(const GP<DjVuDocument> & _doc, QWidget * _parent) :
      parent(_parent), doc(_doc), port(1, 0)
{
   DEBUG_MSG("QDDocSaver::QDDocSaver(): Creating 'Doc Info' dialog...\n");
   DEBUG_MAKE_INDENT(3);

   progress_dialog=0;

   // note that OLD_BUNDLED, OLD_INDEXED and SINGLE_PAGE type
   // have no dir info - see get_djvm_dir() in DjVuDocument.h
   if (doc->get_doc_type()==DjVuDocument::BUNDLED ||
       doc->get_doc_type()==DjVuDocument::INDIRECT)
   {
      GP<DjVmDir> dir = doc->get_djvm_dir();
     // New formats may contain more than pages
     int comp = 0;
     int ncomp = dir->get_files_num();
     comp_ids.resize(ncomp - 1);
     comp_files.resize(ncomp - 1);
     comp_done.resize(ncomp - 1);
     GPList<DjVmDir::File> flist = dir->get_files_list();
     for(GPosition pos=flist; pos; ++pos, ++comp)
       comp_ids[comp] = flist[pos]->get_load_name();
   } else { 
     // Obsolete format can be only contain pages
     int ncomp = doc->get_pages_num();
     comp_ids.resize(ncomp - 1);
     comp_files.resize(ncomp - 1);
     comp_done.resize(ncomp - 1);
     for (int comp=0; comp<ncomp; comp++)
       comp_ids[comp] = GUTF8String(comp);
   }

   connect(&port, SIGNAL(sigNotifyError(const GP<DjVuPort> &, const GUTF8String &)),
           this, SLOT(slotNotifyError(const GP<DjVuPort> &, const GUTF8String &)));
   connect(&port, SIGNAL(sigNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)),
           this, SLOT(slotNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)));
}

#include "qd_doc_saver_moc.inc"
