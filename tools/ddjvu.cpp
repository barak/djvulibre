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

/** @name ddjvu

    {\bf Synopsis}
    \begin{verbatim}
        ddjvu [options] djvufilename [pnmfilename]
    \end{verbatim}

    {\bf Description} --- File #"ddjvu.cpp"# illustrates how to decode and
    render a DjVu file using class #DjVuImage#.  This program decodes all
    variants of DjVu files, including the wavelet files produced by \Ref{c44},
    and produces PNM files (see \Ref{PNM and RLE file formats}).

    {\bf Arguments} --- Argument #djvufilename# is the name of the input DjVu
    file.  A single dash #"-"# represents the standard input.  Argument
    #pnmfilename# is the name of the output PNM file.  Omitting this argument
    or specifying a single dash #"-"# represents the standard output.

    {\bf Output Resolution} --- Three options control the resolution of the 
    output PNM image.  At most one of these three options can be specified.
    The default resolution, used when no other option is specified, is equivalent
    to specifying #-scale 100#.
    \begin{description}
    \item[-N] This option (e.g #"-3"# or #"-19"#) specifies a subsampling
       factor #N#.  Rendering the full DjVu image would create an image whose
       dimensions are #N# times smaller than the DjVu image size.
    \item[-scale N] This option takes advantage of the #dpi# field stored in
       the #"INFO"# chunk of the DjVu image (cf. \Ref{DjVuInfo}).  Argument
       #N# is a magnification percentage relative to the adequate resolution
       for a 100dpi device such as a screen.
    \item[-size WxH] This option provides total control on the resolution and
       the aspect ratio of the image.  The vertical and horizontal resolutions
       will be separately adjusted in such a way that the complete DjVu image
       is rendered into a PNM file of width #W# and height #H#.
    \end{description}
    
    {\bf Rendering Mode Selection} --- The default rendering mode merges all
    the layers of the DjVu image and outputs an adequate PNM file. IW44 files
    Compound djVu files and Photo DjVu files are always rendered as PPM
    files. Bilevel DjVu files are rendered as PBM files if the
    subsampling factor is 1.  Otherwise, they are rendered as PGM files
    because the resolution change gives better results with anti-aliasing.
    Three options alter this default behavior.
    \begin{description}
    \item[-black] Renders only the foreground layer mask.  This mode does not
       work with IW44 files because these files have no foreground layer mask.
       The output file will be a PBM file if the subsampling factor is 1.
       Otherwise the output file will be an anti-aliased PGM file.
    \item[-foreground] Renders only the foreground layer on a white
       background.  This mode works only with Compound DjVu files. The output
       file always is a PPM file.
    \item[-background] Renders only the background layer. This mode works only
       with Compound DjVu files and IW44 files. The output file always is a PPM
       file.
    \end{description}

    {\bf Other Options} --- The following two options are less commonly used:
    \begin{description}
    \item[-segment WxH+X+Y] Selects an image segment to render. Conceptually,
       #ddjvu# renders the full page using the specified resolution, and
       then extracts a subimage of width #W# and height #H#, starting at
       position (#X#,#Y#) relative to the bottom left corner of the page.
       Both operations of course happen simultaneously.  Rendering a small
       subimage is much faster than rendering the complete image.  Note that
       the output PNM file will always have size #WxH#.
    \item[-v] Causes #ddjvu# to print abundant information about the
       structure of the DjVu file, the compression ratios, the memory usage,
       and the decoding and rendering times.
    \item[-page N] This can be used to decode a specific page of a multipage
       document. Page numbers start from #1#. If this option is omitted,
       #1# is assumed.
    \end{description}

    @memo
    Decodes and renders a DjVu file.
    @author
    Yann Le Cun <yann@research.att.com>\\
    L\'eon Bottou <leonb@research.att.com>
    @version
    #$Id$# */
//@{
//@}

#include "GException.h"
#include "GSmartPointer.h"
#include "GRect.h"
#include "GPixmap.h"
#include "GBitmap.h"
#include "DjVuImage.h"
#include "DjVuDocument.h"
#include "GOS.h"
#include "ByteStream.h"
#include "DjVuMessage.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef UNDER_CE
#include <windows.h>
// This captures stderr to a file for this compilation unit.
FILE * logfile = _wfreopen( TEXT("\\temp\\ddjvu_log.txt"), TEXT("a"), stderr );
#endif

