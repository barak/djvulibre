//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either Version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C- 
//C- DjVuLibre-3.5 is derived from the DjVu(r) Reference Library from
//C- Lizardtech Software.  Lizardtech Software has authorized us to
//C- replace the original DjVu(r) Reference Library notice by the following
//C- text (see doc/lizard2002.djvu and doc/lizardtech2007.djvu):
//C-
//C-  ------------------------------------------------------------------
//C- | DjVu (r) Reference Library (v. 3.5)
//C- | Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- | The DjVu Reference Library is protected by U.S. Pat. No.
//C- | 6,058,214 and patents pending.
//C- |
//C- | This software is subject to, and may be distributed under, the
//C- | GNU General Public License, either Version 2 of the license,
//C- | or (at your option) any later version. The license should have
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include "GURL.h"
#include "GString.h"
#include "BSByteStream.h"

int
main(int argc, char ** argv)
{
  if (argc!=2)
    {
      fprintf(stderr, "Usage: %s <var_name>\n", argv[0]);
      exit(1);
    }
  
  const char * name=argv[1];
  
  // First read all data into memory
  GP<ByteStream> gstr_in = ByteStream::create("rb");
  ByteStream &str_in = *gstr_in;
  GP<ByteStream> gmem_str1 = ByteStream::create();
  ByteStream &mem_str1 = *gmem_str1;
  mem_str1.copy(str_in);
  
  // Now compute block size for BSByteStream
  int block_size=1+(mem_str1.size() >> 10);
  if (block_size<10) block_size=10;
  if (block_size>1024) block_size=1024;
  
  // Compress data
  GP<ByteStream> mem_str2 = ByteStream::create();
  {
    GP<ByteStream> bs_str = BSByteStream::create(mem_str2, block_size);
    mem_str1.seek(0);
    bs_str->copy(mem_str1);
    bs_str = 0;
  }
  
  // Finally: generate file
  printf("/* This is an AUTOMATICALLY generated file (by bin2cpp). STAY AWAY! */\n"
         "\n"
         "#include \"cin_data.h\"\n"
         "\n"
         "static int %s_len=%d;\n"
         "static const char %s_data[]=\"\\\n" , 
         name, mem_str2->size(), name);
  
  mem_str2->seek(0);
  int chars=0;
  int size=0;
  while(1)
    {
      unsigned char ch;
      if (!mem_str2->read(&ch, 1)) break;
      printf("\\%03o", ch);
      chars+=4;
      if (chars>70)
        {
          chars=0;
          printf("\\\n");
        }
      size++;
     }
  
  printf("\";\n"
         "\n"
         "class %s_class\n"
         "{\n"
         "public:\n"
         "   %s_class(void)\n"
         "   {\n"
         "      CINData::add(\"%s\", %s_data, %s_len);\n"
         "   }\n"
         "};\n"
         "\n"
         "static %s_class %s_var;\n\n", 
         name, name, name, name, name, name, name);
  exit(0);
}

