//C-  -*- C++ -*-
//C- -----------------------------------------------------------
//C- DjVuLibre-XmlTools-3.5 
//C- Copyright (c) 2002  Bill C. Riemers, Leon Bottou and Yann Le Cun.
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

/** @name xml2utf16

    {\bf Synopsis}
    \begin{verbatim}
        xml2utf16 [<inputfile>] [<outputfile>]
    \end{verbatim}

    @author
    Dr Bill C Riemers <bcr@lizardtech.com>
    @version
    #$Id$# */
//@{
//@}




#include "GException.h"
#include "UnicodeByteStream.h"
#include "ByteStream.h"
#include "DjVuMessage.h"
#include "GURL.h"

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

void
usage(const GUTF8String &name)
{
  DjVuPrintErrorUTF8(
          "%s -- xml to utf16 encoding conversion\n"
          "  Copyright © 2001 LizardTech, Inc. All Rights Reserved.\n"
          "Usage: %s <infile> <outfile>\n",(const char *)name,(const char *)name);
}

int 
main(int argc, char **argv)
{
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
  GArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
    dargv[i]=GNativeString(argv[i]);
  if(argc < 3 || dargv[1] == "-help" || dargv[1] == "--help")
  {
    usage(GURL::Filename::UTF8(dargv[0]).fname());
    exit(0);
  }
  GURL::Filename::UTF8 inurl((argc<2)?GUTF8String("-"):dargv[1]);
  GURL::Filename::UTF8 outurl((argc<3)?GUTF8String("-"):dargv[argc-1]);
  G_TRY
  {
    GP<ByteStream> bs(ByteStream::create(inurl,"rb")); 
    {
      GP<XMLByteStream> uni=XMLByteStream::create(bs);
      bs=ByteStream::create();
      GUTF8String ustr;
      while((ustr=uni->gets()).length())
      {
        wchar_t *wbuf;
        GPBuffer<wchar_t> gbuf(wbuf,ustr.length()+1);
        ustr.ncopy(wbuf,ustr.length()+1);
        if(sizeof(wchar_t) == sizeof(unsigned short))
        {
          bs->writall(wbuf,wcslen(wbuf)*sizeof(unsigned short));
        }else
        {
          for(wchar_t *ptr=wbuf;*ptr;++ptr)
          {
            unsigned long w=*ptr;
            unsigned short w1, w2;
            int count=GStringRep::UCS4toUTF16(w,w1,w2);
            if(count > 0)
            {
              bs->writall(&w1,sizeof(w1));
              if(count > 1)
              {
                bs->writall(&w2,sizeof(w2));
              }
            }
          }
        }
      }
    }
    bs->seek(0L);
    GP<ByteStream> outbs=ByteStream::create(outurl,"wb");
    static const unsigned short tag=0xFEFF;
    outbs->writall(&tag,sizeof(tag));
    outbs->copy(*bs);
  }
  G_CATCH(ex)
  {
    ex.perror();
    exit(1);
  }
  G_ENDCATCH;
  exit(0);
#ifdef WIN32
  return 0;
#endif
}


