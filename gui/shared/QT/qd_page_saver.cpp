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
// $Id: qd_page_saver.cpp,v 1.13 2007/03/25 20:48:25 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qd_page_saver.h"
#include "GOS.h"
#include "ByteStream.h"
#include "DataPool.h"
#include "qlib.h"
#include "debug.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qlistbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>




//***************************************************************************
//*************************** QDFileFormatDialog dialog *********************
//***************************************************************************

class QDFileFormatDialog : public QeDialog
{
   Q_OBJECT
private:
   QRadioButton	* separate_butt;
   QRadioButton	* bundled_butt;
   QRadioButton	* merged_butt;
public:
   enum { BUNDLED, SEPARATE, MERGED };

   int		format(void) const;
   
   QDFileFormatDialog(QWidget * parent=0, const char * name=0);
   ~QDFileFormatDialog(void) {}
};

int
QDFileFormatDialog::format(void) const
{
   return
      separate_butt->isChecked() ? SEPARATE :
      bundled_butt->isChecked() ? BUNDLED : MERGED;
}

QDFileFormatDialog::QDFileFormatDialog(QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   QWidget * start=startWidget();
   
   setCaption(tr("DjVu page file format"));

   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 5, "vlay");

   QLabel * label=new QLabel(tr("It seems that contents of this page are "
                                "scattered over more than one file.\n"
			        "You have the following options:"), start);
   label->setMinimumWidth(300);
   label->setAlignment(WordBreak);
   vlay->addWidget(label);
   
   vlay->addSpacing(10);

   QGridLayout * glay=new QGridLayout(3, 2, 10);
   vlay->addLayout(glay);
   glay->setColStretch(1, 1);
   
   separate_butt=new QRadioButton("", start, "separate_butt");
   separate_butt->setChecked(FALSE);
   glay->addWidget(separate_butt, 0, 0);
   label=new QLabel(tr("Create all these files. This is useful if you plan to "
                       "save several page to insert them into another document. "
                       "The shared files will remain shared."),
                    start, "separate_label");
   label->setAlignment(WordBreak);
   glay->addWidget(label, 0, 1);

   bundled_butt=new QRadioButton("", start, "bundled_butt");
   bundled_butt->setChecked(TRUE);
   glay->addWidget(bundled_butt, 1, 0);
   label=new QLabel(tr("Pack all these files into one bundle "
                       "(so called BUNDLED format). This is convenient because "
                       "you will have only one file and will still be able "
                       "to split it into many when necessary."), 
                    start, "bundled_label");
   label->setAlignment(WordBreak);
   glay->addWidget(label, 1, 1);

   merged_butt=new QRadioButton("", start, "merged_butt");
   merged_butt->setChecked(FALSE);
   glay->addWidget(merged_butt, 2, 0);
   label=new QLabel(tr("Merge chunks from all files and store them "
                       "into one file. Use this if you need the simplest "
                       "structure of the file and you do not plan to separate "
                       "the chunks again."),
                    start, "merged_label");
   label->setAlignment(WordBreak);
   glay->addWidget(label, 2, 1);

   QButtonGroup * grp=new QButtonGroup(start, "buttongroup");
   grp->hide();
   grp->insert(separate_butt);
   grp->insert(bundled_butt);
   grp->insert(merged_butt);

   vlay->addSpacing(15);
   
   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QPushButton * ok_butt=new QPushButton(tr("&OK"), start, "ok_butt");
   butt_lay->addWidget(ok_butt);
   QPushButton * cancel_butt=new QPushButton(tr("&Cancel"), start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);
   ok_butt->setDefault(TRUE);
   
      // Connecting signals and slots
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
}

//***************************************************************************
//*************************** QDFilesListDialog dialog **********************
//***************************************************************************

class QDFilesListDialog : public QeDialog
{
   Q_OBJECT
private:
public:
   QDFilesListDialog(const GP<DjVmDoc> & doc, const QString & dname,
		     QWidget * parent=0, const char * name=0);
   ~QDFilesListDialog(void) {}
};

