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

#include "io.h"
#include "debug.h"
#include "GContainer.h"
#include "GThreads.h"

#ifdef PLUGIN
#include "GException.h"
#else
#include "exc_sys.h"
#include "exc_msg.h"
#include "throw_error.h"
#endif

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>

#define TYPE_INTEGER	1
#define TYPE_DOUBLE	2
#define TYPE_STRING	3
#define TYPE_POINTER	4
#define TYPE_ARRAY	5

static GCriticalSection	read_lock, write_lock;


static void
Read(int fd, void * buffer, int length,
     int refresh_pipe, void (* refresh_cb)(void))
{
#ifdef PLUGIN
   int size=length;
   int maxfd=refresh_pipe>fd ? refresh_pipe : fd;
   char * ptr=(char *) buffer;
   while(size>0)
   {
      fd_set read_fds;
      FD_ZERO(&read_fds);
      FD_SET(fd, &read_fds);
      if (refresh_pipe>=0 && refresh_cb)
        FD_SET(refresh_pipe, &read_fds);
      struct timeval tv;
      tv.tv_sec=5;
      tv.tv_usec=0;
      int rc=select(maxfd+1, &read_fds, 0, 0, &tv);
      if (rc>0 && FD_ISSET(fd, &read_fds))
        {
          errno = 0;
          int res=read(fd, ptr, size);
          if (res<0 && errno==EINTR)
            continue;
          if (res<0) 
            G_THROW(GUTF8String("PipeError: Failed to read data from pipe:\n") + strerror(errno));
          if (res==0) 
            G_THROW("PipeError: Unexpected end of pipe encountered");
          size-=res; ptr+=res;
        }
      if (rc<0 && errno!=EINTR)
        G_THROW(GUTF8String("PipeError: Failed to read data from pipe:\n") + strerror(errno));
      if (refresh_cb) refresh_cb();
   }
#else
   try
   {
      int size=length;
      char * ptr=(char *) buffer;
      while(size>0)
      {
        errno = 0;
        int res = read(fd, ptr, size);
        if (res<0 && errno==EINTR)
          continue;
        if (res<0) 
          ThrowError("Read", "Failed to read data from pipe");
        if (res==0)
          throw SYSTEM_ERROR("Read", "Unexpected end of pipe encountered");
        size-=res; ptr+=res;
      }
   } catch(Exception & exc)
   {
      throw PipeError(exc);
   }
#endif
}

static void
Write(int fd, const void * buffer, int length)
{
#ifdef SIGPIPE
  // Disable SIGPIPE and leave it that way!
  sigset_t mask;
  struct sigaction act;
  sigemptyset(&mask);
  sigaddset(&mask, SIGPIPE);
  sigprocmask(SIG_BLOCK, &mask, 0);
  sigaction(SIGPIPE, 0, &act);
  act.sa_handler = SIG_IGN;
  sigaction(SIGPIPE, &act, 0);
#endif

#ifdef PLUGIN
  int size=length;
  const char * ptr=(const char *) buffer;
  while(size>0)
    {
      errno = 0;
      int res=write(fd, ptr, size);
      if (res<0 && errno==EINTR) continue;
      if (res<=0)
        G_THROW(GUTF8String("PipeError: Failed to write data into pipe:\n") + strerror(errno));
      size-=res; ptr+=res;
    }
#else
  try
    {
      int size=length;
      const char * ptr=(const char *) buffer;
      while(size>0)
        {
          errno = 0;
          int res=write(fd, ptr, size);
          if (res<0 && errno==EINTR) continue;
          if (res<=0) 
            ThrowError("Write", "Failed to write data into pipe");
          size-=res; ptr+=res;
        }
    } 
  catch(Exception & exc)
    {
      throw PipeError(exc);
    }
#endif
}

void
WriteString(int fd, const char * str)
{
   GCriticalSectionLock lock(&write_lock);
   
   if (!str) str="";

   DEBUG_MSG("WriteString(): Writing string '" << str << "' to fd=" << fd << "\n");
   int type=TYPE_STRING;
   int length=strlen(str);
   Write(fd, &type, sizeof(type));
   Write(fd, &length, sizeof(length));
   Write(fd, str, length+1);
}

void
WriteInteger(int fd, int var)
{
   GCriticalSectionLock lock(&write_lock);
   
   DEBUG_MSG("WriteInteger(): Writing int '" << var << "' to fd=" << fd << "\n");
   int type=TYPE_INTEGER;
   Write(fd, &type, sizeof(type));
   Write(fd, &var, sizeof(var));
}

void
WriteDouble(int fd, double var)
{
   GCriticalSectionLock lock(&write_lock);
   
   DEBUG_MSG("WriteDouble(): Writing double '" << var << "' to fd=" << fd << "\n");
   int type=TYPE_DOUBLE;
   Write(fd, &type, sizeof(type));
   Write(fd, &var, sizeof(var));
}

void
WritePointer(int fd, const void * ptr)
{
   GCriticalSectionLock lock(&write_lock);
   
   DEBUG_MSG("WritePointer(): Writing ptr '" << ptr << "' to fd=" << fd << "\n");
   int type=TYPE_POINTER;
   Write(fd, &type, sizeof(type));
   Write(fd, &ptr, sizeof(ptr));
}

