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

#include "DjVmDir0.h"
#include "ByteStream.h"
#include "debug.h"

int
DjVmDir0::get_size(void) const
      // WARNING! make sure, that get_size(), encode() and decode() are in sync
{
   int size=0;

   size+=2;	// number of files
   for(int i=0;i<num2file.size();i++)
   {
      FileRec & file=*num2file[i];
      size+=file.name.length()+1;	// file name
      size+=1;				// is IFF file
      size+=4;				// file offset
      size+=4;				// file size
   };

   return size;
}

#ifndef NEED_DECODER_ONLY
void
DjVmDir0::encode(ByteStream & bs)
      // WARNING! make sure, that get_size(), encode() and decode() are in sync
{
   bs.write16(num2file.size());
   for(int i=0;i<num2file.size();i++)
   {
      FileRec & file=*num2file[i];
      bs.writestring(file.name);
      bs.write8(0);
      bs.write8(file.iff_file);
      bs.write32(file.offset);
      bs.write32(file.size);
   }
}
#endif

void
DjVmDir0::decode(ByteStream & bs)
      // WARNING! make sure, that get_size(), encode() and decode() are in sync
{
   name2file.empty();
   num2file.empty();

   for(int i=bs.read16();i>0;i--)
   {
      GUTF8String name;
      char ch;
      while(bs.read(&ch, 1) && ch) name+=ch;
      bool iff_file=bs.read8()?true:false;
      int offset=bs.read32();
      int size=bs.read32();
      add_file(name, iff_file, offset, size);
   };
}

GP<DjVmDir0::FileRec>
DjVmDir0::get_file(const GUTF8String &name)
{
   if (name2file.contains(name))
     return name2file[name];
   return 0;
}

GP<DjVmDir0::FileRec>
DjVmDir0::get_file(int file_num)
{
   if (file_num<num2file.size()) return num2file[file_num];
   return 0;
}

void
DjVmDir0::add_file(
  const GUTF8String &name, bool iff_file, int offset, int size)
{
   DEBUG_MSG("DjVmDir0::add_file(): name='" << name << "', iff=" << iff_file <<
	     ", offset=" << offset << "\n");
   
   if (name.search('/') >= 0)
     G_THROW( ERR_MSG("DjVmDir0.no_slash") );
   
   GP<FileRec> file=new FileRec(name, iff_file, offset, size);
   name2file[name]=file;
   num2file.resize(num2file.size());
   num2file[num2file.size()-1]=file;
}
