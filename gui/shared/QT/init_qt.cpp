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
// $Id: init_qt.cpp,v 1.25 2008/08/05 20:52:26 bpearlmutter Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "debug.h"
#include "GString.h"
#include "DjVuMessage.h"
#include "GURL.h"

#include <qfileinfo.h>
#include <qtranslator.h>
#include <qstyle.h>

#include "qlib.h"
#include "qx_imager.h"
#include "djvu_file_cache.h"
#include "execdir.h"
#include "init_qt.h"

#include "prefs.h"

#if THREADMODEL==COTHREADS
#include "qd_thr_yielder.h"
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef HAVE_X11_EXTENSIONS_XEXT_H
#include <X11/extensions/Xext.h>
#endif


Display		* displ;
Visual		* visual;
Colormap	colormap;
int		depth;

// ----------------- error handlers

static void
qtErrorHandler(QtMsgType type, const char *msg)
{
   switch(type)
   {
      case QtDebugMsg:
	 fprintf(stderr, "QT Debug: %s\n", msg);
	 break;
      case QtWarningMsg:
        if (!strstr(msg, "QGManager"))	// Ignore buggy QGManager errors
	    fprintf(stderr, "QT Warning: %s\n", msg);
	 break;
      case QtFatalMsg:
        fprintf(stderr, "QT Fatal: %s\n", msg);
         break;
   }
}

typedef int x11_error_handler_t(Display*,XErrorEvent*);
x11_error_handler_t *x11PreviousErrorHandler = 0;

typedef int x11_ext_error_handler_t(Display *, char *, char *);
x11_ext_error_handler_t *x11PreviousExtErrorHandler = 0;

static int
x11ErrorHandler(Display *displ, XErrorEvent *event)
{
  char buffer[1024];
  switch (event->error_code)
    {
    case BadPixmap:
    case BadAccess:
#ifdef NDEBUG
      // do not even say anything
      break;  
#endif
    case BadMatch:
    case BadDrawable:
    case BadAtom:
    case BadCursor:
    case BadFont:
    case BadName:
    case BadValue:
    case BadGC:
#if QT_VERSION < 220
      if (event->error_code == BadMatch && event->request_code == 42) break;
      XGetErrorText(displ, event->error_code, buffer, 1024);
      fprintf(stderr, "X11: %s\n", buffer);
      break;
#endif
    default:
      // use whatever error handler was set by the toolkit
      if (x11PreviousErrorHandler)
        return x11PreviousErrorHandler(displ, event);
      XGetErrorText(displ, event->error_code, buffer, 1024);      
      fprintf(stderr, "X11: %s\n", buffer);
      abort();
    }
  return 0;
}

#ifdef QT2
#ifdef HAVE_X11_EXTENSIONS_XEXT_H
static int
x11ExtErrorHandler(Display *dpy, char *extname, char *cause)
{
  // Work around useless warning in Qt2
  if (strcmp(extname,"RENDER") != 0)
    if (x11PreviousExtErrorHandler)
      return (*x11PreviousExtErrorHandler)(dpy, extname, cause);
  return 0;
}
#endif
#endif

static void
InstallErrorHandlers(void)
{
   DEBUG_MSG("InstallErrorHandlers(): Installing custom error handlers\n");
   qInstallMsgHandler(&qtErrorHandler);
   if (x11PreviousErrorHandler == 0)
     x11PreviousErrorHandler = (x11_error_handler_t *)
       XSetErrorHandler(x11ErrorHandler);
#ifdef QT2
#ifdef HAVE_X11_EXTENSIONS_XEXT_H
   if (x11PreviousExtErrorHandler == 0)
     x11PreviousExtErrorHandler = (x11_ext_error_handler_t *) 
       XSetExtensionErrorHandler(x11ExtErrorHandler);
   if (x11PreviousExtErrorHandler == 0)
     x11PreviousExtErrorHandler = (x11_ext_error_handler_t *)
       XSetExtensionErrorHandler(x11ExtErrorHandler);
#endif
#endif
}



// ----------------- language translator


static bool
InstallLangTranslator(void)
{
#ifndef QT1
#ifdef QT2
   // guess the char set 
  QFont::CharSet char_set = QFont::charSetForLocale();
   // load locale sepcific font 
  QFont font;
  font.setCharSet(char_set);
  switch (char_set)
    {
#if QT_VERSION >= 220
    case QFont::Set_Big5:
    case QFont::Big5:
      font.setPointSize(16);
      font.setWeight(50);
      break;
#endif
    default:
      break;
    }
   if ( font != QApplication::font() ) 
     QApplication::setFont( font, TRUE );
#endif
   // load language translation files
   GURL urla = getDjViewDataFile("qt.qm");
   if (urla.is_file()) 
     {
       QTranslator *trans = new QTranslator(qApp);
       if (trans->load(QStringFromGString(urla.pathname())))
         qApp->installTranslator(trans);
     }
   GURL pn = GURL::Filename::UTF8(DjVuMessage::programname());
   GURL urlb = getDjViewDataFile(pn.fname() + ".qm");
   if (urlb.is_file())
     {
       QTranslator *trans = new QTranslator(qApp);
       if (trans->load(QStringFromGString(urlb.pathname())))
         qApp->installTranslator(trans);
     }
   else
     {
       GURL urlc = getDjViewDataFile("djview.qm");
       if (urlc.is_file())
         {
           QTranslator *trans = new QTranslator(qApp);
           if (trans->load(QStringFromGString(urlc.pathname())))
             qApp->installTranslator(trans);
         }
     }
   return true;
#endif
   return false;
}




