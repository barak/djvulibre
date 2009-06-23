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
// $Id: qd_viewer_shell.cpp,v 1.28 2007/03/25 20:48:23 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "DjVuImage.h"
#include "DjVuMessage.h"
#include "DataPool.h"

#include "qd_viewer_shell.h"
#include "cin_data.h"

#include "qlib.h"
#include "names.h"
#include "execdir.h"
#include "djvu_viewer_res.h"
#include "DjVuDocument.h"
#include "qd_wpaper.h"
#include "qd_about_dialog.h"
#include "version.h"

#include <qapplication.h>
#include <qwidgetlist.h>
#include <qkeycode.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qimage.h>

#include <time.h>

#define START_STATUS QT_TRANSLATE_NOOP("QDViewerShell", \
           "Select \"File\" and then \"Open\" to load a DjVu file")

void
QDViewerShell::openFile(const QString &qfname, const QString &qid)
{
   GUTF8String fname = GStringFromQString(qfname);
   GUTF8String id = GStringFromQString(qid);
   DEBUG_MSG("QDViewerShell::openFile(): opening file '" << fname << "'\n");
   DEBUG_MAKE_INDENT(3);
   GURL url;

   if (fname != "-")
   {
      url=GURL::Filename::UTF8(GURL::expand_name(fname));
      if (!! id)
      {
	 url=GURL::UTF8(url.fname()+id, url.base());
      }
   }
   openURL(url);
}

void
QDViewerShell::openURL(const GURL & url)
      // Empty URL means read stdin
      // The URL may contain a bunch of CGI arguments, which will
      // be processed by DjVuViewer. Here we will just open the
      // file associated with the url and will pump its data into
      // DjVuViewer.
{
   DEBUG_MSG("QDViewerShell::openURL(): opening url '" << url << "'\n");
   DEBUG_MAKE_INDENT(3);

   if (!url.is_local_file_url())
     G_THROW(ERR_MSG("QDViewerShell.cant_display_remote") 
             + GUTF8String("\t")
             +(GUTF8String) url);

   QDViewer::PluginData plugin_data;
   GUTF8String fname="-";
   GURL clean_url;
   
   if (!url.is_empty())
     {
       clean_url=url;
       clean_url.clear_all_arguments();
       fname=clean_url.UTF8Filename();
       
       DArray<GUTF8String> cgi_names=url.djvu_cgi_names();
       DArray<GUTF8String> cgi_values=url.djvu_cgi_values();
       if (cgi_names.size()>0)
         plugin_data.parse(cgi_names, cgi_values);
     }
   
   try
     {
       char ch;
       GP<ByteStream> gstr=ByteStream::create(GURL::Filename::UTF8(fname),"rb");
       gstr->read(&ch, 1);
     } 
   catch(...)
     {
       QCString msg = tr("Failed to open file '%1' for reading").
         arg(QStringFromGString(fname)).utf8();
       G_THROW((const char*)msg);
     }
   
   if (fname!="-") 
     QeFileDialog::lastLoadDir=QFileInfo(QStringFromGString(fname)).dirPath();
   
   try
     {
       main->raiseWidget(wpaper);
       
       if (djvu)
         {
           djvu->detach();
           djvu=0;
         }
       djvu=new DjVuViewer(0, plugin_data);
       connect(djvu, SIGNAL(sigShowStatus(const QString &)),
               this, SLOT(slotShowStatus(const QString &)));
       connect(djvu, SIGNAL(sigGetURL(const GURL &, const GUTF8String &)),
               this, SLOT(slotGetURL(const GURL &, const GUTF8String &)));
       
       djvu->attach(vparent);
       main->raiseWidget(vparent);
       
       DataPool::load_file(clean_url);
       GP<DataPool> pool=DataPool::create(clean_url);
       
       // Pass the pool to the DjVuViewer
       djvu->newStream(clean_url, pool);
       slotAboutToShowMenu();
       QDViewer * viewer = djvu->getQDViewer();
       if (viewer)
         {
           connect(viewer, SIGNAL(sigQueryFullScreen(bool &)),
                   this, SLOT(slotQueryFullScreen(bool &)));
           connect(viewer, SIGNAL(sigToggleFullScreen(void)),
                   this, SLOT(slotToggleFullScreen(void)));
         }
       
     } 
   catch(const GException & exc)
     {
       main->raiseWidget(wpaper);
       if (djvu)
         {
           djvu->detach();
           djvu=0;
         }
       slotShowStatus(tr(START_STATUS));
       slotAboutToShowMenu();
       throw;
     }
}

