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

/** @name djvmcvt

    {\bf Synopsis}\\
    \begin{verbatim}
        djvmcvt -b[undled] <doc_in.djvu> <doc_out.djvu>

	or
	
	djvmcvt -i[ndirect] <doc_in.djvu> <dir_out> <idx_fname.djvu>
    \end{verbatim}

    {\bf Description} ---
    File #"djvmcvt.cpp"# and the program #djvmcvt# serve the purpose of
    convertion of obsolete DjVu documents into the new formats. The program
    can also read documents in the new formats, so you can use it to
    perform conversion between #BUNDLED# and #INDIRECT# formats. This is a
    simple illustration of the capabilities of \Ref{DjVuDocument} class.

    As a matter of fact, there are two ways to make conversion between
    different formats:
    \begin{enumerate}
       \item If the input format is one of obsolete formats (#OLD_BUNDLED#
             or #OLD_INDEXED#) then the conversion can be done by
	     \Ref{DjVuDocument} only.
       \item If the input format is one of new formats (#BUNDLED# or
             #INDIRECT#) then the best candidate to perform conversion
	     is \Ref{DjVmDoc} class. It will do it at the lowest possible
	     with the least expenses.
    \end{enumerate}
    
    {\bf Arguments} ---
    Depending on the output format, the number and types of arguments
    differ. The second argument though (#<doc_in.djvu>#) is the same in both
    cases, and depending on the format of input document, it means:
    \begin{itemize}
       \item {\bf OLD_BUNDLED} format: just name of the document
       \item {\bf OLD_INDEXED} format: name of any page of the document
       \item {\bf BUNDLED} format: name of the document
       \item {\bf INDIRECT} format: name of the top-level file with the
             list of all pages of the document.
    \end{itemize}.

    So, in order to do conversion choose one of syntaxes below:
    \begin{itemize}
       \item To create a new {\em BUNDLED} document

             #djvmcvt -b[undled] <doc_in.djvu> <doc_out.djvu>#

	     This will read the document referenced by #<doc_in.djvu># as
	     descrived above, will convert it into the #BUNDLED#
	     format and will save the results into the #<doc_out.djvu># file.
	     
       \item To create a new {\em INDIRECT} document

             #djvmcvt -i[ndirect] <doc_in.djvu> <dir_out> <idx_fname.djvu>#

	     This will read the input document referenced by #<doc_in.djvu>#
	     as described above, will convert it into the #INDIRECT#
	     format and will save it into the #<dir_out># directory. Since
	     DjVu multipage documents in the #INDIRECT# formats are
	     represented by a bunch of files, you have to specify a directory
	     name where all of the files will be saved. In addition to these
	     files the program will also create a top-level file named
	     #<idx_fname.djvu># with the list of all pages and components
	     composing the given DjVu document. Whenever you need to open
	     this document later, open this top-level file.
    \end{itemize}
	     
    @memo
    DjVu multipage document converter.
    @author
    Andrei Erofeev <eaf@geocities.com>
    @version
    #$Id$# */


#include "DjVuDocument.h"
#include "DjVmDoc.h"
#include "ByteStream.h"
#include "GOS.h"
#include "DjVuMessage.h"
#include "debug.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

static const char * progname;

static void Usage(void)
{
   DjVuPrintErrorUTF8(
#ifdef DJVULIBRE_VERSION
     "DJVMCVT --- DjVuLibre-" DJVULIBRE_VERSION "\n"
#endif
     "DjVu multipage document conversion utility\n"
     "\n"
     "Usage:\n"
     "\n"
     "  To convert any DjVu multipage document into the new BUNDLED format:\n"
     "	  %s -b[undled] <doc_in.djvu> <doc_out.djvu>\n"
     "	  where <doc_out.djvu> is the name of the output file.\n"
     "\n"
     "  To convert any DjVu multipage document into the new INDIRECT format:\n"
     "	  %s -i[ndirect] <doc_in.djvu> <dir_out> <idx_fname.djvu>\n"
     "	  where <dir_out> is the name of the output directory, and\n"
     "	  <idx_fname.djvu> is the name of the top-level document index file.\n"
     "\n"
     "The <doc_in.djvu> specifies the document to be converted.\n"
     "For OLD_BUNDLED and BUNDLED formats, this is the name of the document file.\n"
     "For INDIRECT format, this is the name of the top-level index file.\n"
     "For OLD_INDEXED format, this is the name of any page file.\n"
     "\n", progname, progname);
}

static void
do_bundled(GArray<GUTF8String> &argv)
      // <progname> -b[undled] <file_in> <file_out>
{
   const int argc=argv.hbound()+1;
   if (argc!=4) { Usage(); exit(1); }
   const GURL::Filename::UTF8 url2(argv[2]);
   const GURL::Filename::UTF8 url3(argv[3]);
   GP<DjVuDocument> doc = DjVuDocument::create_wait(url2);
   GP<ByteStream> str=ByteStream::create(url3, "wb");
   doc->write(str);
}

static void
do_indirect(GArray<GUTF8String> &argv)
      // <progname> -i[ndirect] <file_in> <dir_out> <idx_fname>
{
   const int argc=argv.hbound()+1;
   if (argc!=5) { Usage(); exit(1); }
   const GURL::Filename::UTF8 url2(argv[2]);
   GP<DjVuDocument> doc = DjVuDocument::create_wait(url2);
   const GURL::Filename::UTF8 url3(argv[3]);
   doc->expand(url3, argv[4]);
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

  if (argc<2) { Usage(); exit(1); }

   bool bundled=true;
   G_TRY {
      if (!dargv[1].cmp("-b", 2)) bundled=true;
      else if (!dargv[1].cmp("-i", 2)) bundled=false;
      else { Usage(); exit(1); }

      if (bundled) do_bundled(dargv);
      else do_indirect(dargv);
   } G_CATCH(exc) {
      DjVuPrintErrorUTF8("%s\n", exc.get_cause());
      exit(1);
   } G_ENDCATCH;

   exit(0);
#ifdef WIN32
   return 0;
#endif
}
