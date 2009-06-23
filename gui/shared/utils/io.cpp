//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either Version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C- 
//C- DjVuLibre-3.5 is derived from the DjVu(r) Reference Library from
//C- Lizardtech Software.  Lizardtech Software has authorized us to
//C- replace the original DjVu(r) Reference Library notice by the following
//C- text (see doc/lizard2002.djvu and doc/lizardtech2007.djvu):
//C-
//C-  ------------------------------------------------------------------
//C- | DjVu (r) Reference Library (v. 3.5)
//C- | Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- | The DjVu Reference Library is protected by U.S. Pat. No.
//C- | 6,058,214 and patents pending.
//C- |
//C- | This software is subject to, and may be distributed under, the
//C- | GNU General Public License, either Version 2 of the license,
//C- | or (at your option) any later version. The license should have
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
// $Id: io.cpp,v 1.10 2007/03/25 20:48:28 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "io.h"
#include "debug.h"
#include "GContainer.h"
#include "GThreads.h"
#include "GException.h"

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/time.h>

#define TYPE_INTEGER	1
#define TYPE_DOUBLE	2
#define TYPE_STRING	3
#define TYPE_POINTER	4
#define TYPE_ARRAY	5

static GCriticalSection	read_lock;
static GCriticalSection write_lock;
const char *PipeError::Tag = "PIPE";

static void
Read(int fd, void * buffer, int length,
     int refresh_pipe, void (* refresh_cb)(void))
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
        G_THROW(PipeError::Tag);
      if (res==0)
        G_THROW(PipeError::Tag);
      size-=res; ptr+=res;
    }
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
  int size=length;
  const char * ptr=(const char *) buffer;
  while(size>0)
    {
      errno = 0;
      int res=write(fd, ptr, size);
      if (res<0 && errno==EINTR) 
        continue;
      if (res<=0) 
        G_THROW(PipeError::Tag);
          size-=res; ptr+=res;
    }
}

void
WriteString(int fd, const char * str)
{
   GCriticalSectionLock lock(&write_lock);
   if (!str) str="";
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
   int type=TYPE_INTEGER;
   Write(fd, &type, sizeof(type));
   Write(fd, &var, sizeof(var));
}

void
WriteDouble(int fd, double var)
{
   GCriticalSectionLock lock(&write_lock);
   int type=TYPE_DOUBLE;
   Write(fd, &type, sizeof(type));
   Write(fd, &var, sizeof(var));
}

void
WritePointer(int fd, const void * ptr)
{
   GCriticalSectionLock lock(&write_lock);
   int type=TYPE_POINTER;
   Write(fd, &type, sizeof(type));
   Write(fd, &ptr, sizeof(ptr));
}

void
WriteArray(int fd, const TArray<char> & array)
{
   GCriticalSectionLock lock(&write_lock);
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
   int type;
   Read(fd, &type, sizeof(type), refresh_pipe, refresh_cb);
   if (type != TYPE_STRING)
     G_THROW(PipeError::Tag);
   int length;
   Read(fd, &length, sizeof(length), refresh_pipe, refresh_cb);
   TArray<char> array(length+1);
   Read(fd, (char *) array, length+1, refresh_pipe, refresh_cb);
   GUTF8String result=(char *) array;
   return result;
}

int
ReadInteger(int fd, int refresh_pipe, void (* refresh_cb)(void))
{
   GCriticalSectionLock lock(&read_lock);
   int type;
   Read(fd, &type, sizeof(type), refresh_pipe, refresh_cb);
   if (type!=TYPE_INTEGER)
     G_THROW(PipeError::Tag);
   int var;
   Read(fd, &var, sizeof(var), refresh_pipe, refresh_cb);
   return var;
}

double
ReadDouble(int fd, int refresh_pipe, void (* refresh_cb)(void))
{
   GCriticalSectionLock lock(&read_lock);
   int type;
   Read(fd, &type, sizeof(type), refresh_pipe, refresh_cb);
   if (type!=TYPE_DOUBLE)
     G_THROW(PipeError::Tag);
   double var;
   Read(fd, &var, sizeof(var), refresh_pipe, refresh_cb);
   return var;
}

void *
ReadPointer(int fd, int refresh_pipe, void (* refresh_cb)(void))
{
   GCriticalSectionLock lock(&read_lock);
   int type;
   Read(fd, &type, sizeof(type), refresh_pipe, refresh_cb);
   if (type!=TYPE_POINTER) 
     G_THROW(PipeError::Tag);
   void * ptr;
   Read(fd, &ptr, sizeof(ptr), refresh_pipe, refresh_cb);
   return ptr;
}

TArray<char>
ReadArray(int fd, int refresh_pipe, void (* refresh_cb)(void))
{
   GCriticalSectionLock lock(&read_lock);
   int type;
   Read(fd, &type, sizeof(type), refresh_pipe, refresh_cb);
   if (type!=TYPE_ARRAY)
     G_THROW(PipeError::Tag);
   int size;
   Read(fd, &size, sizeof(size), refresh_pipe, refresh_cb);
   TArray<char> array(size-1);
   Read(fd, (char *) array, size, refresh_pipe, refresh_cb);
   return array;
}