static double flag_scale = -1;
static int    flag_size = -1;
static int    flag_subsample = -1;
static int    flag_segment = -1;
static int    flag_verbose = 0;
static int    flag_mode = 0;
static bool   use_rle=false;

struct DDJVUGlobal 
{
  // Globals that need static initialization
  // are grouped here to work around broken compilers.
  GRect fullrect;
  GRect segmentrect;
};

static DDJVUGlobal& g(void)
{
  DDJVUGlobal g;
  return g;
}


static void
convert(const GURL &from, const GURL &to, int page_num)
{
  unsigned long start, stop;

  // Create DjVuDocument
  GP<DjVuDocument> doc=DjVuDocument::create_wait(from);
  if (! doc->wait_for_complete_init())
    G_THROW( ERR_MSG("ddjvu.failed") );
  
  // Create DjVuImage
  start=GOS::ticks();
  GP<DjVuImage> dimg=doc->get_page(page_num);
  if (!dimg || ! dimg->wait_for_complete_decode() )
    G_THROW( ERR_MSG("ddjvu.failed") );
  stop=GOS::ticks();

  // Verbose
  if (flag_verbose)
    {
      DjVuFormatErrorUTF8( "%s",(const char*)dimg->get_long_description() );
      DjVuFormatErrorUTF8( "%s\t%lu", ERR_MSG("ddjvu.decode"), stop - start);
    }

  // Check
  DjVuInfo *info = dimg->get_info();
  int colorp = dimg->is_legal_photo();
  int blackp = dimg->is_legal_bilevel();
  int compoundp = dimg->is_legal_compound();
  if (flag_verbose)
    {
      if (compoundp)
        DjVuWriteError( ERR_MSG("ddjvu.compound") );
      else if (colorp)
        DjVuWriteError( ERR_MSG("ddjvu.photo") );
      else if (blackp)
        DjVuWriteError( ERR_MSG("ddjvu.bilevel") );
      // Without included files
      DjVuFormatErrorUTF8( "%s\t%4.1f", ERR_MSG("ddjvu.memory"), 
              (double)(dimg->get_djvu_file()->get_memory_usage())/1024 );
    }    
  if (!compoundp && !colorp && !blackp)
    { 
      DjVuPrintErrorUTF8("%s","Warning: This is not a well formed DjVu image\n");
      if (!info)
        G_THROW( ERR_MSG("ddjvu.no_info") ); 
    }

  // Setup rectangles
  if (flag_size<0 && flag_scale<0 && flag_subsample<0)
    flag_scale = 100;
  if (flag_subsample>0)
    flag_scale = (double) info->dpi / flag_subsample;
  if (flag_scale>0)
    {
      int w = (int)(dimg->get_width() * flag_scale / info->dpi);
      int h = (int)(dimg->get_height() * flag_scale / info->dpi);
      if (w<1) w=1;
      if (h<1) h=1;
      g().fullrect = GRect(0,0, w, h);
    }
  if (flag_segment < 0)
    g().segmentrect = g().fullrect;

  // Render
  GP<GPixmap> pm;
  GP<GBitmap> bm;
  start = GOS::ticks();
  switch(flag_mode)
    {
    case 's':
      bm = dimg->get_bitmap(g().segmentrect, g().fullrect);
      break;
    case 'f':
      pm = dimg->get_fg_pixmap(g().segmentrect, g().fullrect);
      break;
    case 'b':
      pm = dimg->get_bg_pixmap(g().segmentrect, g().fullrect);
      break;
    default:
      pm = dimg->get_pixmap(g().segmentrect, g().fullrect);
      if (! pm)
        bm = dimg->get_bitmap(g().segmentrect, g().fullrect);
      break;
    }
  stop = GOS::ticks();
  if (flag_verbose)
    {
      DjVuFormatErrorUTF8( "%s\t%lu", ERR_MSG("ddjvu.render"), stop - start);
    }

  // Save image
  if (pm) 
    {
      GP<ByteStream> out=ByteStream::create(to,"wb");
      pm->save_ppm(*out);
    }
  else if (bm) 
    {
      GP<ByteStream> out=ByteStream::create(to,"wb");
      if (bm->get_grays() > 2) {
        bm->save_pgm(*out);
      } else if(use_rle) {
        bm->save_rle(*out);
      } else {
        bm->save_pbm(*out);
      }
    }
  else 
    {
      G_THROW( ERR_MSG("ddjvu.cant_render") );
    }
}


