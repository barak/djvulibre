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

#include "DjVuNavDir.h"
#include "debug.h"
#include "GException.h"
#include "GOS.h"
#include "ByteStream.h"
#include "GURL.h"
#include <ctype.h>

DjVuNavDir::DjVuNavDir(const GURL &dirURL)
{
   if (!dirURL) G_THROW( ERR_MSG("DjVuNavDir.zero_dir") );
   baseURL=dirURL.base();
}

DjVuNavDir::DjVuNavDir(ByteStream & str, const GURL &dirURL)
{
   if (!dirURL) G_THROW( ERR_MSG("DjVuNavDir.zero_dir") );
   
   baseURL=GURL(dirURL).base();
   
   decode(str);
}

void
DjVuNavDir::decode(ByteStream & str)
{
   GCriticalSectionLock lk(&lock);
   
   GList<GUTF8String> tmp_page2name;
   int eof=0;
   while(!eof)
   {
      char buffer[1024];
      char * ptr;
      for(ptr=buffer;ptr-buffer<1024;ptr++)
	 if ((eof=!str.read(ptr, 1)) || *ptr=='\n') break;
      if (ptr-buffer==1024) G_THROW( ERR_MSG("DjVuNavDir.long_line") );
      *ptr=0;
      if (!strlen(buffer)) continue;

      if (!tmp_page2name.contains(buffer))
	 tmp_page2name.append(buffer);
   };

   // Now copying lists to arrays for faster access later
   int pages=tmp_page2name.size();
   page2name.resize(pages-1);

   int cnt;
   GPosition pos;
   for(pos=tmp_page2name, cnt=0;pos;++pos, cnt++)
      page2name[cnt]=tmp_page2name[pos];
   
   // Now creating reverse mapping (strings => numbers)
   for(cnt=0;cnt<pages;cnt++)
   {
      name2page[page2name[cnt]]=cnt;
      url2page[GURL::UTF8(page2name[cnt],baseURL)]=cnt;
   }
}

#ifndef NEED_DECODER_ONLY
void
DjVuNavDir::encode(ByteStream & str)
{
   GCriticalSectionLock lk(&lock);

   for(int i=0;i<page2name.size();i++)
   {
      GUTF8String & name=page2name[i];
      str.writall((const char*)name, name.length());
      str.writall("\n", 1);
   };
}
#endif //NEED_DECODER_ONLY

int
DjVuNavDir::get_pages_num(void) const
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);
   
   return page2name.size();
}

int
DjVuNavDir::name_to_page(const char * name) const
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);

   if (!name2page.contains(name)) return -1;
   return name2page[name];
}

int
DjVuNavDir::url_to_page(const GURL & url) const
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);

   if (!url2page.contains(url)) return -1;
   return url2page[url];
}

GUTF8String
DjVuNavDir::page_to_name(int page) const
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);
   
   if (page<0) 
      G_THROW( ERR_MSG("DjVuNavDir.neg_page") );
   if (page>=page2name.size())
      G_THROW( ERR_MSG("DjVuNavDir.large_page") );
   return page2name[page];
}

GURL
DjVuNavDir::page_to_url(int page) const
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);
   
   return GURL::UTF8(page_to_name(page),baseURL);
}

void
DjVuNavDir::insert_page(int where, const char * name)
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);

   int pages=page2name.size();
   if (where<0) where=pages;
   
   page2name.resize(pages);
   for(int i=pages;i>where;i--)
      page2name[i]=page2name[i-1];
   page2name[where]=name;
   name2page[name]=where;
   url2page[GURL::UTF8(name,baseURL)]=where;
}

#ifndef NEED_DECODER_ONLY
void
DjVuNavDir::delete_page(int page_num)
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);

   int pages=page2name.size();
   
   if (page_num<0 || page_num>=pages)
      G_THROW( ERR_MSG("DjVuNavDir.bad_page") );

   for(int i=page_num;i<pages-1;i++)
      page2name[i]=page2name[i+1];
   page2name.resize(--pages-1);
}
#endif

