//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.5)
//C- 
//C- Copyright © 1997-2001 LizardTech, Inc. All Rights Reserved.
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

// - Author: Leon Bottou, 05/1997

#include <string.h>
#include "GThreads.h"
#include "GSmartPointer.h"
#include "GException.h"


// ------ STATIC CRITICAL SECTION

static GCriticalSection gcsCounter;


// ------ GPENABLED


GPEnabled::~GPEnabled()
{
  if (count > 0)
    G_THROW( ERR_MSG("GSmartPointer.suspicious") );
}

void
GPEnabled::destroy()
{
  if (count >= 0)
    G_THROW( ERR_MSG("GSmartPointer.suspicious") );
  delete this;
}

void 
GPEnabled::ref()
{
  gcsCounter.lock();
  count++;
  gcsCounter.unlock();
}

void 
GPEnabled::unref()
{
  gcsCounter.lock();
  if (! --count) 
    count = -1;
  gcsCounter.unlock();
  if (count < 0)
    destroy();
}


// ------ GPBASE


GPBase&
GPBase::assign (GPEnabled *nptr)
{
  gcsCounter.lock();
  if (nptr)
    {
      if (nptr->count >= 0)  
        nptr->count++;
      else
        nptr = 0;
    }
  if (ptr)
    {
      GPEnabled *old = ptr;
      ptr = nptr;
      if (! --old->count) 
        old->count = -1;
      gcsCounter.unlock();      
      if (old->count < 0)
        old->destroy();
    }
  else
    {
      ptr = nptr;
      gcsCounter.unlock();
    }
  return *this;
}

GPBase&
GPBase::assign (const GPBase &sptr)
{
  gcsCounter.lock();
  if (sptr.ptr) 
    {
      sptr.ptr->count++;
    }
  if (ptr)
    {
      GPEnabled *old = ptr;
      ptr = sptr.ptr;
      if (! --old->count) 
        old->count = -1;
      gcsCounter.unlock();      
      if (old->count < 0)
        old->destroy();
    }
  else
    {
      ptr = sptr.ptr;
      gcsCounter.unlock();
    }
  return *this;
}

void 
GPBufferBase::swap(GPBufferBase &other)
{
  void * const temp_ptr=ptr;
  ptr=other.ptr;
  other.ptr=temp_ptr;
  const size_t temp_num=num;
  num=other.num;
  other.num=temp_num;
}

void
GPBufferBase::resize(const size_t n, const size_t t)
{
  if(!n && !ptr)
  {
    num=0;
  }else
  {
    const size_t s=ptr?(((num<n)?num:n)*t):0;
    void *nptr;
    GPBufferBase gnptr(nptr, n, t);
    if(s)
    {
      memcpy(nptr, ptr, s);
    }
    swap(gnptr);
  }
}

void
GPBufferBase::set(const size_t t,const char c)
{
  if(num)
  {
    memset(ptr,c,num*t);
  }
}

