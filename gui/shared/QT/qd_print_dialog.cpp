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
// $Id: qd_print_dialog.cpp,v 1.31 2008/08/19 12:34:31 bpearlmutter Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qd_print_dialog.h"
#include "qlib.h"
#include "debug.h"
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
#include <qvalidator.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qprogressbar.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qobjectlist.h>
#include <qtabwidget.h>
#ifndef QT1
#include <qwhatsthis.h>
#endif

#include <unistd.h>
#include <errno.h>
#include <signal.h>

// -- MAXPATHLEN
#ifndef MAXPATHLEN
# ifdef _MAX_PATH
#  define MAXPATHLEN _MAX_PATH
# else
#  define MAXPATHLEN 1024
# endif
#else
# if ( MAXPATHLEN < 1024 )
#  undef MAXPATHLEN
#  define MAXPATHLEN 1024
# endif
#endif

/** Avoid name clashes on solaris! */
#ifdef PS
# undef PS
#endif
#ifdef EPS
# undef EPS
#endif

static const QString print_page_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","of the current page");
static const QString print_custom_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","of the custom pages");
static const QString print_doc_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","of the document");
static const QString print_win_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","of the window");

static const QString fit_page_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","Scale to Fit");
static const QString one_to_one_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","One to One");
static const QString current_zoom_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","Current Zoom");
static const QString custom_zoom_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","Custom Zoom:");

static const QString recto_verso_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","rectos and versos");
static const QString recto_only_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","rectos only");
static const QString verso_only_str
  = QT_TRANSLATE_NOOP("QDPrintDialog","versos only");


const QString
QDPrintDialog::id2str(int id)
{
   return
     id==PRINT_PAGE   ? tr(print_page_str) :
     id==PRINT_CUSTOM ? tr(print_custom_str) :
     id==PRINT_DOC    ? tr(print_doc_str) :
     id==PRINT_WIN    ? tr(print_win_str) : 
     QString::null;
}

