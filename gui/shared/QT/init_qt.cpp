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
//#include "qt_hack.h"
#include "debug.h"
#include "GString.h"
#include "DjVuMessage.h"
#include "GURL.h"
#include "exc_msg.h"
#include "exc_res.h"
#include "qx_imager.h"

#include "prefs.h"

extern char             * progname;

#if THREADMODEL==COTHREADS
#include "qd_thr_yielder.h"
#endif

#include <iostream.h>
#include <signal.h>
#include <stdio.h>

#include <qwindowsstyle.h>
#include <qmotifstyle.h>
#if ( QT_VERSION >= 220 )
#include <qmotifplusstyle.h>
#include <qsgistyle.h>
#endif
#if ( QT_VERSION >= 210 )
#include <qplatinumstyle.h>
#include <qcdestyle.h>
#endif

#include <q1xcompatibility.h>

static QWindowsStyle *windows = 0;
static QMotifStyle *motif = 0;
#if ( QT_VERSION >= 220 )
static QMotifPlusStyle *motifplus = 0;
static QSGIStyle *sgistyle = 0;
#endif
#if ( QT_VERSION >= 210 )
static QPlatinumStyle *platinum = 0;
static QCDEStyle *cde = 0;
#endif

#define QTNAMESPACE_QT Qt

Display		* displ;
Visual		* visual;
Colormap	colormap;
int		depth;

static QeApplication * qa;

static char * appFont, * appBGCol, * appFGCol, * appName, * mwGeometry;
static char * appStyle, * displ_name, * visual_name;
static bool install_cmap;
#ifndef NO_DEBUG
static bool xsynchronize;
#endif

static void
FakeDisplayStructure(Display * displ, Visual * visual,
		     Colormap colormap, int depth)
{
   DEBUG_MSG("FakeDisplayStructure(): Modifying the DISPLAY structure\n");
   
   int screen_num=DefaultScreen(displ);
   DefaultVisual(displ, screen_num)=visual;
   DefaultColormap(displ, screen_num)=colormap;
   DefaultDepth(displ, screen_num)=depth;

   XColor cell;
   if (XParseColor(displ, colormap, "white", &cell) &&
       XAllocColor(displ, colormap, &cell))
      WhitePixel(displ, screen_num)=cell.pixel;
   
   if (XParseColor(displ, colormap, "black", &cell) &&
       XAllocColor(displ, colormap, &cell))
      BlackPixel(displ, screen_num)=cell.pixel;
}

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

static int (*x11PreviousErrorHandler)(Display * displ, XErrorEvent * event) = 0;

