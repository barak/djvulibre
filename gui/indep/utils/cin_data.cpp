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


#include "cin_data.h"
#include "BSByteStream.h"

CINData::Item::Item(const char * name_, const char * data_, int length_)
{
   name=name_;
   data.resize(length_-1);
   memcpy(data, data_, length_);
}

GPList<CINData::Item>	* CINData::items;

GPList<CINData::Item> *
CINData::get_items(void)
{
   if (!items) items=new GPList<Item>();
   return items;
}

GP<ByteStream>
CINData::get(const char * name)
{
   GPList<Item> * items=get_items();

   for(GPosition pos=*items;pos;++pos)
   {
      GP<Item> item=(*items)[pos];
      if (item->name==name)
      {
	    // Decompress data and put it into a MemoryByteStream
	 GP<ByteStream> mem_str_in=ByteStream::create((const char *) item->data, item->data.size());
	 GP<ByteStream> bs_str=BSByteStream::create(mem_str_in);
	 
	 GP<ByteStream> mem_str_out=ByteStream::create();
	 mem_str_out->copy(*bs_str);
	 mem_str_out->seek(0);
	 return mem_str_out;	 
      }
   }
   return 0;
}

void
CINData::add(const char * name, const char * data, int length)
{
   GP<Item> new_item=new Item(name, data, length);
   
   GPList<Item> * items=get_items();

   for(GPosition pos=*items;pos;++pos)
   {
      GP<Item> item=(*items)[pos];
      if (item->name==name)
      {
	 *item=*new_item;
	 return;
      }
   }

   items->append(new_item);
}
