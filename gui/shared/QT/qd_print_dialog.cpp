//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C- 
//C- DjVuLibre-3.5 is derived from the DjVu(r) Reference Library
//C- distributed by Lizardtech Software.  On July 19th 2002, Lizardtech 
//C- Software authorized us to replace the original DjVu(r) Reference 
//C- Library notice by the following text (see doc/lizard2002.djvu):
//C-
//C-  ------------------------------------------------------------------
//C- | DjVu (r) Reference Library (v. 3.5)
//C- | Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- | The DjVu Reference Library is protected by U.S. Pat. No.
//C- | 6,058,214 and patents pending.
//C- |
//C- | This software is subject to, and may be distributed under, the
//C- | GNU General Public License, Version 2. The license should have
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
// $Id$
// $Name$

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qd_print_dialog.h"
#include "qlib.h"
#include "debug.h"
#include "exc_msg.h"
#include "exc_misc.h"
#include "djvu_base_res.h"
#include "GOS.h"
#include "ByteStream.h"

#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qbuttongroup.h>
#include <qlayout.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qtooltip.h>
#include <qapplication.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qprogressbar.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qobjectlist.h>
#include <qtabwidget.h>

#include <unistd.h>
#include <errno.h>
#include <signal.h>

static const QString print_page_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","current page");
static const QString print_custom_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","custom pages");
static const QString print_doc_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","document");
static const QString print_win_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","window");

static const QString fit_page_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","Scale to Fit");
static const QString one_to_one_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","One to One");
static const QString current_zoom_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","Current Zoom");
static const QString custom_zoom_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","Custom Zoom:");

const QString
QDPrintDialog::id2str(int id)
{
   return
     id==PRINT_PAGE ? tr(print_page_str) :
     id==PRINT_CUSTOM ? tr(print_custom_str) :
     id==PRINT_DOC ? tr(print_doc_str) :
     id==PRINT_WIN ? tr(print_win_str) : 
     QString::null;
}

int
QDPrintDialog::str2id(const QString &str)
{
   return
     str==tr(print_page_str) ? PRINT_PAGE :
     str==tr(print_custom_str) ? PRINT_CUSTOM :
     str==tr(print_doc_str) ? PRINT_DOC :
     str==tr(print_win_str) ? PRINT_WIN 
     : -1;
}

void
QDPrintDialog::setPSFormat(bool ps)
{
   if (ps)
     ps_butt->setChecked(TRUE);
   else 
     eps_butt->setChecked(TRUE);
}

void
QDPrintDialog::setPortrait(bool portrait, bool autoorient)
{
  if (autoorient)
    autoorient_butt->setChecked(TRUE);
  else if (portrait) 
    portrait_butt->setChecked(TRUE);
  else 
    landscape_butt->setChecked(TRUE);
}

void
QDPrintDialog::setColorMode(bool color)
{
   if (color) 
     color_butt->setChecked(TRUE);
   else 
     grey_butt->setChecked(TRUE);
}

void
QDPrintDialog::setPSLevel(int level)
{
   if (level==3) 
     level3_butt->setChecked(TRUE);
   else if (level==2) 
     level2_butt->setChecked(TRUE);
   else 
     level1_butt->setChecked(TRUE);
}

void
QDPrintDialog::setZoom(int zoom)
{
  QString setting;
  if (zoom<0)
    setting= tr(fit_page_str);
  else if (zoom==100)
    setting= tr(one_to_one_str);
  else if (zoom==cur_zoom)
    setting = tr(current_zoom_str);
  else
    setting = tr(custom_zoom_str);
  setComboBoxCurrentItem(zoom_menu, setting);
}

void
QDPrintDialog::setCurZoom(int zoom)
{
  cur_zoom=zoom;
  if (zoom_menu->currentText()==tr(current_zoom_str))
    zoom_spin->setValue(cur_zoom);
}

void
QDPrintDialog::setPrint(What what)
{
  const QString str=id2str(what);
  setComboBoxCurrentItem(what_menu, str);
}

void
QDPrintDialog::setFileName(const QString &qname)
{
  file_text->setText(qname);
}

void
QDPrintDialog::setCommand(const QString &qcmd)
{
  printer_text->setText(qcmd);
}

void
QDPrintDialog::setBookMode(bool mode, int sign)
{
  bk_mode_butt->setChecked(mode);
  bk_sign_spin->setValue(sign);
}

void
QDPrintDialog::setBookTwo(bool two, int align, int fold, int thick)
{
  bk_two_butt->setChecked(two);
  bk_align_spin->setValue(align);
  bk_fold_spin->setValue(fold);
  bk_thick_spin->setValue(thick);
}

