//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002-2003  Leon Bottou and Yann Le Cun.
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


#include "GException.h"
#include "GSmartPointer.h"
#include "GRect.h"
#include "GPixmap.h"
#include "GBitmap.h"
#include "DjVuImage.h"
#include "DjVuDocument.h"
#include "DjVuToPS.h"
#include "GOS.h"
#include "ByteStream.h"
#include "DjVuMessage.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>



void
usage(void)
{
  DjVuPrintErrorUTF8(
#ifdef DJVULIBRE_VERSION
          "DJVUPS --- DjVuLibre-" DJVULIBRE_VERSION "\n"
#endif
          "Convert DjVu documents into PostScript files\n"
          "\n"
          "Usage: djvups [<options>] [<document.djvu>]\n"
          "Recognized options are:\n"
          "  -page=<pagelists>                   (default: print all)\n"
          "  -format=<ps|eps>                    (default: ps)\n"
          "  -level=<1|2|3>                      (default: 2)\n"
          "  -orient=<auto|portrait|landscape>   (default: auto)\n"
          "  -mode=<color|bw|fore|back>          (default: color)\n"
          "  -zoom=<auto|25...2400)              (default: auto)\n"
          "  -color=<yes|no>                     (default: yes)\n"
          "  -gray                               (same as -color=no)\n"
          "  -colormatch=<yes|no>                (default: yes)\n"
          "  -gamma=<0.3...5.0>                  (default: 2.2)\n"
          "  -copies=<1...999999>                (default: 1)\n"
          "  -frame=<yes|no>                     (default: no)\n"
          "  -cropmarks=<yes|no>                 (default: no)\n"
          "  -text=<yes|no>                      (default: no)\n"
          "  -booklet=<no|recto|verso|yes>       (default: no)\n"
          "  -bookletmax=<n>                     (default: 0)\n"
          "  -bookletalign=<n>                   (default: 0)\n"
          "  -bookletfold=<n>[+<m>]              (default: 36+240)\n"
          "\n");
  exit(1);
}

void
complain(GUTF8String option, GUTF8String message)
{
  DjVuPrintErrorUTF8("djvups: (parsing option '%s'): %s\n", 
                     (const char *) option,
                     (const char *) message );
  exit(1);
}



