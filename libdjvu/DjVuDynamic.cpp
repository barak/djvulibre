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

