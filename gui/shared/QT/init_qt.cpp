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

#include <qfileinfo.h>
#include <qtranslator.h>

#include "djvu_file_cache.h"
#include "init_qt.h"
#include "debug.h"
#include "GString.h"
#include "DjVuMessage.h"
#include "GURL.h"
#include "exc_msg.h"
#include "exc_res.h"
#include "qx_imager.h"

#include "prefs.h"

#if THREADMODEL==COTHREADS
#include "qd_thr_yielder.h"
#endif
#include <signal.h>
#include <stdio.h>

#include <q1xcompatibility.h>

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

static int (*x11PreviousErrorHandler)(Display*,XErrorEvent*) = 0;

static int
x11ErrorHandler(Display *displ, XErrorEvent *event)
{
  char buffer[1024];
  switch (event->error_code)
    {
    case BadPixmap:
    case BadAccess:
#ifdef NO_DEBUG
      // do not even say anything
      break;  
#endif
    case BadDrawable:
    case BadAtom:
    case BadCursor:
    case BadFont:
    case BadMatch:
    case BadName:
    case BadValue:
    case BadGC:
      // display message but try to continue
      XGetErrorText(displ, event->error_code, buffer, 1024);
      fprintf(stderr, "X11: %s\n", buffer);
      break;
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

static void
InstallErrorHandlers(void)
{
   DEBUG_MSG("InstallErrorHandlers(): Installing custom error handlers\n");
   qInstallMsgHandler(&qtErrorHandler);
   if (x11PreviousErrorHandler == 0)
     x11PreviousErrorHandler = XSetErrorHandler(x11ErrorHandler);
}



// ----------------- language translator


static bool
InstallLangTranslator(void)
{
#ifndef QT1
   // guess the char set 
  QFont::CharSet char_set = QFont::charSetForLocale();
  
  // load locale sepcific font 
#ifdef QT2
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
   bool have_translation=false;
   QString encoding=QFont::encodingName(char_set);
   QString lang = "lang_"+encoding+".qm";
   GList<GURL> paths = DjVuMessage::GetProfilePaths();
   for(GPosition pos=paths; !have_translation && pos; ++pos)
     {
       const GURL::UTF8 url(GStringFromQString(lang),paths[pos]);
       if(url.is_file())
         {
           lang=QStringFromGString(url.pathname());
           have_translation=true;
         }
     }
   if (!have_translation)
     return false;
   QTranslator *translator = new QTranslator(qApp);
   if ( translator->load(lang) )
     qApp->installTranslator(translator);
#endif
   return true;
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

   // initialize application
   QApplication::setColorSpec( QApplication::ManyColor );
   new QeApplication(argc, argv);
   
   // Setup applications properties
   DjVuPrefs prefs;
   qeApp->gamma = prefs.dScreenGamma;
   qeApp->geometry = "800x600";
   if (geometry.length())
     qeApp->geometry = geometry;
   if (name.length())
     qeApp->setName(name);
   
   // Customize style
#ifndef QT1
   if (style == "cde")
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
   InstallErrorHandlers();
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



