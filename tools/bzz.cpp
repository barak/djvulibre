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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// BZZ -- a frontend for BSByteStream

/** @name bzz

    \begin{description}
    \item[Compression:]
    #bzz -e[<blocksize>] <infile> <outfile>#
    \item[Decompression:]
    #bzz -d <infile> <outfile>#
    \end{description}    

    Program bzz is a simple front-end for the Burrows Wheeler encoder
    implemented in \Ref{BSByteStream.h}.  Although this compression model is
    not currently used in DjVu files, it may be used in the future for
    encoding textual data chunks.  Argument #blocksize# is expressed in
    kilobytes and must be in range 200 to 4096.  The default value is 2048.
    Arguments #infile# and #outfile# are the input and output filenames. A
    single dash (#"-"#) can be used to represent the standard input or output.

    @memo
    General purpose compression/decompression program
    @author
    L\'eon Bottou <leonb@research.att.com> -- initial implementation
    @version
    $Id$ */
//@{
//@}

#include "GException.h"
#include "ByteStream.h"
#include "BSByteStream.h"
#include "GOS.h"
#include "GURL.h"
#include "DjVuMessage.h"
#include <locale.h>
#include <stdlib.h>

static const char *program = "(unknown)";

void
usage(void)
{
  DjVuPrintErrorUTF8(
          "BZZ -- ZPCoded Burrows Wheeler compression\n"
          "  Copyright Â© 1999-2000 LizardTech, Inc. All Rights Reserved.\n"
          "Usage [encoding]: %s -e[<blocksize>] <infile> <outfile>\n"
          "Usage [decoding]: %s -d <infile> <outfile>\n"
          "  Argument <blocksize> must be in range [900..4096] (default 1100).\n"
          "  Arguments <infile> and <outfile> can be '-' for stdin/stdout.\n",
          program, program);
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
      if(argc < 2)
        usage();
      // Get program name
      program=dargv[0]=GOS::basename(dargv[0]);
      // Obtain default mode from program name
      int blocksize = -1;
      if (dargv[0] == "bzz")
        blocksize = 1100;
      else if (dargv[0] == "unbzz")
        blocksize = 0;
      // Parse arguments
      if (argc>=2 && dargv[1][0]=='-')
        {
          if (dargv[1][1]=='d' && dargv[1][2]==0)
            {
              blocksize = 0;
            }
          else if (dargv[1][1]=='e')
            {
              blocksize = 2048;
              if (dargv[1][2])
                 blocksize = dargv[1].substr(2, dargv[1].length()).toInt(); //atoi(2+(const char *)dargv[1]);
            }
          else 
            usage();
          dargv.shift(-1);
          argc--;
        }
      if (blocksize < 0)
        usage();
      // Obtain filenames
      const GURL::Filename::UTF8 inurl((argc>=2)?dargv[1]:GUTF8String("-"));
      const GURL::Filename::UTF8 outurl((argc>=3)?dargv[2]:GUTF8String("-"));
      if (argc >= 4)
        usage();
      // Action
      GP<ByteStream> in=ByteStream::create(inurl,"rb");
      GP<ByteStream> out=ByteStream::create(outurl,"wb");
      if (blocksize)
        {
          GP<ByteStream> gbsb=BSByteStream::create(out, blocksize);
          gbsb->copy(*in);
        }
      else 
        {
          GP<ByteStream> gbsb=BSByteStream::create(in);
          out->copy(*gbsb);
        }
    }
  G_CATCH(ex)
    {
      ex.perror();
      exit(1);
    }
  G_ENDCATCH;
  return 0;
}

