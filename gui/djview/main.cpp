//C-  -*- C++ -*-
//C- -----------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- DjVuLibre-3.5 is derived from the DjVu (r) Reference Library.
//C- 
//C- DjVu (r) Reference Library (v. 3.5)
//C- Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
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
//C- -----------------------------------------------------------
// 
// $Id$
// $Name$

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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


static void 
ShowUsage(void)
{
  DjVuPrintErrorUTF8("%s\n",
#ifdef DJVULIBRE_VERSION
          "DJVIEW --- DjVuLibre-" DJVULIBRE_VERSION "\n"
#endif
          "DjVu document viewer.\n\n"
          "Usage: djview <options_list> <file_name>\n"
          "Options:\n"
          "     -file <file_name>       - alternative way to specify file name\n"
          "     -page <page_num>        - number of the page to display\n"
          "     -help                   - print this message\n"
          "     -style=motif            - Motif look and feel\n"
          "     -style=windows          - Win95 look and feel\n"
          "     -geometry <geometry>    - specify X11 startup geometry\n"
          "     -display <display>      - specify X11 display name\n"
          "     -fn <fontname>          - specify Qt/X11 default font\n"
          "     -bg <color>             - specify Qt/X11 default background color\n"
          "     -fg <color>             - specify Qt/X11 default foreground color\n"
#ifndef QT1
          "     -btn <color>            - specify Qt/X11 default button color\n"
          "     -visual TrueColor       - forces use of a TrueColor visual (8 bit display)\n"
          "     -ncols <count>          - limit color palette use (8 bit display)\n"
          "     -cmap                   - install private colormap (8 bit display)\n"
#endif
#ifndef NO_DEBUG
#ifdef RUNTIME_DEBUG_ONLY
          "	-debug[=<level>]        - print debug information\n"
#endif
#endif
          "     -fix                    - fix configuration of Netscape helpers\n"
          );
}
      

int
main(int argc, char ** argv)
{
  DEBUG_MSG("main(): Starting the program\n");
  DEBUG_MAKE_INDENT(3);
  
  int i;
  int rc = 1;
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
   
  // Remove double dashes
  for (i=1; i<argc; i++)
    {
      char *arg = argv[i];
      if (arg[0] != '-')
        continue;
      if (arg[1] == '-')
        arg = argv[i] = arg + 1;
    }
  
  // Searches debug option
#ifndef NO_DEBUG
  const char *debug = 0;
  for (i=1; i<argc; i++)
    if (!strncmp(argv[i],"-debug",6)) 
      {
        debug = "1";
        if (argv[i][6] == '=')
          debug = argv[i]+7;
      }
  if (! debug)
    debug = getenv("DJVIEW_DEBUG");
  if (debug)
    {
      int level = atoi(debug);
      if (level > 0 && level < 32)
        DEBUG_SET_LEVEL(level);
      FILE *fd=fopen("/tmp/djview.log", "w");
      if (fd) DjVuDebug::set_debug_file(fd);
    }
#endif

  // Search options that require immediate action
  for (i=1; i<argc; i++)
    {
      if (!strcmp(argv[i],"-help") || 
          !strcmp(argv[i],"-h")    ||
          !strcmp(argv[i],"-?") )
        {
          ShowUsage();
          exit(0);
        }
      else if (!strncmp(argv[i],"-fix", 4))
        {
          fixMime();
          fixPlugins();
          exit(0);
        }
      else if (!strcmp(argv[i],"-netscape"))
        {
          DEBUG_MSG("working in cooperation with netscape\n");
          WorkWithNetscape();  // never exits
          exit(0); 
        }
    }
  
  // Initialize QT
  DEBUG_MSG("working as a standalone viewer\n");
  InitializeQT(argc, argv);
  

  // Forward
  G_TRY
    {
      // Scans remaining arguments
      int page_num = 0;
      GUTF8String file_name;
      for (i=1; i<argc; i++)
        {
          const char *arg = argv[i];
          
          if (!strncmp(arg,"-file=",6))
            arg = arg + 6;
          else if (!strcmp(arg,"-file") && i<argc-1)
            arg = argv[++i];
          else if (!strcmp(arg,"-page") && i<argc-1)
            {
              if (page_num)
                DjVuPrintErrorUTF8("djview: duplicate page specification\n");
              page_num = atoi(argv[++i]);
              continue;
            }
          else if (!strncmp(arg,"-page=",6) && i<argc-1)
            {
              if (page_num)
                DjVuPrintErrorUTF8("djview: warning: duplicate page specification\n");
              page_num = atoi(arg+6);
              continue;
            }
          else if (!strcmp(arg,"-name") && i<argc-1)
            {
              // We already processed this one (init_qt.cpp)
              i++;
              continue;
            }
          else if (!strncmp(arg,"-debug",6) ||
                   !strncmp(arg,"-geometry=",10) ||
                   !strcmp(arg,"-name=") )
            {
              // We already processed this one (above and in init_qt.cpp)
              continue;
            }
          else if (arg[0] == '-')
            {
              DjVuPrintErrorUTF8("djview: warning: unrecognized option '%s'\n", arg);
              continue;
            }
          
          if (! file_name)
            file_name = GNativeString(arg);
          else
            DjVuPrintErrorUTF8("djview: warning: duplicate filename specification\n");
        }
      if (page_num < 0)
        DjVuPrintErrorUTF8("djview: warning: illegal page number\n");
      if (page_num > 0 && !file_name)
        DjVuPrintErrorUTF8("djview: warning: page specification without a file name\n");
      
      // We are ready to fly
      DjVuPrefs prefs;
      get_file_cache()->set_max_size(prefs.pcacheSize*1024*1024);
#if defined(SIGCLD)
      signal(SIGCLD, SIG_IGN);
#elif defined (SIGCHLD)
      signal(SIGCHLD, SIG_IGN);
#endif
      QDViewerShell * shell=new QDViewerShell(0, "main_window");
      qeApp->setWidgetGeometry(shell);
      shell->show();
      
      if (file_name.length())
        {
          GURL url = GURL::Filename::UTF8(file_name);
          if (!url.is_local_file_url())
            throw ERROR_MESSAGE("main",ERR_MSG("main.cant_display_remote"));
          if (page_num > 0)
            {
              // Get rid of page specification via '#'
              if (url.hash_argument().length())
                url.clear_hash_argument();
              // And append new page selector
              url.add_djvu_cgi_argument("PAGE", GUTF8String(page_num));
            }
          shell->openURL(url);
        }
    
      qApp->connect( qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()) );
      rc = qApp->exec();
    }
  G_CATCH(ex)
    {
      ex.perror();
      rc = 1;
    }
  G_ENDCATCH;
  CleanupQT();
  exit(rc);
}


