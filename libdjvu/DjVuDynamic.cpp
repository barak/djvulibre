//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C- 
//C- DjVuLibre-3.5 is derived from the DjVu(r) Reference Library
//C- distributed by Lizardtech Software.  On July 19th 2002, Lizardtech 
//C- Software authorized us to replace the original DjVu(r) Reference 
//C- Library notice by the following text (see etc/PATENT.djvu):
//C-
//C-  ------------------------------------------------------------------
//C- | DjVu (r) Reference Library (v. 3.5)
//C- | Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- | The DjVu Reference Library is protected by U.S. Pat. No.
//C- | 6,058,214 and patents pending.
//C- |
//C- | This software is subject to, and may be distributed under, the
//C- | GNU General Public License, Version 2. The license should have
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
// 
// $Id$
// $Name$

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// From: Leon Bottou, 1/31/2002
// This is purely Lizardtech stuff.

#include "GException.h"
#include "DjVuDynamic.h"
#include "GURL.h"
#include "GString.h"

#if defined(WIN32) || HAS_DLOPEN 

#if defined(WIN32)
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

class DjVuDynamicLib : public GPEnabled
{
public:
  DjVuDynamicLib(const GUTF8String &name);
  ~DjVuDynamicLib();
  static GP<DjVuDynamicLib> create(const GUTF8String &name,GUTF8String &error);
  void *lookup(const GUTF8String &name);
private:
  const GUTF8String name;
  GMap<GUTF8String,void *> smap;
#ifdef WIN32
  HINSTANCE handle;
#else
  void *handle;
#endif
};

DjVuDynamicLib::DjVuDynamicLib(const GUTF8String &xname)
: name(xname), handle(0) {}

GP<DjVuDynamicLib>
DjVuDynamicLib::create(const GUTF8String &name, GUTF8String &error)
{
  DjVuDynamicLib * const lib=new DjVuDynamicLib(name);
  GP<DjVuDynamicLib> retval(lib);
#ifdef WIN32
  lib->handle=LoadLibrary((const char *)GNativeString(name));
#else
  lib->handle=dlopen((const char *)GNativeString(name),RTLD_LAZY);
#endif
  if(!lib->handle)
  {
#ifndef WIN32
    const GUTF8String mesg=GNativeString(dlerror());
    if(mesg.length())
    {
      error=(ERR_MSG("DjVuDynamicLib.failed_open2") "\t")+name+"\t"+mesg;
    }else
#endif
    {
      error=(ERR_MSG("DjVuDynamicLib.failed_open") "\t")+name;
    }
    retval=0;
  }
  return retval;
}

DjVuDynamicLib::~DjVuDynamicLib()
{
  if(handle)
  {
#ifdef WIN32
    FreeLibrary(handle);
#else
    dlclose(handle);
#endif
  }
}

void *
DjVuDynamicLib::lookup(const GUTF8String &name)
{
  GPosition pos = smap.contains(name);
  if(handle && !pos)
  {
#ifdef WIN32
    smap[name]=GetProcAddress(handle,(const char *)name);
#else
    smap[name]=dlsym(handle,(const char *)name);
#endif
    pos = smap.contains(name);
  }
  return pos ? smap[pos] : 0;
}

static DjVuDynamicLib *
LookupDjVuDynamic(const GUTF8String &libname,GUTF8String &error)
{
  static GPMap<GUTF8String, DjVuDynamicLib> lmap;
  DjVuDynamicLib *retval=0;
  GPosition pos = lmap.contains(libname);
  if(! pos)
    {
      retval = DjVuDynamicLib::create(libname,error);
      lmap[libname]= retval;
    }
  else
    {
      retval = lmap[pos];
    }
  if(!retval)
    {
      error=(ERR_MSG("DjVuDynamicLib.failed_open") "\t")+libname;
    }
  return retval;
}

DjVuDynamic::DjVuDynamic(const GUTF8String &libname)
{
  lib = LookupDjVuDynamic(libname,error);
}

DjVuDynamic::DjVuDynamic(const GURL &liburl)
{
  lib = LookupDjVuDynamic(liburl.UTF8Filename(),error);
}

void *
DjVuDynamic::lookup(const GUTF8String &symname, const bool nothrow)
{
  void *retval=0;
  if (lib)
    {
      retval=lib->lookup(symname); 
    }
  else if(!nothrow)
    {
      G_THROW(error);
    }
  return retval;
}
#else // defined(WIN32) || HAS_DLOPEN

DjVuDynamic::DjVuDynamic(const GUTF8String &libname)
{
  error=(ERR_MSG("DjVuDynamicLib.failed_open") "\t")+libname;
}

DjVuDynamic::DjVuDynamic(const GURL &liburl)
{
  error=(ERR_MSG("DjVuDynamicLib.failed_open") "\t")+liburl.get_string();
}

void *
DjVuDynamic::lookup(const GUTF8String &symname, const bool nothrow=false)
{
  if(!nothrow)
    G_THROW(error);
  return 0;
}
#endif // defined(WIN32) || HAS_DLOPEN 

DjVuDynamic::DjVuDynamic(void)
{
  error=ERR_MSG("DjVuDynamicLib.failed_open") "\t" "(null)";
}

