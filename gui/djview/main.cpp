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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <locale.h>

#include "DjVuGlobal.h"
#include "ZPCodec.h"		// Wants to be before QT

#include "prefs.h"
#include "netscape.h"
#include "debug.h"
#include "GThreads.h"
#include "init_qt.h"
#include "qd_viewer_shell.h"
#include "exc_msg.h"
#include "DjVuDocument.h"
#include "GURL.h"
#include "djvu_file_cache.h"
#include "qlib.h"
#include "execdir.h"
#include "mime_check.h"
#include "version.h"

char		* progname;

/******************************************************************************
*************** Override memory operators to make them thread safe ************
******************************************************************************/
#if THREADMODEL!=COTHREADS

#ifdef NEED_DJVU_MEMORY
#error "NEED_DJVU_MEMORY conflicts with plugin's memory management."
#endif

// It's meaningless and dangerous to use GCriticalSections in the case
// of cothreads. Dangerous - because every usage of "new" even in QT library
// will result in a context switch. The worst thing is when it happens
// inside the QT library

static GCriticalSection * mem_lock;
void * operator new(size_t size)
{
   if (!mem_lock)
   {
      mem_lock=(GCriticalSection *) malloc(sizeof(GCriticalSection));
      new ((void *) mem_lock) GCriticalSection;
   }
   GCriticalSectionLock glock(mem_lock);
   return malloc(size+4);
}

void * operator new[](size_t size)
{
   if (!mem_lock)
   {
      mem_lock=(GCriticalSection *) malloc(sizeof(GCriticalSection));
      new ((void *) mem_lock) GCriticalSection;
   }
   GCriticalSectionLock glock(mem_lock);
   return malloc(size+4);
}

void operator delete(void * addr)
{
   try
   {
      if (addr)
      {
	 if (!mem_lock)
	 {
	    mem_lock=(GCriticalSection *) malloc(sizeof(GCriticalSection));
	    new ((void *) mem_lock) GCriticalSection;
	 }
	 GCriticalSectionLock glock(mem_lock);
	 free(addr);
      }
   } catch(const GException & exc)
   {
      cerr << exc.get_cause() << "\n";
   }
}
   
void operator delete[](void * addr)
{
   try
   {
      if (addr)
      {
	 if (!mem_lock)
	 {
	    mem_lock=(GCriticalSection *) malloc(sizeof(GCriticalSection));
	    new ((void *) mem_lock) GCriticalSection;
	 }
	 GCriticalSectionLock glock(mem_lock);
	 free(addr);
      }
   } catch(const GException & exc)
   {
      cerr << exc.get_cause() << "\n";
   }
}
#endif

static void ShowUsage(void)
{
   cout << "\
DJVU Viewer Version " << DJVIEW_VERSION_STR << "\n\n" <<
"Usage:\n\
	djview <options_list> <file_name>\n\
Options:\n\
	-file <file_name>	- alternative way to specify file name\n\
	-page <page_num>	- number of the page to display\n\
	-visual <number>	- use given visual instead of \"the best\"\n\
	-install		- force installation of private colormap\n\
	-help			- print this message\n"
#ifndef NO_DEBUG
"	-debug[=<level>]	- print debug information\n"
#endif
"	-style=motif		- Motif look and feel\n\
	-style=windows		- Win95 look and feel\n\
	-display <display>	- use given display\n\
	-fn <font_name>		- set default font\n\
	-fg <color>		- set default foreground\n\
	-bg <color>		- set default background\n\
	-geometry <geometry>	- set startup geometry\n\
	-fix_mime_types		- checks $HOME/.mime.types file\n\n";
}
      
/******************************************************************************
***************************** main() function *********************************
******************************************************************************/

