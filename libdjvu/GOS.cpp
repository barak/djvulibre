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

#include "GException.h"
#include "GThreads.h"
#include "GOS.h"
#include "GURL.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#ifdef WIN32
#include <atlbase.h>
#include <windows.h>

#ifndef UNDER_CE
#include <direct.h>
#endif
#endif   // end win32



#ifdef UNIX
# include <errno.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <fcntl.h>
# include <pwd.h>
# include <stdio.h>
# include <unistd.h>
#endif

#ifdef macintosh
#include <unix.h>
#include <errno.h>
#include <unistd.h>
#endif

// -- TRUE FALSE
#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

// -- MAXPATHLEN
#ifndef MAXPATHLEN
#ifdef _MAX_PATH
#define MAXPATHLEN _MAX_PATH
#else
#define MAXPATHLEN 1024
#endif
#else
#if ( MAXPATHLEN < 1024 )
#undef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
#endif

// static const char filespecslashes[] = "file://";
static const char slash='/';
static const char percent='%';
static const char localhostspec1[] = "//localhost/";
static const char backslash='\\';
static const char colon=':';

static const char localhost[] = "localhost";
static const char localhostspec2[] = "///";
static const char filespec[] = "file:";
static const char dot='.';
static const char nillchar=0;
#if defined(UNIX)
  static const char tilde='~';
  static const char root[] = "/";
#elif defined(WIN32)
  static const char root[] = "\\";
#elif defined(macintosh)
  static char const * const root = &nillchar; 
#else
#error "Define something here for your operating system"
#endif


// -----------------------------------------
// Functions for dealing with filenames
// -----------------------------------------

static inline int
finddirsep(const GUTF8String &fname)
{
#if defined(UNIX)
  return fname.rsearch('/',0);
#elif defined(WIN32)
  return fname.rcontains("\\/",0);
#elif defined(macintosh)
  return fname.rcontains(":/",0);
#else
#error "Define something here for your operating system"
#endif  
}


// basename(filename[, suffix])
// -- returns the last component of filename and removes suffix
//    when present. works like /bin/basename.
GUTF8String 
GOS::basename(const GUTF8String &gfname, const char *suffix)
{
  if(!gfname.length())
    return gfname;

  const char *fname=gfname;
#ifdef WIN32
  // Special cases
  if (fname[1] == colon)
  {
    if(!fname[2])
    {
      return gfname;
    }
    if (!fname[3] && (fname[2]== slash || fname[2]== backslash))
    {
      char string_buffer[4];
      string_buffer[0] = fname[0];
      string_buffer[1] = colon;
      string_buffer[2] = backslash; 
      string_buffer[3] = 0; 
      return string_buffer;
    }
  }
#endif


  // Allocate buffer
  GUTF8String retval(gfname,finddirsep(gfname)+1,(unsigned int)(-1));
  fname=retval;

  // Process suffix
  if (suffix)
  {
    if (suffix[0]== dot )
      suffix ++;
    if (suffix[0])
    {
      const GUTF8String gsuffix(suffix);
      const int sl = gsuffix.length();
      const char *s = fname + strlen(fname);
      if (s > fname + sl)
      {
        s = s - (sl + 1);
        if(*s == dot && (GUTF8String(s+1).downcase() == gsuffix.downcase()))
        {
          retval.setat((int)((size_t)s-(size_t)fname),0);
        }
      }
    }
  }
  return retval;
}



// errmsg --
// -- A small helper function returning a 
//    stdio error message in a static buffer.

static GNativeString 
errmsg()
{
  GNativeString buffer;
#ifdef REIMPLEMENT_STRERROR
  const char *errname = "Unknown libc error";
  if (errno>0 && errno<sys_nerr)
    errname = sys_errlist[errno];
#else
#ifndef UNDER_CE
  const char *errname = strerror(errno);
  buffer.format("%s (errno = %d)", errname, errno);
#else
  const char *errname = "Unknown error from GOS.cpp under Windows CE" ;
  buffer.format("%s (errno = %d)", errname, -1);
#endif
#endif
  return buffer;
}