// ----------------- initialize


void
InitializeQT(int &argc, char ** argv)
      // It parses the command line arguments and initializes QApplication
      // like it QApplication(argc, argv) would do it with the exception
      // that we force it to use OUR displ:visual:colormap:depth data
{
   DEBUG_MSG("InitializeQT(): Initializing QApplication\n");
   DEBUG_MAKE_INDENT(3);

   // extract useful arguments
   QString name;
   QString geometry;
   QString style;
   for (int i=0; i<argc-1; i++) 
     {
       char *arg = argv[i];
       if (arg[0] != '-')
         continue;
       if (arg[1] == '-')
         argv[i] = arg = arg + 1;
       arg = arg + 1;
       if (!strcmp(arg,"geometry"))
         geometry = argv[++i];
       else if (!strncmp(arg,"geometry=",9))
         geometry = arg+9;
       else if (!strcmp(arg,"name"))
         name = argv[++i];
       else if (!strncmp(arg,"name=",5))
         name = arg+5;
       else if (!strncmp(arg,"style=",6))
         style = arg+6;
     }

   // QT and X11 error handlers
   InstallErrorHandlers();

   // Make sure Xgl does not select transparent visuals
#if HAVE_SETENV
   setenv("XLIB_SKIP_ARGB_VISUALS", "1", 1);
#else
   putenv("XLIB_SKIP_ARGB_VISUALS=1");
#endif

   // initialize application
   QApplication::setColorSpec( QApplication::ManyColor );
   new QeApplication(argc, argv);
   
   // Setup applications properties
   DjVuPrefs prefs;
   qeApp->gamma = prefs.dScreenGamma;
   qeApp->geometry = "680x480";
   if (geometry.length())
     qeApp->geometry = geometry;
   if (name.length())
     qeApp->setName(name);
   
   // Customize CDE style
#ifdef QT2
   if (qApp->style().inherits("QCDEStyle"))
     {
       QPalette p( QColor( 75, 123, 130 ) );
       p.setColor( QPalette::Active, QColorGroup::Base, QColor( 55, 77, 78 ) );
       p.setColor( QPalette::Inactive, QColorGroup::Base, QColor( 55, 77, 78 ) );
       p.setColor( QPalette::Disabled, QColorGroup::Base, QColor( 55, 77, 78 ) );
       p.setColor( QPalette::Active, QColorGroup::Highlight, Qt::white );
       p.setColor( QPalette::Active, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
       p.setColor( QPalette::Inactive, QColorGroup::Highlight, Qt::white );
       p.setColor( QPalette::Inactive, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
       p.setColor( QPalette::Disabled, QColorGroup::Highlight, Qt::white );
       p.setColor( QPalette::Disabled, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
       p.setColor( QPalette::Active, QColorGroup::Foreground, Qt::white );
       p.setColor( QPalette::Active, QColorGroup::Text, Qt::white );
       p.setColor( QPalette::Active, QColorGroup::ButtonText, Qt::white );
       p.setColor( QPalette::Inactive, QColorGroup::Foreground, Qt::white );
       p.setColor( QPalette::Inactive, QColorGroup::Text, Qt::white );
       p.setColor( QPalette::Inactive, QColorGroup::ButtonText, Qt::white );
       p.setColor( QPalette::Disabled, QColorGroup::Foreground, Qt::lightGray );
       p.setColor( QPalette::Disabled, QColorGroup::Text, Qt::lightGray );
       p.setColor( QPalette::Disabled, QColorGroup::ButtonText, Qt::lightGray );
       qApp->setPalette( p, TRUE );
     }
#endif
  
   // Qt twiddles
   InstallLangTranslator();
   
#ifdef QT1
   // The following block is to workaround bug in KWM packages with SuSE 6.1
   // This requires modified QT library, which will not be setting
   // maximum size of a window when ::kwm_bug is TRUE.
   Atom atom=XInternAtom(displ, "KWM_RUNNING", True);
   if (atom)
   {
     Window root=qt_xrootwin();
     Atom type;
     int format;
     u_long items, bytes_to_go;
     u_char * prop;
     if (XGetWindowProperty(displ, root, atom, 0, 1,
                            False, atom, &type, &format, &items,
                            &bytes_to_go, &prop)==Success && prop && type)
       {
	 extern bool kwm_bug;
	 if (*(u_int32 *) prop) 
           kwm_bug = true;
	 XFree(prop);
      }
   }
#endif

   // Cothreads
#if THREADMODEL==COTHREADS
   if ( !QDThrYielder::isInitialized() ) 
     QDThrYielder::initialize();
#endif

   // Imager
#ifdef QT1
   Display *displ = QPaintDevice::x__Display;
   void *visual = QPaintDevice::x11Visual();
   Colormap colormap = (Colormap) QPaintDevice::x11Colormap();
   int depth = QPaintDevice::x11Depth();
#else
   Display *displ = QPaintDevice::x11AppDisplay();
   void *visual = QPaintDevice::x11AppVisual();
   Colormap colormap = (Colormap)QPaintDevice::x11AppColormap();
   int depth = QPaintDevice::x11AppDepth();
#endif
   new QXImager(displ, visual, colormap, depth, false, prefs.optimizeLCD);
}




// ----------------- terminate cleanly

void 
CleanupQT(void)
{
  if (qeImager)
    delete qeImager;
  qeImager = 0;
  qxImager = 0;
  if (qeApp)
    delete qeApp;
  qeApp = 0;
}