int
QDPrintDialog::str2id(const QString &str)
{
   return
     str==tr(print_page_str)   ? PRINT_PAGE :
     str==tr(print_custom_str) ? PRINT_CUSTOM :
     str==tr(print_doc_str)    ? PRINT_DOC :
     str==tr(print_win_str)    ? PRINT_WIN 
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
  if (zoom<=0)
    setting= tr(fit_page_str);
  else if (zoom==100)
    setting= tr(one_to_one_str);
  else if (zoom==cur_zoom)
    setting = tr(current_zoom_str);
  else 
    setting = tr(custom_zoom_str);
  zoom_spin->setValue(zoom<=0 ? 100 : zoom);
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
QDPrintDialog::setBookletMode(int mode, int sign)
{
  bool bkflag = true;
  QString settings = recto_verso_str;
  switch( (DjVuToPS::Options::BookletMode) mode )
    {
    default:
    case DjVuToPS::Options::OFF: 
      bkflag = false; break;
    case DjVuToPS::Options::RECTO:
      settings = recto_only_str; break;
    case DjVuToPS::Options::VERSO:
      settings = verso_only_str; break;
    case DjVuToPS::Options::RECTOVERSO:
      settings = recto_verso_str; break;
    }
  bk_mode_butt->setChecked(bkflag);
  setComboBoxCurrentItem(bk_mode_menu, tr(settings));
  bk_max_spin->setValue(sign/4);
}

void
QDPrintDialog::setBookletParm(int align, int fold, int xfold)
{
  bk_align_spin->setValue(align);
  bk_fold_spin->setValue(fold);
  bk_xfold_spin->setValue(xfold/10);
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
QDPrintDialog::slotWhatChanged(const QString & qtext)
{
  custompages_text->setEnabled(qtext==tr(print_custom_str));
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
   cropmark_chk->setEnabled(ps_mode);
   QString qtext = zoom_menu->currentText();
   zoom_spin->setEnabled(ps_mode && qtext==tr(custom_zoom_str));
   bk_mode_butt->setEnabled(ps_mode);
   adjustWhat();
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
  bmode = bmode && ps_butt->isChecked();
  QObject *obj;
  QObjectListIt it(*bk_normal->children());
  for( ; (obj = it.current()); ++it) 
    if (obj->isWidgetType() && (QWidget*)obj != bk_mode_butt) 
      ((QWidget*)obj)->setEnabled(bmode);
  bk_advanced->setEnabled(bmode);
}

void
QDPrintDialog::slotHelp(void)
{
#ifndef QT1
  QWhatsThis::enterWhatsThisMode();
#endif
}

struct Interrupted { int dummy; };

void
QDPrintDialog::refresh_cb(void * cl_data)
{
   QDPrintDialog * th=(QDPrintDialog *) cl_data;
   qApp->processEvents();
   if (th->interrupt_printing)
      throw Interrupted();
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
  high = 1;
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
            {
              QCString msg = tr("No custom pages were specified").utf8();
              G_THROW((const char*)msg);
            }
          
          if (file_butt->isChecked())
            {
              QString fname=file_text->text();
              struct stat st;
              if (stat(fname, &st)>=0)
                if (QMessageBox::warning(this, "DjVu",
                                         tr("File '%1' already exists.\n"
                                            "Are you sure you want to overwrite it?")
                                         .arg(fname),
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
         bool printFrame=rectmark_chk->isChecked();
         bool printCropMarks=cropmark_chk->isChecked();

         int  bookletMode=(int)DjVuToPS::Options::OFF;
         if (bk_mode_butt->isChecked())
	   {
	     if (! strcmp(bk_mode_menu->currentText(), tr(recto_verso_str)))
	       bookletMode=(int)DjVuToPS::Options::RECTOVERSO;
	     else if (! strcmp(bk_mode_menu->currentText(), tr(recto_only_str)))
	       bookletMode=(int)DjVuToPS::Options::RECTO;
	     else if (! strcmp(bk_mode_menu->currentText(), tr(verso_only_str)))
	       bookletMode=(int)DjVuToPS::Options::VERSO;
	   }
         int  bookletMax=4 * bk_max_spin->value();
         int  bookletAlign=bk_align_spin->value();
         int  bookletFold=bk_fold_spin->value();
         int  bookletXFold=bk_xfold_spin->value()*10;

	 int  printLevel=2;
         if (level3_butt->isChecked())
           printLevel = 3;
         else if (level1_butt->isChecked())
           printLevel = 1;

         int zoom = zoom_spin->value();
         if (! strcmp(zoom_menu->currentText(), tr(fit_page_str)))
           zoom = 0;
         else if (! strcmp(zoom_menu->currentText(), tr(one_to_one_str)))
           zoom = 100;
         else if (! strcmp(zoom_menu->currentText(), tr(current_zoom_str)))
           zoom = cur_zoom;
         
	 GP<ByteStream> pstr;
         if (printToFile)
           {
             fdesc = 0;
             fname = printFile;
             pstr = ByteStream::create(
                      GURL::Filename::UTF8(GStringFromQString(fname)),"wb");
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
              if (! fdesc)
                {
                  QCString msg = tr("Cannot launch specified print command").utf8();
                  G_THROW((const char*)msg);
                }
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
	 opt.set_mode(displ_mode==IDC_DISPLAY_BACKGROUND 
                      ? DjVuToPS::Options::BACK :
		      displ_mode==IDC_DISPLAY_BLACKWHITE 
                      ? DjVuToPS::Options::BW :
		      displ_mode==IDC_DISPLAY_FOREGROUND 
                      ? DjVuToPS::Options::FORE 
                      : DjVuToPS::Options::COLOR);
	 opt.set_format(printPS 
                        ? DjVuToPS::Options::PS 
                        : DjVuToPS::Options::EPS);
	 opt.set_level(printLevel);
         if (printAutoOrient)
           opt.set_orientation(DjVuToPS::Options::AUTO);
         else if (printPortrait)
           opt.set_orientation(DjVuToPS::Options::PORTRAIT);
         else
           opt.set_orientation(DjVuToPS::Options::LANDSCAPE);
	 opt.set_color(printColor);
	 opt.set_zoom(zoom);
	 opt.set_copies(copies_spin->value());
	 opt.set_frame(printFrame);
         opt.set_cropmarks(printCropMarks);
         opt.set_bookletmode((DjVuToPS::Options::BookletMode)bookletMode);
         opt.set_bookletmax(bookletMax);
         opt.set_bookletalign(bookletAlign);
         opt.set_bookletfold(bookletFold,bookletXFold);
         
	 print->set_refresh_cb(refresh_cb, this);
	 print->set_dec_progress_cb(progress_cb, this);
	 print->set_prn_progress_cb(progress_cb, this);
	 print->set_info_cb(info_cb, this);
	 if ((what==PRINT_DOC || what==PRINT_CUSTOM) && doc)
	    print->print(*pstr, doc, 
                         (what==PRINT_CUSTOM && customPages.length()) ?
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
               = !zoom;
             disk_prefs.printZoom = prefs->printZoom 
               = ((disk_prefs.printFitPage) ? 100 : zoom);
             disk_prefs.printAllPages = prefs->printAllPages 
               = (what==PRINT_DOC || what==PRINT_CUSTOM);
             disk_prefs.bookletMode = prefs->bookletMode 
               = bookletMode;
             disk_prefs.bookletMax = prefs->bookletMax 
               = bookletMax;
             disk_prefs.bookletAlign = prefs->bookletAlign 
               = bookletAlign;
             disk_prefs.bookletFold = prefs->bookletFold 
               = bookletFold;
             disk_prefs.bookletXFold = prefs->bookletXFold 
               = bookletXFold;
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

   if (!prefs) 
     G_THROW("Zero PREFERENCES passed as input.");
   if (!doc) 
     G_THROW("Zero document passed as input.");
   if (!dimg) 
     G_THROW("Zero image passed as input.");
   
   if (print_rect.isempty())
     print_rect=GRect(0, 0, dimg->get_width(), dimg->get_height());
   cur_page_num=doc->url_to_page(dimg->get_djvu_file()->get_url());

   QWidget *main = startWidget();
   QWidget *start = main;
   QLabel *label;
   QButtonGroup *bg;
   QVBoxLayout *vlay, *bg_lay;
   QHBoxLayout *hlay, *bg_hlay;

   // Creating the Tab widget
   setCaption(tr("DjVu Print Dialog"));
   QVBoxLayout *mainvlay = new QVBoxLayout(start, 10);
   QTabWidget *tabwidget = new QTabWidget(main, "tabwidget");
   mainvlay->addWidget(tabwidget);

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
#ifndef QT1
   QWhatsThis::add(bg,
                   tr("<b>Printing selected pages:</b>"
                      "<br>The page specification box is enabled when"
                      " the <i>custom pages</i> mode is selected. "
                      " Enter page ranges separated by commas."
                      " Blank pages can be specified as page <b>0</b>."
                      " The last page can be referred to as page <b>$</b>."
                      "<ul>"
                      "<li>Simple case: <b>1,2,10-12</b>.</li>"
                      "<li>Printing multiple pages: <b>1,1,2,2</b>.</li>"
                      "<li>Inserting a blank page: <b>1-10,0,11-$</b>.</li>"
                      "<li>Printing in reverse: <b>$-1</b>.</li>"
                      "</ul>") );
#endif

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
#ifndef QT1
   QWhatsThis::add(gb,
                   tr("<b>Print Destination:</b>"
                      "<ul>"
                      "<li>Enter the file name when printing to a file.</li>"
                      "<li>Enter a print command to select a printer."
                      " Command line options are of course allowed."
                      " Command print commands include:"
                      "<br><b>lp -d</b><i>printername</i>,"
                      "<br><b>lpr -P</b><i>printername</i>,"
                      "<br><b>kprinter --stdin</b>."
                      "</ul>"));
#endif

   // Creating the POSTSCRIPT tab
   start = new QWidget(tabwidget,"postscript");
   tabwidget->addTab(start,tr("Postscript"));
   vlay = new QVBoxLayout(start, 10);

   // *** Creating 'File format' frame 
   bg=format_bg=new QButtonGroup(tr("PostScript File format"), start);
   vlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   ps_butt=new QRadioButton(tr("&PostScript File (*.ps)"), 
                            bg, "ps_butt");
   bg_lay->addWidget(ps_butt);
   eps_butt=new QRadioButton(tr("&Encapsulated PostScript File (*.eps)"), 
                             bg, "eps_butt");
   bg_lay->addWidget(eps_butt);
#ifndef QT1
   QWhatsThis::add(bg,
                   tr("<b>PostScript file format:</b>"
                      "These options are enabled when printing"
                      " to a file.<ul>"
                      "<li><i>Postscript files</i> provide"
                      " more configuration options.</li>"  
                      "<li><i>Encapsulated PostScript files</i> are"
                      " nicely imported by other applications."
                      " On the other hand they can only contain"
                      " a single page with predefined scaling.</li>"
                      "</ul>"));
#endif

   // *** Creating 'PostScript level' frame
   bg=new QButtonGroup(tr("PostScript language level"), start);
   vlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   level1_butt=new QRadioButton(tr("Level &1 (almost obsolete)"), bg);
   bg_lay->addWidget(level1_butt);
   level2_butt=new QRadioButton(tr("Level &2 (default)"), bg);
   bg_lay->addWidget(level2_butt);
   level3_butt=new QRadioButton(tr("Level &3 (might print faster)"), bg);
   bg_lay->addWidget(level3_butt);
#ifndef QT1
   QWhatsThis::add(bg,
                   tr("<b>PostScript language level:</b>"
                      "<br>Level 1 is only useful with very old printers."
                      " Level 2 works with most printers. Level 3 might"
                      " print faster on recent color printers."));
#endif
   
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
#ifndef QT1
   QWhatsThis::add(bg,
                   tr("<b>Scaling:</b>"
                      "<br>Option <i>scale to fit</i> accomodates"
                      " whetever paper size your printer uses."
                      " Option <i>one to one</i> attempts to reproduce"
                      " the initial document size."));
#endif
   
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
#ifndef QT1
   QWhatsThis::add(bg,
                   tr("<b>Orientation:</b>"
                      "<br>Automatic orientation chooses portrait"
                      " or landscape on a page per page basis."));
#endif

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
#ifndef QT1
   QWhatsThis::add(bg,
                   tr("<b>Marks:</b>"
                      "<br>The image frame is useful to"
                      " delimit the page area. The crop marks are useful"
                      " to cut the paper to the correct size."));
#endif


   // Creating the BOOKLET tab
   start = new QWidget(tabwidget,"book");
   tabwidget->addTab(start,tr("Booklet"));
   vlay = new QVBoxLayout(start, 10);

   // *** Creating 'Booklet options' frame
   bk_normal = bg = new QButtonGroup(tr("Booklet options"),start);
   vlay->addWidget(bg);
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   bk_mode_butt = new QCheckBox(tr("Print sheets suitable"
                                   " for folding booklet(s)."), 
                                bg);
   bg_lay->addWidget(bk_mode_butt);
   hlay = new QHBoxLayout(bg_lay);
   label = new QLabel(tr("Each booklet contains"),bg);
   hlay->addWidget(label);
   bk_max_spin = new QSpinBox(bg);
   bk_max_spin->setMinValue(0);
   bk_max_spin->setPrefix(tr("at most") + " ");
   bk_max_spin->setSuffix(" " + tr("sheet(s)."));
   bk_max_spin->setSpecialValueText(tr("as many sheets as needed."));
   hlay->addWidget(bk_max_spin);
   hlay->addStretch(1);
   hlay = new QHBoxLayout(bg_lay);
   label = new QLabel(tr("Print"),bg);
   hlay->addWidget(label);
   bk_mode_menu = new QComboBox(false, bg, "bk_mode_menu");
   bk_mode_menu->insertItem(tr(recto_verso_str));
   bk_mode_menu->insertItem(tr(recto_only_str));
   bk_mode_menu->insertItem(tr(verso_only_str));
   hlay->addWidget(bk_mode_menu);
   hlay->addStretch(1);
#ifndef QT1
   QWhatsThis::add(bg,
                   tr("<b>Producing booklets:</b>"
                      "<br>The booklet mode prints the selected"
                      " pages as sheets suitable for folding one or several"
                      " booklets. Several booklets might be produced when"
                      " a maximum number of sheets per booklet is specified."
                      " You can either use a duplex printer or print"
                      " rectos and versos separately."));
#endif

   // *** Creating 'Advanced booklet options' frame
   bg = new QButtonGroup(tr("Advanced booklet options"),start);
   vlay->addWidget(bg);
   bk_advanced = bg;
   bg_lay=new QVBoxLayout(bg, 10);
   bg_lay->addSpacing(bg->fontMetrics().height());
   bg_lay->addStrut(bg->fontMetrics().width(bg->title()));
   hlay = new QHBoxLayout(bg_lay, 10); 
   label = new QLabel(tr("Shift rectos and versos by"),bg);
   hlay->addWidget(label);
   bk_align_spin = new QSpinBox(-36,36,1,bg);
   bk_align_spin->setSuffix(tr(" points."));
   hlay->addWidget(bk_align_spin);
   hlay->addStretch(1);
   hlay = new QHBoxLayout(bg_lay, 10); 
   label = new QLabel(tr("Center margins:"),bg);
   hlay->addWidget(label);
   bk_fold_spin = new QSpinBox(0,72,1,bg);
   bk_fold_spin->setSuffix(tr(" points"));
   hlay->addWidget(bk_fold_spin);
   label = new QLabel(tr("plus"),bg);
   hlay->addWidget(label);
   bk_xfold_spin = new QSpinBox(0,199,1,bg);
   bk_xfold_spin->setSuffix(tr("/100"));
   hlay->addWidget(bk_xfold_spin);   
   label = new QLabel(tr("per sheet."),bg);
   hlay->addWidget(label);
   hlay->addStretch(1);
#ifndef QT1
   QWhatsThis::add(bg,
                   tr("<b>Advanced booklet options:</b>"
                      "<br>Shifting rectos and versos is useful"
                      " with poorly aligned duplex printers."
                      " The center margins determine how much"
                      " space is left between the pages to fold the"
                      " sheets. This space slowly increases from the"
                      " inner sheet to the outer sheet."));
#endif
   
   // Creating the OK/Cancel buttons 
   QHBoxLayout * prg_lay=new QHBoxLayout(mainvlay,2);
   prog_widget=new QWidgetStack(main, "prog_widget");
   prg_lay->addWidget(prog_widget, 1);
   progress=new QProgressBar(prog_widget, "progress_bar");
   progress->setIndicatorFollowsStyle(FALSE);
   progress->setCenterIndicator(TRUE);
   save_butt=new QCheckBox(tr("&Save settings"), prog_widget, "save_butt");
   save_butt->setChecked(TRUE);
   prog_widget->raiseWidget(save_butt);
#ifndef QT1
   QPushButton *help_butt=new QPushButton(tr("Help"), main, "help_butt");
   prg_lay->addWidget(help_butt);
#endif
   QPushButton *ok_butt=new QPushButton(tr("&OK"), main, "ok_butt");
   ok_butt->setDefault(TRUE);
   prg_lay->addWidget(ok_butt);
   cancel_butt=new QPushButton(tr("&Cancel"), main, "cancel_butt");
   prg_lay->addWidget(cancel_butt);

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
   connect(ok_butt, SIGNAL(clicked(void)), 
           this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), 
           this, SLOT(reject(void)));
#ifndef QT1
   connect(help_butt, SIGNAL(clicked(void)), 
           this, SLOT(slotHelp(void)));
#endif

   // Setting buttons states
   if (!prefs->printToFile) 
     prefs->printPS=1;
   if (!prefs->printPS) 
     prefs->printPortrait = 1; 
   setPSFormat(prefs->printPS);
   setPortrait(prefs->printPortrait 
               || !prefs->printPS, prefs->printAutoOrient);
   setColorMode(prefs->printColor);
   setPSLevel(prefs->printLevel);
   setFileName(QStringFromGString(prefs->printFile));
   setCommand(QStringFromGString(prefs->printCommand));
   setBookletMode(prefs->bookletMode, prefs->bookletMax);
   setBookletParm(prefs->bookletAlign, 
                  prefs->bookletFold, prefs->bookletXFold);
   printToFile(prefs->printToFile);
   if (prefs->printFitPage) 
     setZoom(0);
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