QDViewerShell *
QDViewerShell::createNewShell(const GURL & url) const
{
   QDViewerShell * v=new QDViewerShell(0, "qd_viewer_shell");
   qeApp->setWidgetGeometry(v->topLevelWidget());
   v->show();
   if (!url.is_empty()) v->openURL(url);
   return v;
}

void
QDViewerShell::about(void)
{
  QDAboutDialog *d = new QDAboutDialog(this, "about", TRUE);
  d->exec();
  delete d;
}

void
QDViewerShell::help(void)
{
  GURL helpurl = getDjViewDataFile(DJVIEW_HELP_DJVU);
  if (helpurl.is_empty() )
    {
      QString mesg = 
        tr("Failed to locate file '%1'").arg(DJVIEW_HELP_DJVU);
      showError(this, tr("DjVu Error"), mesg);
    }
  else
    {
      createNewShell(helpurl);
    }
}

bool
QDViewerShell::isFullScreen() const
{
  return fullscreen;
}

void
QDViewerShell::slotToggleFullScreen()
{
#ifndef QT1
  if (isVisible())
    {
      hide();
      fullscreen = ! fullscreen;
      if (fullscreen)
        {
          menu->hide();
          status_frame->hide();
          showFullScreen();
        }
      else 
        {
          menu->show();
          status_frame->show();
          showNormal();
        }
    }
#endif
}

void
QDViewerShell::slotQueryFullScreen(bool &p)
{
  p |= fullscreen;
}

void
QDViewerShell::slotMenuCB(int cmd)
{
   try
   {
      if (cmd==IDC_EXIT) 
         qApp->quit();
      else if (cmd==IDC_CLOSE)
	 qeApp->killWidget(this);
      else if (cmd==IDC_OPEN)
      {
	 static const char * filters[]={ "*.djvu", "*.djv", 
                                         "*.iw44", "*.iw4", "*", 0 };

	 QeFileDialog dialog(QeFileDialog::lastLoadDir, filters[0],
			     this, "file_dialog", TRUE);
	 dialog.setCaption(tr("DjVu: Select DjVu file to open"));
	 dialog.setForWriting(FALSE);
	 dialog.setFilters((const char **) filters);

	 if (dialog.exec()==QDialog::Accepted)
	    openFile(dialog.selectedFile());
      } else if (cmd==IDC_NEW_WINDOW)
      {
	 createNewShell(GURL());
      } if (cmd==IDC_ABOUT_DEJAVU)
      {
	 about();
      } else if (cmd==IDC_HELP_DEJAVU)
      {
	 help();
      } else if (cmd==IDC_FULL_SCREEN)
      {
	 slotToggleFullScreen();
      } else
      {
	 QDViewer * viewer;
	 if (djvu && (viewer=djvu->getQDViewer()))
	    viewer->processCommand(cmd);
      }
   } catch(const GException & exc)
   {
      showError(this, tr("DjVu Error"), exc);
   }
}

void
QDViewerShell::slotGetURLTimeout(void)
{
   try
   {
      DEBUG_MSG("QDViewerShell::slotGetURLTimeout() called\n");
      DEBUG_MAKE_INDENT(3);
      DEBUG_MSG("url=" << gu_url << ", tgt='" << gu_target << "'\n");

      if (djvu!=gu_djvu)
      {
	 DEBUG_MSG("but the DjVuViewer, that ordered this stream has already gone => return\n");
	 return;
      }
      
      if (gu_url.is_local_file_url())
      {
	 GUTF8String fname=gu_url.UTF8Filename();
	 if (gu_target.length()) createNewShell(gu_url);
	 else
	 {
	    DEBUG_MSG("tgt='' => open file and send the stream to the same obj\n");
	    try
	    {
	       char ch;
	       GP<ByteStream> str
                 = ByteStream::create(GURL::Filename::UTF8(fname),"rb");
	       str->read(&ch, 1);
	    } catch(...)
	    {
		  // Don't want to report 'file not found' errors
	       QString mesg = tr("Failed to open file '%1'")
                 .arg( QStringFromGString(fname) );
	       slotShowStatus(mesg);
	       DEBUG_MSG("Failed to open url=" << gu_url << "\n");
	       return;
	    }
	       // Create the pool and pass it to DjVuViewer
	    GP<DataPool> pool=DataPool::create(gu_url);
	    if (djvu)
            {
              djvu->newStream(gu_url, pool);
            }
	 }
      } else
      {
	 G_THROW(ERR_MSG("main.cant_display_remote"));
      }
   } catch(const GException & exc)
   {
      showError(this, tr("DjVu Error"), exc);
   }
}