int
main(int argc, char ** argv)
{
   DEBUG_MSG("main(): Starting the program\n");
   DEBUG_MAKE_INDENT(3);

  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
   
   try
   {
      int i;
      int netscape=0;
      char * ptr;
      for(ptr=progname=argv[0];*ptr;ptr++)
	 if (*ptr=='/') progname=ptr+1;

	 // Remove double dashes if present
      for(i=1;i<argc;i++)
      {
	 char * opt=argv[i];
	 if (opt[0]=='-' && opt[1]=='-' && opt[2]!=0)
	    for(char * ptr=opt;*ptr;ptr++)
	       ptr[0]=ptr[1];
      }

#ifndef NO_DEBUG
      {
	 const char * debug=getenv("DEBUG");
	 if (debug)
	 {
	    int level=atoi(debug);
	    if (level<0) level=0;
	    if (level>32) level=32;
	    DEBUG_SET_LEVEL(level);
	 }

         FILE *fd=fopen("/tmp/djview.log", "w");
         if (fd) DjVuDebug::set_debug_file(fd);
      }
#endif

      ParseQTArgs(argc, argv);

      int page_num=0;
      GUTF8String file_name;
      for(i=1;i<argc;i++)
#ifndef NO_DEBUG
	 if (!strncmp(argv[i], "-debug", 6))
	 {
	    DEBUG_MSG("found '-debug' flag\n");
	    int level=1;
	    if (strlen(argv[i])>6 && argv[i][6]=='=')
	       level=atoi(argv[i]+7);
	    if (level<0) level=0;
	    if (level>32) level=32;

	    DEBUG_MSG("setting debug level to " << level << "\n");
	    DEBUG_SET_LEVEL(level);
	 } else
#endif
	 if (!strcmp(argv[i], "-netscape"))
         {
	    DEBUG_MSG("found '-netscape' flag\n");
	    netscape=1;
	 } else if (!strcmp(argv[i], "-file"))
	 {
	    DEBUG_MSG("found '-file' flag\n");
	    if (i==argc-1)
	    {
	       cerr << "Missing value for flag '-file'. Ignoring...";
	       continue;
	    }
	    file_name=argv[++i];
	    DEBUG_MSG("file_name=" << (const char *) file_name << "\n");
	 } else if (!strcmp(argv[i], "-page"))
	 {
	    if (i==argc-1)
	    {
	       cerr << "Option '-page' must be followed by the page number.\n";
	       continue;
	    }
	    page_num=atoi(argv[++i]);
	    if (page_num<=0)
	    {
	       cerr << "Page number must be positive. Option '-page' ignored.\n";
	       page_num=0;
	    };
	 } else if (!strncmp(argv[i], "-page=", 6))
	 {
	    page_num=atoi(argv[i]+6);
	 } else if (!strcmp(argv[i], "-help") ||
		    !strcmp(argv[i], "--help") ||
		    !strcmp(argv[i], "-?") ||
		    !strcmp(argv[i], "/?"))
	 {
	    ShowUsage();
	    exit(0);
	 } else if (argv[i][0]!='-' || !argv[i][1])
	 {
	    file_name=argv[i];
	    DEBUG_MSG("file_name=" << file_name << "\n");
	 } else if (!strcmp(argv[i], "-fix_mime_types"))
	 {
	    fixMimeTypes();
	    exit(0);
	 } else cerr << "Unknown option '" << argv[i] << "' encountered.\n";

      if (page_num>0 && !file_name.length())
      {
	 cerr << "Option '-page' is meaningless when the file name is omitted.\n";
	 page_num=0;
      }

	 // HERE page_num is encoded as:
	 //    =0: not specified
	 //    >0: number from 1 to whatever

      DjVuPrefs prefs;
      get_file_cache()->set_max_size(prefs.pcacheSize*1024*1024);      
      
      signal(SIGCLD, SIG_IGN);

      if (!netscape)
      {
	 DEBUG_MSG("working as a standalone viewer\n");
	 InitStandalone(argc, argv);

      	 QDViewerShell * shell=new QDViewerShell(0, "main_window");
	 qeApp->setWidgetGeometry(shell);
	 shell->setDjVuDir(GStringFromQString(getExecDir(argv[0])));
	 shell->show();

	 try
	 {
	    if (file_name.length())
	    {
	       GURL url;
	       try
	       {
		  url=GURL::Filename::UTF8(file_name);
	       } catch(...) {}

	       if (!url.is_local_file_url())
		  throw ERROR_MESSAGE("main",ERR_MSG("main.cant_display_remote"));
	       if (page_num>0)
	       {
		     // Get rid of page specification via '#'
		  if (url.hash_argument().length())
		     url.clear_hash_argument();
		  
		     // And append new page selector
		  url.add_djvu_cgi_argument("PAGE", GUTF8String(page_num));
	       }
	       shell->openURL(url);
	    }
	 } catch(const GException & exc)
	 {
	    showError(shell, "DjVu Error", exc);
	 }

	 try
	 {
	    // window instances are managed in qd_viewer_shell
	    //qApp->connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()) );
	    exit(qApp->exec());
	 } catch(const GException & exc)
	 {
	    fprintf(stderr, "%s\n", (const char *) exc.get_cause());
	    exit(1);
	 }
      } else
      {
	 DEBUG_MSG("working in cooperation with netscape\n");
	 WorkWithNetscape();
      }

   } catch(const GException & exc)
   {
      exc.perror();
      exit(1);
   }
   return 0;
}

