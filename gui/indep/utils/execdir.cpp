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

#include "DjVuMessageLite.h"
#include "DjVuMessage.h"

#include "execdir.h"


static GList<GURL>
parsePATH(void)
{
  GList<GURL> retval;
  const char *path=getenv("PATH");
  if(path)
  {
    GNativeString p(path);
    int from=0;
    for(int to;(to=p.search(':',from))>0;from=to+1)
    {
      if(to > from)
      {
        retval.append(GURL::Filename::Native(p.substr(from,to-from)));
      }
    }
    if((from+1)<(int)p.length())
    {
      retval.append(GURL::Filename::Native(p.substr(from,-1)));
    }
  }
  return retval;
}

GURL 
getExecDir(void)
{
  // This is the same as GetModulePath in DjVuMessage.cpp
 GURL retval;
 GUTF8String &xprogramname=DjVuMessage::programname();
 if(xprogramname.length())
 {
   if(xprogramname[1]=='/'
     ||!xprogramname.cmp("../",3)
     ||!xprogramname.cmp("./",2))
   {
     retval=GURL::Filename::UTF8(xprogramname);
   }
   if(retval.is_empty() || !retval.is_file())
   {
     GList<GURL> paths(parsePATH());
     GMap<GUTF8String,void *> pathMAP;
     for(GPosition pos=paths;pos;++pos)
     {
       retval=GURL::UTF8(xprogramname,paths[pos]);
       const GUTF8String path(retval.get_string());
       if(!pathMAP.contains(path))
       {
         if(retval.is_file())
           break;
         pathMAP[path]=0;
       }
     }
   }
   if (! retval.is_empty() )
     retval = retval.follow_symlinks();
   if (! retval.is_empty() )
     retval = retval.base();
 }
 return retval;
}

GURL 
getDjVuDataFile(const char *fname)
{
  GList<GURL> paths = DjVuMessage::GetProfilePaths();
  GUTF8String file = fname;
  for (GPosition pos=paths; pos; ++pos)
    {
      GURL url = GURL(file, paths[pos]);
      if(url.is_file())
        return url;
    }
  return GURL();
}