void
usage()
{
  DjVuPrintErrorUTF8("%s",
#ifdef DJVULIBRE_VERSION
          "DDJVU --- DjVuLibre-" DJVULIBRE_VERSION "\n"
#endif
          "DjVu decompression utility\n\n"
          "Usage: ddjvu [options] [<djvufile> [<pnmfile>]]\n\n"
          "Options:\n"
          "  -v                  Prints various informational messages.\n"
          "  -scale N            Selects display scale (default: 100%).\n"
          "  -size  WxH          Selects size of rendered image.\n"
          "  -segment WxH+X+Y    Selects which segment of the rendered image\n"
          "  -black              Only renders the stencil(s).\n"
          "  -foreground         Only renders the foreground layer.\n"
          "  -background         Only renders the background layer.\n"
          "  -N                  Subsampling factor from full resolution.\n"
	  "  -page <page>        Decode page <page> (for multipage documents).\n"
	  "  -rle                Decode black stencil to RLE.\n"
          "\n"
          "The output will be a PBM, PGM or PPM file depending of its content.\n"
          "If <pnmfile> is a single dash or omitted, the decompressed image\n"
          "is sent to the standard output.  If <djvufile> is a single dash or\n"
          "omitted, the djvu file is read from the standard input.\n\n");
  exit(1);
}

static inline void
syntax_error(void)
{
  G_THROW( ERR_MSG("ddjvu.syntax"));
}

GRect
geometry(const GUTF8String &s)
{
  int pos;
  const int w = s.toLong(0,pos);
  if (pos < 0 || w<=0 || s[pos] !='x')
    syntax_error();
  const int h = s.toLong(pos+1,pos);
  if((pos<0)||(h<=0))
    syntax_error();
  const int len=s.length();
  int xmin=0;
  int ymin=0;
  if (pos<len)
  {
    if ((s[pos]!='+')&&(s[pos] !='-'))
      syntax_error();
    xmin = s.toLong(pos+1,pos);
    if ((pos >= 0)&&(pos<len)) 
    {
      if ((s[pos]!='+')&&(s[pos] !='-'))
        syntax_error();
      ymin = s.toLong(pos+1,pos);
      if ((pos >= 0)&&(pos<len)) 
        syntax_error();
    }
  }
  return GRect(xmin,ymin,w,h);
}




