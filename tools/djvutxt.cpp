//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.5)
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
// 
// $Id$
// $Name$

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// DJVUTXT -- DjVu TXT extractor

/** @name djvutxt

    {\bf Synopsis}
    \begin{verbatim}
        djvutxt [--page <page_num>] <djvu_file_in> [<txt_file_out>]
    \end{verbatim}

    {\bf Description} --- File #"djvutxt.cpp"# illustrates how to use
    \Ref{DjVuDocument}, \Ref{DjVuImage}, \Ref{DjVuText>, and \Ref{DjVuTXT}
    to retrieve textual information stored inside a #TXT*# chunk of a DjVu
    document.

    #TXT*# chunks should have been created with the help of an OCR
    engine, and are used to allow indexing and searching of the DjVu
    document. The chunks contain the ASCII text itself, and layout information
    allowing the DjVu plugins to highlight found text.

    This utility can be used to extract text from #TXT*# chunks and
    output it to a file or standard output.
    
    {\bf Arguments}:
    \begin{itemize}
       \item {\bf <djvu_file_in>} - Name of input DjVu file.
       \item {\bf <txt_file_out>} - Name of the file where ASCII text
             will be stored. #-# means standard output.
    \end{itemize}

    {\bf --page} option can be used to select a particular page from the
    {\bf <djvu_file_in>} for processing.
    
    @memo #TXT*# chunks extractor
    @author
    Andrei Erofeev <eaf@geocities.com> -- initial implementation
    @version
    #$Id$# */
//@{
//@}


#include "DjVuDocument.h"
#include "ByteStream.h"
#include "DjVuText.h"
#include "DjVuImage.h"
#include "GString.h"
#include "GOS.h"
#include "DjVuMessage.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static const char * progname;

static void
usage(void)
{
   DjVuPrintErrorUTF8("\
DJVUTXT -- DjVu TXT* chunks extractor\n\
   Copyright Â© 1999-2000 LizardTech, Inc. All Rights Reserved.\n\
\n\
Usage:\n\
	%s [--page <page_num>] <djvu_file_in> [<txt_file_out>]\n\
\n\
The program will decode and output to <txt_file_out> ASCII text from\n\
every TXT* (TXTa or TXTz) chunk found in the source <djvu_file_in>\n\
DjVu document.\n\
\n\
TXT* chunks contain information about the text present within the given\n\
page, and its location. These chunks should have been generated with\n\
the help of an OCR engine, and encoded using capabilities provided by\n\
this library.\n\n", progname);
}

static void
doPage(const GP<DjVuDocument> & doc, int page_num,
       ByteStream & str_out)
{
   GP<DjVuImage> dimg=doc->get_page(page_num);
   if (!dimg)
      G_THROW("Failed to decode page.");
   GP<ByteStream> text_str=dimg->get_text();
   if (text_str)
   {
      GP<DjVuText> text=DjVuText::create();
      text->decode(text_str);
      GP<DjVuTXT> txt=text->txt;
      if (txt)
      {
        str_out.write((const char *) txt->textUTF8,
		       txt->textUTF8.length());
      }
   }
}

int
main(int argc, char ** argv)
{
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
  GArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
    dargv[i]=GNativeString(argv[i]);
  progname=dargv[0]=GOS::basename(dargv[0]);
   
#ifdef DEBUG
   {
      const GUTF8String debug(GOS::getenv("DEBUG"));
      if (debug.length())
      {
//	 int level=debug.is_int()?atoi((const char *)debug):0;
         int level=debug.is_int()?debug.toInt():0;
         if (level<1) level=1;
         if (level>32) level=32;
//	 DEBUG_SET_LEVEL(level);
      }
   }
#endif
   
   G_TRY {
      GUTF8String name_in, name_out;
      int page_num=-1;

      for(int i=1;i<argc;i++)
      {
	 if ( (dargv[i] == GUTF8String("-")) || dargv[i][0]!='-')
	 {
	    if (!name_in.length())
	    {
	       if (dargv[i] == GUTF8String( "-"))
	       {
		  DjVuPrintErrorUTF8("%s","Can't read from standard input.\n\n");
		  usage();
		  exit(1);
	       } else name_in=dargv[i];
	    } else
	    {
	       if (!name_out.length())
		  name_out=dargv[i];
	       else
	       {
		  usage();
		  exit(1);
	       }
	    }
	 } else
	 {
	    if (dargv[i][0]=='-' && dargv[i][1]=='-')
	       dargv[i]=(const char*)dargv[i]; //1+(const char *)dargv[i];
	    if (dargv[i] == GUTF8String("--page"))
	    {
	       if (i+1>=argc)
	       {
		  DjVuPrintErrorUTF8("%s","--page option must be followed by a number.\n\n");
		  usage();
		  exit(1);
	       }
	       i++;
	       page_num=dargv[i].toInt() - 1;//atoi(dargv[i])-1;
	       if (page_num<0)
	       {
		  DjVuPrintErrorUTF8("%s","Page number must be positive.\n\n");
		  usage();
		  exit(1);
	       }
	    } else if (dargv[i] == GUTF8String("--help"))
	    {
	       usage();
	       exit(1);
	    } else
            {
               DjVuPrintErrorUTF8("Unrecognized option '%s' encountered.\n\n", (const char *)dargv[i]);
            }
	 }
      }

      if (name_in.length()==0)
      {
	 DjVuPrintErrorUTF8("%s","The name of the input file is missing.\n\n");
	 usage();
	 exit(1);
      }
      if (name_out.length()==0)
	 name_out="-";

      GP<DjVuDocument> doc=DjVuDocument::create_wait(GURL::Filename::UTF8(name_in));
      GP<ByteStream> gstr_out=ByteStream::create(GURL::Filename::UTF8(name_out), "w");
      ByteStream &str_out=*gstr_out;
      if (page_num<0)
	 for(page_num=0;page_num<doc->get_pages_num();page_num++)
	    doPage(doc, page_num, str_out);
      else
	 doPage(doc, page_num, str_out);
   } G_CATCH(exc) {
      exc.perror();
      exit(1);
   } G_ENDCATCH;
   exit(0);
#ifdef WIN32
   return 0;
#endif
}