void
WriteArray(int fd, const TArray<char> & array)
{
   GCriticalSectionLock lock(&write_lock);
   
   DEBUG_MSG("WriteArray(): Writing array [" << array.lbound() << "..." <<
	      array.hbound() << "]\n");
   int type=TYPE_ARRAY;
   int size=array.size();
   Write(fd, &type, sizeof(type));
   Write(fd, &size, sizeof(size));
   Write(fd, (const char *) array, array.size());
}

GUTF8String
ReadString(int fd, int refresh_pipe, void (* refresh_cb)(void))
{
   GCriticalSectionLock lock(&read_lock);
   
   DEBUG_MSG("ReadString(): Attempting to read string from fd=" << fd << "\n");
   int type;
   Read(fd, &type, sizeof(type), refresh_pipe, refresh_cb);
#ifdef PLUGIN
   if (type!=TYPE_STRING) 
     G_THROW("PipeError: Unexpected type of data read from the pipe.");
#else
   if (type!=TYPE_STRING)
     throw PIPE_ERROR("ReadString", "Unexpected type of data read from the pipe.");
#endif
   
   int length;
   Read(fd, &length, sizeof(length), refresh_pipe, refresh_cb);
   
   TArray<char> array(length+1);
   Read(fd, (char *) array, length+1, refresh_pipe, refresh_cb);
   
   GUTF8String result=(char *) array;
   DEBUG_MSG("ReadString(): Read " << (const char *) result << "\n");
   return result;
}

int
ReadInteger(int fd, int refresh_pipe, void (* refresh_cb)(void))
{
   GCriticalSectionLock lock(&read_lock);
   
   DEBUG_MSG("ReadInteger(): Attempting to read int from fd=" << fd << "\n");
   int type;
   Read(fd, &type, sizeof(type), refresh_pipe, refresh_cb);
#ifdef PLUGIN
   if (type!=TYPE_INTEGER) 
     G_THROW("PipeError: Unexpected type of data read from the pipe.");
#else
   if (type!=TYPE_INTEGER)
     throw PIPE_ERROR("ReadInteger", "Unexpected type of data read from the pipe.");
#endif
   
   int var;
   Read(fd, &var, sizeof(var), refresh_pipe, refresh_cb);
   
   DEBUG_MSG("ReadInteger(): Read " << var << "\n");
   return var;
}

double
ReadDouble(int fd, int refresh_pipe, void (* refresh_cb)(void))
{
   GCriticalSectionLock lock(&read_lock);
   
   DEBUG_MSG("ReadDouble(): Attempting to read double from fd=" << fd << "\n");
   int type;
   Read(fd, &type, sizeof(type), refresh_pipe, refresh_cb);
#ifdef PLUGIN
   if (type!=TYPE_DOUBLE) 
     G_THROW("PipeError: Unexpected type of data read from the pipe.");
#else
   if (type!=TYPE_DOUBLE)
     throw PIPE_ERROR("ReadDouble", "Unexpected type of data read from the pipe.");
#endif
   
   double var;
   Read(fd, &var, sizeof(var), refresh_pipe, refresh_cb);
   
   DEBUG_MSG("ReadDouble(): Read " << var << "\n");
   return var;
}

void *
ReadPointer(int fd, int refresh_pipe, void (* refresh_cb)(void))
{
   GCriticalSectionLock lock(&read_lock);
   
   DEBUG_MSG("ReadPointer(): Attempting to read pointer from fd=" << fd << "\n");
   int type;
   Read(fd, &type, sizeof(type), refresh_pipe, refresh_cb);
#ifdef PLUGIN
   if (type!=TYPE_POINTER) 
     G_THROW("PipeError: Unexpected type of data read from the pipe.");
#else
   if (type!=TYPE_POINTER)
     throw PIPE_ERROR("ReadPointer", "Unexpected type of data read from the pipe.");
#endif
   
   void * ptr;
   Read(fd, &ptr, sizeof(ptr), refresh_pipe, refresh_cb);
   
   DEBUG_MSG("ReadPointer(): Read " << ptr << "\n");
   return ptr;
}

TArray<char>
ReadArray(int fd, int refresh_pipe, void (* refresh_cb)(void))
{
   GCriticalSectionLock lock(&read_lock);
   
   DEBUG_MSG("ReadArray(): Attempting to read array from fd=" << fd << "\n");
   int type;
   Read(fd, &type, sizeof(type), refresh_pipe, refresh_cb);
#ifdef PLUGIN
   if (type!=TYPE_ARRAY) 
     G_THROW("PipeError: Unexpected type of data read from the pipe.");
#else
   if (type!=TYPE_ARRAY)
     throw PIPE_ERROR("ReadArray", "Unexpected type of data read from the pipe.");
#endif
   
   int size;
   Read(fd, &size, sizeof(size), refresh_pipe, refresh_cb);
   
   TArray<char> array(size-1);
   Read(fd, (char *) array, size, refresh_pipe, refresh_cb);
   
   DEBUG_MSG("ReadArray(): Done reading array\n");
   return array;
}

void
ReadResult(int fd, const char * cmd_name, int refresh_pipe, void (* refresh_cb)(void))
{
   GCriticalSectionLock lock(&read_lock);
   
   DEBUG_MSG("ReadResult(): reading result code and, maybe, error message\n");
   GUTF8String res=ReadString(fd, refresh_pipe, refresh_cb);
   if (res!=OK_STRING)
   {
      GUTF8String error=ReadString(fd, refresh_pipe, refresh_cb);
#ifdef PLUGIN
      G_THROW(error);
#else
      throw ErrorMessage(cmd_name, error);
#endif
   }
}
