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
  GP<ByteStream> gstr_in=ByteStream::create(GURL::Filename::UTF8("-"),"rb");
  ByteStream &str_in=*gstr_in;
  GP<ByteStream> gmem_str1=ByteStream::create();
  ByteStream &mem_str1=*gmem_str1;
  mem_str1.copy(str_in);
  
  // Now compute block size for BSByteStream
  int block_size=1+(mem_str1.size() >> 10);
  if (block_size<10) block_size=10;
  if (block_size>1024) block_size=1024;
  
  // Compress data
  GP<ByteStream> mem_str2=ByteStream::create();
  {
    GP<ByteStream> bs_str=BSByteStream::create(mem_str2, block_size);
    mem_str1.seek(0);
    bs_str->copy(mem_str1);
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