void
QDViewerShell::slotGetURL(const GURL & url, const GUTF8String &target)
{
   gu_djvu=sender();
   gu_url=url;
   gu_target=target.length() ? target : GUTF8String();   
   gu_timer.start(0, TRUE);
}

void
QDViewerShell::slotAboutToShowMenu(void)
{
  // The viewer does most of the job
  QDViewer * viewer = 0;
  if (djvu)
    viewer=djvu->getQDViewer();
  if (!viewer)
    setItemsEnabled(menu, FALSE);
  else 
    viewer->setupMenu(menu);
  // Check presence of other windows
  int instances = 0;
  QWidgetList  *list = QApplication::topLevelWidgets();
  for (QWidget *widget = list->first(); widget; widget=list->next())
    if (widget->isVisible() && !widget->isDesktop() && !widget->isPopup())
#ifndef QT1
      if (!widget->isPopup() && !widget->testWFlags(WStyle_Dialog))
#endif
        instances++;
  delete list;
  // Enable the top-level things, Open and Exit and some more
  for(u_int i=0;i<menu->count();i++)
    menu->setItemEnabled(menu->idAt(i), TRUE);
  menu->setItemEnabled(IDC_OPEN, TRUE);
  menu->setItemEnabled(IDC_NEW_WINDOW, TRUE);
  menu->setItemEnabled(IDC_CLOSE, (instances!=1) ? TRUE : FALSE);
  menu->setItemEnabled(IDC_EXIT, TRUE);
  menu->setItemEnabled(IDC_ABOUT_DEJAVU, TRUE);
  menu->setItemEnabled(IDC_HELP_DEJAVU, TRUE);
  // Set full screen mode
  menu->setItemChecked(IDC_FULL_SCREEN, isFullScreen());
}

void
QDViewerShell::slotShowStatus(const QString &qstatus)
{
  GUTF8String gstatus = GStringFromQString(qstatus);
  gstatus = DjVuMessage::LookUpUTF8(gstatus);
  for (unsigned int i=0; i<gstatus.length(); i++)
    if (gstatus[i]=='\t' || gstatus[i]=='\n')
      gstatus.setat(i,' ');
  status_bar->setText(QStringFromGString(gstatus));
}

void
QDViewerShell::closeEvent(QCloseEvent * e)
{
   // we need to accept the closing request first
   // in order to prevent an infinite recursion
   // see qwidget.cpp
   
   // close/hide the window then book it to be killed
   // note that the shell CAN'T be initialized with
   // WDestructiveClose
   e->accept();
   slotMenuCB(IDC_CLOSE);
}

