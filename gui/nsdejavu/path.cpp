//C-  -*- C++ -*-
//C-
//C- DjVu® Unix Viewer (v. 3.5)
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
//C-
// 
// $Id$
// $Name$

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"
#include "path.h"
#include "GException.h"
#include "names.h"

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdarg.h>

#ifndef MAXPATHLEN
#ifdef _MAX_PATH
#define MAXPATHLEN _MAX_PATH
#else
#define MAXPATHLEN 1024
#endif
#endif
#if ( MAXPATHLEN < 1024 )
#undef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

/* Logging */
#define DBG(x) /**/

/* Stuff for easily dealing with string allocation */

struct strpool_data {
  struct strpool_data *next;
  char data[1];
};

typedef struct strpool {
  struct strpool_data *first;
} strpool;

static void
strpool_init(strpool *pool)
{
  pool->first = 0;
}

static char *
strpool_alloc(strpool *pool, int n)
{
  char *b = new char[sizeof(struct strpool_data)+n];
  struct strpool_data *d = (struct strpool_data *)b;
  d->next = pool->first;
  d->data[n] = 0;
  pool->first = d;
  return d->data;
}

static void
strpool_fini(strpool *pool)
{
  struct strpool_data *d;
  while ((d = pool->first))
    {
      pool->first = d->next;
      delete (char*) d;
    }
}

static const char *
strconcat(strpool *pool, ...)
{
  va_list ap;
  int n = 0;
  char *d, *r;
  const char *s;
  va_start(ap, pool);
  while ((s = va_arg(ap, const char *)))
    n += strlen(s);
  va_end(ap);
  r = d = strpool_alloc(pool, n);
  va_start(ap, pool);
  while ((s = va_arg(ap, const char *)))
    while (*s)
      *d++ = *s++;
  *d = 0;
  return r;
}

/* Stuff to test and manipulate filenames */

static bool
is_file(const char *filename)
{
  DBG(("is_file(%s)\n",filename));
  struct stat buf;
  if (stat(filename,&buf) >= 0)
    if (! (buf.st_mode & S_IFDIR)) {
      DBG(("yes\n"));
      return true;
    }
  return false;
}

static bool
is_executable(const char *filename)
{
  if (!is_file(filename))
    return false;
  DBG(("is_executable(%s)\n", filename));
  if (access(filename, X_OK)<0)
    return false;
  DBG(("yes\n"));
  return true;
}

static const char *
dirname(strpool *pool, const char *fname)
{
  const char *s = fname + strlen(fname);
  while (s>fname && s[-1]=='/') s--;
  while (s>fname && s[-1]!='/') s--;
  while (s>fname && s[-1]=='/') s--;
  if (s == fname)
    return ( (s[0] == '/') ? "/" : "." );
  int n = s - fname;
  char *ret = strpool_alloc(pool, n);
  strncpy(ret, fname, n);
  return ret;
}

static const char *
pathclean(strpool *pool, const char *n)
{
  DBG(("Cleaning '%s'\n",n));
  char *ret = strpool_alloc(pool, strlen(n));
  char *d = ret;
  bool slash = false;
  if (n[0] == '/')
    *d++ = '/';
  while (*n)
    {
      if (n[0] == '/') {
        while (n[0] == '/')
          n += 1;
        continue;
      }
      if (n[0]=='.' && (n[1]=='/' || !n[1])) {
        n += 1;
        continue;
      }
      if (n[0]=='.' && n[1]=='.' 
          && (n[2]=='/' || !n[2])
          && d>ret && d[-1]!='/') {
        *d = 0;
        while (d>ret && d[-1] != '/')
          d -= 1;
        if (d[0]=='.' && d[1]=='.' && !d[2]) {
          d += 2;
        } else {
          n += 2;
          slash = false;
          continue;
        }
      }
      if (slash)
        *d++ = '/';
      while (n[0] && n[0] != '/')
        *d++ = *n++;
      slash = (n[0] == '/');
    }
  if (d == ret)
    *d++ = '.';
  *d = 0;
  DBG(("Got '%s'\n",ret));
  return ret;
}

static const char *
follow_symlinks(strpool *pool, const char *fname)
{
  const char *ret = fname;
#ifdef S_IFLNK
  int lnklen;
  char lnkbuf[MAXPATHLEN+1];
  while ((lnklen = readlink(ret, lnkbuf, sizeof(lnkbuf))) > 0) 
    {
      lnkbuf[lnklen] = 0;
      DBG(("Symlink %s -> %s\n", ret, lnkbuf));
      if (lnkbuf[0] != '/')
        ret = strconcat(pool, dirname(pool, ret), "/", lnkbuf, 0);
      else
        ret = lnkbuf;
      ret = pathclean(pool,ret);
      DBG(("Resolving as %s\n", ret));
    }
#endif
  return ret;
}