static int
x11ErrorHandler(Display * displ, XErrorEvent * event)
{
  char buffer[1024];
  switch (event->error_code)
    {
    case BadPixmap:
    case BadAccess:
      // do not even say anything
#ifdef NO_DEBUG
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


static bool
InstallLangTranslator(void)
{
#ifndef QT1
   static QTranslator *translator = 0;      
   
   if ( translator ) return TRUE;

   // guess the char set 
   QFont::CharSet char_set = QFont::charSetForLocale();
   
   // get the encoding name of current locale
   QString encoding=QFont::encodingName(char_set);
   
   // load language translation files
   bool have_translation=false;
   QString lang = "lang_"+encoding+".qm";     /// encoding and not locale ???  LYB
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
      return FALSE;
   
   // load locale sepcific font 
#ifdef QT2
   QFont font;
   font.setCharSet(char_set);
   // for testing only 
#if ( QT_VERSION >= 220 )
   if ( char_set == QFont::Set_Big5 
        || char_set == QFont::Big5 )
     {
       font.setPointSize(16);
       font.setWeight(50);
     }
#endif
   if ( font != QApplication::font() ) {
      //cout << "loading new font\n";
      QApplication::setFont( font, TRUE );
   }
#endif
   if ( translator )
   {
      qApp->removeTranslator( translator );
      delete translator;
   }	
   translator = new QTranslator(0);
   translator->load(lang);
   qApp->installTranslator(translator);
#endif
   return TRUE;
}


static void
shift(int & argc, char ** argv, int index)
{
   for(int i=index;i+1<argc;i++) argv[i]=argv[i+1];
   argc--;
}

void
ParseQTArgs(int & argc, char ** argv)
{
   DEBUG_MSG("ParseQTArgs(): Parsing QT-related arguments\n");
   DEBUG_MAKE_INDENT(3);
   
   for(int i=1;i<argc;)
   {
      if (!strcmp(argv[i], "-display"))
      {
	 DEBUG_MSG("found '-display' flag\n");
	 if (i<argc-1)
	 {
	    displ_name=argv[i+1];
	    shift(argc, argv, i+1);
	    DEBUG_MSG("displ_name=" << displ_name << "\n");
	 } else cerr << "Missing value for flag '-display'. Ignoring...";
	 shift(argc, argv, i);
      } else if (!strcmp(argv[i], "-install"))
      {
	 DEBUG_MSG("found '-install' flag\n");
	 shift(argc, argv, i);
	 install_cmap=1;
      } else if (!strcmp(argv[i], "-visual"))
      {
	 DEBUG_MSG("found '-visual' flag\n");
	 if (i<argc-1)
	 {
	    visual_name=argv[i+1];
	    shift(argc, argv, i+1);
	    DEBUG_MSG("visual_name=" << visual_name << "\n");
	 } else cerr << "Missing value for flag '-visual'. Ignoring...\n";
	 shift(argc, argv, i);
      } else if (!strcmp(argv[i], "-fn") || !strcmp(argv[i], "-font"))
      {
	 DEBUG_MSG(argv[i] << " found\n");
	 if (i<argc-1)
	 {
	    appFont=argv[i+1];
	    shift(argc, argv, i+1);
	 } else cerr << "Option '" << argv[i] << "' must be followed by an argument.\n";
	 shift(argc, argv, i);
      } else if (!strcmp(argv[i], "-bg") || !strcmp(argv[i], "-background"))
      {
	 DEBUG_MSG(argv[i] << " found\n");
	 if (i<argc-1)
	 {
	    appBGCol=argv[i+1];
	    shift(argc, argv, i+1);
	 } else cerr << "Option '" << argv[i] << "' must be followed by an argument.\n";
	 shift(argc, argv, i);
      } else if (!strcmp(argv[i], "-fg") || !strcmp(argv[i], "-foreground"))
      {
	 DEBUG_MSG(argv[i] << " found\n");
	 if (i<argc-1)
	 {
	    appFGCol=argv[i+1];
	    shift(argc, argv, i+1);
	 } else cerr << "Option '" << argv[i] << "' must be followed by an argument.\n";
	 shift(argc, argv, i);
      } else if (!strcmp(argv[i], "-name"))
      {
	 DEBUG_MSG(argv[i] << " found\n");
	 if (i<argc-1)
	 {
	    appName=argv[i+1];
	    shift(argc, argv, i+1);
	 } else cerr << "Option '" << argv[i] << "' must be followed by an argument.\n";
	 shift(argc, argv, i);
      } else if (!strcmp(argv[i], "-geometry"))
      {
	 DEBUG_MSG(argv[i] << " found\n");
	 if (i<argc-1)
	 {
	    mwGeometry=argv[i+1];
	    shift(argc, argv, i+1);
	 } else cerr << "Option '" << argv[i] << "' must be followed by an argument.\n";
	 shift(argc, argv, i);
      } else if (!strcmp(argv[i], "-style"))
      {
	 DEBUG_MSG(argv[i] << " found\n");
	 if (i<argc-1)
	 {
	    appStyle=argv[i+1];
	    shift(argc, argv, i+1);
	 } else cerr << "Option '" << argv[i] << "' must be followed by an argument.\n";
	 shift(argc, argv, i);
      } else if (!strncmp(argv[i], "-style=", 7))
      {
	 DEBUG_MSG(argv[i] << " found\n");
         appStyle = argv[i]+7;
	 shift(argc, argv, i);
#ifndef NO_DEBUG
      } else if (!strcmp(argv[i], "-sync"))
      {
	 DEBUG_MSG(argv[i] << " found\n");
         xsynchronize = true;
	 shift(argc, argv, i);
#endif
      } else i++;
   }
}

void
InitializeQT(int argc, char ** argv)
      // It parses the command line arguments and initializes QApplication
      // like it QApplication(argc, argv) would do it with the exception
      // that we force it to use OUR displ:visual:colormap:depth data
{
   DEBUG_MSG("InitializeQT(): Initializing QApplication\n");
   DEBUG_MAKE_INDENT(3);

   FakeDisplayStructure(displ, visual, colormap, depth);

   qa=new QeApplication(displ);
   DjVuPrefs prefs;
   qa->gamma=prefs.dScreenGamma;

   if (appFont)
   {
      QFont font;
      font.setRawMode(TRUE);
      font.setFamily(appFont);
      QApplication::setFont(font);
   }
   if (appBGCol || appFGCol)
   {
      QColor bg;
      QColor fg;
      if (appBGCol) bg=QColor(appBGCol);
      else bg=QTNAMESPACE_QT::lightGray;
      if (appFGCol) fg=QColor(appFGCol);
      else fg=QTNAMESPACE_QT::black;
      QColorGroup cg(fg, bg, bg.light(),
		     bg.dark(), bg.dark(150), fg, QTNAMESPACE_QT::white);
      QColor disabled((fg.red()+bg.red())/2,
		      (fg.green()+bg.green())/2,
		      (fg.blue()+bg.blue())/2 );
      QColorGroup dcg(disabled, bg, bg.light(125),
		      bg.dark(), bg.dark(150), disabled, QTNAMESPACE_QT::white);
      QPalette pal(cg, dcg, cg);
      QApplication::setPalette(pal);
   }
   if (appName) qApp->setName(appName);
   else qApp->setName("DjVu");
   
   if (mwGeometry) qeApp->geometry=mwGeometry;
   else qeApp->geometry="800x600";

   if (appStyle)
   {
      QString style=appStyle; style.lower();
      if (style=="motif")
      {
	 if ( !motif ) motif = new QMotifStyle();
	 QApplication::setStyle(motif);
#if ( QT_VERSION >= 220 )
      }else if (style=="motifplus")
      {
	 if ( !motifplus ) motifplus = new QMotifPlusStyle();
	 QApplication::setStyle(motifplus);
      }else if (style=="sgi")
      {
	 if ( !sgistyle ) sgistyle = new QSGIStyle();
	 QApplication::setStyle(sgistyle);
#endif
#if ( QT_VERSION >= 210 )
      }else if (style=="platinum")
      {
	 if ( !platinum ) platinum = new QPlatinumStyle();
	 QApplication::setStyle(platinum);
      }else if (style=="cde")
      {
	 // from example themes.cpp
	 if ( !cde ) cde = new QCDEStyle();
	 QApplication::setStyle(cde);
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
#endif
      }else if (style=="windows")
      {
	 if ( !windows ) windows=new QWindowsStyle();
	 QApplication::setStyle(windows);
      }
      else cerr << "Unrecognized style '" << style << "' encountered.\n";
   } else
   {
      if ( !windows ) windows=new QWindowsStyle();
      QApplication::setStyle(windows);
   }      
  
   //HackQT();
   InstallErrorHandlers();
   InstallLangTranslator();
   
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
#ifdef QT1
	 extern bool kwm_bug;
	 if (*(u_int32 *) prop) kwm_bug=true;
#endif
	 XFree(prop);
      }
   }

#if THREADMODEL==COTHREADS
   if (!QDThrYielder::isInitialized()) QDThrYielder::initialize();
#endif
}