QDFilesListDialog::QDFilesListDialog(const GP<DjVmDoc> & doc, 
                                     const QString & dname,
				     QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   QWidget * start=startWidget();
   
   setCaption(tr("DjVu: File list"));

   QVBoxLayout *vlay=new QVBoxLayout(start, 10, 5, "vlay");

   QLabel *label=new QLabel(tr("The following files will be "
                               "created in directory '%1'.")
                            .arg(dname), 
                            start);
   label->setMinimumWidth(300);
   label->setAlignment(WordBreak);
   vlay->addWidget(label);
   
   QListBox * rc = new QListBox(start, "files_list");
   rc->setSelectionMode(QListBox::NoSelection);
   rc->setColumnMode(QListBox::FitToWidth);
   GP<DjVmDir> dir = doc->get_djvm_dir();
   GPList<DjVmDir::File> files_list = dir->get_files_list();
   for(GPosition pos=files_list;pos;++pos)
     rc->insertItem(QStringFromGString(files_list[pos]->get_save_name()));
   vlay->addSpacing(10);
   vlay->addWidget(rc);
   
   QLabel *label2 = new QLabel(tr("Do you want to proceed?"),start);
   vlay->addSpacing(10);
   vlay->addWidget(label2);
   vlay->addSpacing(10);
   
   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QPushButton * ok_butt=new QPushButton(tr("&Yes"), start, "ok_butt");
   butt_lay->addWidget(ok_butt);
   QPushButton * cancel_butt=new QPushButton(tr("&No"), start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);
   ok_butt->setDefault(TRUE);
   
      // Connecting signals and slots
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
}

//****************************************************************************
//******************************* QDPageSaver ********************************
//****************************************************************************

static bool
addToDjVm(const GP<DjVmDoc> & doc, const GP<DjVuFile> & file,
	  bool page, GMap<GURL, void *> & map)
      // Will return TRUE if the file has been added to DjVmDoc
{
   GURL url=file->get_url();

   if (!map.contains(url))
   {
      map[url]=0;

      if (!file->contains_chunk("NDIR"))
      {
	 GP<DataPool> data=file->get_djvu_data(false, true);
      
	 GPList<DjVuFile> files_list=file->get_included_files(false);
	 for(GPosition pos=files_list;pos;++pos)
	 {
	    GP<DjVuFile> f=files_list[pos];
	    if (!addToDjVm(doc, f, false, map))
	       data=DjVuFile::unlink_file(data, f->get_url().fname());
	 }

	 GUTF8String name=url.fname();
	 GP<DjVmDir::File> frec=DjVmDir::File::create(name, name, name,
						      page ? DjVmDir::File::PAGE :
						      DjVmDir::File::INCLUDE);
	 doc->insert_file(frec, data, -1);
	 return true;
      }
   }
   return false;
}

GP<DjVmDoc>
QDPageSaver::getDjVmDoc(void)
      // Will create a DJVM BUNDLED document with the page contents, but
      // without NDIR chunks
{
   GMap<GURL, void *> map;
   GP<DjVmDoc> doc=DjVmDoc::create();
   addToDjVm(doc, djvu_file, true, map);
   return doc;
}

static QString
getDir(GUTF8String name)
{
   QString dir = QStringFromGString(name);
   while(!QFileInfo(dir).isDir()) dir=QFileInfo(dir).dirPath();
   return dir;
}

void
QDPageSaver::saveSeparate(void)
{
   GURL file_url=djvu_file->get_url();

   QString save_dir=QeFileDialog::lastSaveDir;
   if (!QFileInfo(save_dir).isDir())
      if (file_url.is_local_file_url())
	 save_dir=getDir(file_url.UTF8Filename());
   if (!QFileInfo(save_dir).isDir()) save_dir=QDir::currentDirPath();
   
   QeFileDialog fd(save_dir, "*", parent, "djvu_fd", TRUE);
   fd.setCaption(QeFileDialog::tr("Select directory for page files..."));
   fd.setMode(QeFileDialog::Directory);
     
   if (fd.exec()==QDialog::Accepted)
   {
      QeFileDialog::lastSaveDir=save_dir;
      GP<DjVmDoc> doc=getDjVmDoc();
      QDFilesListDialog dlg(doc, fd.selectedFile(), parent);
      GUTF8String idxname = "index.djvu";
      if (dlg.exec()==QDialog::Accepted)
        doc->expand(GURL::Filename::UTF8(GStringFromQString(fd.selectedFile())), 
                    idxname);
   }
}