int
main(int argc, char **argv)
{
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
  GArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
    dargv[i]=GNativeString(argv[i]);

  G_TRY
    {
      // Variables
      DjVuToPS printer;
      DjVuToPS::Options &options = printer.options;
      GUTF8String pages;
      GURL docname;
  
      // Process options
      while (argc>1 && dargv[1][0]=='-')
        {
          // get rid of initial dashes
          const char *s1 = (const char*)dargv[1];
          if (*s1 == '-')
            s1 ++;
          if (*s1 == '-')
            s1 ++;
          if (*s1 == 0)
            break;
          // search equal sign
          const char *s2 = s1;
          while (*s2 && *s2 != '=')
            s2 ++;
          // separate arguments
          GUTF8String s( s1, s2-s1 );
          GUTF8String arg( s2[0] && s2[1] ? s2+1 : "" );

          // rumble!
          if (s == "page" || s == "pages")
            {
              if (pages.length())
                pages = pages + ",";
              pages = pages + arg;
            }
          else if (s == "format")
            {
              if (arg == "ps")
                options.set_format(DjVuToPS::Options::PS);
              else if (arg == "eps")
                options.set_format(DjVuToPS::Options::EPS);
              else
                complain(dargv[1],"Invalid format. Use \"ps\" or \"eps\".");
            }
          else if (s == "level")
            {
              int endpos;
              int lvl = arg.toLong(0, endpos);
              if (endpos != (int)arg.length() || lvl < 1 || lvl > 4)
                complain(dargv[1],"Invalid Postscript language level.");
              options.set_level(lvl);
            }
          else if (s == "orient" || s == "orientation")
            {
              if (arg == "a" || arg == "auto" )
                options.set_orientation(DjVuToPS::Options::AUTO);
              if (arg == "l" || arg == "landscape" )
                options.set_orientation(DjVuToPS::Options::LANDSCAPE);
              if (arg == "p" || arg == "portrait" )
                options.set_orientation(DjVuToPS::Options::PORTRAIT);
              else
                complain(dargv[1],"Invalid orientation. Use \"auto\", "
                         "\"landscape\" or \"portrait\".");
            }
          else if (s == "mode")
            {
              if (arg == "c" || arg == "color" )
                options.set_mode(DjVuToPS::Options::COLOR);
              else if (arg == "black" || arg == "bw")
                options.set_mode(DjVuToPS::Options::BW);
              else if (arg == "fore" || arg == "foreground")
                options.set_mode(DjVuToPS::Options::FORE);
              else if (arg == "back" || arg == "background" )
                options.set_mode(DjVuToPS::Options::BACK);
              else
                complain(dargv[1],"Invalid mode. Use \"color\", \"bw\", "
                         "\"foreground\", or \"background\".");
            }
          else if (s == "zoom")
            {
              if (arg == "auto" || arg == "fit" || arg == "fit_page")
                options.set_zoom(0);
              else if (arg == "1to1" || arg == "onetoone")
                options.set_zoom(100);                
              else 
                {
                  int endpos;
                  int z = arg.toLong(0,endpos);
                  if (endpos != (int)arg.length() || z < 25 || z > 2400)
                    complain(dargv[1],"Invalid zoom factor.");
                  options.set_zoom(z);
                }
            }
          else if (s == "color")
            {
              if (arg == "yes" || arg == "")
                options.set_color(true);
              else if (arg == "no")
                options.set_color(false);
              else
                complain(dargv[1],"Invalid argument. Use \"yes\" or \"no\".");
            }
          else if (s == "gray" || s == "grayscale")
            {
              if (arg.length())
                complain(dargv[1],"No argument was expected.");
              options.set_color(false);
            }
          else if (s == "srgb" || s == "colormatch")
            {
              if (arg == "yes" || arg == "")
                options.set_sRGB(true);
              else if (arg == "no")
                options.set_sRGB(false);
              else
                complain(dargv[1],"Invalid argument. Use \"yes\" or \"no\".");
            }
          else if (s == "gamma")
            {
              int endpos;
              double g = arg.toDouble(0,endpos);
              if (endpos != (int)arg.length() || g < 0.3 || g > 5.0)
                complain(dargv[1],"Invalid gamma factor. Use a number "
                         "in range 0.3 ... 5.0.");
              options.set_gamma(g);
            }
          else if (s == "copies")
            {
              int endpos;
              int n = arg.toLong(0, endpos);
              if (endpos != (int)arg.length() || n < 1 || n > 999999)
                complain(dargv[1],"Invalid number of copies.");
              options.set_copies(n);
            }
          else if (s == "frame")
            {
              if (arg == "yes" || arg == "")
                options.set_frame(true);
              else if (arg == "no")
                options.set_frame(false);
              else
                complain(dargv[1],"Invalid argument. Use \"yes\" or \"no\".");
            }
          else if (s == "cropmarks")
            {
              if (arg == "yes" || arg == "")
                options.set_cropmarks(true);
              else if (arg == "no")
                options.set_cropmarks(false);
              else
                complain(dargv[1],"Invalid argument. Use \"yes\" or \"no\".");
            }
          else if (s == "text")
            {
              if (arg == "yes" || arg == "")
                options.set_text(true);
              else if (arg == "no")
                options.set_text(false);
              else
                complain(dargv[1],"Invalid argument. Use \"yes\" or \"no\".");
            }
          else if (s == "booklet")
            {
              if (arg == "no")
                options.set_bookletmode(DjVuToPS::Options::OFF);
              else if (arg == "recto")
                options.set_bookletmode(DjVuToPS::Options::RECTO);
              else if (arg == "verso")
                options.set_bookletmode(DjVuToPS::Options::VERSO);
              else if (arg == "rectoverso" || arg=="yes" || arg=="")
                options.set_bookletmode(DjVuToPS::Options::RECTOVERSO);
              else 
                complain(dargv[1],"Invalid argument."
                         "Use \"no\", \"yes\", \"recto\", or \"verso\".");
            }
          else if (s == "bookletmax")
            {
              int endpos;
              int n = arg.toLong(0, endpos);
              if (endpos != (int)arg.length() || n < 0 || n > 999999)
                complain(dargv[1],"Invalid argument.");
              options.set_bookletmax(n);
            }
          else if (s == "bookletalign")
            {
              int endpos;
              int n = arg.toLong(0, endpos);
              if (endpos != (int)arg.length() || n < -720 || n > +720)
                complain(dargv[1],"Invalid argument.");
              options.set_bookletalign(n);
            }
          else if (s == "bookletfold")
            {
              int endpos;
              int m = 250;
              int n = arg.toLong(0, endpos);
              if (endpos <= (int)arg.length() && arg[endpos]=='+')
                m = arg.toLong(endpos+1, endpos);
              if (endpos != (int)arg.length() || m<0 || m>720 || n<0 || n>9999 )
                complain(dargv[1],"Invalid argument.");
              options.set_bookletfold(n,m);
            }
          else
            {
              DjVuPrintErrorUTF8("djvups: Unrecognized option "
                                 "\"%s\"\n",(const char*)dargv[1]);
              usage();
            }
          // Next option
          argc -= 1;
          dargv.shift(-1);
        }

      // Obtain document name
      if (argc == 1)
        docname = GURL::Filename::UTF8("-");
      else if (argc == 2)
        docname = GURL::Filename::UTF8(dargv[1]);
      else
        usage();
      
      // Issue warnings
      if ( options.get_sRGB() && options.get_level() < 2)
        DjVuPrintErrorUTF8("Color matching requires PostScript "
                           "level 2 or greater.\n");

      // Open document
      GP<DjVuDocument> doc = DjVuDocument::create_wait(docname);
      if (! doc->wait_for_complete_init())
        G_THROW("Decoding failed.  Nothing can be done.");
      
      // Print
      GP<ByteStream> obs = ByteStream::create("w");
      printer.print(*obs, doc, pages );
    }
  G_CATCH(ex)
    {
      ex.perror();
      exit(1);
    }
  G_ENDCATCH;

  return 0;
}