void
QDPrintDialog::printToFile(int file)
{
  if (file) 
    file_butt->setChecked(TRUE);
  else 
    printer_butt->setChecked(TRUE);
}

void
QDPrintDialog::slotBrowse(void)
{
  int ps_mode=ps_butt->isChecked();
  const char * filter=ps_mode ? "*.ps" : "*.eps";
  QeFileDialog dialog(QFileInfo(file_text->text()).dirPath(),
                      filter, this, "file_dialog", TRUE);
  dialog.setCaption(tr("Select output file name"));
  dialog.setForWriting(1);
  if (dialog.exec()==QDialog::Accepted)
    file_text->setText(dialog.selectedFile());
}

void
QDPrintDialog::slotWhatChanged(const QString & qtext)
{
  custompages_text->setEnabled(qtext==tr(print_custom_str));
}

void
QDPrintDialog::adjustWhat(void)
{
   bool force_one_copy=false;
   if (eps_butt->isChecked())
     force_one_copy=true;
   int id=-1;
   QString qtext;
   if (what_menu->count()>0)
     qtext = what_menu->currentText();
   id=str2id(qtext);
   what_menu->clear();
   what_menu->insertItem(tr(print_custom_str));
   what_menu->insertItem(tr(print_page_str));
   what_menu->insertItem(tr(print_doc_str));
   if (!print_rect.isempty())
     what_menu->insertItem(tr(print_win_str));
   if (id>=0)
     setPrint((What) id);
   else
     setPrint(PRINT_PAGE);
#ifndef QT1
   // This is needed to establish the proper geometry(!)
   what_menu->sizeHint();
   what_menu->updateGeometry();
#endif
   copies_spin->setEnabled(!force_one_copy);
   if (force_one_copy)
     copies_spin->setValue(1);
}

void
QDPrintDialog::slotFormatChanged(void)
{
   char buffer[MAXPATHLEN+1];
   strcpy(buffer, file_text->text());

   int ps_mode=ps_butt->isChecked();

   char * ptr=buffer, * dot;
   for(ptr=dot=buffer;*ptr;ptr++)
      if (*ptr=='.') dot=ptr;
   if (dot!=buffer)
   {
      *dot=0;
      if (ps_mode) strcat(buffer, ".ps");
      else strcat(buffer, ".eps");
      file_text->setText(buffer);
   }
   
   if (!ps_mode && landscape_butt->isChecked()) 
     autoorient_butt->setChecked(TRUE);
   if (!ps_mode && bk_mode_butt->isChecked())
     bk_mode_butt->setChecked(FALSE);
   portrait_butt->setEnabled(ps_mode);
   landscape_butt->setEnabled(ps_mode);
   autoorient_butt->setEnabled(ps_mode);
   zoom_menu->setEnabled(ps_mode);
   QString qtext = zoom_menu->currentText();
   zoom_spin->setEnabled(ps_mode && qtext==tr(custom_zoom_str));
   bk_mode_butt->setEnabled(ps_mode);
   adjustWhat();
}

void
QDPrintDialog::slotDstChanged(void)
{
   int printer=printer_butt->isChecked();
   dst_widget->raiseWidget(printer ? printer_widget : file_widget);
   eps_butt->setEnabled(!printer);
   ps_butt->setEnabled(!printer);
   if (printer) ps_butt->setChecked(TRUE);
}

void
QDPrintDialog::slotZoomChanged(const QString & qtext)
{
  int ps_mode=ps_butt->isChecked();
  zoom_spin->setEnabled(ps_mode && qtext==tr(custom_zoom_str));
  if (qtext==tr(one_to_one_str))
    zoom_spin->setValue(100);
  else if (qtext==tr(current_zoom_str))
    zoom_spin->setValue(cur_zoom);
}

void
QDPrintDialog::slotBookChanged(void)
{
  bool bmode = bk_mode_butt->isChecked();
  bool tmode = bk_two_butt->isChecked();
  if (! ps_butt->isChecked())
    bmode = tmode = false;
  bk_sign_spin->setEnabled(bmode);
  bk_sign_lbl->setEnabled(bmode);
  bk_two_butt->setEnabled(bmode);
  bk_align_spin->setEnabled(bmode && tmode);
  bk_thick_spin->setEnabled(bmode && tmode);
  bk_fold_spin->setEnabled(bmode && tmode);
  bk_align_lbl->setEnabled(bmode && tmode);
  bk_thick_lbl->setEnabled(bmode && tmode);
  bk_fold_lbl->setEnabled(bmode && tmode);
}