void
QDPageSaver::saveBundled(void)
{
   const char * filters[]={ "*.djvu", "*.djv", 0, 0 };
   QString filter2 = tr("All files (*)");
   filters[2] = filter2;
 
   GURL file_url=djvu_file->get_url();

   QString save_dir=QeFileDialog::lastSaveDir;
   if (!QFileInfo(save_dir).isDir())
      if (file_url.is_local_file_url())
	 save_dir=getDir(file_url.UTF8Filename());
   if (!QFileInfo(save_dir).isDir()) save_dir=QDir::currentDirPath();
   
   QeFileDialog fd(save_dir, filters[0], parent, "djvu_fd", TRUE);
   fd.setFilters((const char **) filters);
   fd.setCaption(QeFileDialog::tr("Select a name for the DjVu page..."));
   {
     GUTF8String gfname=GURL::expand_name(file_url.fname(),
                                          GStringFromQString(save_dir));
     fd.setSelection(QStringFromGString(gfname));
   }
      
   if (fd.exec()==QDialog::Accepted)
   {
      QeFileDialog::lastSaveDir=save_dir;
      GP<DjVmDoc> doc=getDjVmDoc();
      GURL selected=GURL::Filename::UTF8(GStringFromQString(fd.selectedFile()));
      DataPool::load_file(selected);
      GP<ByteStream> str_out=ByteStream::create(selected, "wb");
      doc->write(str_out);
   }
}

void
QDPageSaver::saveMerged(void)
{
   static const char* filters[]={ "*.djvu", "*.djv", 0, 0 };
   QString filter2 = tr("All files (*)");
   filters[2] = filter2;
 
   GURL file_url=djvu_file->get_url();

   QString save_dir=QeFileDialog::lastSaveDir;
   if (!QFileInfo(save_dir).isDir())
      if (file_url.is_local_file_url())
	 save_dir=getDir(file_url.UTF8Filename());
   if (!QFileInfo(save_dir).isDir()) save_dir=QDir::currentDirPath();
   
   QeFileDialog fd(save_dir, filters[0], parent, "djvu_fd", TRUE);
   fd.setFilters((const char **) filters);
   fd.setCaption(QeFileDialog::tr("Select a name for the DjVu page..."));
   {
     GUTF8String gfname=GURL::expand_name(file_url.fname(),
                                          GStringFromQString(save_dir));
     fd.setSelection(QStringFromGString(gfname));
   }
      
   if (fd.exec()==QDialog::Accepted)
   {
      QeFileDialog::lastSaveDir=save_dir;
      GP<DataPool> data=djvu_file->get_djvu_data(true, true);
      GP<ByteStream> str_in=data->get_stream();

      GURL selected=GURL::Filename::UTF8(GStringFromQString(fd.selectedFile()));
      DataPool::load_file(selected);
      GP<ByteStream> gstr_out=ByteStream::create(selected,"wb");
      ByteStream &str_out=*gstr_out;
      static char sample[]="AT&T";
      char buffer[5];
      data->get_data(buffer, 0, 4);
      if (memcmp(buffer, sample, 4))
	 str_out.writall("AT&T", 4);
      str_out.copy(*str_in);
   }
}

static int
getFilesNum(const GP<DjVuFile> & file, GMap<GURL, void *> & map)
      // Will return the number of files (starting from 'file' and
      // including 'file'), which do not contain NDIR chunk.
{
   int cnt=0;
   
   GURL url=file->get_url();

   if (!map.contains(url))
   {
      map[url]=0;

      if (!file->contains_chunk("NDIR"))
      {
	 cnt++;
	 GPList<DjVuFile> files_list=file->get_included_files(false);
	 for(GPosition pos=files_list;pos;++pos)
	    cnt+=getFilesNum(files_list[pos], map);
      }
   }

   return cnt;
}

int
QDPageSaver::getFilesNum(void)
      // Will return the number of files w/o NDIR chunk. Basically, these
      // are the files, which need to be saved.
{
   GMap<GURL, void *> map;
   return ::getFilesNum(djvu_file, map);
}

void
QDPageSaver::save(void)
{
   DEBUG_MSG("QDPageSaver::save(): saving the stuff\n");
   DEBUG_MAKE_INDENT(3);

   if (!djvu_file->is_all_data_present())
     {
       QCString msg = tr("Cannot save the page because data is missing.").utf8();
       G_THROW((const char*)msg);
     }
   int format=QDFileFormatDialog::MERGED;

      // First see if we have a choice of formats...
   if (getFilesNum()>1)
   {
      QDFileFormatDialog dlg(parent);
      if (dlg.exec()!=QDialog::Accepted) return;
      format=dlg.format();
   }

      // Now do the actual saving...
   switch(format)
   {
      case QDFileFormatDialog::SEPARATE:
	 saveSeparate();
	 break;

      case QDFileFormatDialog::BUNDLED:
	 saveBundled();
	 break;

      case QDFileFormatDialog::MERGED:
	 saveMerged();
	 break;
   }
}

QDPageSaver::QDPageSaver(const GP<DjVuFile> & _djvu_file, QWidget * _parent) :
      djvu_file(_djvu_file), parent(_parent)
{
}

#include "qd_page_saver_moc.inc"