// -----------------------------------------
// Functions for measuring time
// -----------------------------------------

// ticks() --
// -- returns the number of milliseconds elapsed since 
//    a system dependent date.
unsigned long 
GOS::ticks()
{
#ifdef UNIX
  struct timeval tv;
  if (gettimeofday(&tv, NULL) < 0)
    G_THROW(errmsg());
  return (unsigned long)( ((tv.tv_sec & 0xfffff)*1000) 
                          + (tv.tv_usec/1000) );
#else
#ifdef WIN32
  DWORD clk = GetTickCount();
  return (unsigned long)clk;
#else
#ifdef macintosh
  return (unsigned long)((double)TickCount()*16.66);
#else
#error "Define something here for your operating system"
#endif
#endif  
#endif
}

// sleep(int milliseconds) --
// -- sleeps during the specified time (in milliseconds)
void 
GOS::sleep(int milliseconds)
{
#ifdef UNIX
  struct timeval tv;
  tv.tv_sec = milliseconds / 1000;
  tv.tv_usec = (milliseconds - (tv.tv_sec * 1000)) * 1000;
#if defined(THREADMODEL) && (THREADMODEL==COTHREADS)
  GThread::select(0, NULL, NULL, NULL, &tv);
#else
  select(0, NULL, NULL, NULL, &tv);
#endif
#endif
#ifdef WIN32
  Sleep(milliseconds);
#endif
#ifdef macintosh
    unsigned long tick = ticks(), now;
    while (1) {
        now = ticks();
        if ((tick+milliseconds) < now)
            break;
        GThread::yield();
    }
#endif
}

  

#if 0
/*MBCS*/
GString
GOS::encode_mbcs_reserved(const char * filename)
      // Called from ByteStream to encode new OS-safe filenames
{
   const char *hex = "0123456789ABCDEF";
   
   GString res;

   for(const char * ptr=filename;*ptr;ptr++)
   {
#ifdef WIN32
 		if (IsDBCSLeadByte((BYTE)*ptr)) { //MBCS DBCS
			// escape sequence
			res+=hex[(*ptr >> 4) & 0xf];
			res+=hex[(*ptr) & 0xf];
			ptr++;
			if (*ptr){
				// escape sequence
				res+=hex[(*ptr >> 4) & 0xf];
				res+=hex[(*ptr) & 0xf];
			}
			continue;
		}
#endif
     if ((*ptr>='a' && *ptr<='z')
        || (*ptr>='A' && *ptr<='Z')
        || (*ptr>='0' && *ptr<='9')
        || (strchr("$-_.+!*'(),:;", *ptr))) // Added : because of windows!
      {
        res+=*ptr;
      }else
      {
      // escape sequence
        //res+=percent;
        res+=hex[(*ptr >> 4) & 0xf];
        res+=hex[(*ptr) & 0xf];
      }
   }
   
   return res;
}
/*MBCS*/
#endif

// -----------------------------------------
// Testing
// -----------------------------------------

#if defined(AUTOCONF) && !defined(HAVE_STRERROR)
#define NEED_STRERROR
#elif defined(sun) && ! defined(svr4)
#define NEED_STRERROR
#endif
#ifdef NEED_STRERROR
// strerror() is not defined under SunOS.
char *
strerror(int errno)
{
  extern int sys_nerr;
  extern char *sys_errlist[];
  if (errno>0 && errno<sys_nerr) 
    return sys_errlist[errno];
  return "unknown stdio error";
}
#endif

