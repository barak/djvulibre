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
   QDDocNameDialog(const char * message, const char * doc_name,
		   QWidget * parent=0, const char * name=0);
   ~QDDocNameDialog(void) {};
};

void
QDDocNameDialog::slotBrowse(void)
{
   GUTF8String doc_full_name=GStringFromQString(text->text());
   QFileInfo fi=QFileInfo(QStringFromGString(doc_full_name));
   QString doc_dir=fi.dirPath();
   if (!QFileInfo(doc_dir).isDir()) doc_dir=QeFileDialog::lastSaveDir;
   if (!QFileInfo(doc_dir).isDir()) doc_dir=QDir::currentDirPath();
   doc_full_name=GURL::expand_name(GOS::basename(doc_full_name), doc_dir);
   
   static char * filters[]={ "*.djvu", "*.djv", "All files (*)", 0 };
   QeFileDialog fd(doc_dir, filters[0], this, "djvu_fd", TRUE);
   fd.setFilters((const char **) filters);
   fd.setCaption("Select DjVu document file name...");
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
	 GUTF8String mesg="File '"+fname+"' already exists.\n"
				  "Are you sure you want to overwrite it?\n";
	 if (QMessageBox::warning(this, "File already exists",
				  QStringFromGString(mesg),
				  "&Yes", "&No", 0, 0, 1)==1)
         {
           return;
         }
      } catch(...) {}

	 // Try to create the named file.
      try
      {
	 GUTF8String fname=GStringFromQString(text->text());
	 GP<ByteStream> str=ByteStream::create(GURL::Filename::UTF8(fname),"a");
      } catch(const GException & exc)
      {
         QString qmesg(exc.get_cause());
	 showError(this, "DjVu Error", qmesg);
	 return;
      }
   }
   QeDialog::done(rc);
}

QDDocNameDialog::QDDocNameDialog(const char * message, const char * doc_name,
				 QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   QWidget * start=startWidget();

   setCaption("Document file name");

   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 5, "vlay");

   QLabel * label=new QLabel(message, start, "message_label");
   vlay->addWidget(label);
   label->setMaximumHeight(label->sizeHint().height());

   QHBoxLayout * hlay=new QHBoxLayout(vlay);

   label=new QLabel("File name: ", start, "fname_label");
   hlay->addWidget(label);
   label->setMaximumHeight(label->sizeHint().height());

   text=new QLineEdit(start, "text");
   text->setText(doc_name);
   hlay->addWidget(text, 1);

   QPushButton * browse_butt=new QPushButton("&Browse", start, "browse_butt");
   hlay->addWidget(browse_butt);
   browse_butt->setMaximumHeight(browse_butt->sizeHint().height());
   
   QHBoxLayout * butt_lay=new QHBoxLayout(vlay);
   butt_lay->addStretch(1);
   QPushButton * ok_butt=new QPushButton("&OK", start, "ok_butt");
   butt_lay->addWidget(ok_butt);
   QPushButton * cancel_butt=new QPushButton("&Cancel", start, "cancel_butt");
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
   QDSavedFilesDialog(const GP<DjVuDocument> & doc, const char * dir_name,
		      const char * doc_name, QWidget * parent=0, const char * name=0);
   ~QDSavedFilesDialog(void) {};
};

QDSavedFilesDialog::QDSavedFilesDialog(const GP<DjVuDocument> & doc,
				       const char * dir_name,
				       const char * doc_name,
				       QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   DEBUG_MSG("QDSavedFilesDialog::QDSavedFilesDialog(): Creating files list...\n");
   DEBUG_MAKE_INDENT(3);

   setCaption("Files list to save");

   QWidget * start=startWidget();

   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 0, "vlay");
   QString message = tr("This will create the following files");
   if (doc->get_doc_type()==DjVuDocument::OLD_BUNDLED ||
       doc->get_doc_type()==DjVuDocument::OLD_INDEXED)
     message = message + tr(" (plus additional included files)");
   message = message + tr(" into the directory '") + dir_name + "'.";
   QLabel *label = new QLabel(message, start);
   label->setAlignment(AlignLeft | WordBreak);
   vlay->addWidget(label);
   vlay->addStrut(300);

   label=new QLabel("Are you sure you want to do it?\n", start);
   vlay->addWidget(label);

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
   vlay->addWidget(rc);
   vlay->addSpacing(10);
   QHBoxLayout * butt_lay=new QHBoxLayout(vlay, 5);
   butt_lay->addStretch(1);
   QPushButton * yes_butt=new QPushButton("&Yes", start, "yes_butt");
   butt_lay->addWidget(yes_butt);
   QPushButton * no_butt=new QPushButton("&No", start, "no_butt");
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
   ::showError(parent, "DjVu Error", QStringFromGString(msg));
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
	 switch(QMessageBox::information(parent, "File format",
					 "Would you like to save the document in\n"
					 "BUNDLED format (one file) or in INDIRECT\n"
					 "format (many files: ideal for web browsing)?\n",
					 "&Bundled", "&Indirect", "&Cancel", 0, 2))
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
      doc_full_name=GURL::expand_name(doc_name, doc_dir);

      GUTF8String message;
      if (saveas_bundled) message="Please enter the document file name in the field below.";
      else message="Please enter the name of the top-level file in the field below.\n"
		   "All other files will be saved in the same directory.";
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
           showError(parent, "DjVu Error",
           QString("The chosen name coincides with the name of\n"
             "one of the files composing this multipage document.\n"
             "\n"
             "Please try again."));
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
	 
	 progress_dialog=new QProgressDialog("Please wait while the file is being downloaded...        ",
					     "&Cancel", 100, parent, "progress_dialog", FALSE);
	 progress_dialog->setCaption("Please wait");
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
			  "Data stream from Netscape has been stopped.");
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

	 progress_dialog=new QProgressDialog("Please wait while all pages are being loaded...        ",
					     "&Cancel", 100, parent, "progress_dialog", FALSE);
	 progress_dialog->setCaption("Please wait");
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
	    QDSavedFilesDialog sd(doc, doc_dir, doc_name,
				  parent, "saved_files_dialog");
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