static const char *
pathelem(strpool *pool, const char **pathptr)
{
  if (*pathptr) {
    const char *s = strchr(*pathptr,':');
    if (s) {
      int n = s - *pathptr;
      char *ret = strpool_alloc(pool, n);
      strncpy(ret, *pathptr, n);
      *pathptr += n+1;
      return ret;
    }
    s = *pathptr;
    *pathptr = 0;
    return s;
  }
  return 0;
}

static const char *stdpath = 
"/usr/local/lib/netscape/plugins:"
"/usr/local/netscape/plugins:"
"/usr/lib/netscape/plugins:"
"/opt/netscape/plugins";

static const char *
get_plugin_path(strpool *pool)
{
  const char *env;
  const char *dir;
  // NPX_PLUGIN_PATH
  if ((env = getenv("NPX_PLUGIN_PATH"))) {
    while ((dir = pathelem(pool, &env))) {
      const char *lib = strconcat(pool, dir, "/", LIBRARY_NAME, 0);
      if (is_file(lib))
        return lib;
    }
  }
  // HOME
  if ((env = getenv("HOME"))) {
    const char *lib = strconcat(pool, env, "/.netscape/plugins/", LIBRARY_NAME, 0);
    if (is_file(lib))
      return lib;
  }
  // MOZILLA_HOME
  if ((env = getenv("MOZILLA_HOME"))) {
    const char *lib = strconcat(pool, env, "/plugins/", LIBRARY_NAME, 0);
    if (is_file(lib))
      return lib;
  }
  // OTHER
  env = stdpath;
  while ((dir = pathelem(pool, &env))) {
    const char *lib = strconcat(pool, dir, "/", LIBRARY_NAME, 0);
    if (is_file(lib))
      return lib;
  }
  return 0;
}

GUTF8String 
GetPluginPath(void)
{
  static GUTF8String path;
  if (! path) 
    {
      strpool apool;
      strpool_init(&apool);
      const char *p = get_plugin_path(&apool);
      path = GUTF8String(p);
      strpool_fini(&apool);
    }
  return path;
}

static const char *
get_viewer_path(strpool *pool)
{
  const char *env;
  const char *dir;
  const char *test;
  /* Environment variable NPX_DJVIEW overrides everything */
  if ((env = getenv("NPX_DJVIEW")))
    if (is_executable(env))
      return env;
  /* Try relative to plugin path */
  if ((env = (const char*)GetPluginPath())) {
    const char *envs = follow_symlinks(pool, env);
#ifdef AUTOCONF
    dir = dirname(pool, envs);
    test = strconcat(pool, dir, "/../../../bin/", DJVIEW_NAME, 0);
    if (is_executable(test))
      return test;
    test = strconcat(pool, dir, "/../../bin/", DJVIEW_NAME, 0);
    if (is_executable(test))
      return test;
#else /* ! AUTOCONF */
    dir = dirname(pool, env);
    test = strconcat(pool, dir, "/../", DEJAVU_DIR, "/", DJVIEW_NAME,0);
    if (is_executable(test))
      return test;
    dir = dirname(pool, envs);
    test = strconcat(pool, dir, "/../", DEJAVU_DIR, "/", DJVIEW_NAME,0);
    if (is_executable(test))
      return test;
#endif
  }
  /* Try ${bindir} */
#ifdef AUTOCONF
#ifdef DIR_BINDIR
  test = strconcat(pool,DIR_BINDIR,"/",DJVIEW_NAME,0);
  if (is_executable(test))
    return test;
#endif
#endif
  /* Try in the shell path */
  if ((env = getenv("PATH")))
    while ((dir = pathelem(pool, &env))) {
      test = strconcat(pool, dir, "/", DJVIEW_NAME, 0);
      if (is_executable(test))
        return test;
    }
  /* Deep trouble */
  return 0;
}

GUTF8String 
GetViewerPath(void)
{
  static GUTF8String path;
  if (! path) 
    {
      strpool apool;
      strpool_init(&apool);
      path = GUTF8String( get_viewer_path(&apool) );
      strpool_fini(&apool);
    }
  return path;
}

static const char *
get_library_path(strpool *pool)
{
  const char *env = 0;
  if ((env = (const char*)GetPluginPath())) {
    env = dirname(pool, env);
    env = strconcat(pool, env, "/../", DEJAVU_DIR, 0);
    env = pathclean(pool, env);
    DBG(("Library dir is %s\n", env));
    return env;
  }
  return 0;
}

GUTF8String 
GetLibraryPath(void)
{
  static GUTF8String path;
  if (! path) 
    {
      strpool apool;
      strpool_init(&apool);
      path = GUTF8String( get_library_path(&apool) );
      strpool_fini(&apool);
    }
  return path;
}