// cwd([dirname])
// -- changes directory to dirname (when specified).
//    returns the full path name of the current directory. 
GUTF8String 
GOS::cwd(const GUTF8String &dirname)
{
#if defined(UNIX) || defined(macintosh) 
  if (dirname.length() && chdir(dirname.getUTF82Native())==-1)//MBCS cvt
    G_THROW(errmsg());
  char *string_buffer;
  GPBuffer<char> gstring_buffer(string_buffer,MAXPATHLEN+1);
  char *result = getcwd(string_buffer,MAXPATHLEN);
  if (!result)
    G_THROW(errmsg());
  return GNativeString(result).getNative2UTF8();//MBCS cvt
#elif defined(UNDER_CE)
  return GUTF8String(dot) ;
#elif defined (WIN32)
  char drv[2];
  if (dirname.length() && _chdir(dirname.getUTF82Native())==-1)//MBCS cvt
    G_THROW(errmsg());
  drv[0]= dot ; drv[1]=0;
  char *string_buffer;
  GPBuffer<char> gstring_buffer(string_buffer,MAXPATHLEN+1);
  char *result = getcwd(string_buffer,MAXPATHLEN);
  GetFullPathName(drv, MAXPATHLEN, string_buffer, &result);
  return GNativeString(string_buffer).getNative2UTF8();//MBCS cvt
#else
#error "Define something here for your operating system"
#endif 
}

GUTF8String
GOS::getenv(const GUTF8String &name)
{
  GUTF8String retval;
#ifndef UNDER_CE
  if(name.length())
  {
    const char *env=::getenv(name.getUTF82Native());
    if(env)
    {
      retval=GNativeString(env);
    }
  }
#endif
  return retval;
}

// -----------------------------------------
// Testing
// -----------------------------------------

#ifdef TEST

int main(int argc, char **argv)
{

   GArray<GString> dargv(0, argc-1);
   for ( int i=0; i<argc; ++i)
   {
      GString g(argv[i]);
      dargv[i]=g.getNative2UTF8();
   }

  GString op;
  if (argc>1) 
    op = dargv[1];
  if (op == "is_file" && argc==3) {
      DjVuPrintMessage("%d\n", GOS::is_file(dargv[2])?1:0);
      return 0;
  } else if (op =="is_dir" && argc==3) {
    DjVuPrintMessage("%d\n", GOS::is_dir(dargv[2]));
    return 0;
  } else if (op == "dirname" && argc==3) {
    DjVuPrintMessage("%s\n", (const char*)GOS::dirname(dargv[2]));
    return 0;
  } else if (op == "basename" && argc==3) {
    DjVuPrintMessage("%s\n", (const char*)GOS::basename(dargv[2]));
    return 0;
  } else if (op == "basename" && argc==4) {
    DjVuPrintMessage("%s\n", (const char*)GOS::basename(dargv[2], dargv[3]));
    return 0;
  } else if (op == "cwd" && argc==2) {
    DjVuPrintMessage("%s\n", (const char*)GOS::cwd());
    return 0;
  } else if (op == "cwd" && argc==3) {
    DjVuPrintMessage("%s\n", (const char*)GOS::cwd(dargv[2]));
    return 0;
  } else if (op == "cleardir" && argc==3) {
    DjVuPrintMessage("%d\n", GOS::cleardir(dargv[2]));
    return 0;
  } else if (op == "expand_name" && argc==3) {
    DjVuPrintMessage("%s\n", (const char*)GOS::expand_name(dargv[2]));
    return 0;
  } else if (op == "expand_name" && argc==4) {
    DjVuPrintMessage("%s\n", (const char*)GOS::expand_name(dargv[2], dargv[3]));
    return 0;
  } else if (op == "ticks" && argc==2) {
    DjVuPrintMessage("%lu\n", GOS::ticks());
    return 0;
  } else if (op == "sleep" && argc==3) {
//    GOS::sleep(atoi(argv[2]));
     GOS::sleep(dargv[2].toInt());
    return 0;
  } else if (op == "filename_to_url" && argc==3) {
    DjVuPrintMessage("%s\n", (const char*)GOS::filename_to_url(dargv[2]));
    return 0;
  } else if (op == "url_to_filename" && argc==3) {
    DjVuPrintMessage("%s\n", (const char*)GOS::url_to_filename(dargv[2]));
    return 0;
  }
  DjVuPrintErrorUTF8("%s\n","syntax error");
  return 10;
}

#endif