QDViewerShell::QDViewerShell(QWidget * parent, const char * name)
   : QMainWindow(parent, name) // DON'T use WDestructiveClose
{
   fullscreen = false;
   setCaption(tr("DjVu Standalone Viewer"));

      // Set the icon for the shell
   GP<ByteStream> str=CINData::get("ppm_djview_icon");
   if (str)
   {
      GP<GPixmap> gpix=GPixmap::create(*str);
      setIcon(createIcon(*gpix));
   }

   connect(&gu_timer, SIGNAL(timeout(void)), this, SLOT(slotGetURLTimeout(void)));
   
   menu = menuBar();
#ifdef QT2
   menu->setSeparator(QMenuBar::InWindowsStyle);
#endif
   QPopupMenu * file_pane=new QPopupMenu(this);
   file_pane->insertItem(tr("&Open"), IDC_OPEN);
   file_pane->insertItem(tr("&New Window"), IDC_NEW_WINDOW);
   file_pane->insertSeparator();
   file_pane->insertItem(tr("Save D&ocument As..."), IDC_SAVE_DOC_AS);
   file_pane->insertItem(tr("Save P&age As..."), IDC_SAVE_PAGE_AS);
   file_pane->insertItem(tr("&Export page"), IDC_EXPORT_PAGE);
   file_pane->insertSeparator();
   file_pane->insertItem(tr("&Find"), IDC_SEARCH);
   file_pane->insertItem(tr("&Preferences"), IDC_PREFERENCES);
   file_pane->insertSeparator();
   file_pane->insertItem(tr("P&rint Page"), IDC_PRINT_PAGE);
   file_pane->insertItem(tr("Pr&int Document"), IDC_PRINT_DOC);
   file_pane->insertSeparator();
   file_pane->insertItem(tr("&Close"), IDC_CLOSE);
   file_pane->insertItem(tr("E&xit"), IDC_EXIT);
   connect(file_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   connect(file_pane, SIGNAL(activated(int)), this, SLOT(slotMenuCB(int)));
   menu->insertItem(tr("&File"), file_pane, IDC_FILE);

   QPopupMenu * mode_pane=new QPopupMenu(this);
   mode_pane->setCheckable(TRUE);
   mode_pane->insertItem(tr("&Color"), IDC_DISPLAY_COLOR);
   mode_pane->insertItem(tr("Black and &White"), IDC_DISPLAY_BLACKWHITE);
   mode_pane->insertItem(tr("&Background"), IDC_DISPLAY_BACKGROUND);
   mode_pane->insertItem(tr("&Foreground"), IDC_DISPLAY_FOREGROUND);
#ifndef QT1
   mode_pane->insertSeparator();
   mode_pane->insertItem(tr("Full &Screen"), IDC_FULL_SCREEN);
#endif
   connect(mode_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   connect(mode_pane, SIGNAL(activated(int)), this, SLOT(slotMenuCB(int)));
   menu->insertItem(tr("&Display"), mode_pane, IDC_MODE);

   QPopupMenu * zoom_pane=new QPopupMenu(this);
   zoom_pane->setCheckable(TRUE);
   zoom_pane->insertItem("&300 %", IDC_ZOOM_300);
   zoom_pane->insertItem("15&0 %", IDC_ZOOM_150);
   zoom_pane->insertItem("&100 %", IDC_ZOOM_100);
   zoom_pane->insertItem("&75 %", IDC_ZOOM_75);
   zoom_pane->insertItem("&50 %", IDC_ZOOM_50);
   zoom_pane->insertItem("&25 %", IDC_ZOOM_25);
   zoom_pane->insertItem(tr("&Custom..."), IDC_ZOOM_CUSTOM);
   zoom_pane->insertSeparator();
   zoom_pane->insertItem(tr("One &to One"), IDC_ZOOM_ONE2ONE);
   zoom_pane->insertItem(tr("&Stretch"), IDC_ZOOM_STRETCH);
   zoom_pane->insertItem(tr("Fit &Width"), IDC_ZOOM_WIDTH);
   zoom_pane->insertItem(tr("Fit &Page"), IDC_ZOOM_PAGE);
   zoom_pane->insertSeparator();
   zoom_pane->insertItem(tr("Zoom &In"), IDC_ZOOM_ZOOMIN);
   zoom_pane->insertItem(tr("Zoom &Out"), IDC_ZOOM_ZOOMOUT);
   connect(zoom_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   connect(zoom_pane, SIGNAL(activated(int)), this, SLOT(slotMenuCB(int)));
   menu->insertItem(tr("&Zoom"), zoom_pane, IDC_ZOOM);

   QPopupMenu * nav_pane=new QPopupMenu(this);
   nav_pane->insertItem(tr("&Next Page"), IDC_NAV_NEXT_PAGE);
   nav_pane->insertItem(tr("&Previous Page"), IDC_NAV_PREV_PAGE);
#if 0
   nav_pane->insertItem(tr("&+10 Pages"), IDC_NAV_NEXT_PAGE10);
   nav_pane->insertItem(tr("&-10 Pages"), IDC_NAV_PREV_PAGE10);
#endif
   nav_pane->insertItem(tr("&First Page"), IDC_NAV_FIRST_PAGE);
   nav_pane->insertItem(tr("&Last Page"), IDC_NAV_LAST_PAGE);
   nav_pane->insertItem(tr("&Goto Page..."), IDC_NAV_GOTO_PAGE);
   nav_pane->insertSeparator();
   nav_pane->insertItem(tr("&Back"), IDC_HISTORY_BACK);
   nav_pane->insertItem(tr("&Forward"), IDC_HISTORY_FORW);
   connect(nav_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   connect(nav_pane, SIGNAL(activated(int)), this, SLOT(slotMenuCB(int)));
   menu->insertItem(tr("&Navigate"), nav_pane, IDC_NAVIGATE);

   QPopupMenu * info_pane=new QPopupMenu(this);
   info_pane->insertItem(tr("&Page Information"), IDC_ABOUT_PAGE);
   info_pane->insertItem(tr("&Document Information"), IDC_ABOUT_DOC);
   info_pane->insertItem(tr("Show &Thumbnails"), IDC_THUMB_SHOW);
   info_pane->insertItem(tr("Show &Bookmarks"), IDC_BOOKMARKS_SHOW);
   connect(info_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   connect(info_pane, SIGNAL(activated(int)), this, SLOT(slotMenuCB(int)));
   menu->insertItem(tr("&Info"), info_pane, IDC_INFO);

   menu->insertSeparator();
   QPopupMenu * help_pane=new QPopupMenu(this);
   help_pane->insertItem(tr("&About"), IDC_ABOUT_DEJAVU);
   GURL helpurl = getDjViewDataFile(DJVIEW_HELP_DJVU);
   if (! helpurl.is_empty() )
     help_pane->insertItem(tr("&Help"), IDC_HELP_DEJAVU);
   connect(help_pane, SIGNAL(aboutToShow(void)), this, SLOT(slotAboutToShowMenu(void)));
   connect(help_pane, SIGNAL(activated(int)), this, SLOT(slotMenuCB(int)));
   menu->insertItem(tr("&Help"), help_pane);

   QWidget * central=new QWidget(this);
   QVBoxLayout * vlay=new QVBoxLayout(central, 0, 0, "vlay");
   main=new QWidgetStack(central, "main");
   vlay->addWidget(main, 1);
   status_frame=new QFrame(central);
   status_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
   vlay->addWidget(status_frame);
   QHBoxLayout * flay=new QHBoxLayout(status_frame, 2, 0);
   status_bar=new QLabel(tr("Status bar"), status_frame, "status_bar");
   status_bar->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));
   status_bar->setFrameStyle(QFrame::Panel | QFrame::Sunken);
   status_bar->setFixedHeight(status_bar->sizeHint().height());
   status_bar->setAlignment(AlignVCenter | AlignLeft);
   flay->addWidget(status_bar);
   setCentralWidget(central);

      // Create the startup image
   wpaper=0;
   str=CINData::get("bmp_djview_front");
   if (str)
   {
      GTArray<char> data;
      char buffer[1024];
      int length;
      while((length=str->read(buffer, 1024)))
      {
	 int data_size=data.size();
	 data.resize(data_size+length-1);
	 memcpy((char *) data+data_size, buffer, length);
      }
      QImage qimg;
      if (qimg.loadFromData((u_char *) (char *) data, data.size()))
      {
	 QPixmap qpix(qimg.width(), qimg.height(), x11Depth());
	 QPainter p(&qpix);
	 p.drawImage(0, 0, qimg);
	 p.end();

	    // OK. Successfully decoded image. Get rid of default
	    // wallpaper and create the label with the pixmap
	 QLabel * l=new QLabel(main, "wpaper");
	 l->setAlignment(AlignCenter);
	 l->setBackgroundColor(white);
	 l->setPixmap(qpix);
	 wpaper=l;
      }
   }
   if (!wpaper)
   {
      QDWPaper * wp=new QDWPaper(main, "wpaper");
      wp->setText(tr(START_STATUS));
      wpaper=wp;
   }

   vparent=new QWidget(main, "vparent");
   main->raiseWidget(wpaper);

   status_bar->setText(tr(START_STATUS));
   slotAboutToShowMenu();
}

QDViewerShell::~QDViewerShell(void)
{
   DEBUG_MSG("QDViewerShell::QDViewerShell(): destroying itself\n");
   DEBUG_MAKE_INDENT(3);
}

// END OF FILE