#ifdef UNDER_CE
int
oldmain(int argc, char **argv)
{
	flag_scale = -1;
	flag_size = -1;
	flag_subsample = -1;
	flag_segment = -1;
	flag_verbose = 0;
	flag_mode = 0;

	// Output command arguments that oldmain receives to logfile.
	fprintf( logfile, "Command arguments: " );
	for ( int i = 0; i < argc; i++ )
	{
		fprintf( logfile, "%s ", argv[i] );
	}
	fprintf( logfile, "\n\n" );
#else
#include <locale.h>
int
main(int argc, char **argv)
{
#endif
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
  GArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
    dargv[i]=GNativeString(argv[i]);
   G_TRY
    {
      // Process options
      int page_num=-1;
      while (argc>1 && dargv[1][0]=='-' && dargv[1][1])
        {
          GUTF8String s(dargv[1]);
          int pos;
          int n=s.toLong(0,pos);
          if (dargv[1] == GUTF8String("-v"))
            {
              flag_verbose = 1;
            }
          else if (s == "-scale")
            {
              if (argc<=2)
                G_THROW( ERR_MSG("ddjvu.no_scale") );
              if (flag_subsample>=0 || flag_scale>=0 || flag_size>=0)
                G_THROW( ERR_MSG("ddjvu.dupl_scale") );
              argc -=1;
              dargv.shift(-1);
              s = dargv[1];
              int pos;
              flag_scale = s.toDouble(0,pos);
              if((pos<0)||(pos == (int)s.length()))
                G_THROW( ERR_MSG("ddjvu.bad_scale") );
              if (s[pos] == '%') 
              {
                pos++;
              }
              if (pos < (int)s.length())
                G_THROW( ERR_MSG("ddjvu.bad_scale"));
            }
          else if ( s == "-size")
            {
              if (argc<=2)
                G_THROW( ERR_MSG("ddjvu.no_size") );
              if (flag_subsample>=0 || flag_scale>=0 || flag_size>=0)
                G_THROW( ERR_MSG("ddjvu.dupl_size"));
              argc -=1; dargv.shift(-1); s = dargv[1];
              g().fullrect = geometry(s);
              flag_size = 1;
              if (g().fullrect.xmin || g().fullrect.ymin)
                G_THROW( ERR_MSG("ddjvu.bad_size"));
            }
          else if (s == "-segment")
            {
              if (argc<=2)
                G_THROW( ERR_MSG("ddjvu.no_seg") );
              if (flag_segment>=0)
                G_THROW( ERR_MSG("ddjvu.dupl_seg") );
              argc -=1; dargv.shift(-1); s = dargv[1];
              g().segmentrect = geometry(s);
              flag_segment = 1;
            }
          else if (s == "-rle")
            {
              if (flag_mode)
                G_THROW( ERR_MSG("ddjvu.dupl_render"));
              flag_mode = 's';
              use_rle=true;
            }
          else if (s == "-black")
            {
              if (flag_mode)
                G_THROW( ERR_MSG("ddjvu.dupl_render"));
              flag_mode = 's';
            }
          else if (s == "-foreground")
            {
              if (flag_mode)
                G_THROW( ERR_MSG("ddjvu.dupl_render"));
              flag_mode = 'f';
            }
          else if (s == "-background")
            {
              if (flag_mode)
                G_THROW( ERR_MSG("ddjvu.dupl_render"));
              flag_mode = 'b';
            }
	  else if (s == "-page")
	    {
	      if (argc<=2)
                G_THROW( ERR_MSG("ddjvu.no_page") );
              if (page_num>=0)
                G_THROW( ERR_MSG("ddjvu.dupl_page") );
              argc -=1;
              dargv.shift(-1);
              s = dargv[1];
              page_num=s.toInt(); // atoi(s);
	      if (page_num<=0)
                G_THROW( ERR_MSG("ddjvu.bad_page"));
	      page_num--;
	    }
          else if (n < 0 && pos == (int)s.length())
            {
              flag_subsample=(-n);
            }
          else
            {
              usage();
            }
          argc -= 1;
          dargv.shift(-1);
        }
      if (page_num<0) page_num=0;
      // Process remaining arguments
      if (argc == 3) 
        convert(GURL::Filename::UTF8(dargv[1]),GURL::Filename::UTF8(dargv[2]), page_num);
      else if (argc == 2)
        convert(GURL::Filename::UTF8(dargv[1]),GURL::Filename::UTF8("-"), page_num);
      else if (argc == 1)
        convert(GURL::Filename::UTF8("-"),GURL::Filename::UTF8("-"), page_num);
      else
        usage();
    }
  G_CATCH(ex)
    {
      ex.perror();
      exit(1);
    }
  G_ENDCATCH;

  return 0;
}


#ifdef UNDER_CE
#define SEPCHARS TEXT( " " )
#define TIME_BUFSIZE 20
#define ARG_BUFSIZE 20
int WINAPI WinMain (HINSTANCE hInstance,
		             HINSTANCE hPrevInstance,
                     LPTSTR lpCmdLine,
                     int nCmdShow)
{
	int argc = 1, i = 0;
	char *argv[20];
	TCHAR *arg;
	TCHAR timestamp[TIME_BUFSIZE];

	GetTimeFormat( LOCALE_SYSTEM_DEFAULT, 0, NULL, NULL, 
		timestamp, TIME_BUFSIZE );
	
	// Ouput header to ddjvuce log file with timestamp
	fwprintf( logfile, TEXT("\n--------------------------------\n") );
	fwprintf( logfile, TEXT("ddjvuce log file:  ") );
	fwprintf( logfile, timestamp );
	fwprintf( logfile, TEXT("\n--------------------------------\n") );
	fwprintf( logfile, TEXT("\n") );

	// Use wcstok to get arguments from lpCmdLine, then convert them
	// to chars for oldmain().
	argv[0]="ddjvu";
	arg = wcstok( lpCmdLine, SEPCHARS );
	while( arg != NULL )
	{
		argv[argc] = (char*) malloc( wcslen( arg ) + 1 );
		for ( i = 0; i < wcslen( arg); i++ )
		{
			argv[argc][i] = arg[i];
		}
		argv[argc][i] = '\0';
		arg = wcstok( NULL, SEPCHARS );
		argc++;
	}
	
	oldmain( argc, argv );
	
	for( i = 0; i < argc; i++ )
		free( argv[argc] );
	
	fclose( logfile );
	/*
   argv[1]="-v" ;
   // Vanilla flavored conversion
   argv[2]="\\\\temp\\input.djvu" ;
   argv[3]="\\\\temp\\output_default.pnm" ;
   oldmain(4, argv);

   // Scale at 50%
   argv[2]="-scale";
   argv[3]="50" ;
   argv[4]="\\\\My Documents\\input.djvu" ;
   argv[5]="\\\\My Documents\\output_scale50.pnm" ;
   oldmain (6, argv) ;
 
   // Scale at 150%
   argv[2]="-scale";
   argv[3]="150" ;
   argv[4]="\\\\My Documents\\input.djvu" ;
   argv[5]="\\\\My Documents\\output_scale150.pnm" ;
   oldmain (6, argv) ;
 
   // Size at 100x100
   argv[2]="-size";
   argv[3]="100x100" ;
   argv[4]="\\\\My Documents\\input.djvu" ;
   argv[5]="\\\\My Documents\\output_size100x100.pnm" ;
   oldmain (6, argv) ;

   // Segment 100x100+20+20
   argv[2]="-segment";
   argv[3]="100x100+20+20" ;
   argv[4]="\\\\My Documents\\input.djvu" ;
   argv[5]="\\\\My Documents\\output_segment100x100p20p20.pnm" ;
   oldmain (6, argv) ;

   // Mask only
   argv[2]="-black";
   argv[3]="\\\\My Documents\\input.djvu" ;
   argv[4]="\\\\My Documents\\output_mask.pnm" ;
   oldmain (5, argv) ;

   // FG only
   argv[2]="-foreground";
   argv[3]="\\\\My Documents\\input.djvu" ;
   argv[4]="\\\\My Documents\\output_fg.pnm" ;
   oldmain (5, argv) ;

   // BG only
   argv[2]="-background";
   argv[3]="\\\\My Documents\\input.djvu" ;
   argv[4]="\\\\My Documents\\output_bg.pnm" ;
   oldmain (5, argv) ;

   // Subsampling
   argv[2]="-2";
   argv[3]="\\\\My Documents\\input.djvu" ;
   argv[4]="\\\\My Documents\\output_subsample2.pnm" ;
   oldmain (5, argv) ;

   // Get Page 2 of the multipage document
   argv[2]="-page";
   argv[3]="2" ;
   argv[4]="\\\\My Documents\\input.djvu" ;
   argv[5]="\\\\My Documents\\output_page2.pnm" ;
   oldmain (6, argv) ;

   // Get the mask of Page 2 of the multipage document
   argv[2]="-page";
   argv[3]="2" ;
   argv[4]="-black" ;
   argv[5]="\\\\My Documents\\input.djvu" ;
   argv[6]="\\\\My Documents\\output_page2_mask.pnm" ;
   oldmain (7, argv) ; */

/* 

   "DDJVU -- DjVu decompression utility\n"
          "  Copyright (c) AT&T 1999.  All rights reserved\n"
          "Usage: ddjvu [options] [<djvufile> [<pnmfile>]]\n\n"
          "Options:\n"
          "  -v                  Prints various informational messages.\n"
          "  -scale N            Selects display scale (default: 100%%).\n"
          "  -size  WxH          Selects size of rendered image.\n"
          "  -segment WxH+X-Y    Selects which segment of the rendered image\n"
          "  -black              Only renders the stencil(s).\n"
          "  -foreground         Only renders the foreground layer.\n"
          "  -background         Only renders the background layer.\n"
          "  -N                  Subsampling factor from full resolution.\n"
	  "  -page <page>        Decode page <page> (for multipage documents).\n"
          "\n"
     
*/
   return 0 ;
}
#endif
