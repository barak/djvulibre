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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "GException.h"
#include "DjVuMessageLite.h"
#include "debug.h"

// - Author: Leon Bottou, 05/1997

GException::GException() 
  : cause(0), file(0), func(0), line(0), source(GException::GINTERNAL)
{
}

const char * const
GException::outofmemory = ERR_MSG("GException.outofmemory");

GException::GException(const GException & exc) 
  : file(exc.file), func(exc.func), line(exc.line), source(exc.source)
{
  if (exc.cause && exc.cause!=outofmemory) 
    {
      char *s = new char[strlen(exc.cause)+1];
      strcpy(s, exc.cause);
      cause = s;
    }
  else
    {
      cause = exc.cause;
    }
}

GException::GException (const char *xcause, const char *file, int line,
   const char *func, const source_type xsource)
  : file(file), func(func), line(line), source(xsource)
{
  // good place to set a breakpoint and DEBUG message too. 
  // It'd hard to track exceptions which seem to go from nowhere
#ifdef DEBUG_MSG
  DEBUG_MSG("GException::GException(): cause=" << (xcause ? xcause : "unknown") << "\n");
#endif
  if (xcause && xcause!=outofmemory) 
    {
      char *s = new char[strlen(xcause)+1];
      strcpy(s, xcause);
      cause = s;
    }
  else
    {
      cause = xcause;
    }
}

GException::~GException(void)
{
  if (cause && cause!=outofmemory ) 
    delete [] const_cast<char*>(cause); 
  cause=file=func=0;
}

GException & 
GException::operator=(const GException & exc)
{
  if (cause && cause!=outofmemory) 
    delete [] const_cast<char*>(cause);
  cause = 0;
  file = exc.file;
  func = exc.func;
  line = exc.line;
  source=exc.source;
  if (exc.cause && exc.cause!=outofmemory) 
    {
      char *s = new char[strlen(exc.cause)+1];
      strcpy(s, exc.cause);
      cause = s;
    }
  else
    {
      cause = exc.cause;
    }
  return *this;
}

void
GException::perror(void) const
{
  fflush(0);
  DjVuPrintErrorUTF8("*** ");
  DjVuMessageLite::perror(get_cause());
  if (file && line>0)
    DjVuPrintErrorUTF8("*** (%s:%d)\n", file, line);    
  else if (file)
    DjVuPrintErrorUTF8("*** (%s)\n", file);
  if (func)
    DjVuPrintErrorUTF8("*** '%s'\n", func);    
  DjVuPrintErrorUTF8("\n");
}

const char* 
GException::get_cause(void) const
{
  if (! cause)
    return "Invalid exception";
  return cause;
}

int
GException::cmp_cause(const char s1[] , const char s2[])
{
  int retval;
  if(! s2 || !s2[0])
  {
    retval=(s1&&s1[0])?1:(-1);
  }else if(! s1 || !s1[0])
  {
    retval=(-1);
  }else
  {
    const char *end_s1=strpbrk(s1,"\t\n");
    const int n1=end_s1?(int)((size_t)end_s1-(size_t)s1):strlen(s1);
    const char *end_s2=strpbrk(s1,"\t\n");
    const int n2=end_s2?(int)((size_t)end_s2-(size_t)s2):strlen(s2);
    retval=(n1==n2)?strncmp(s1,s2,n1):strcmp(s1,s2);
  }
  return retval;
}

int
GException::cmp_cause(const char s2[]) const
{
  return cmp_cause(cause,s2);
}

#ifdef USE_EXCEPTION_EMULATION

GExceptionHandler *GExceptionHandler::head = 0;

void
GExceptionHandler::emthrow(const GException &gex)
{
  if (head)
    {
      head->current = gex;
      longjmp(head->jump, -1);
    }
  else
    {
      DjVuPrintErrorUTF8("\n*** Unhandled exception");
      gex.perror();
#ifndef UNDER_CE
      abort();
#else
      exit(EXIT_FAILURE);
#endif
    }
}

#else // ! USE_EXCEPTION_EMULATION

static int abort_on_exception = 0;

void 
#ifndef NO_LIBGCC_HOOKS
GExceptionHandler::exthrow(const GException &ex)
#else
GExceptionHandler::exthrow(const GException ex)
#endif /* NO_LIBGCC_HOOKS */
{
  if (abort_on_exception) 
    abort();
  throw ex;
}

void 
GExceptionHandler::rethrow(void)
{
  if (abort_on_exception) 
    abort();
  throw;
}

#endif



// ------ MEMORY MANAGEMENT HANDLER

#ifndef NEED_DJVU_MEMORY
// This is not activated when C++ memory management
// is overidden.  The overriding functions handle
// memory exceptions by themselves.
#if defined(_MSC_VER)
// Microsoft is different!
static int throw_memory_error(size_t) { G_THROW(GException::outofmemory); return 0; }
#ifndef UNDER_CE
// Does not exist under CE, check for null and throw an exception instead.
static int (*old_handler)(size_t) = _set_new_handler(throw_memory_error);
#endif
#else // !_MSC_VER
// Standard C++
static void throw_memory_error() { G_THROW(GException::outofmemory); }
static void (*old_handler)() = set_new_handler(throw_memory_error);
#endif // !_MSC_VER
#endif // !NEED_DJVU_MEMORY
