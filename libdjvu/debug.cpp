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
#ifdef NO_DEBUG
#undef NO_DEBUG
#endif

#include "debug.h"
#include "GString.h"
#include "ByteStream.h"
#include "GURL.h"

#if ( DEBUGLVL > 0 )

#include "GThreads.h"
#include "GContainer.h"
#include <stdarg.h>
#include <stdio.h>
#ifndef UNDER_CE
#include <errno.h>
#endif


#ifndef UNIX
#ifndef WIN32
#ifndef macintosh
#define UNIX
#endif
#endif
#endif

static GCriticalSection debug_lock;
#ifdef RUNTIME_DEBUG_ONLY
static int              debug_level = 0;
#else
static int              debug_level = DEBUGLVL;
#endif
static int              debug_id;
static GP<ByteStream>   debug_file;
static int              debug_file_count;

#if THREADMODEL==NOTHREADS
static DjVuDebug debug_obj;
#else
static GMap<long, DjVuDebug> &
debug_map(void)
{
  static GMap<long, DjVuDebug> xmap;
  return xmap;
}
#endif

DjVuDebug::DjVuDebug()
  : block(0), indent(0)
{
  id = debug_id++;
  set_debug_file(stderr);
}

DjVuDebug::~DjVuDebug()
{
#ifdef UNIX
  if (--debug_file_count == 0)
    {
//      if (debug_file && (debug_file != stderr))
//        fclose(debug_file);
      debug_file = 0;
    }
#endif
}

void   
DjVuDebug::format(const char *fmt, ... )
{
  if (! block)
    {
      va_list ap;
      va_start(ap, fmt);
      GUTF8String buffer(fmt,ap);
      va_end(ap);
      GCriticalSectionLock glock(&debug_lock);
      if(debug_file)
      {
        debug_file->writestring(buffer);
        debug_file->flush();
      }
#ifdef WIN32
      else
      {
        USES_CONVERSION;
        OutputDebugString(A2CT((const char *)buffer));
      }
#endif
    }
}

void   
DjVuDebug::set_debug_level(int lvl)
{
  debug_level = lvl;
}

void
DjVuDebug::set_debug_url(const GURL &url)
{
// #ifdef UNIX
//  GCriticalSectionLock glock(&debug_lock);
//  if (debug_file && (debug_file != stderr))
//    fclose(debug_file);
  debug_file = 0;
  if (!url.is_empty())
  {
    debug_file = ByteStream::create(url, "w");
    if (! debug_file)
      debug_file = ByteStream::create(2,0,false);
  }else
  {
    debug_file = ByteStream::create(2,0,false);
  }
// #endif
}

void
DjVuDebug::set_debug_file(FILE * file)
{
#ifdef UNIX
//  GCriticalSectionLock glock(&debug_lock);
//  if (debug_file && (debug_file != stderr))
//    fclose(debug_file);
  debug_file = 0;
  debug_file = ByteStream::create(file,"w",(file!=stderr));
#endif
}

void
DjVuDebug::modify_indent(int rindent)
{
  indent += rindent;
}

DjVuDebug& 
DjVuDebug::lock(int lvl, int noindent)
{
  int threads_num=1;
  debug_lock.lock();
  // Find Debug object
#if THREADMODEL==NOTHREADS
  // Get no-threads debug object
  DjVuDebug &dbg = debug_obj;
#else
  // Get per-thread debug object
  long threadid = (long) GThread::current();
  DjVuDebug &dbg = debug_map()[threadid];
  threads_num=debug_map().size();
#endif
  // Check level
  dbg.block = (lvl > debug_level);
  // Output thread id and indentation
  if (! noindent)
    {
      if (threads_num>1)
        dbg.format("[T%d] ", dbg.id);
      int ind = dbg.indent;
      char buffer[257];
      memset(buffer,' ', sizeof(buffer)-1);
      buffer[sizeof(buffer)-1] = 0;
      while (ind > (int)sizeof(buffer)-1)
        {
          dbg.format("%s", buffer);
          ind -= sizeof(buffer)-1;
        }
      if (ind > 0)
        {
          buffer[ind] = 0;
          dbg.format("%s", buffer);
        }
    }
  // Return
  return dbg;
}

void
DjVuDebug::unlock()
{
  debug_lock.unlock();
}

#define OP(type, fmt) \
DjVuDebug& DjVuDebug::operator<<(type arg)\
{ format(fmt, arg); return *this; }

DjVuDebug& DjVuDebug::operator<<(bool arg)
{
   format("%s", arg ? "TRUE" : "FALSE"); return *this;
}

OP(char, "%c")
OP(unsigned char, "%c")
OP(int, "%d")
OP(unsigned int, "%u")
OP(short int, "%hd")
OP(unsigned short int, "%hu")
OP(long, "%ld")
OP(unsigned long, "%lu")
OP(float, "%g")
OP(double, "%g")
OP(const void * const, "0x%08x")

DjVuDebug& DjVuDebug::operator<<(const char * const ptr) 
{
  GUTF8String buffer(ptr?ptr:"(null)");
  if(buffer.length() > 255)
  {
    buffer=buffer.substr(0,252)+"...";
  }
  format("%s", (const char *)buffer);
  return *this; 
}

DjVuDebug& DjVuDebug::operator<<(const unsigned char * const ptr) 
{ 
  return operator<<( (const char *) ptr );
}

DjVuDebugIndent::DjVuDebugIndent(int inc)
  : inc(inc)
{
  DjVuDebug &dbg = DjVuDebug::lock(0,1);
  dbg.modify_indent(inc);
  dbg.unlock();
}

DjVuDebugIndent::~DjVuDebugIndent()
{
  DjVuDebug &dbg = DjVuDebug::lock(0,1);
  dbg.modify_indent(-inc);
  dbg.unlock();
}

#endif