void
QDPrintDialog::refresh_cb(void * cl_data)
{
   QDPrintDialog * th=(QDPrintDialog *) cl_data;
   qApp->processEvents();
   if (th->interrupt_printing)
      throw Interrupted("QDPrintDialog::refresh_cb", "Printing interrupted.");
}

void
QDPrintDialog::progress_cb(double done, void * cl_data)
{
  QDPrintDialog * th=(QDPrintDialog *) cl_data;
  double &low = th->progress_low;
  double &high = th->progress_high;
  double progress = low;
  if (done >= 1)
    progress = high;
  else if (done >= 0)
    progress = low + done * (high-low);
  th->progress->setProgress((int)(progress*100));
  QDPrintDialog::refresh_cb(cl_data);
}

void
QDPrintDialog::info_cb(int page_num, int page_cnt, int tot_pages,
		       DjVuToPS::Stage stage, void * cl_data)
{
  QDPrintDialog * th=(QDPrintDialog *) cl_data;
  double &low = th->progress_low;
  double &high = th->progress_high;
  low = 0;
  high = 100;
  if (tot_pages > 0) 
    {
      double step = 1.0 / (double)tot_pages;
      low = (double)page_cnt * step;
      if (stage != DjVuToPS::DECODING) 
	low += step / 2.0;
      high = low  + step / 2.0;
    }
  if (low < 0)
    low = 0;
  if (low > 1)
    low = 1;
  if (high < low)
    high = low;
  if (high > 1)
    high = 1;
  th->progress->setProgress((int)(low*100));
  QDPrintDialog::refresh_cb(cl_data);
}

void
QDPrintDialog::setAlmostDisabled(bool disabled)
{
  QObject *obj;
  const QObjectList *lst = startWidget()->children();
  if (lst) 
    {
      QObjectListIt it(*lst);
      for( ; (obj = it.current()); ++it) 
	if (obj->isWidgetType()) 
	  ((QWidget*)obj)->setEnabled(!disabled);
    }
  if (disabled) 
    {
      cancel_butt->setEnabled(TRUE);
      progress->setEnabled(TRUE);
    }
}

