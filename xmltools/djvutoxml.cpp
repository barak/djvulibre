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

#include "DjVuDocument.h"
#include "GOS.h"
#include "DjVuMessage.h"
#include "ByteStream.h"
#include "DjVuAnno.h"
#include "DjVuText.h"
#include "DjVuImage.h"
#include "debug.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>

static void
usage(void)
{
  DjVuPrintErrorUTF8("Usage: %s [--with[out]-anno] [--with[out]-text] <inputfile> <outputfile>\n",(const char *)GOS::basename(DjVuMessage::programname()));
}

//------------------------ implementation ------------------------
int
main(int argc, char * argv[], char *env[])
{
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
  GArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
    dargv[i]=GNativeString(argv[i]);

  GUTF8String name;
  int notext=(-1);		// You can turn off outputting text
  int noanno=(-1);		//  or annotations here!
  if(argc>0)
  {
    const char * prog=(const char *)dargv[0];
    name=GOS::basename(dargv[0]);
    for(;argc>1;--argc, dargv.shift(-1))
    {
      const GUTF8String &arg=dargv[1];
      if(arg == "--with-text")
      {
        notext=0;
        if(noanno<0)
          noanno=1;
      }else if(arg == "--without-text")
      {
        notext=1;
        if(noanno<0)
          noanno=0;
      }else if(arg == "--with-anno")
      {
        noanno=0;
        if(notext<0)
          notext=1;
      }else if(arg == "--without-anno")
      {
        noanno=1;
        if(notext<0)
        {
          notext=0;
        }
      }else
      {
        break;
      }
    }
    dargv[0]=prog;
  }
  int flags=0;
  if(noanno > 0)
    flags|=DjVuImage::NOMAP;
  if(notext > 0)
    flags|=DjVuImage::NOTEXT;
#ifdef DEBUG_SET_LEVEL
  {
    static GUTF8String debug=GOS::getenv("DEBUG");
    if (debug.length())
    {
      int level=debug.is_int()?debug.toInt():0;
      if (level<1) level=1;
      if (level>32) level=32;
      DEBUG_SET_LEVEL(level);
    }
  }
#endif
    
  G_TRY
  {
      GUTF8String name_in, name_out;
      int page_num=-1;
      
      for(int i=1;i<argc;i++)
      {
        GUTF8String arg(dargv[i]);
        if (arg == "-" || arg[0] != '-' || arg[1] != '-')
        {
          if (!name_in.length())
          {
            if (arg == "-")
            {
              DjVuMessage::perror( ERR_MSG("DjVuToXML.std_read") );
              usage();
              exit(1);
            }
            name_in=arg;
          } else if (!name_out.length())
          {
            name_out=arg;
          }else
          {
            usage();
            exit(1);
          }
        } else if (arg == "--page")
        {
          if (i+1>=argc)
          {
            DjVuMessage::perror( ERR_MSG("DjVuToXML.no_num") );
            usage();
            exit(1);
          }
          i++;
          page_num=dargv[i].toInt() - 1;
          if (page_num<0)
          {
            DjVuMessage::perror( ERR_MSG("DjVuToXML.negative_num") );
            usage();
            exit(1);
          }
        } else if (arg == "--help")
        {
          usage();
          exit(1);
        } else
        {
          DjVuMessage::perror( ERR_MSG("DjVuToXML.unrecog_opt") "\t"+arg);
        }
      }
      
      if (!name_in.length())
      {
        DjVuMessage::perror( ERR_MSG("DjVuToXML.no_name") );
        usage();
        exit(1);
      }
      if (!name_out.length())
      {
        name_out="-";
      }
      
      GP<DjVuDocument> doc=DjVuDocument::create_wait(GURL::Filename::UTF8(name_in));

      GP<ByteStream> gstr_out=ByteStream::create(GURL::Filename::UTF8(name_out), "w");
      doc->writeDjVuXML(gstr_out,flags);
//      ByteStream &str_out=*gstr_out;
//      extract.writeXMLprolog( str_out);
//      extract.writeMainElement( str_out, doc, page_num );

  }
  G_CATCH(exc)
  {
    exc.perror();
    exit(1);
  }
  G_ENDCATCH;
  return 0;
}