static GUTF8String
class2name(int vis_class)
{
   GUTF8String name;
   switch(vis_class)
   {
      case StaticGray: name="StaticGray"; break;
      case GrayScale: name="GrayScale"; break;
      case StaticColor: name="StaticColor"; break;
      case PseudoColor: name="PseudoColor"; break;
      case TrueColor: name="TrueColor"; break;
      case DirectColor: name="DirectColor"; break;
   }
   return name;
}

void
InitStandalone(int argc, char ** argv)
      // Call this function when you need a brand-new connection to display
      // If you have display/visual/colormap/depth already (set globally)
      // then just call InitializeQT()
{
   DEBUG_MSG("InitStandalone(): initializing standalone application\n");
   DEBUG_MAKE_INDENT(3);

   bool announce_visual=false;	// Tells if the chosen visual should be announced
   int cmd_visual_id=-1;
   if (visual_name)
   {
      int base=10;
      if (visual_name[0]=='0')
	 if (visual_name[1]=='x') base=16;
	 else base=8;
      char * ptr;
      cmd_visual_id=strtol(visual_name, &ptr, base);
      DEBUG_MSG("cmd_visual_id=" << cmd_visual_id << "\n");
      if (cmd_visual_id<=0)
      {
	 cerr << "Invalid value for opion '-visual': should be positive.\n";
	 announce_visual=true;
      }
   }

   displ=XOpenDisplay(displ_name);
   if (!displ) throw ERROR_MESSAGE("WorkStandalone",
				   GUTF8String("Failed to open display ")+
				   XDisplayName(displ_name));

   int screen_num=DefaultScreen(displ);
   VisualID def_vis_id=XVisualIDFromVisual(DefaultVisual(displ, screen_num));
   XVisualInfo vinfo_template;
   long vinfo_mask=0;
   XVisualInfo * vinfo_list=0;
   int vinfo_num=0;
   vinfo_template.screen=screen_num;
   vinfo_mask|=VisualScreenMask;
   DEBUG_MSG("getting information about available visuals matching template\n");
   if (!(vinfo_list=XGetVisualInfo(displ, vinfo_mask, &vinfo_template, &vinfo_num)))
      throw OUT_OF_MEMORY("WorkStandalone", "Failed to get the list of supported visuals.");
   DEBUG_MSG("got " << vinfo_num << " available visuals\n");

      // Get the requested visual name and depth
   GUTF8String cmd_visual_name;
   int cmd_visual_depth=-1;
   if (cmd_visual_id>0)
   {
      for(int i=0;i<vinfo_num;i++)
	 if ((int) vinfo_list[i].visualid==cmd_visual_id)
	 {
	    cmd_visual_name=class2name(vinfo_list[i].c_class);
	    cmd_visual_depth=vinfo_list[i].depth;
	    break;
	 }
      if (!cmd_visual_name.length())
      {
	 cerr << "Invalid visual ID 0x" << hex << cmd_visual_id << dec << ".\n";
	 announce_visual=true;
	 cmd_visual_id=-1;
      }
   }
   
      // Exlude not supported visuals from the list
   for(int i=0;i<vinfo_num;i++)
   {
      XVisualInfo * item=&vinfo_list[i];
	 // Note: QPixmap::convertToImage() fails for TrueColor visuals
	 // with depth=8. So we exclude it from the list of supported
	 // visuals. Luckily for us, Netscape doesn't choose it either,
	 // and in standalone mode there is usually a PseudoColor available
	 // along with this 8-bit TrueColor
	 // Note: DirectColor visual is poorly supported by QT. Since all
	 // cells in the colormap for DirectColor are read-writtable, QT
	 // is unable to allocate any color in it, and slows down tremendously
	 // sometimes dying with a SIGSEGV. Thus, we avoid DirectColor completely
      if (!(item->c_class==TrueColor && item->depth>8 ||
	    item->c_class==PseudoColor ||
	    item->c_class==StaticColor ||
	    item->c_class==StaticGray && item->depth==8 ||
	    item->c_class==GrayScale && item->depth==8))
      {
	 DEBUG_MSG("visual with ID=" << item->visualid << " is not supported\n");
	 for(int j=i;j<vinfo_num-1;j++)
	    vinfo_list[j]=vinfo_list[j+1];
	 vinfo_num--;
	 i--;
      }
   }
   DEBUG_MSG("only " << vinfo_num << " supported visuals left\n");

   XVisualInfo * found_vis_info=0;
   
      // Check that we support the requested visual (if it was requested)
   if (cmd_visual_id>0)
   {
      for(int i=0;i<vinfo_num;i++)
	 if ((int) vinfo_list[i].visualid==cmd_visual_id)
	 {
	    found_vis_info=&vinfo_list[i];
	    break;
	 }
      if (!found_vis_info)
      {
	 cerr << "Requested visual with ID 0x" << hex << cmd_visual_id << dec <<
	    " (" << cmd_visual_name << ", " << cmd_visual_depth << "bpp) is not supported.\n";
	 announce_visual=true;
      }
   }

   if (!found_vis_info)
   {
	 // Choose the best visual among the left ones

	 // Find maximum depth. We will then look for visuals with this
	 // depth only.
      int i=0, max_depth=0;
      for(i=0;i<vinfo_num;i++)
	 if (vinfo_list[i].depth>max_depth)
	    max_depth=vinfo_list[i].depth;
      DEBUG_MSG("found max depth=" << max_depth << "\n");
      if (max_depth<8)
      {
	 char buffer[1024];
	 sprintf(buffer, "Sorry, but display depth %d is not supported.\n"
		 "Please run this program on a different hardware.", max_depth);
	 throw ERROR_MESSAGE("WorkStandalone", buffer);
      }

      if (max_depth==8)
      {
	    // For this depth don't be too smart. See if default visual
	    // is supported and use it.
	 for(int i=0;i<vinfo_num;i++)
	    if (vinfo_list[i].visualid==def_vis_id)
	    {
	       DEBUG_MSG("for 8-bit display forcing default visual\n");
	       found_vis_info=&vinfo_list[i];
	       break;
	    }
      }

	 // Now just find a visual with the maximum depth. All of them
	 // are supported, so I just scan the list starting from more
	 // preferred visual types
      if (!found_vis_info)
	 for(i=0;i<vinfo_num;i++)
	    if (vinfo_list[i].depth==max_depth)
	       if (vinfo_list[i].c_class==TrueColor)
	       {
		  found_vis_info=&vinfo_list[i];
		  break;
	       }
      if (!found_vis_info)
	 for(i=0;i<vinfo_num;i++)
	    if (vinfo_list[i].depth==max_depth)
	       if (vinfo_list[i].c_class==PseudoColor)
	       {
		  found_vis_info=&vinfo_list[i];
		  break;
	       }
      if (!found_vis_info)
	 for(i=0;i<vinfo_num;i++)
	    if (vinfo_list[i].depth==max_depth)
	       if (vinfo_list[i].c_class==StaticColor)
	       {
		  found_vis_info=&vinfo_list[i];
		  break;
	       }
   }
   if (!found_vis_info)
      throw ERROR_MESSAGE("WorkStandalone", "Sorry, but your display is not supported.\n"
			  "I can work only with TrueColor, PseudoColor or StaticColor visuals.");
   DEBUG_MSG("using " << class2name(found_vis_info->c_class) << " visual\n");
   if (def_vis_id==found_vis_info->visualid && !install_cmap)
   {
      DEBUG_MSG("this is default visual and we're not forced to use own cmap => just go on\n");
      visual=DefaultVisual(displ, screen_num);
      colormap=DefaultColormap(displ, screen_num);
      depth=DefaultDepth(displ, screen_num);
   } else
   {
      DEBUG_MSG("this is NOT a default visual => use it\n");
	 // Want to see QT on its knees? Try to use AllocAll instead of AllocNone.
      colormap=XCreateColormap(displ, RootWindow(displ, screen_num),
			       found_vis_info->visual, AllocNone);
      visual=found_vis_info->visual;
      depth=found_vis_info->depth;
      DEBUG_MSG("created new colormap\n");
   }
   XFree(vinfo_list);

   if (announce_visual)
      cerr << "Using visual with ID 0x" << hex << visual->visualid << dec <<
	 " (" << class2name(visual->c_class) << ", " << depth << "bpp).\n";
   
   DjVuPrefs prefs;
   InitializeQT(argc, argv);
   new QXImager(displ, visual, colormap, depth, false, prefs.optimizeLCD);
#ifndef NO_DEBUG
   if (xsynchronize)  XSynchronize(displ, True);
#endif     
      // I don't want QT try to quit this application when the viewer's
      // window is closed. There may be an editor's one as well. So:
      // just create a dummy main widget
   QWidget * dummy_main=new QWidget();
   qApp->setMainWidget(dummy_main);
#ifndef STANDALONE_USE_CACHE
   get_file_cache()->enable(0);
#endif
}