void
QDPrintDialog::done(int rc)
{
  DEBUG_MSG("QDPrintDialog::done() called\n");
  DEBUG_MAKE_INDENT(3);
  
  if (rc==Accepted)
    {
      DEBUG_MSG("OK pressed...\n");
      FILE *fdesc = 0;
      QString fname;
      
      try
        {
          int what=str2id(what_menu->currentText());
          QString customPages=custompages_text->text();
          if (what==PRINT_CUSTOM && customPages.length()==0)
	    throw ERROR_MESSAGE("QDPrintDialog::done",
				"Empty \"Custom pages\" list specified.");
          
          if (file_butt->isChecked())
            {
              QString fname=file_text->text();
              struct stat st;
              if (stat(fname, &st)>=0)
                if (QMessageBox::warning(this, "DjVu",
                                         tr("File '") + fname + tr("' already exists.\n") +
                                         tr("Are you sure you want to overwrite it?"),
                                         tr("&Yes"), tr("&No"), 0, 0, 1))
		  return;
            }
          
	 printing=1;
	 interrupt_printing=0;
	 setAlmostDisabled(TRUE);
	 bool printToFile=file_butt->isChecked();
         QString printFile=file_text->text();
         QString printCommand=printer_text->text();
	 bool printColor=color_butt->isChecked();
	 bool printPortrait=portrait_butt->isChecked();
         bool printAutoOrient=autoorient_butt->isChecked();
	 bool printPS=ps_butt->isChecked();
         bool bookMode=bk_mode_butt->isChecked();
         int  bookSign=bk_sign_spin->value();
         bool bookTwo=bk_two_butt->isChecked();
         int  bookAlign=bk_align_spin->value();
         int  bookFold=bk_fold_spin->value();
         int  bookThick=bk_thick_spin->value();
         bool printFrame=rectmark_chk->isChecked();
         bool printCropMarks=cropmark_chk->isChecked();

	 int  printLevel=2;
         if (level3_butt->isChecked())
           printLevel = 3;
         else if (level1_butt->isChecked())
           printLevel = 1;
         
         int zoom = zoom_spin->value();
         if (! strcmp(zoom_menu->currentText(), tr(fit_page_str)))
           zoom = DjVuToPS::Options::FIT_PAGE;
         else if (! strcmp(zoom_menu->currentText(), tr(one_to_one_str)))
           zoom = 100;
         else if (! strcmp(zoom_menu->currentText(), tr(current_zoom_str)))
           zoom = cur_zoom;
         
	 GP<ByteStream> pstr;
         if (printToFile)
           {
             fdesc = 0;
             fname = printFile;
             pstr = ByteStream::create(GURL::Filename::UTF8(GStringFromQString(fname)),"wb");
           }
         else
           {
#ifdef SIGPIPE
              // Disable SIGPIPE and leave it that way!
              sigset_t mask;
              struct sigaction act;
              sigemptyset(&mask);
              sigaddset(&mask, SIGPIPE);
              sigprocmask(SIG_BLOCK, &mask, 0);
              sigaction(SIGPIPE, 0, &act);
              act.sa_handler = SIG_IGN;
              sigaction(SIGPIPE, &act, 0);
#endif
              // Open pipe to command
              fdesc = popen((const char*)printCommand, "w");
              if (!fdesc)
                throw ERROR_MESSAGE("QDPrintDialog::done",
                                    "Cannot launch specified print command");
              pstr = ByteStream::create(fdesc, "wb", false);
           }

         prog_widget->raiseWidget(progress);
	 progress->reset();
	 GP<DjVuToPS> print = new DjVuToPS();

         print->options.set_sRGB(true);
	 if (prefs->dPrinterGamma>0) {
           print->options.set_sRGB(false);
           print->options.set_gamma(prefs->dPrinterGamma);
	 } else if (printLevel < 2) {
           QMessageBox::information(this, "DjVu",
                                    tr("Print quality will be lower because\n"
                                       "PostScript level 1 cannot perform\n"
                                       "automatic color matching."),
                                    tr("&OK"), 0, 0, 0, 0);
         }
         
	 DjVuToPS::Options & opt=print->options;
	 opt.set_mode(displ_mode==IDC_DISPLAY_BACKGROUND ? DjVuToPS::Options::BACK :
		      displ_mode==IDC_DISPLAY_BLACKWHITE ? DjVuToPS::Options::BW :
		      displ_mode==IDC_DISPLAY_FOREGROUND ? DjVuToPS::Options::FORE :
		      DjVuToPS::Options::COLOR);
	 
	 opt.set_format(printPS ? DjVuToPS::Options::PS : DjVuToPS::Options::EPS);
	 opt.set_level(printLevel);
         if (printAutoOrient)
           opt.set_orientation(DjVuToPS::Options::AUTO);
         else if (printPortrait)
           opt.set_orientation(DjVuToPS::Options::PORTRAIT);
         else
           opt.set_orientation(DjVuToPS::Options::LANDSCAPE);
	 opt.set_color(printColor);
	 opt.set_zoom((DjVuToPS::Options::Zoom) zoom);
	 opt.set_copies(copies_spin->value());
	 opt.set_frame(printFrame);
#if 0
         opt.set_cropmark(printCropMarks);
         if (!bookMode) 
           opt.set_book_signature( 0 );
         else if (bookSign < 4)
           opt.set_book_signature( -1 );
         else
           opt.set_book_signature( bookSign );
         if (!bookMode)
           opt.set_side_by_side(false);
         else
           opt.set_side_by_side(bookTwo, bookAlign, bookFold, bookThick);
#endif
	 print->set_refresh_cb(refresh_cb, this);
	 print->set_dec_progress_cb(progress_cb, this);
	 print->set_prn_progress_cb(progress_cb, this);
	 print->set_info_cb(info_cb, this);
	 if ((what==PRINT_DOC || what==PRINT_CUSTOM) && doc)
	    print->print(*pstr, doc, (what==PRINT_CUSTOM && customPages.length()) ?
			(const char *) customPages : 0);
	 else
	 {
	    GRect img_rect(0, 0, dimg->get_width(), dimg->get_height());
	    GRect prn_rect=(what==PRINT_WIN) ? print_rect : img_rect;
	    if (prn_rect.isempty())
	       prn_rect=img_rect;
	    print->print(*pstr, dimg, prn_rect, img_rect);
	 }
	 prog_widget->raiseWidget(save_butt);
	 pstr->flush();
         if (fdesc)
           pclose(fdesc);
      
         if (save_butt->isChecked())
           {
             DEBUG_MSG("updating preferences\n");
             DjVuPrefs disk_prefs;
             disk_prefs.printLevel = prefs->printLevel
               = printLevel;
             disk_prefs.printPS = prefs->printPS
               = printPS;
             disk_prefs.printToFile = prefs->printToFile
               = printToFile;
             disk_prefs.printColor = prefs->printColor
               = printColor;
             disk_prefs.printAutoOrient = prefs->printAutoOrient
               = printAutoOrient;
             disk_prefs.printPortrait = prefs->printPortrait
               = printPortrait;
             disk_prefs.printFile = prefs->printFile
               = GStringFromQString(printFile);
             disk_prefs.printCommand = prefs->printCommand 
               = GStringFromQString(printCommand);
             disk_prefs.printFitPage = prefs->printFitPage 
               = (zoom==DjVuToPS::Options::FIT_PAGE);
             disk_prefs.printZoom = prefs->printZoom 
               = ((disk_prefs.printFitPage) ? 100 : zoom);
             disk_prefs.printAllPages = prefs->printAllPages 
               = (what==PRINT_DOC || what==PRINT_CUSTOM);
             disk_prefs.bookMode = prefs->bookMode 
               = bookMode;
             disk_prefs.bookSign = prefs->bookSign
               = bookSign;
             disk_prefs.bookTwo = prefs->bookTwo
               = bookTwo;
             disk_prefs.bookAlign = prefs->bookAlign
               = bookAlign;
             disk_prefs.bookFold = prefs->bookFold
               = bookFold;
             disk_prefs.bookThick = prefs->bookThick
               = bookThick;
             disk_prefs.printFrame = prefs->printFrame
               = printFrame;
             disk_prefs.printCropMarks = prefs->printCropMarks
               = printCropMarks;
             disk_prefs.save();
           }
         setAlmostDisabled(FALSE);
         setSensitivity();
         emit sigDone();
	 QeDialog::done(rc);
      } 
      catch(Interrupted &)
      {
	 if (fdesc) pclose(fdesc);
	 if (!! fname) unlink(fname);
	 prog_widget->raiseWidget(save_butt);
	 printing=0;
	 setAlmostDisabled(FALSE);
	 setSensitivity();
      } 
      catch(const GException & exc)
      {
	 if (fdesc) pclose(fdesc);
	 if (!! fname) unlink(fname);
	 prog_widget->raiseWidget(save_butt);
	 printing=0;
	 setAlmostDisabled(FALSE);
	 setSensitivity();
	 showError(this, exc);
      }
   } 

   else
   {
     if (printing) {
       DEBUG_MSG("interrupting printing\n");
       interrupt_printing=1;
     } else {
       emit sigDone();
       QeDialog::done(rc);
     }
   }
}

void
QDPrintDialog::setSensitivity(void)
{
  slotFormatChanged();
  slotWhatChanged(what_menu->currentText());
  slotDstChanged();
  slotBookChanged();
  adjustWhat();
}

QDPrintDialog::QDPrintDialog(const GP<DjVuDocument> & _doc,
			     const GP<DjVuImage> & _dimg,
			     DjVuPrefs * _prefs, int _displ_mode,
			     int _cur_zoom, const GRect & _print_rect,
			     QWidget * parent, const char * name, bool modal) :
      QeDialog(parent, name, modal), doc(_doc), dimg(_dimg), 
      cur_page_num(0), progress_low(0), progress_high(1),
      prefs(_prefs), displ_mode(_displ_mode), cur_zoom(_cur_zoom),
      print_rect(_print_rect)
{
   DEBUG_MSG("QDPrintDialog(): Initializing...\n");

   printing=0;
   interrupt_printing=0;

   if (!prefs) throw ERROR_MESSAGE("QDPrintDialog::QDPrintDialog",
				   "Zero PREFERENCES passed as input.");
   if (!doc) throw ERROR_MESSAGE("QDPrintDialog::QDPrintDialog",
				 "Zero document passed as input.");
   if (!dimg) throw ERROR_MESSAGE("QDPrintDialog::QDPrintDialog",
				  "Zero image passed as input.");
   
   if (print_rect.isempty())
     print_rect=GRect(0, 0, dimg->get_width(), dimg->get_height());
   cur_page_num=doc->url_to_page(dimg->get_djvu_file()->get_url());

   QWidget *start = startWidget();
   QLabel *label;
   QButtonGroup *bg;
   QVBoxLayout *vlay, *bg_lay;
   QHBoxLayout *hlay, *bg_hlay;
   QGridLayout *glay;
   
   setCaption(tr("DjVu Print Dialog"));

   // Creating the Tab widget
   vlay = new QVBoxLayout(start, 10);
   QTabWidget *tabwidget = new QTabWidget(start, "tabwidget");
   vlay->addWidget(tabwidget);

   // Creating the OK/Cancel buttons 
   QHBoxLayout * prg_lay=new QHBoxLayout(vlay);
   prog_widget=new QWidgetStack(start, "prog_widget");
   prg_lay->addWidget(prog_widget, 1);
   progress=new QProgressBar(prog_widget, "progress_bar");
   progress->setIndicatorFollowsStyle(FALSE);
   progress->setCenterIndicator(TRUE);
   save_butt=new QCheckBox(tr("&Save settings"), prog_widget, "save_butt");
   save_butt->setChecked(FALSE);
   prog_widget->raiseWidget(save_butt);
   QPushButton * ok_butt=new QPushButton(tr("&OK"), start, "ok_butt");
   ok_butt->setDefault(TRUE);
   prg_lay->addWidget(ok_butt);
   cancel_butt=new QPushButton(tr("&Cancel"), start, "cancel_butt");
   prg_lay->addWidget(cancel_butt);

   // Creating the MAIN tab
   start = new QWidget(tabwidget,"main");
   tabwidget->addTab(start,tr("Main"));
   vlay = new QVBoxLayout(start, 10);
   hlay = new QHBoxLayout(vlay, 10);

   // *** Creating 'What to Print' frame
   bg = what_bg = new QButtonGroup(tr("What to print"), start);
   hlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   bg_hlay=new QHBoxLayout(bg_lay);
   copies_spin=new QSpinBox(1, 99, 1, bg, "print_spin");
   copies_spin->setSpecialValueText(tr("1 copy"));
   copies_spin->setSuffix(tr(" copies"));
   copies_spin->setValue(1);
   bg_hlay->addWidget(copies_spin);
   label=new QLabel(tr("of the"), bg);
   bg_hlay->addWidget(label);
   what_menu=new QComboBox(FALSE, bg, "print_menu");
   bg_hlay->addWidget(what_menu);
   bg_hlay=new QHBoxLayout(bg_lay);
   custompages_label=new QLabel(tr("C&ustom pages:"), bg);
   bg_hlay->addWidget(custompages_label);
   custompages_text=new QLineEdit(bg, "custompages_text");
   custompages_text->setMaxLength(128);
   custompages_text->setEnabled(FALSE);
   bg_hlay->addWidget(custompages_text, 1);
   custompages_label->setBuddy(custompages_text);
   static const QString tip=
     tr("Enter a list of page ranges here\n"
        "separated by commas like 1,2,10-12.\n")+
     tr("To make multiple copies try 1,1,1,1.\n")+
     tr("To print in reverse try 10-1.");
   QToolTip::add(custompages_label, tip);
   QToolTip::add(custompages_text, tip);
   
   // *** Creating 'Color model' frame
   bg=color_bg=new QButtonGroup(tr("Color model"), start);
   hlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   color_butt=new QRadioButton(tr("&Color"), bg, "color_butt");
   bg_lay->addWidget(color_butt);
   grey_butt=new QRadioButton(tr("&GreyScale"), bg, "grey_butt");
   bg_lay->addWidget(grey_butt);

   // *** Creating the "destination" frame
   QGroupBox * gb=new QGroupBox(tr("Print destination"), start, "gb");
   vlay->addWidget(gb);
   QVBoxLayout * gb_lay=new QVBoxLayout(gb, 10);
   gb_lay->addSpacing(gb->fontMetrics().height());
   QHBoxLayout * gb_lay1=new QHBoxLayout(gb_lay);
   QWidget * top_w=new QWidget(gb, "top_w");
   gb_lay1->addStretch(1);
   gb_lay1->addWidget(top_w);
   gb_lay1->addStretch(1);
   QWidgetStack * bot_w=new QWidgetStack(gb, "bot_w");
   gb_lay->addWidget(bot_w);

   QHBoxLayout * top_lay=new QHBoxLayout(top_w, 0, 5);
   label=new QLabel(tr("Send Image To:"), top_w, "crazy_label");
   top_lay->addWidget(label);
   QButtonGroup * dst_bg=new QButtonGroup(top_w);
   dst_bg->setFrameStyle(QFrame::NoFrame);
   top_lay->addWidget(dst_bg);
   QHBoxLayout * dst_lay=new QHBoxLayout(dst_bg, 0, 5, "dst_lay");
   printer_butt=new QRadioButton(tr("Printe&r"), dst_bg, "printer_butt");
   dst_lay->addWidget(printer_butt);
   file_butt=new QRadioButton(tr("F&ile"), dst_bg, "file_butt");
   dst_lay->addWidget(file_butt);

   dst_widget=bot_w;
   printer_widget=new QWidget(bot_w, "printer_widget");
   QHBoxLayout * print_lay=new QHBoxLayout(printer_widget, 0, 5, "print_lay");
   label=new QLabel(tr("Print Command:"), printer_widget);
   print_lay->addWidget(label);
   printer_text=new QLineEdit(printer_widget, "printer_text");
   printer_text->setMaxLength(128);
   print_lay->addWidget(printer_text, 1);
   file_widget=new QWidget(bot_w, "file_widget");
   QHBoxLayout * file_lay=new QHBoxLayout(file_widget, 0, 5, "file_lay");
   label=new QLabel(tr("File Name:"), file_widget);
   file_lay->addWidget(label);
   file_text=new QLineEdit(file_widget, "file_text");
   file_text->setMaxLength(128);
   file_lay->addWidget(file_text, 1);
   QPushButton * browse_butt=new QPushButton(tr("Browse..."), file_widget,
                                             "browse_butt");
   file_lay->addWidget(browse_butt);
   

   // Creating the POSTSCRIPT tab
   start = new QWidget(tabwidget,"postscript");
   tabwidget->addTab(start,tr("Postscript"));
   vlay = new QVBoxLayout(start, 10);

   // *** Creating 'File format' frame 
   bg=format_bg=new QButtonGroup("PostScript File format", start);
   vlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   ps_butt=new QRadioButton(tr("&PostScript File (*.ps)"), bg, "ps_butt");
   bg_lay->addWidget(ps_butt);
   eps_butt=new QRadioButton("&Encapsulated PostScript File (*.eps)", bg, "eps_butt");
   bg_lay->addWidget(eps_butt);

   // *** Creating 'PostScript level' frame
   bg=new QButtonGroup(tr("PostScript language level"), start);
   vlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   level1_butt=new QRadioButton(tr("Level &1 (almost obsolete)"), 
                                bg, "level1_butt");
   bg_lay->addWidget(level1_butt);
   QToolTip::add(level1_butt,
                 tr("Generate a portable PostScript. This option is useful\n"
                    "with very old printers and produces very large files.") );
   level2_butt=new QRadioButton(tr("Level &2 (default)"), 
                                bg, "level2_butt");
   bg_lay->addWidget(level2_butt);
   QToolTip::add(level2_butt,
                 tr("Create a smaller and faster PostScript output.\n"
                    "This is the best choice for most printers.") );
   level3_butt=new QRadioButton(tr("Level &3 (might print faster)"), 
                                bg, "level3_butt");
   bg_lay->addWidget(level3_butt);
   QToolTip::add(level3_butt,
                 tr("Create the smallest and fastest PostScript output.\n"
                    "Only use this option with recent PS Level 3 printers.\n"
                    "Some printers might print very slowly or not at all.") );
   

   // Creating the POSITION tab
   start = new QWidget(tabwidget,"position");
   tabwidget->addTab(start,tr("Position"));
   vlay = new QVBoxLayout(start, 10);
   hlay = new QHBoxLayout(vlay, 10);

   // *** Creating 'Scaling' frame
   bg = scale_bg = new QButtonGroup(tr("Scaling"), start);
   hlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   zoom_menu=new QComboBox(FALSE, bg, "zoom_menu");
   zoom_menu->insertItem(tr(fit_page_str));
   zoom_menu->insertItem(tr(one_to_one_str));
   zoom_menu->insertItem(tr(current_zoom_str));
   zoom_menu->insertItem(tr(custom_zoom_str));
   bg_lay->addWidget(zoom_menu);
   zoom_spin=new QSpinBox(5, 999, 1, bg, "zoom_spin");
   zoom_spin->setSuffix("%");
   zoom_spin->setEnabled(FALSE);
   zoom_spin->setValue(100);
   bg_lay->addWidget(zoom_spin);
   
   // *** Creating 'Orientation' frame
   bg = orient_bg = new QButtonGroup(tr("Orientation"), start);
   hlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   autoorient_butt=new QRadioButton(tr("&Auto"), bg, "autoorient_butt");
   bg_lay->addWidget(autoorient_butt);
   portrait_butt=new QRadioButton(tr("Po&rtrait"), bg, "portrait_butt");
   bg_lay->addWidget(portrait_butt);
   landscape_butt=new QRadioButton(tr("&Landscape"), bg, "landscape_butt");
   bg_lay->addWidget(landscape_butt);

   // *** Creating 'Marks' frame
   bg = new QButtonGroup(tr("Marks"),start);
   vlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   rectmark_chk = new QCheckBox(tr("Print image &frame"),bg);
   bg_lay->addWidget(rectmark_chk);
   cropmark_chk = new QCheckBox(tr("Print &crop marks"),bg);
   bg_lay->addWidget(cropmark_chk);

   // Creating the BOOK tab
   start = new QWidget(tabwidget,"book");
   tabwidget->addTab(start,tr("Book"));
   vlay = new QVBoxLayout(start, 10);
   bg = new QButtonGroup(tr("Booklet printing options"),start);
   vlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   glay=new QGridLayout(bg_lay, 6, 2);
   
   bk_mode_butt = new QCheckBox(tr("Reorder pages for printing a book"), bg);
   glay->addMultiCellWidget(bk_mode_butt,0,0,0,1);
   label = bk_sign_lbl = new QLabel(tr(" Maximal signature size"),bg);
   glay->addWidget(label,1,0);
   bk_sign_spin = new QSpinBox(bg);
   bk_sign_spin->setMinValue(3);
   bk_sign_spin->setSuffix(tr(" pages"));
   bk_sign_spin->setSpecialValueText(tr("unlimited"));
   glay->addWidget(bk_sign_spin,1,1);
   bk_two_butt = new QCheckBox(tr("Print pages side by side"),bg);
   glay->addMultiCellWidget(bk_two_butt,2,2,0,1);
   label = bk_align_lbl = new QLabel(tr(" Adjust recto/verso alignment"),bg);
   glay->addWidget(label,3,0);
   bk_align_spin = new QSpinBox(-20,20,1,bg);
   bk_align_spin->setSuffix(tr(" points"));
   glay->addWidget(bk_align_spin,3,1);
   label = bk_fold_lbl = new QLabel(tr(" Extra folding space"),bg);
   glay->addWidget(label,4,0);
   bk_fold_spin = new QSpinBox(0,144,1,bg);
   bk_fold_spin->setSuffix(tr(" points"));
   glay->addWidget(bk_fold_spin,4,1);
   label = bk_thick_lbl = new QLabel(tr(" Paper thickness"),bg);
   glay->addWidget(label,5,0);
   bk_thick_spin = new QSpinBox(0,1000,1,bg);
   bk_thick_spin->setSuffix(tr(" centipoints"));
   glay->addWidget(bk_thick_spin,5,1);
   QToolTip::add(bk_sign_spin,
                 tr("The signature size is the number of sides\n"
                    "which will be folded and bound together."));
   QToolTip::add(bk_align_spin,
                 tr("Corrects recto/verso alignment on\n"
                    "certain printers."));
   QToolTip::add(bk_fold_spin,
                 tr("Controls how much space separates the\n"
                    "contents of two pages on the same sheet."));
   QToolTip::add(bk_thick_spin,
                 tr("Controls how much extra space is needed\n"
                    "to fold outer sheets in a signature."));

   // Connecting signals
   connect(ps_butt, SIGNAL(toggled(bool)), 
           this, SLOT(slotFormatChanged(void)));
   connect(eps_butt, SIGNAL(toggled(bool)), 
           this, SLOT(slotFormatChanged(void)));
   connect(file_butt, SIGNAL(toggled(bool)), 
           this, SLOT(slotDstChanged(void)));
   connect(printer_butt, SIGNAL(toggled(bool)), 
           this, SLOT(slotDstChanged(void)));
   connect(browse_butt, SIGNAL(clicked(void)), 
           this, SLOT(slotBrowse(void)));
   connect(what_menu, SIGNAL(activated(const QString &)),
	   this, SLOT(slotWhatChanged(const QString &)));
   connect(zoom_menu, SIGNAL(activated(const QString &)),
	   this, SLOT(slotZoomChanged(const QString &)));
   connect(bk_mode_butt, SIGNAL(toggled(bool)), 
           this, SLOT(slotBookChanged(void)));
   connect(bk_two_butt, SIGNAL(toggled(bool)), 
           this, SLOT(slotBookChanged(void)));
   connect(ok_butt, SIGNAL(clicked(void)), 
           this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), 
           this, SLOT(reject(void)));
   
   // Setting buttons states
   if (!prefs->printToFile) 
     prefs->printPS=1;
   if (!prefs->printPS) 
     prefs->printPortrait = 1; 
   setPSFormat(prefs->printPS);
   setPortrait(prefs->printPortrait || !prefs->printPS, prefs->printAutoOrient);
   setColorMode(prefs->printColor);
   setPSLevel(prefs->printLevel);
   setFileName(QStringFromGString(prefs->printFile));
   setCommand(QStringFromGString(prefs->printCommand));
   setBookMode(prefs->bookMode, prefs->bookSign);
   setBookTwo(prefs->bookTwo, prefs->bookAlign, prefs->bookFold, prefs->bookThick);
   printToFile(prefs->printToFile);
   if (prefs->printFitPage) 
     setZoom(-1);
   else 
     setZoom(prefs->printZoom);
   setPrint(prefs->printAllPages ? PRINT_DOC : PRINT_PAGE);
   if (displ_mode == IDC_DISPLAY_BLACKWHITE)
     {
       color_butt->setEnabled(false);
       grey_butt->setEnabled(false);
     }
   rectmark_chk->setChecked(prefs->printFrame);
   cropmark_chk->setChecked(prefs->printCropMarks);
   setSensitivity();
}
