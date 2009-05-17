/*
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
*/

#include "config.h"

#ifndef DJVIEW_NAME
# define DJVIEW_NAME "djview"
#endif

#ifndef DJVIEW4_NAME
# define DJVIEW4_NAME "djview4"
#endif

#ifndef DJVIEW3_NAME
# define DJVIEW3_NAME "djview3"
#endif

#ifndef LIBRARY_NAME
# if defined(hpux)
#  define LIBRARY_NAME    "nsdejavu.sl"
# elif defined(WIN32) || defined(__CYGWIN32__)
#  define LIBRARY_NAME    "nsdejavu.dll"
# else
#  define LIBRARY_NAME    "nsdejavu.so"
# endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>      
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#elif HAVE_SYS_TIME_H
# include <sys/time.h>
#else
# include <time.h>
#endif
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#else
# include <wait.h>
#endif
#if !HAVE_WORKING_VFORK
# define vfork fork
#endif
#ifndef MAXPATHLEN
# ifdef _MAX_PATH
#  define MAXPATHLEN _MAX_PATH
# else
#  define MAXPATHLEN 1024
# endif
#endif
#if ( MAXPATHLEN < 1024 )
# undef MAXPATHLEN
# define MAXPATHLEN 1024
#endif

#define XP_UNIX 1
#define MOZ_X11 1
#include "npapi.h"
#include "npruntime.h"
#include "npupp.h"

#ifndef TRUE
# define TRUE 1
#endif
#ifndef FALSE
# define FALSE 0
#endif
#ifndef offsetof
# define offsetof(TYPE,MEMBER) ((size_t)&((TYPE*)0)->MEMBER)
#endif
#ifndef min
# define min(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef USE_XT
# if HAVE_XT
#  define USE_XT 1
# else
#  define USE_XT 0
# endif
#endif
#ifndef USE_GLIB
# if HAVE_GLIB
#  define USE_GLIB 1
# else
#  define USE_GLIB 0
# endif
#endif

#if USE_XT
# include <X11/Intrinsic.h>
# include <X11/IntrinsicP.h>
# include <X11/StringDefs.h>
# include <X11/Shell.h>
# pragma weak XtAddCallback
# pragma weak XtAddEventHandler
# pragma weak XtAppAddInput
# pragma weak XtRemoveCallback
# pragma weak XtRemoveEventHandler
# pragma weak XtRemoveInput
# pragma weak XtStrings
# pragma weak XtVaGetValues
# pragma weak XtWidgetToApplicationContext
# pragma weak XtWindowToWidget
#endif

#if USE_GLIB
# include <glib.h>
# pragma weak g_io_channel_unix_new
# pragma weak g_io_channel_unref
# pragma weak g_io_add_watch
# pragma weak g_source_remove
#endif




/* ------------------------------------------------------------ */
/* IO */

typedef struct
{
   int  cmd_mode;
   int  cmd_zoom;
   int  imgx;
   int  imgy;
} SavedData;


#define OK_STRING	"OK"
#define ERR_STRING	"ERR"

#define CMD_SHUTDOWN		0
#define CMD_NEW			1
#define CMD_DETACH_WINDOW	2
#define CMD_ATTACH_WINDOW	3
#define CMD_RESIZE		4
#define CMD_DESTROY		5
#define CMD_PRINT		6
#define CMD_NEW_STREAM		7
#define CMD_WRITE		8
#define CMD_DESTROY_STREAM	9
#define CMD_SHOW_STATUS		10
#define CMD_GET_URL		11
#define CMD_GET_URL_NOTIFY	12
#define CMD_URL_NOTIFY		13
#define CMD_HANDSHAKE		14
#define CMD_SET_DJVUOPT         15
#define CMD_GET_DJVUOPT         16
#define CMD_ON_CHANGE           17

#define TYPE_INTEGER	1
#define TYPE_DOUBLE	2
#define TYPE_STRING	3
#define TYPE_POINTER	4
#define TYPE_ARRAY	5

/* All these routines return
 *  - positive value on success
 *  - zero on end-of-file condition
 *  - negative on error
 */

static int
Read(int fd, void * buffer, int length,
     int refresh_pipe, void (* refresh_cb)(void))
{
  int size = length;
  int maxfd = (refresh_pipe>fd) ? refresh_pipe : fd;
  char *ptr = (char*)buffer;
  int res;
  int rc;

  while(size>0)
    {
      fd_set read_fds;
      struct timeval tv;
      FD_ZERO(&read_fds);
      FD_SET(fd, &read_fds);
      if (refresh_pipe>=0 && refresh_cb)
        FD_SET(refresh_pipe, &read_fds);
      tv.tv_sec=5;
      tv.tv_usec=0;
      rc = select(maxfd+1, &read_fds, 0, 0, &tv);
      if (rc>0 && FD_ISSET(fd, &read_fds))
        {
          errno = 0;
          res = read(fd, ptr, size);
          if (res<0 && errno==EINTR)
            continue;
          if (res<0) 
            return -1;
          if (res==0) 
            return 0;
          size -= res; 
          ptr += res;
        }
      if (rc<0 && errno!=EINTR)
        return -1;
      if (refresh_cb) 
        refresh_cb();
    }
  return length;
}

static int
Write(int fd, const void * buffer, int length)
{
  int size = length;
  const char *ptr = (const char*)buffer;
#if HAVE_SIGACTION
  sigset_t new_mask, old_mask;
  struct sigaction new_action, old_action;
#else
  void *oldhandler;
#endif
  int res;

#if HAVE_SIGACTION
  sigemptyset(&new_mask);
  sigaddset(&new_mask, SIGPIPE);
  sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
#else
  oldhandler = signal(SIGPIPE, SIG_IGN);
#endif
  while(size>0)
    {
      errno = 0;
      res = write(fd, ptr, size);
      if (res<0 && errno==EINTR) 
        continue;
      if (res <= 0)
        break;
      size-=res; 
      ptr+=res;
    }
#if HAVE_SIGACTION
  sigaction(SIGPIPE, 0, &new_action);
  new_action.sa_handler=SIG_IGN;
  new_action.sa_flags=SA_NODEFER;
  sigaction(SIGPIPE, &new_action, &old_action);
  sigprocmask(SIG_SETMASK, &old_mask, 0);
  sigaction(SIGPIPE, &old_action, 0);
#else
  signal(SIGPIPE, oldhandler);
#endif
  if (size > 0)
    return -1;
  return 0;
}

static int
WriteStringLen(int fd, const char *str, int length)
{
  int type = TYPE_STRING;
  if ( (Write(fd, &type, sizeof(type)) < 0) ||
       (Write(fd, &length, sizeof(length)) < 0) ||
       (Write(fd, str, length+1) < 0) )
    return -1;
  return 1;
}

static int
WriteString(int fd, const char *str)
{
  int length;
  if (! str) 
    str = "";
  length = strlen(str);
  return WriteStringLen(fd, str, length);
}

static int
WriteInteger(int fd, int var)
{
  int type=TYPE_INTEGER;
  if ( (Write(fd, &type, sizeof(type)) < 0) ||
       (Write(fd, &var, sizeof(var)) < 0) )
    return -1;
  return 1;
}

static int
WritePointer(int fd, const void *ptr)
{
  int type=TYPE_POINTER;
  if ( (Write(fd, &type, sizeof(type)) < 0) ||
       (Write(fd, &ptr, sizeof(ptr)) < 0) )
    return -1;
  return 1;
}

static int
WriteArray(int fd, int size, const char *array)
{
  int type=TYPE_ARRAY;
  if ( (Write(fd, &type, sizeof(type)) < 0) ||
       (Write(fd, &size, sizeof(size)) < 0) ||
       (Write(fd, array, size) < 0) )
    return -1;
  return 1;
}

static int
ReadString(int fd, char **pstr, int refresh_pipe, void (* refresh_cb)(void))
{
   char *ptr;
   int type, length, status;
   *pstr = 0;
   status = Read(fd, &type, sizeof(type), refresh_pipe, refresh_cb);
   if (status <= 0) 
     return status;
   if (type != TYPE_STRING)
     return -1;
   status = Read(fd, &length, sizeof(length), refresh_pipe, refresh_cb);
   if (status <= 0)
     return status;
   if (length < 0)
     return -1;
   if (! (ptr = malloc(length+1)))
     return -1;
   status = Read(fd, (char*)ptr, length+1, refresh_pipe, refresh_cb);
   if (status <= 0) {
     free(ptr);
     return status;
   }
   *pstr = ptr;
   return 1;
}

static int
ReadInteger(int fd, int *pvar, int refresh_pipe, void (* refresh_cb)(void))
{
  int type, status;
  status = Read(fd, &type, sizeof(type), refresh_pipe, refresh_cb);
  if (status <= 0) 
    return status;
  if (type != TYPE_INTEGER)
    return -1;
  status = Read(fd, pvar, sizeof(*pvar), refresh_pipe, refresh_cb);
  return status;
}

static int 
ReadPointer(int fd, void **pvar, int refresh_pipe, void (* refresh_cb)(void))
{
  int type, status;
  status = Read(fd, &type, sizeof(type), refresh_pipe, refresh_cb);
  if (status <= 0) 
    return status;
  if (type != TYPE_POINTER)
    return -1;
  status = Read(fd, pvar, sizeof(*pvar), refresh_pipe, refresh_cb);
  return status;
}

static int
ReadResult(int fd, int refresh_pipe, void (* refresh_cb)(void))
{
  char *res;
  int status;
  status = ReadString(fd, &res, refresh_pipe, refresh_cb);
  if (status <= 0)
    return status;
  status = +1;
  if (strcmp(res, OK_STRING))
    status = -2;
  free(res);
  return status;
}


/* ------------------------------------------------------------ */
/* PATHS */


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
  char *b = malloc(sizeof(struct strpool_data)+n);
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
      if (d) free(d);
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

static int
is_file(const char *filename)
{
  struct stat buf;
  if (stat(filename,&buf) >= 0)
    if (! (buf.st_mode & S_IFDIR))
      return TRUE;
  return FALSE;
}

static int
is_executable(const char *filename)
{
  if (!is_file(filename))
    return FALSE;
  if (access(filename, X_OK)<0)
    return FALSE;
  return TRUE;
}

static const char *
dirname(strpool *pool, const char *fname)
{
  int n;
  char *ret;
  const char *s = fname + strlen(fname);
  while (s>fname && s[-1]=='/') s--;
  while (s>fname && s[-1]!='/') s--;
  while (s>fname && s[-1]=='/') s--;
  if (s == fname)
    return ( (s[0] == '/') ? "/" : "." );
  n = s - fname;
  ret = strpool_alloc(pool, n);
  strncpy(ret, fname, n);
  return ret;
}

static const char *
pathclean(strpool *pool, const char *n)
{
  char *ret = strpool_alloc(pool, strlen(n));
  char *d = ret;
  int slash = FALSE;
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
          slash = FALSE;
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
      if (lnkbuf[0] != '/')
        ret = strconcat(pool, dirname(pool, ret), "/", lnkbuf, 0);
      else
        ret = lnkbuf;
      ret = pathclean(pool,ret);
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
"/usr/lib/mozilla/plugins:"
"/usr/local/lib/netscape/plugins:"
"/usr/local/netscape/plugins:"
"/usr/lib/netscape/plugins:"
"/opt/netscape/plugins";

static const char *
get_plugin_path(strpool *pool)
{
  const char *env;
  const char *dir;
  /* MOZ_PLUGIN_PATH */
  if ((env = getenv("MOZ_PLUGIN_PATH"))) {
    while ((dir = pathelem(pool, &env))) {
      const char *lib = strconcat(pool, dir, "/", LIBRARY_NAME, 0);
      if (is_file(lib))
        return lib;
    }
  }
  /* NPX_PLUGIN_PATH */
  if ((env = getenv("NPX_PLUGIN_PATH"))) {
    while ((dir = pathelem(pool, &env))) {
      const char *lib = strconcat(pool, dir, "/", LIBRARY_NAME, 0);
      if (is_file(lib))
        return lib;
    }
  }
  /* $HOME/.{mozilla,netscape}/plugins */
  if ((env = getenv("HOME"))) {
    const char *lib;
    lib = strconcat(pool, env, "/.mozilla/plugins/", LIBRARY_NAME, 0);
    if (is_file(lib))
      return lib;
    lib = strconcat(pool, env, "/.netscape/plugins/", LIBRARY_NAME, 0);
    if (is_file(lib))
      return lib;
  }
  /* MOZILLA_HOME */
  if ((env = getenv("MOZILLA_HOME"))) {
    const char *lib = strconcat(pool, env, "/plugins/", LIBRARY_NAME, 0);
    if (is_file(lib))
      return lib;
  }
  /* OTHER */
  env = stdpath;
  while ((dir = pathelem(pool, &env))) {
    const char *lib = strconcat(pool, dir, "/", LIBRARY_NAME, 0);
    if (is_file(lib))
      return lib;
  }
  return 0;
}

static const char *
GetPluginPath(void)
{
  static char path[MAXPATHLEN+1];
  if (! path[0]) 
    {
      const char *p;
      strpool apool;
      strpool_init(&apool);
      if ((p = get_plugin_path(&apool)))
        strncpy(path, p, MAXPATHLEN);
      path[MAXPATHLEN] = 0;
      strpool_fini(&apool);
    }
  return path;
}

static const char *
get_viewer_path(strpool *pool)
{
  int i;
  const char *env = 0;
  const char *envs = 0;
  static const char *djview[] = { DJVIEW_NAME, DJVIEW4_NAME, DJVIEW3_NAME, 0 };
  /* Environment variable NPX_DJVIEW overrides everything */
  if ((env = getenv("NPX_DJVIEW")))
    if (is_executable(env))
      return env;
  /* Locate plugin path */
  if ((env = GetPluginPath()))
    envs = follow_symlinks(pool, env);
  /* Try the following names */
  for (i=0; djview[i]; i++)
    {
      const char *dir;
      const char *test;
      if (envs) {
        /* Try relative to plugin path */
        dir = dirname(pool, envs);
        test = strconcat(pool, dir, "/../../../bin/", djview[i], 0);
        if (is_executable(test))
          return test;
        test = strconcat(pool, dir, "/../../bin/", djview[i], 0);
        if (is_executable(test))
          return test;
        dir = dirname(pool, env);
        test = strconcat(pool, dir, "/../DjVu/", djview[i],0);
        if (is_executable(test))
          return test;
        dir = dirname(pool, envs);
        test = strconcat(pool, dir, "/../DjVu/", djview[i],0);
        if (is_executable(test))
          return test;
      }
      /* Try ${bindir} */
#if defined(DIR_BINDIR)
      test = strconcat(pool,DIR_BINDIR,"/",djview[i],0);
      if (is_executable(test))
        return test;
#endif
      /* Try in the shell path */
      if ((env = getenv("PATH")))
        while ((dir = pathelem(pool, &env))) {
          test = strconcat(pool, dir, "/", djview[i], 0);
          if (is_executable(test))
            return test;
        }
    }
  /* Deep trouble */
  return 0;
}

static const char *
GetViewerPath(void)
{
  static char path[MAXPATHLEN+1];
  if (! path[0]) 
    {
      const char *p;
      strpool apool;
      strpool_init(&apool);
      if ((p = get_viewer_path(&apool)))
        strncpy(path, p, MAXPATHLEN);
      path[MAXPATHLEN] = 0;
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
    env = strconcat(pool, env, "/../DjVu", 0);
    env = pathclean(pool, env);
    return env;
  }
  return 0;
}

static const char *
GetLibraryPath(void)
{
  /* This is no longer needed since djview no longer uses the provided path.
     This is kept to allow interoperability between nsdejavu.so and previous
     version of djview. LYB. */
  static char path[MAXPATHLEN+1];
  if (! path[0]) 
    {
      const char *p;
      strpool apool;
      strpool_init(&apool);
      if ((p = get_library_path(&apool)))
        strncpy(path, p, MAXPATHLEN);
      path[MAXPATHLEN] = 0;
      strpool_fini(&apool);
    }
  return path;
}


/* ------------------------------------------------------------ */
/* MAP */

struct map_entry_s {
  struct map_entry_s *next;
  void *key;
  void *val;
};

typedef struct {
  int nelems;
  int nbuckets;
  struct map_entry_s **buckets;
} Map;

static int
hash(void *v, int nbuckets)
{
  return  (((long)v ^ ((long)v>>7)) % nbuckets);
}

static void
map_purge(Map *m)
{
  int i;
  struct map_entry_s *p;
  if (m->buckets) {
    for (i=0; i<m->nbuckets; i++)
      while ((p = m->buckets[i])) {
        m->buckets[i] = p->next;
        free(p);
      }
    free(m->buckets);
  }
  m->buckets = 0;
  m->nbuckets = 0;
  m->nelems = 0;
}

static void
map_reorganize(Map *m)
{
  int new_nbuckets = 17;
  struct map_entry_s **new_buckets = 0;
  struct map_entry_s *p;
  int i;
  if (m->nbuckets > new_nbuckets)
    new_nbuckets = m->nbuckets * 2 - 1;
  new_buckets = malloc(new_nbuckets * sizeof(struct map_entry*));
  if (!new_buckets) return;
  memset(new_buckets, 0, new_nbuckets * sizeof(struct map_entry*));
  for (i=0; i<m->nbuckets; i++)
    while ((p = m->buckets[i]))
      {
        int h = hash(p->key, new_nbuckets);
        m->buckets[i] = p->next;
        p->next = new_buckets[h];
        new_buckets[h] = p;
      }
  if (m->nbuckets)
    free(m->buckets);
  m->buckets = new_buckets;
  m->nbuckets = new_nbuckets;
}

static void*
map_lookup(Map *m, void *key)
{
  int h;
  struct map_entry_s *q;
  if (m->nbuckets) {
    h = hash(key, m->nbuckets);
    for (q=m->buckets[h]; q; q=q->next)
      if (q->key == key)
        return q->val;
  }
  return NULL;
}

static int
map_remove(Map *m , void *key)
{
  int h;
  struct map_entry_s **pq;
  if (m->nbuckets) {
    h = hash(key, m->nbuckets);
    for (pq=&(m->buckets[h]); *pq; pq=&((*pq)->next))
      if ((*pq)->key == key) {
        struct map_entry_s *q = *pq;
        (*pq) = q->next;
        free(q);
        return 1;
      }
  }
  return 0;
}
  
static int
map_insert(Map *m, void *key, void *val)
{
  int h;
  struct map_entry_s *q;
  if (m->nelems * 3 >= m->nbuckets * 2)
    map_reorganize(m);
  if (! m->nbuckets)
    return -1;
  if (! val)
    return map_remove(m, key);
  h = hash(key, m->nbuckets);
  for (q=m->buckets[h]; q; q=q->next)
    if (q->key == key) {
      q->val = val;
      return 1;
    }
  if (! (q = malloc(sizeof(struct map_entry_s))))
    return -1;
  q->next = m->buckets[h];
  q->key = key;
  q->val = val;
  m->buckets[h] = q;
  return 1;
}



/* ------------------------------------------------------------ */
/* INSTANCE */


typedef struct {
  Window	window;
  NPP		np_instance;
  int		full_mode;
  int           xembed_mode;
#if USE_XT
  Window        client;
  Widget	widget;
#endif
  NPObject     *npobject;
  NPVariant     onchange;
} Instance;


static Instance *
instance_new(NPP np_instance, int full_mode)  
{
  Instance *this = malloc(sizeof(Instance));
  if (this) 
    {
      memset(this, 0, sizeof(Instance));
      this->np_instance = np_instance;
      this->full_mode = full_mode;
      this->xembed_mode = 0;
      this->npobject = 0;
      VOID_TO_NPVARIANT(this->onchange);
    }
  return this;
}

static void
instance_free(Instance *this)
{
  if (this)
    {
      NPN_ReleaseVariantValue(&this->onchange);
      memset(this, 0, sizeof(Instance));
      free(this);
    }
}



/* ------------------------------------------------------------ */
/* DELAYED REQUESTS */

typedef struct {
  struct DelayedRequest_s *first;
  struct DelayedRequest_s *last;
} DelayedRequestList;

typedef struct DelayedRequest_s {
  struct DelayedRequest_s *next;
  int req_num;
  void *id;
  char *status;
  char *url;
  char *target;
} DelayedRequest;

static DelayedRequest *
delayedrequest_append(DelayedRequestList *dlist)
{
  DelayedRequest *ptr = malloc(sizeof(DelayedRequest));
  if (ptr) 
    {
      memset(ptr, 0, sizeof(DelayedRequest));
      if (!dlist->first) 
        dlist->first = ptr;
      if (dlist->last)
        dlist->last->next = ptr;
      dlist->last = ptr;
    }
  return ptr;
}

static DelayedRequest *
delayedrequest_pop(DelayedRequestList *dlist)
{
  DelayedRequest *ptr = dlist->first;
  if (ptr)
    {
      dlist->first = ptr->next;
      if (dlist->last == ptr)
        dlist->last = 0;
      ptr->next = 0;
    }
  return ptr;
}

static void
delayedrequest_free(DelayedRequest *ptr)
{
  if (ptr->status)
    free(ptr->status);
  if (ptr->url)
    free(ptr->url);
  if (ptr->target)
    free(ptr->target);
  free(ptr);
}

static void
delayedrequest_purge(DelayedRequestList *dlist)
{
  DelayedRequest *reqp;
  while ((reqp = delayedrequest_pop(dlist)))
    delayedrequest_free(reqp);
}


/*******************************************************************************
******************************* Preserved Data  ********************************
*******************************************************************************/



#define ENV_DJVU_STORAGE_PTR	"_DJVU_STORAGE_PTR"

/* The plugin can be freely loaded/unloaded by Netscape, 
 * so any static variable is at risk of being destroyed at any time
 * after NPP_Shutdown() is called. */

/* -- These variables need to be destroyed and reinitialized
 *    when the plugin is reloaded. 
 */
static int			delay_pipe[2];
static Map                      instance, strinstance;
static DelayedRequestList	delayed_requests;
static NPIdentifier             npid_getdjvuopt;
static NPIdentifier             npid_setdjvuopt;
static NPIdentifier             npid_onchange;
static NPIdentifier             npid_version;
#if USE_XT
static XtInputId		input_id, delay_id;
#endif
#if USE_GLIB
static gint                     input_gid, delay_gid;
#endif

/* -- These variables need to be saved and restored
 *    when the plugin is reloaded. 
 */
static int		pipe_read = -1;
static int              pipe_write = -1;
static int              rev_pipe = -1;
static int              scriptable = 0;
static int              xembedable = 0;
static unsigned long	white, black;
static Colormap         colormap;

typedef struct
{
  int pipe_read, pipe_write, rev_pipe;
  int scriptable, xembedable;
  unsigned long white, black;
  Colormap colormap;
}  SavedStatic;

static void
SaveStatic(void)
     /* Saves static variables from Group #3 into the Netscape's
        environment. Next time nsdejavu.so is loaded, we will
        read their values again. */
{
  SavedStatic *storage = 0;
  char *value = getenv(ENV_DJVU_STORAGE_PTR);
  if (value) 
    sscanf(value, "%p", &storage);
  if (! storage)
    {
      char *buffer = malloc(128);
      if (buffer) {
        storage = malloc(sizeof(SavedStatic));
        if (storage) {
          sprintf(buffer, ENV_DJVU_STORAGE_PTR "=%p", (void*)storage);
          putenv(buffer);
        }
      }
    }
  if (storage)
    {
      storage->pipe_read = pipe_read;
      storage->pipe_write = pipe_write;
      storage->rev_pipe = rev_pipe;
      storage->scriptable = scriptable;
      storage->xembedable = xembedable;
      storage->white = white;
      storage->black = black;
      storage->colormap = colormap;
    }
}

static void
LoadStatic(void)
     /* Loads static variables, which have been saved into the 
        environment via SaveStatic() by the previous instance of 
        nsdejavu.so */
{
  SavedStatic *storage = 0;
  char *value = getenv(ENV_DJVU_STORAGE_PTR);
  if (value) 
    sscanf(value, "%p", &storage);
  if (storage)
    {
      pipe_read = storage->pipe_read;
      pipe_write = storage->pipe_write;
      rev_pipe = storage->rev_pipe;
      scriptable = storage->scriptable;
      xembedable = storage->xembedable;
      white = storage->white;
      black = storage->black;
      colormap = storage->colormap;
    } 
}



/*******************************************************************************
 ********************************** Callbacks **********************************
 *******************************************************************************/

static int  Detach(void * id);
static int  Resize(void * id);
static void CloseConnection(void);
static int  IsConnectionOK(int);
static void ProgramDied(void);
static int  StartProgram(void);


/******* Pipe functions ********/

static void
process_delayed_requests(void)
{
  char ch;
  DelayedRequest *reqp;
  Instance *inst;
  read(delay_pipe[0], &ch, 1);
  while((reqp = delayedrequest_pop(&delayed_requests)))
    {
      switch(reqp->req_num)
        {
        case CMD_SHOW_STATUS:
          if ((inst = map_lookup(&instance, reqp->id)))
            if (inst->window) 
              NPN_Status(inst->np_instance, reqp->status);
          break;
        case CMD_GET_URL:
          if ((inst = map_lookup(&instance, reqp->id)))
            {
              const char *target = (reqp->target && reqp->target[0]) 
                ? reqp->target : 0;
              NPN_GetURL(inst->np_instance, reqp->url, target);
            }
          break;
        case CMD_GET_URL_NOTIFY:
          if ((inst = map_lookup(&instance, reqp->id)))
            {
              const char *target = (reqp->target && reqp->target[0]) 
                ? reqp->target : 0;
              if (NPN_GetURLNotify(inst->np_instance, reqp->url, target, 0)
                  != NPERR_NO_ERROR )
                NPN_GetURL(inst->np_instance, reqp->url, target);
            }
          break;
        case CMD_ON_CHANGE:
          if ((inst = map_lookup(&instance, reqp->id)) 
              && NPVARIANT_IS_STRING(inst->onchange) )
            {
              NPString *code = &NPVARIANT_TO_STRING(inst->onchange);
              NPP npp = inst->np_instance;
              NPVariant res;
              VOID_TO_NPVARIANT(res);
              NPN_Evaluate(npp, inst->npobject, code, &res);
              NPN_ReleaseVariantValue(&res);
            }
          break;
        default:
          break;
        }
      delayedrequest_free(reqp);
    }
}

static void
process_requests(void)
{
  if (!IsConnectionOK(FALSE))
    {
    problem:
      ProgramDied();
      return;
    }
  for(;;)
    {
      struct timeval tv; 
      fd_set read_fds;
      int req_num;
      if (ReadInteger(rev_pipe,&req_num,0,0) <= 0)
        goto problem;
      switch(req_num)
        {
        case CMD_SHOW_STATUS:
          {
            DelayedRequest *reqp = delayedrequest_append(&delayed_requests);
            if (!reqp) return;
            reqp->req_num = req_num;
            if ( (ReadPointer(rev_pipe,&reqp->id,0,0) <= 0) ||
                 (ReadString(rev_pipe,&reqp->status,0,0) <= 0) ) 
              goto problem;
            write(delay_pipe[1], "1", 1);
          }
          break;
        case CMD_GET_URL:
        case CMD_GET_URL_NOTIFY:
          {
            DelayedRequest *reqp = delayedrequest_append(&delayed_requests);
            if (!reqp) return;
            reqp->req_num = req_num;
            if ( (ReadPointer(rev_pipe,&reqp->id,0,0) <= 0) ||
                 (ReadString(rev_pipe,&reqp->url,0,0) <= 0) ||
                 (ReadString(rev_pipe,&reqp->target,0,0) <= 0) )
              goto problem;
            write(delay_pipe[1], "1", 1);
          }
          break;
        case CMD_ON_CHANGE:
          {
            DelayedRequest *reqp = delayedrequest_append(&delayed_requests);
            if (!reqp) return;
            reqp->req_num = req_num;
            if ( (ReadPointer(rev_pipe,&reqp->id,0,0) <= 0) )
              goto problem;
            write(delay_pipe[1], "1", 1);
          }
        default:
          break;
        }
      /* Process as many requests as possible before returning control
         back to Netscape. This pipe tends to overflow, so we don't
         want data to stay there too long. */
      FD_ZERO(&read_fds);
      FD_SET(rev_pipe, &read_fds);
      tv.tv_sec=0; 
      tv.tv_usec=0;
      if (select(rev_pipe+1, &read_fds, 0, 0, &tv) != 1)
        break;
      if (!FD_ISSET(rev_pipe, &read_fds))
        break;
    }
}

static void
check_requests(void)
{
  if (rev_pipe)
    {
      fd_set read_fds;
      struct timeval tv;
      int rc;
      FD_ZERO(&read_fds);
      FD_SET(rev_pipe, &read_fds);
      tv.tv_sec = 0;
      tv.tv_usec = 0;
      rc = select(rev_pipe+1, &read_fds, 0, 0, &tv);
      if (rc>0)
        process_requests();
    }
}




/******* Xt Callbacks ********/

#if USE_XT

static XtInputId
xt_add_input_fd(XtAppContext app, int fd, XtInputCallbackProc proc)
{
  return XtAppAddInput(app, fd, (XtPointer) XtInputReadMask, proc, 0);
}

static void
Delay_cb(XtPointer ptr, int * fd, XtInputId *xid)
{
  process_delayed_requests();
}

static void
Input_cb(XtPointer ptr, int * fd, XtInputId *xid)
{
  process_requests();
}

static void
Destroy_cb(Widget w, XtPointer cl_data, XtPointer ptr)
{
  Detach((void*)cl_data);
}

static void
Resize_hnd(Widget w, XtPointer cl_data, XEvent * event, Boolean * cont)
{
  /* This function is necessary because Netscape 
     sometimes doesn't resize the drawing area. */
  *cont = True;
  if (event->type == ConfigureNotify)	/* There can be a GravityNotify too */
    {
      Instance *inst;
      void *id = (void*)cl_data;
      if ((inst = map_lookup(&instance, id)))
        if (Resize(id) <= 0)
          ProgramDied();
    }
}

static const long event_mask = 
  (KeyPressMask|KeyReleaseMask|SubstructureNotifyMask);

static void
Simulate_focus(Display *dpy, Window client, int yes)
{
  XEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.xfocus.display = dpy;
  ev.xfocus.type = (yes ? FocusIn : FocusOut);
  ev.xfocus.window = client;
  ev.xfocus.mode = NotifyNormal;
  ev.xfocus.detail = NotifyPointer;
  XSendEvent(dpy, client, False, NoEventMask, &ev);
}

static void
Event_hnd(Widget w, XtPointer cl_data, XEvent * event, Boolean * cont)
{
  Instance *inst;
  void *id = (void*)cl_data;
  *cont = True;
  if ((inst = map_lookup(&instance, id)))
    {
      Widget   wid = inst->widget;
      Display *dpy = XtDisplay(wid);
      Window   win = XtWindow(wid);
      XEvent    ev = *event;
      switch(event->type)
        {
        case KeyPress:
          if ((ev.xkey.window = inst->client))
            XSendEvent(dpy, inst->client, False, KeyPressMask, &ev );
          break;
        case KeyRelease:
          if ((ev.xkey.window = inst->client))
            XSendEvent(dpy, inst->client, False, KeyReleaseMask, &ev );
          break;
        case DestroyNotify:
          if (ev.xdestroywindow.window == inst->client)
            inst->client = 0;
          break;
        case ReparentNotify:
          if (ev.xreparent.window == inst->client &&
              ev.xreparent.parent != win)
            inst->client = 0;
          else if (ev.xreparent.parent == win)
            if ((inst->client = ev.xreparent.window))
              Simulate_focus(dpy, inst->client, 1);
          break;
        }
    }
}

#endif

/******* Gtk Callbacks ********/

#if USE_GLIB

static gint
g_source_add_input_fd(int fd, GIOFunc func)
{
  GIOCondition cond = G_IO_IN | G_IO_PRI | G_IO_ERR | G_IO_HUP;
  GIOChannel *channel = g_io_channel_unix_new(fd);
  gint result = g_io_add_watch(channel, cond, func, 0);
  g_io_channel_unref(channel);
  return result;
}

static gboolean
Delay_gcb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  process_delayed_requests();
  return TRUE;
}

static gboolean
Input_gcb(GIOChannel *source, GIOCondition condition, gpointer data)
{
  process_requests();
  return TRUE;
}

#endif




/*******************************************************************************
 ***************************** Copying the colormap *****************************
*******************************************************************************/

/* This is no longer needed since djview no longer uses the provided colormap.
   This is kept to allow interoperability between nsdejavu.so and previous
   version of djview. LYB. */


static void
CopyColormap(Display *displ, Visual *visual, Screen *screen, Colormap cmap)
{
  unsigned long *pixels = 0;
  XColor *colors = 0;
  int n;

  if (colormap)
    return;
  
  if (cmap == DefaultColormapOfScreen(screen))
    {
      /* This is also what we do should a problem occur */
    problem:
      if (pixels)
        free(pixels);
      if (colors)
        free(colors);
      colormap = cmap;
      return;
    }
  
  /* Depends on the visual class */
  switch(visual->class)
    {
    case StaticGray:
    case StaticColor:
    case TrueColor:
      /* Colormap for which there is nothing to copy */
      colormap = XCreateColormap(displ, RootWindowOfScreen(screen), 
                                 visual, AllocNone);
      break;

    case DirectColor:
      /* Use a simple ramp. QT does not like these anyway. */
      colormap = XCreateColormap(displ, RootWindowOfScreen(screen), 
                                 visual, AllocNone);
      for (n=0; n<visual->map_entries; n++)
        {
          XColor cell;
          cell.pixel = n;
          cell.red = cell.green = cell.blue = (n * 0x10000)/visual->map_entries;
          cell.flags = DoRed | DoGreen | DoBlue;
          XAllocColor(displ, colormap, &cell);
        }
      break;

    case PseudoColor:
    case GrayScale:
      /* The complicated case */
      {
        /* Make sure the colormap is rich enough.
           This happens to be pretty useless code, btw, as:
             1. Netscape allocates the colorcube
             2. It allocates it as RW, basically leaving no space 
                in the 8bpp colormap that we could use.
           But we will give it a try anyway...
        */
        int i,j,k;
        int again = TRUE;
        static unsigned short r1[] 
          = { 0x0000, 0x9999, 0xffff };
        static unsigned short r2[] 
          = { 0x0000, 0x3333, 0x6666, 0x9999, 0xcccc, 0xffff };
	/* Allocate 3x3x3 cube first. It's a good idea to allocate the
           most distinct colors first (from 3x3x3) and then proceed
           with other colors from 6x6x6. Should the colormap be nearly
           full, we will still get a reasonable approximation. */
        for(i=0; again && i<3; i++)
          for(j=0; again && j<3; j++)
            for(k=0; again && k<3; k++)
              {
                XColor cell;
                cell.red = r1[i];
                cell.green = r1[j];
                cell.blue = r1[k];
                cell.flags = DoRed | DoGreen | DoBlue;
                if (!XAllocColor(displ, cmap, &cell))
                  again = FALSE;
              }
	/* Proceed with additional colors of 6x6x6 cube. */
        for(i=0; again && i<6; i++)
          for(j=0; again && j<6; j++)
            for(k=0; again && k<6; k++)
              {
                XColor cell;
                cell.red = r2[i];
                cell.green = r2[j];
                cell.blue = r2[k];
                cell.flags = DoRed | DoGreen | DoBlue;
                if (!XAllocColor(displ, cmap, &cell))
                  again = FALSE;
              }
        /* Now copy the colormap */
        colormap = XCreateColormap(displ, RootWindowOfScreen(screen), 
                                   visual, AllocNone);
        pixels = malloc(visual->map_entries * sizeof(unsigned long));
        if (!pixels) 
          goto problem;
        XAllocColorCells(displ, colormap, False, pixels, 0, 
                         pixels, visual->map_entries);
        colors = malloc(visual->map_entries * sizeof(XColor));
        if (!colors) 
          goto problem;
        for (n=0; n<visual->map_entries; n++)
          {
            colors[n].pixel = n;
            colors[n].flags = DoRed | DoGreen | DoBlue;	/* This can be dropped */
          }
        XQueryColors(displ, cmap, colors, visual->map_entries);
        XStoreColors(displ, colormap, colors, visual->map_entries);
        /* But make the desired ones read-only (so that QT can use them) */
        for (n=0; n<visual->map_entries; n++)
          {
            /* Free one cell exactly (all others are read/write) */
	    XColor cell = colors[n];
            XFreeColors(displ, colormap, &cell.pixel, 1, 0);
            if (!XAllocColor(displ, colormap, &cell))
              goto problem;
            if (cell.pixel != colors[n].pixel )
              {
                cell.pixel = colors[n].pixel;
                XAllocColorCells(displ, colormap, False, 0, 0, &cell.pixel, 1);
                XStoreColor(displ, colormap, &cell);
              }
          }
        /* Calling XInstallColormap is needed on SGIs to ``finish-up'' the new
           colormap.  This should be ok since it contains the same colors as
           Netscape's and we can expect that the currently installed colormap
           is Netscape's.  Otherwise the screen may flash. */
        XSync(displ, False);
        XInstallColormap(displ, colormap);
        /* Cleanup */
        if (colors)
          free(colors);
        if (pixels)
          free(pixels);
      }
      break;
      
    default:
      goto problem;
    }
}

/*******************************************************************************
************************************* Utilities ********************************
*******************************************************************************/

static int
IsConnectionOK(int handshake)
{
  if (pipe_read<=0 || pipe_write<=0 || rev_pipe<=0)
    return FALSE;
  if (handshake)
    {
      if ( (WriteInteger(pipe_write, CMD_HANDSHAKE) <= 0) ||
           (ReadResult(pipe_read, rev_pipe, check_requests) <= 0) )
        return FALSE;
    }
  return TRUE;
}

static void
ProgramDied(void)
{
  CloseConnection();
  /** Do not clear instances here as you used to!
      Netscape will probably call NPP_Destroy() later, and
      you will need to unregister handlers (such as Resize_hnd) */
  StartProgram();
}

static void
CloseConnection(void)
{
  /* Close all connections to the viewer */
#if USE_XT
  if (input_id) 
    XtRemoveInput(input_id); 
  input_id=0;
#endif
#if USE_GLIB
  if (input_gid)
    g_source_remove(input_gid);
  input_gid=0;
#endif
  if (pipe_read>0) 
    close(pipe_read); 
  pipe_read=-1;
  if (pipe_write>0) 
    close(pipe_write); 
  pipe_write=-1;
  if (rev_pipe>0) 
    close(rev_pipe); 
  rev_pipe=-1;
  /* Update environment */
  SaveStatic();
}

static int
Resize(void * id)
{
  /* Instead of selecting ConfigureEvent in the application I catch
     resizeCallback here and send the appropriate request to the
     application */
  Instance *inst;
  if (! (inst = map_lookup(&instance, id)))
    return 1;
  if (inst->xembed_mode)
    return 1;
#if USE_XT
  else if (inst->widget && !inst->xembed_mode)
    {
      Dimension width, height;
      XtVaGetValues(inst->widget, XtNwidth, &width, XtNheight, &height, NULL);
      if (! IsConnectionOK(TRUE))
        return -1;
      if ( (WriteInteger(pipe_write, CMD_RESIZE) <= 0) ||
           (WritePointer(pipe_write, id) <= 0) ||
           (WriteInteger(pipe_write, width) <= 0) ||
           (WriteInteger(pipe_write, height) <= 0) ||
           (ReadResult(pipe_read, rev_pipe, check_requests) <= 0) )
        return -1;
    }
#endif
  return 1;
}

static int
Detach(void * id)
{
  Instance *inst;
  if (! (inst = map_lookup(&instance, id)))
    return 1;
  if (inst->window)
    {
#if USE_XT
      if (inst->widget && !inst->xembed_mode)
        {
          XtRemoveCallback(inst->widget, XtNdestroyCallback, Destroy_cb, id);
          XtRemoveEventHandler(inst->widget, event_mask,
                               False, Event_hnd, id);
          XtRemoveEventHandler(inst->widget, StructureNotifyMask,
                               False, Resize_hnd, id);
          inst->widget = 0;
          inst->client = 0;
        }
#endif
      inst->window = 0;
      if (! IsConnectionOK(TRUE))
        return -1;
      if ( (WriteInteger(pipe_write, CMD_DETACH_WINDOW) <= 0) ||
           (WritePointer(pipe_write, id) <= 0) ||
           (ReadResult(pipe_read, rev_pipe, check_requests) <= 0) )
        return -1;
    }
  return 1;
}

static int
Attach(Display * displ, Window window, void * id)
{
  char *displ_str;
  Instance *inst;
  Screen *screen;
  Colormap cmap;
  Visual *visual;
  char protocol_str[128]; 
  XWindowAttributes attributes;
  int width, height;
#if USE_XT
  XtAppContext app_context = 0;   
  Widget widget = 0;
#endif
  
  XSync(displ, False);
  if (!(inst = map_lookup(&instance, id)))
    return 1;

#if USE_XT
  if (!inst->xembed_mode)
    {
      widget = XtWindowToWidget(displ, window);
      app_context = XtWidgetToApplicationContext(widget);
      XtAddCallback(widget, XtNdestroyCallback, Destroy_cb, id);
      XtAddEventHandler(widget, event_mask, False, Event_hnd, id);
      XtAddEventHandler(widget, StructureNotifyMask, False, Resize_hnd, id);
      if (! input_id)
        input_id = xt_add_input_fd(app_context, rev_pipe, Input_cb);
      if (! delay_id)
        delay_id = xt_add_input_fd(app_context, delay_pipe[0], Delay_cb);
    }
#endif
#if USE_GLIB
  if (inst->xembed_mode)
    {
      if (! input_gid)
        input_gid = g_source_add_input_fd(rev_pipe, Input_gcb);
      if (! delay_gid)
        delay_gid = g_source_add_input_fd(delay_pipe[0], Delay_gcb);
    }
#endif
  
  /* Preparing CMD_ATTACH_WINDOW attributes */
  displ_str = DisplayString(displ);
  if (!displ_str) 
    displ_str=getenv("DISPLAY");
  if (!displ_str) 
    displ_str=":0";
  
  if (XGetWindowAttributes(displ, window, &attributes))
    {
      width = attributes.width;
      height = attributes.height;
      screen = attributes.screen;
      cmap = attributes.colormap;
      visual = attributes.visual;
      if (!colormap)
        {
          /* Allocating black and white colors */
          XColor white_screen, white_exact;
          XColor black_screen, black_exact;
          XAllocNamedColor(displ, cmap, "white", &white_screen, &white_exact);
          white = white_screen.pixel;
          XAllocNamedColor(displ, cmap, "black", &black_screen, &black_exact);
          black = black_screen.pixel;
          CopyColormap(displ, visual, screen, cmap);
        }
      /* Prepare protocol string */
      protocol_str[0]=0;
      if (inst->xembed_mode)
        strcpy(protocol_str, "XEMBED");
#if USE_XT
      else
        {
          /* Old viewers want the background color name instead. */
          unsigned long back_color;
          XColor cell;
          XtVaGetValues(widget, XtNbackground, &back_color, NULL);
          cell.flags = DoRed | DoGreen | DoBlue;
          cell.pixel = back_color;
          XQueryColor(displ, cmap, &cell);
          sprintf(protocol_str, "rgb:%X/%X/%X", cell.red, cell.green, cell.blue);
        }
      if (widget && !inst->xembed_mode)
        XtMapWidget(widget);
#endif
      XSync(displ, False);
      
      /* Send attach command */
      if ( (WriteInteger(pipe_write, CMD_ATTACH_WINDOW) > 0) &&
           (WritePointer(pipe_write, id) > 0) &&
           (WriteString(pipe_write,  displ_str) > 0) &&
           (WriteString(pipe_write,  protocol_str) > 0) &&
           (WriteInteger(pipe_write, window) > 0) &&
           (WriteInteger(pipe_write, colormap) > 0) &&
           (WriteInteger(pipe_write, XVisualIDFromVisual(visual)) > 0) &&
           (WriteInteger(pipe_write, width) > 0) &&
           (WriteInteger(pipe_write, height) > 0) &&
           (ReadResult(pipe_read, rev_pipe, check_requests) > 0) )
        {
          /* Success */ 
          inst->window = window; 
#if USE_XT
          inst->widget = widget; 
          inst->client = 0;
#endif
          return 1;
        }
    }
  /* Failure */
#if USE_XT
  if (widget)
    {
      XtRemoveCallback(widget, XtNdestroyCallback, Destroy_cb, id);
      XtRemoveEventHandler(widget, event_mask, False, Event_hnd, id);
      XtRemoveEventHandler(widget, StructureNotifyMask, False, Resize_hnd, id);
    }
  inst->widget = 0;
  inst->client = 0;
#endif
  inst->window = 0;
  return -1;
}


static void
UnsetVariable(const char *var)
{
  const char *ptr = getenv(var);
  if (ptr && ptr[0])
    {
      /* Putenv() does not work on RH5.1.
         But Unsetenv() is available. */
#ifdef __linux__
      unsetenv(var);
#else
      char *env = malloc(strlen(var)+2);
      strcpy(env, var);
      strcat(env, "=");
      putenv(env);
#endif
    }
}

static int
StartProgram(void)
{
  const char *path;
  int fd[2];
  int _pipe_read;
  int _pipe_write;
  int _rev_pipe;
  void *sigsave;
  pid_t pid;
  char *ptr, *p, *q;
  struct stat st;
  int s;
  
  if (IsConnectionOK(TRUE)) 
    return 0;
  
  path = GetViewerPath();
  if (!path || !path[0])
    return -1;
  if (pipe(fd)<0) 
    return -1;
  pipe_read = fd[0];
  _pipe_write = fd[1];
  if (pipe(fd)<0) 
    return -1;
  pipe_write = fd[1];
  _pipe_read = fd[0];
  
  if (pipe(fd)<0)
    return -1;
  rev_pipe = fd[0];
  _rev_pipe = fd[1];

  /* We want to wait for this child. */
  sigsave = (void*)signal(SIGCHLD,SIG_DFL);
  pid  = vfork();
  if (pid < 0)
    return -1;
  if (! pid)
    {
      /* These three lines look crazy, but the this is the only way I know
	 to orphan a child under all versions of Unix.  Otherwise the
	 SIGCHLD may cause Netscape to crash. */
#ifdef NDEBUG
      setsid();
#endif
      signal(SIGCHLD,SIG_IGN);
      if (vfork()) 
        _exit(0);
      /* Real Child */
      close(pipe_read);
      close(pipe_write);
      close(rev_pipe);
      close(3); dup(_pipe_read); close(_pipe_read);
      close(4); dup(_pipe_write); close(_pipe_write);
      close(5); dup(_rev_pipe); close(_rev_pipe);
      /* Duplication above will guarantee, 
         that the new file descriptors will not be closed on exec.
         Now close all file descriptors which we don't use. For some reasons
         we will inherit some from Netscape if we don't close them. */
      for(s=8;s<1024;s++) 
        close(s);
      
      /* This is needed for RedHat's version of Netscape. */
      UnsetVariable("LD_PRELOAD");
      UnsetVariable("XNLSPATH");
      /* This is needed to disable session management in Qt */
      UnsetVariable("SESSION_MANAGER");      
      
      /* Old autoinstaller fails to set the "executable" flag. */
      if (stat(path, &st)>=0)
        {
          mode_t mode=st.st_mode;
          if (mode & S_IRUSR) mode|=S_IXUSR;
          if (mode & S_IRGRP) mode|=S_IXGRP;
          if (mode & S_IROTH) mode|=S_IXOTH;
          chmod(path, mode);
        }
      execl(path, path, "-netscape", NULL);
      fprintf(stderr,"nsdejavu: failed to execute %s\n", path);
      fflush(stderr);
      _exit(1);
    }
  
  /* Parent */
  close(_pipe_write);
  close(_pipe_read);
  close(_rev_pipe);
  
  /* Wait for the primary child */
  waitpid(pid, &s, 0);
  signal(SIGCHLD, sigsave);
  if (ReadString(pipe_read, &ptr, 0, 0) <= 0)
    {
      CloseConnection();
      return -1;
    }
  scriptable = 0;
  xembedable = 0;
  for (p=ptr; *p; p++)
    {
      if (isspace(*p))
        continue;
      for (q=p; *q; q++)
        if (isspace(*q))
          break;
      s = *q;
      *q = 0;
      if (!strcmp(p, "XEMBED"))
        xembedable = 1;
      if (!strcmp(p, "SCRIPT"))
        scriptable = 1;
      *q = s;
      p = q;
    }
  free(ptr);
  return 1;
}

/*******************************************************************************
***************************** NPRuntime interface ******************************
*******************************************************************************/

static void
npvariantcpy(NPVariant *to, const NPVariant *from)
{
  if (NPVARIANT_IS_OBJECT(*from))
    {
      NPObject *object = NPVARIANT_TO_OBJECT(*from);
      NPN_RetainObject(object);
      OBJECT_TO_NPVARIANT(object, *to);
      return;
    }
  if (NPVARIANT_IS_STRING(*from))
    {
      const NPString *s = &NPVARIANT_TO_STRING(*from);
      char *nstr = NPN_MemAlloc(s->utf8length+1);
      VOID_TO_NPVARIANT(*to);
      if (nstr)
        {
          memcpy(nstr, s->utf8characters, s->utf8length);
          nstr[s->utf8length] = 0;
          STRINGZ_TO_NPVARIANT(nstr, *to);
        }
      return;
    }
  *to = *from;
}

typedef struct FatNPObject {
  NPObject obj;
  NPP npp;
} FatNPObject;

static NPObject *
np_allocate(NPP npp, NPClass *npclass)
{
  FatNPObject *npobj = malloc(sizeof(FatNPObject));
  if (npobj)
    {
      memset(npobj, 0, sizeof(FatNPObject));
      npobj->obj._class = npclass;
      npobj->obj.referenceCount = 1;
      npobj->npp = npp;
    }
  return (NPObject*)npobj;
}

static void
np_deallocate(NPObject *npobj)
{
  free(npobj);
}

static void
np_invalidate(NPObject *npobj)
{
}

static bool 
np_hasmethod(NPObject *npobj, NPIdentifier name)
{
  if (name == npid_getdjvuopt ||
      name == npid_setdjvuopt )
    return 1;
  return 0;
}

static bool
np_invoke(NPObject *npobj, NPIdentifier name,
          const NPVariant *args, uint32_t argCount,
          NPVariant *result)
{
  void *id = 0;
  Instance *inst = 0;
  if (npobj->_class && npobj->_class->allocate == np_allocate)
    if ((id = ((FatNPObject*)npobj)->npp->pdata))
      inst = map_lookup(&instance, id);
  if (inst && name == npid_getdjvuopt)
    {
      // GetDjVuOpt ------------
      if (argCount != 1)
        NPN_SetException(npobj, "Exactly one argument is expected");
      else if (args[0].type != NPVariantType_String)
        NPN_SetException(npobj, "First argument should be a string");
      else
        {
          char *res = 0;
          char *tmp = 0;
          const char *kname = NPVARIANT_TO_STRING(args[0]).utf8characters;
          int klen = NPVARIANT_TO_STRING(args[0]).utf8length;
          if ( (WriteInteger(pipe_write, CMD_GET_DJVUOPT) <= 0) ||
               (WritePointer(pipe_write, id) <= 0) ||
               (WriteStringLen(pipe_write, kname, klen) <= 0) ||
               (ReadResult(pipe_read, rev_pipe, check_requests) <= 0) ||
               (ReadString(pipe_read, &res, 0, 0) <= 0) )
            {
              NPN_SetException(npobj, "Djview program died");
              ProgramDied();
              return 0;
            }
          if (! (tmp = NPN_MemAlloc(strlen(res) + 1)))
            {
              NPN_SetException(npobj, "Out of memory");
              return 0;
            }
          strcpy(tmp, res);
          STRINGZ_TO_NPVARIANT(tmp, *result);
          free(res);
          return 1;
        }
      return 0;
    }
  else if (inst && name == npid_setdjvuopt)
    {
      // SetDjVuOpt ------------
      if (argCount != 2)
        NPN_SetException(npobj, "Exactly two arguments were expected");
      else if (args[0].type != NPVariantType_String)
        NPN_SetException(npobj, "First argument should be a string");
      else
        {
          const char *kname = NPVARIANT_TO_STRING(args[0]).utf8characters;
          int klen = NPVARIANT_TO_STRING(args[0]).utf8length;
          char buffer[32];
          const char *arg = buffer;
          int len = -1;
          if (NPVARIANT_IS_INT32(args[1]))
            sprintf(buffer,"%d", NPVARIANT_TO_INT32(args[1]));
          else if (NPVARIANT_IS_DOUBLE(args[1]))
            sprintf(buffer,"%e", NPVARIANT_TO_DOUBLE(args[1]));
          else if (NPVARIANT_IS_STRING(args[1]))
            {
              arg = NPVARIANT_TO_STRING(args[1]).utf8characters;
              len = NPVARIANT_TO_STRING(args[1]).utf8length;
            }
          else
            {
              NPN_SetException(npobj, "Arg 2 should be a string or a number");
              return 0;
            }
          if (len < 0)
            len = strlen(arg);
          if ( (WriteInteger(pipe_write, CMD_SET_DJVUOPT) <= 0) ||
               (WritePointer(pipe_write, id) <= 0) ||
               (WriteStringLen(pipe_write, kname, klen) <= 0) ||
               (WriteStringLen(pipe_write, arg, len) <= 0) ||
               (ReadResult(pipe_read, rev_pipe, check_requests) <= 0) )
            {
              NPN_SetException(npobj, "Djview program died");
              ProgramDied();
              return 0;
            }
          VOID_TO_NPVARIANT(*result);
          return 1;
        }
      return 0;
    }
  NPN_SetException(npobj, "Unrecognized method");
  return 0;
}

static bool 
np_invokedefault(NPObject *npobj, const NPVariant *args, 
                 uint32 argCount, NPVariant *result)
{
  return 0;
}

static bool 
np_hasproperty(NPObject *npobj, NPIdentifier name)
{
  if (name == npid_onchange || 
      name == npid_version)
    return 1;
  return 0;
}

static bool 
np_getproperty(NPObject *npobj, NPIdentifier name, NPVariant *result)
{
  void *id = 0;
  Instance *inst = 0;
  if (npobj->_class && npobj->_class->allocate == np_allocate)
    if ((id = ((FatNPObject*)npobj)->npp->pdata))
      inst = map_lookup(&instance, id);
  if (inst && name == npid_onchange)
    {
      npvariantcpy(result, &inst->onchange);
      return 1;
    }
  else if (inst && name == npid_version)
    {
      NPVariant res;
      STRINGZ_TO_NPVARIANT("nsdejavu+djview4 (x11)", res);
      npvariantcpy(result, &res);
      return 1;
    }
  return 0;
}

static bool
np_setproperty(NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
  void *id = 0;
  Instance *inst = 0;
  if (npobj->_class && npobj->_class->allocate == np_allocate)
    if ((id = ((FatNPObject*)npobj)->npp->pdata))
      inst = map_lookup(&instance, id);
  if (inst && name == npid_onchange)
    {
      int onchange_flag = 0;
      NPN_ReleaseVariantValue(&inst->onchange);
      npvariantcpy(&inst->onchange, value);
      if (NPVARIANT_IS_STRING(*value))
        onchange_flag = 1;
      else if (! NPVARIANT_IS_NULL(*value) && 
               ! NPVARIANT_IS_VOID(*value) )
        {
          NPN_SetException(npobj, "String or null expected");
          return 0;
        }
      if ( (WriteInteger(pipe_write, CMD_ON_CHANGE) <= 0) ||
           (WritePointer(pipe_write, id) <= 0) ||
           (WriteInteger(pipe_write, onchange_flag) <= 0) ||
           (ReadResult(pipe_read, rev_pipe, check_requests) <= 0) )
        {
          NPN_SetException(npobj, "Djview program died");
          ProgramDied();
          return 0;
        }
      return 1;
    }
  return 0;
}

static bool
np_removeproperty(NPObject *npobj, NPIdentifier name)
{
  NPVariant v;
  VOID_TO_NPVARIANT(v);
  return np_setproperty(npobj, name, &v);
}


static NPClass npclass = {
  NP_CLASS_STRUCT_VERSION,
  np_allocate, np_deallocate, np_invalidate,
  np_hasmethod, np_invoke, np_invokedefault,
  np_hasproperty, 
  np_getproperty, 
  np_setproperty,
  np_removeproperty
};



/*******************************************************************************
***************************** Netscape plugin interface ************************
*******************************************************************************/

NPError
NPP_Initialize(void)
{
  LoadStatic();
  pipe(delay_pipe);
  if (!IsConnectionOK(TRUE))
    {
      CloseConnection();
      if (StartProgram() < 0)
        return NPERR_GENERIC_ERROR;
    }
  if (scriptable)
    {
      npid_getdjvuopt = NPN_GetStringIdentifier("getdjvuopt");
      npid_setdjvuopt = NPN_GetStringIdentifier("setdjvuopt");
      npid_onchange = NPN_GetStringIdentifier("onchange");
      npid_version = NPN_GetStringIdentifier("version");
    }
  return NPERR_NO_ERROR;
}

void
NPP_Shutdown(void)
{
#if USE_XT
  if (input_id) 
    XtRemoveInput(input_id); 
  input_id = 0;
  if (delay_id) 
    XtRemoveInput(delay_id); 
  delay_id = 0;
#endif
#if USE_GLIB
  if (input_gid)
    g_source_remove(input_gid);
  input_gid = 0;
  if (delay_gid)
    g_source_remove(delay_gid);
  delay_gid = 0;
#endif
  close(delay_pipe[0]);
  close(delay_pipe[1]);
  map_purge(&instance);
  map_purge(&strinstance);
  delayedrequest_purge(&delayed_requests);
  SaveStatic();
  if (IsConnectionOK(FALSE))
    WriteInteger(pipe_write, CMD_SHUTDOWN);
}

NPError
NPP_New(NPMIMEType mime, NPP np_inst, uint16 np_mode, int16 argc,
	char* argn[], char* argv[], NPSavedData * saved)
{
  const char *path;
  Instance *inst = 0;
  void *id = 0;
  int i;
  if (!IsConnectionOK(TRUE))
    {
      fprintf(stderr, "nsdejavu: restarting djview (reload the page.)\n");
      CloseConnection();
      StartProgram();
    }
  path = GetLibraryPath();
  if ( (WriteInteger(pipe_write, CMD_NEW) <= 0) ||
       (WriteInteger(pipe_write, np_mode==NP_FULL) <= 0) ||
       (WriteString(pipe_write, path) <= 0) ||
       (WriteInteger(pipe_write, argc) <= 0) )
    {
    problem:
      ProgramDied();
      map_remove(&instance, id);
      return NPERR_GENERIC_ERROR;
    }
  for(i=0;i<argc;i++)
    {
      if ( (WriteString(pipe_write, argn[i]) <= 0) ||
           (WriteString(pipe_write, argv[i]) <= 0) )
        goto problem;
    }
  if (saved && saved->buf && saved->len==sizeof(SavedData))
    {
      SavedData * data = (SavedData *) saved->buf;
      if ( (WriteInteger(pipe_write, 1) <= 0) ||
           (WriteInteger(pipe_write, data->cmd_mode) <= 0) ||
           (WriteInteger(pipe_write, data->cmd_zoom) <= 0) ||
           (WriteInteger(pipe_write, data->imgx) <= 0) ||
           (WriteInteger(pipe_write, data->imgy) <= 0) )
        goto problem;
    } 
  else if (WriteInteger(pipe_write, 0) <= 0)
    goto problem;
  if (ReadResult(pipe_read, rev_pipe, check_requests) <= 0)
    goto problem;
  if (ReadPointer(pipe_read, &id, 0, 0) <= 0)
    goto problem;
  if ((inst = map_lookup(&instance, id)))
    /* This can happen because we do not clear
       the instance array when restarting djview.
       We just undo it... */
    map_remove(&instance, id);
  np_inst->pdata = id;
  if (! (inst = instance_new(np_inst, np_mode==NP_FULL)))
    goto problem;
  if (map_insert(&instance, id, inst) < 0)
    goto problem;
  if (scriptable)
    inst->npobject = NPN_CreateObject(np_inst, &npclass);
#if USE_GLIB
  if (xembedable)
    NPN_GetValue(np_inst, NPNVSupportsXEmbedBool, &inst->xembed_mode);
#endif
#if USE_XT
  if (inst->xembed_mode && XtWindowToWidget)
    {
        NPNToolkitType toolkit = 0;
        if (NPN_GetValue(np_inst, NPNVToolkit, &toolkit) 
            != NPERR_NO_ERROR || toolkit != NPNVGtk2 )
          inst->xembed_mode = 0;
    }
#endif
  fprintf(stderr,"nsdejavu: using the %s protocol.\n",
          (inst->xembed_mode) ? "XEmbed" : "Xt");
#if USE_GLIB
  if (inst->xembed_mode && g_io_add_watch)
    return NPERR_NO_ERROR;
#endif
#if USE_XT
  if (!inst->xembed_mode && XtWindowToWidget)
    return NPERR_NO_ERROR;
#endif
  fprintf(stderr,"nsdejavu: browser does not export the %s symbols.\n",
          (inst->xembed_mode) ? "Glib2" : "Xt" );
  return NPERR_INCOMPATIBLE_VERSION_ERROR;
}


NPError
NPP_Destroy(NPP np_inst, NPSavedData ** save)
{
  Instance *inst = 0;
  void * id = np_inst->pdata;
  SavedData saved_data;
  if (! (inst = map_lookup(&instance, id)))
    return NPERR_INVALID_INSTANCE_ERROR;
  if (inst->npobject)
    NPN_ReleaseObject(inst->npobject);
  inst->npobject = 0;
  NPP_SetWindow(np_inst, 0); 
  map_remove(&instance, id);
  np_inst->pdata=0;
  
  if (IsConnectionOK(FALSE))
    {
      if ( (WriteInteger(pipe_write, CMD_DESTROY) <= 0) ||
           (WritePointer(pipe_write, id) <= 0) ||
           (ReadResult(pipe_read, rev_pipe, check_requests) <= 0) ||
           (ReadInteger(pipe_read, &saved_data.cmd_mode, 0, 0) <= 0) ||
           (ReadInteger(pipe_read, &saved_data.cmd_zoom, 0, 0) <= 0) ||
           (ReadInteger(pipe_read, &saved_data.imgx, 0, 0) <= 0) ||
           (ReadInteger(pipe_read, &saved_data.imgy, 0, 0) <= 0) )
        {
          ProgramDied();
          if (inst)
            instance_free(inst);
          return NPERR_GENERIC_ERROR;
        }
      if (save && !*save && saved_data.cmd_mode>0 && saved_data.cmd_zoom>0)
        {
          SavedData *data = (SavedData*) NPN_MemAlloc(sizeof(SavedData));
          NPSavedData *saved = (NPSavedData*) NPN_MemAlloc(sizeof(NPSavedData));
          if (saved && data)
	    {
              *data=saved_data;
              saved->len = sizeof(SavedData);
              saved->buf = (void*)data;
              *save = saved;
	    }
        }
    }
  if (inst)
    instance_free(inst);
  return NPERR_NO_ERROR;
}

NPError
NPP_SetWindow(NPP np_inst, NPWindow * win_str)
{
  Instance *inst = 0;
  void * id = np_inst->pdata;
  Window cur_window, new_window;

  if (! (inst = map_lookup(&instance, id)))
    return NPERR_INVALID_INSTANCE_ERROR;
  cur_window = inst->window;
  new_window = (win_str) ? (Window) win_str->window : 0;
  if (cur_window)
    {
      if (new_window==cur_window)
        {
          Resize(id);
          return NPERR_NO_ERROR;
        }
      if (Detach(id) < 0)
        {
          ProgramDied();
          return NPERR_GENERIC_ERROR;
        }
    }
  if (new_window)
    {
      Display *displ = 0;
      if (NPN_GetValue(np_inst, NPNVxDisplay, &displ) != NPERR_NO_ERROR)
        displ = ((NPSetWindowCallbackStruct *)(win_str->ws_info))->display;
      if (!IsConnectionOK(FALSE)) 
        return NPERR_GENERIC_ERROR;
      if (Attach(displ, new_window, id) < 0)
        {
          ProgramDied();
          return NPERR_GENERIC_ERROR;
        }
    }
  return NPERR_NO_ERROR;
}


void
NPP_Print(NPP np_inst, NPPrint* printInfo)
{
  Instance *inst = 0;
  void * id = np_inst->pdata;

  if ((inst = map_lookup(&instance, id)))
    if (inst->window)
      {
        if (printInfo && printInfo->mode==NP_FULL)
          printInfo->print.fullPrint.pluginPrinted=TRUE;
        if (IsConnectionOK(FALSE))
          {
            int modefull = printInfo && printInfo->mode==NP_FULL;
            if ( (WriteInteger(pipe_write, CMD_PRINT) <= 0) ||
                 (WritePointer(pipe_write, id) <= 0) ||
                 (WriteInteger(pipe_write, modefull) <= 0) ||
                 (ReadResult(pipe_read, rev_pipe, check_requests) <= 0) )
              {
                ProgramDied();
                return;
              }
          }
      }
}

NPError
NPP_NewStream(NPP np_inst, NPMIMEType type, NPStream *stream,
	      NPBool seekable, uint16 *stype)
{
  Instance *inst = 0;
  void * id = np_inst->pdata;
  void * sid = 0;
  
  if (! (inst = map_lookup(&instance, id)))
    return NPERR_INVALID_INSTANCE_ERROR;
  
  if ( (WriteInteger(pipe_write, CMD_NEW_STREAM) <= 0) ||
       (WritePointer(pipe_write, id) <= 0) ||
       (WriteString(pipe_write, stream->url) <= 0) ||
       (ReadResult(pipe_read, rev_pipe, check_requests) <= 0) ||
       (ReadPointer(pipe_read, &sid, 0, 0) <= 0) )
    {
      ProgramDied();
      return NPERR_GENERIC_ERROR;
    }
  /* Zero sid means that we do not want this stream */
  stream->pdata = sid;
  if (sid != 0)
    map_insert(&strinstance, sid, (void*)1);
  return NPERR_NO_ERROR;
}

int32
NPP_WriteReady(NPP np_inst, NPStream *np_stream)
{
  return 0x7fffffff;
}

int32
NPP_Write(NPP np_inst, NPStream *stream, int32 junk, int32 len, void *buffer)
{
  int res = 0;
  void * sid = stream->pdata;
  
  if (sid)
    {
      if (! map_lookup(&strinstance, sid))
        return res;
      if ( (WriteInteger(pipe_write, CMD_WRITE) <= 0) ||
           (WritePointer(pipe_write, sid) <= 0) ||
           (WriteArray(pipe_write, len, buffer) <= 0) ||
           (ReadResult(pipe_read, rev_pipe, check_requests) <= 0) ||
           (ReadInteger(pipe_read, &res, 0, 0) <= 0) )
        {
          ProgramDied();
          return res;
        }
      /* ZERO means, that the stream ID sid is meaningless to
         the viewer. This may happen from time to time. Especially
         when user hits Reload or when viewer dies and another one
         is started back (of course, the new instance of the viewer
         knows nothing about streams or plugins, which were active
         in the viewer, which just died */
      if (res == 0)
        map_remove(&strinstance, sid);
      return res;
    }
  return len;
}

NPError
NPP_DestroyStream(NPP np_inst, NPStream *stream, NPError reason)
{
  void * sid = stream->pdata;
  if (! map_lookup(&strinstance, sid))
    return NPERR_INVALID_INSTANCE_ERROR;
  if (!IsConnectionOK(FALSE)) 
    return NPERR_GENERIC_ERROR;
  map_remove(&strinstance, sid);
  if ( (WriteInteger(pipe_write, CMD_DESTROY_STREAM) <= 0) ||
       (WritePointer(pipe_write, sid) <= 0) ||
       (WriteInteger(pipe_write, reason==NPRES_DONE) <= 0) ||
       (ReadResult(pipe_read, rev_pipe, check_requests) <= 0) )
    {
      ProgramDied();
      return NPERR_GENERIC_ERROR;
    }
  return NPERR_NO_ERROR;
}

/*******************************************************************************
***************************** Stuff, really stuff ******************************
*******************************************************************************/

void
NPP_StreamAsFile(NPP np_inst, NPStream *stream, const char* fname)
{
}

int16
NPP_HandleEvent(NPP np_inst, void* event)
{
  return 0;
}

void
NPP_URLNotify(NPP np_inst, const char *url, NPReason reason, void *ptr)
{
  if (IsConnectionOK(FALSE))
    {
      int code;
      if (reason==NPRES_DONE)
        code = 0;
      else if (reason==NPRES_USER_BREAK) 
        code = 1;
      else
        code = 2;
      if ( (WriteInteger(pipe_write, CMD_URL_NOTIFY) <= 0) ||
           (WriteString(pipe_write, url) <= 0) ||
           (WriteInteger(pipe_write, code) <= 0) ||
           (ReadResult(pipe_read, rev_pipe, check_requests) <= 0) )
        {
          ProgramDied();
        }
    }
}

char *
NPP_GetMIMEDescription(void)
{
  return 
    "image/x-djvu:djvu,djv:DjVu File;"
    "image/x.djvu::DjVu File;"
    "image/djvu::DjVu File;"
    "image/x-dejavu::DjVu File (obsolete);"
    "image/x-iw44::DjVu File (obsolete);"
    "image/vnd.djvu::DjVu File;" ;
}

jref
NPP_GetJavaClass(void)
{
   return NULL;
}


NPError
NPP_GetValue(NPP np_inst, NPPVariable variable, void *value)
{
  NPError err = NPERR_GENERIC_ERROR;
  switch (variable)
    {
      
    case NPPVpluginNameString:
#if defined(DJVIEW_VERSION_STR)
      *((char **)value) = "DjView-" DJVIEW_VERSION_STR;
#elif defined(DJVULIBRE_VERSION)
      *((char **)value) = "DjVuLibre-" DJVULIBRE_VERSION;
#endif
      err = NPERR_NO_ERROR;
      break;

     case NPPVpluginDescriptionString:
       *((char **)value) =
#if defined(DJVIEW_VERSION_STR)
         "This is the <a href=\"http://djvu.sourceforge.net\">"
         "DjView-" DJVIEW_VERSION_STR "</a> version of the DjVu plugin.<br>"
#elif defined(DJVULIBRE_VERSION)
         "This is the <a href=\"http://djvu.sourceforge.net\">"
         "DjVuLibre-" DJVULIBRE_VERSION "</a> version of the DjVu plugin.<br>"
#endif
         "See <a href=\"http://djvu.sourceforge.net\">DjVuLibre</a>.";
       err = NPERR_NO_ERROR;
       break;

    case NPPVpluginNeedsXEmbed:
      {
        void * id = np_inst->pdata;
        Instance *inst = map_lookup(&instance, id);
        if (! inst || ! inst->xembed_mode) break;
        err = NPERR_NO_ERROR;
        *((NPBool*)value) = TRUE;
      }
      break;

     case NPPVpluginScriptableNPObject:
       if (scriptable)
         {
           void * id = np_inst->pdata;
           Instance *inst = map_lookup(&instance, id);
           if (!inst || !inst->npobject) break;
           err = NPERR_NO_ERROR;
           NPN_RetainObject(inst->npobject);
           *((NPObject**)value) = inst->npobject;
         }
       break;
    
    default:
      break;
    }
  return err;
}





/******************************************************************************
*************** This replaces npunix.c from the Netscape SDK ******************
******** Derived from nspluginwrapper licensed under GPL v2 or later **********
*******************************************************************************/


char *
NP_GetMIMEDescription(void)
{
  return NPP_GetMIMEDescription();
}

NPError
NP_GetValue(void *future, NPPVariable variable, void *value)
{
  return NPP_GetValue((NPP)future, variable, value);
}

NPError
NP_Shutdown(void)
{
  NPP_Shutdown();
  return NPERR_NO_ERROR;
}

static NPNetscapeFuncs mozilla_funcs;

static int mozilla_has_npruntime = 0;

void
NPN_Version(int* plugin_major, int* plugin_minor,
            int* mozilla_major, int* mozilla_minor)
{
  *plugin_major = NP_VERSION_MAJOR;
  *plugin_minor = NP_VERSION_MINOR;
  *mozilla_major = mozilla_funcs.version >> 8;
  *mozilla_minor = mozilla_funcs.version & 0xFF;
}

NPError
NPN_GetValue(NPP instance, NPNVariable variable, void *r_value)
{
  if (mozilla_funcs.getvalue == NULL)
    return NPERR_INVALID_FUNCTABLE_ERROR;
  return CallNPN_GetValueProc(mozilla_funcs.getvalue,
                              instance, variable, r_value);
}

NPError
NPN_GetURL(NPP instance, const char* url, const char* window)
{
  if (mozilla_funcs.geturl == NULL)
    return NPERR_INVALID_FUNCTABLE_ERROR;
  return CallNPN_GetURLProc(mozilla_funcs.geturl, instance, url, window);
}

NPError
NPN_GetURLNotify(NPP instance, const char* url, const char* window, void * data)
{
  if (mozilla_funcs.geturlnotify == NULL)
    return NPERR_INVALID_FUNCTABLE_ERROR;
  return CallNPN_GetURLNotifyProc(mozilla_funcs.geturlnotify, 
                                  instance, url, window, data);
}

NPError
NPN_PostURL(NPP instance, const char* url, const char* window,
            uint32 len, const char* buf, NPBool file)
{
  if (mozilla_funcs.posturl == NULL)
    return NPERR_INVALID_FUNCTABLE_ERROR;
  return CallNPN_PostURLProc(mozilla_funcs.posturl, instance,
                             url, window, len, buf, file);
}

NPError
NPN_RequestRead(NPStream* stream, NPByteRange* rangeList)
{
  if (mozilla_funcs.requestread == NULL)
    return NPERR_INVALID_FUNCTABLE_ERROR;
  return CallNPN_RequestReadProc(mozilla_funcs.requestread,
                                 stream, rangeList);
}

NPError
NPN_NewStream(NPP instance, NPMIMEType type, const char *window,
	      NPStream** stream_ptr)
{
  if (mozilla_funcs.newstream == NULL)
    return NPERR_INVALID_FUNCTABLE_ERROR;
  return CallNPN_NewStreamProc(mozilla_funcs.newstream, instance,
                               type, window, stream_ptr);
}

int32
NPN_Write(NPP instance, NPStream* stream, int32 len, void* buffer)
{
  if (mozilla_funcs.write == NULL)
    return -1;
  return CallNPN_WriteProc(mozilla_funcs.write, instance,
                           stream, len, buffer);
}

NPError
NPN_DestroyStream(NPP instance, NPStream* stream, NPError reason)
{
  if (mozilla_funcs.destroystream == NULL)
    return NPERR_INVALID_FUNCTABLE_ERROR;
  return CallNPN_DestroyStreamProc(mozilla_funcs.destroystream,
                                   instance, stream, reason);
}

void
NPN_Status(NPP instance, const char* message)
{
  CallNPN_StatusProc(mozilla_funcs.status, instance, message);
}

const char*
NPN_UserAgent(NPP instance)
{
  return CallNPN_UserAgentProc(mozilla_funcs.uagent, instance);
}

void*
NPN_MemAlloc(uint32 size)
{
  return CallNPN_MemAllocProc(mozilla_funcs.memalloc, size);
}

void 
NPN_MemFree(void* ptr)
{
  CallNPN_MemFreeProc(mozilla_funcs.memfree, ptr);
}

void 
NPN_ReloadPlugins(NPBool reloadPages)
{
  if (mozilla_funcs.reloadplugins != NULL)
    CallNPN_ReloadPluginsProc(mozilla_funcs.reloadplugins, reloadPages);
}

NPError
NPN_SetValue(NPP instance, NPPVariable variable, void *value)
{
  if (mozilla_funcs.setvalue == NULL)
    return NPERR_INVALID_FUNCTABLE_ERROR;
  return CallNPN_GetValueProc(mozilla_funcs.setvalue, 
                              instance, variable, value);
}

void
NPN_InvalidateRect(NPP instance, NPRect *invalid)
{
  if (mozilla_funcs.invalidaterect != NULL)
    CallNPN_InvalidateRectProc(mozilla_funcs.invalidaterect, 
                               instance, invalid);
}

void
NPN_InvalidateRegion(NPP instance, NPRegion invalid)
{
  if (mozilla_funcs.invalidateregion != NULL)
    CallNPN_InvalidateRegionProc(mozilla_funcs.invalidateregion, 
                                 instance, invalid);
}

void
NPN_ForceRedraw(NPP instance)
{
  if (mozilla_funcs.forceredraw != NULL)
    CallNPN_ForceRedrawProc(mozilla_funcs.forceredraw, instance);
}

NPIdentifier 
NPN_GetStringIdentifier(const NPUTF8 *name)
{
  if (!mozilla_funcs.getstringidentifier || !mozilla_has_npruntime)
    return NULL;
  return CallNPN_GetStringIdentifierProc(mozilla_funcs.getstringidentifier, name);
}

NPIdentifier 
NPN_GetIntIdentifier(int32_t intid)
{
  if (!mozilla_funcs.getintidentifier || !mozilla_has_npruntime)
    return NULL;
  return CallNPN_GetIntIdentifierProc(mozilla_funcs.getintidentifier, intid);
}

bool 
NPN_IdentifierIsString(NPIdentifier identifier)
{
  if (!mozilla_funcs.identifierisstring || !mozilla_has_npruntime)
    return FALSE;
  return CallNPN_IdentifierIsStringProc(mozilla_funcs.identifierisstring, 
                                        identifier);
}

NPUTF8 *
NPN_UTF8FromIdentifier(NPIdentifier identifier)
{
  if (!mozilla_funcs.utf8fromidentifier || !mozilla_has_npruntime)
    return NULL;
  return CallNPN_UTF8FromIdentifierProc(mozilla_funcs.utf8fromidentifier, 
                                                identifier);
}

int32_t 
NPN_IntFromIdentifier(NPIdentifier identifier)
{
  if (!mozilla_funcs.intfromidentifier || !mozilla_has_npruntime)
    return 0;
  return CallNPN_IntFromIdentifierProc(mozilla_funcs.intfromidentifier, 
                                       identifier);
}

NPObject *
NPN_CreateObject(NPP npp, NPClass *aClass)
{
  if (!mozilla_funcs.createobject || !mozilla_has_npruntime)
    return 0;
  return CallNPN_CreateObjectProc(mozilla_funcs.createobject, npp, aClass); 
}

NPObject *
NPN_RetainObject(NPObject *npobj)
{
  if (!mozilla_funcs.retainobject || !mozilla_has_npruntime)
    return npobj;
  return CallNPN_RetainObjectProc(mozilla_funcs.retainobject, npobj);
}

void 
NPN_ReleaseObject(NPObject *npobj)
{
  if (mozilla_funcs.releaseobject && mozilla_has_npruntime)
    CallNPN_ReleaseObjectProc(mozilla_funcs.releaseobject, npobj);
}

void 
NPN_ReleaseVariantValue(NPVariant *npvariant)
{
  if (mozilla_funcs.releasevariantvalue && mozilla_has_npruntime)
    CallNPN_ReleaseVariantValueProc(mozilla_funcs.releasevariantvalue, 
                                    npvariant);
}

bool 
NPN_Evaluate(NPP npp, NPObject* obj, NPString *script, NPVariant *result)
{
  if (!mozilla_funcs.createobject || !mozilla_has_npruntime)
    return 0;
  return CallNPN_EvaluateProc(mozilla_funcs.evaluate, 
                              npp, obj, script, result);
}

void 
NPN_SetException(NPObject *npobj, const NPUTF8 *message)
{
  if (mozilla_funcs.setexception && mozilla_has_npruntime)
    CallNPN_SetExceptionProc(mozilla_funcs.setexception, npobj, message);
}

NPError
NP_Initialize(NPNetscapeFuncs *moz_funcs, NPPluginFuncs *plugin_funcs)
{
  /* basic checks */
  if (moz_funcs == NULL || plugin_funcs == NULL)
    return NPERR_INVALID_FUNCTABLE_ERROR;
  if ((moz_funcs->version >> 8) != NP_VERSION_MAJOR)
    return NPERR_INCOMPATIBLE_VERSION_ERROR;
  if (moz_funcs->size < (offsetof(NPNetscapeFuncs, forceredraw) 
                         + sizeof(NPN_ForceRedrawUPP) ))
    return NPERR_INVALID_FUNCTABLE_ERROR;
  if (plugin_funcs->size < sizeof(NPPluginFuncs))
    return NPERR_INVALID_FUNCTABLE_ERROR;
  
  /* copy mozilla_funcs. */
  memcpy(&mozilla_funcs, moz_funcs, 
         min(moz_funcs->size, sizeof(mozilla_funcs)));

  /* fill plugin_funcs. */
  memset(plugin_funcs, 0, sizeof(*plugin_funcs));
  plugin_funcs->size = sizeof(NPPluginFuncs);
  plugin_funcs->version = (NP_VERSION_MAJOR << 8) + NP_VERSION_MINOR;
  plugin_funcs->newp = NewNPP_NewProc(NPP_New);
  plugin_funcs->destroy = NewNPP_DestroyProc(NPP_Destroy);
  plugin_funcs->setwindow = NewNPP_SetWindowProc(NPP_SetWindow);
  plugin_funcs->newstream = NewNPP_NewStreamProc(NPP_NewStream);
  plugin_funcs->destroystream = NewNPP_DestroyStreamProc(NPP_DestroyStream);
  plugin_funcs->asfile = NewNPP_StreamAsFileProc(NPP_StreamAsFile);
  plugin_funcs->writeready = NewNPP_WriteReadyProc(NPP_WriteReady);
  plugin_funcs->write = NewNPP_WriteProc(NPP_Write);
  plugin_funcs->print = NewNPP_PrintProc(NPP_Print);
  plugin_funcs->event = NULL;
  plugin_funcs->urlnotify = NewNPP_URLNotifyProc(NPP_URLNotify);
  plugin_funcs->javaClass = NULL;
  plugin_funcs->getvalue = NewNPP_GetValueProc(NPP_GetValue);
  plugin_funcs->setvalue = NULL;

  /* check for npruntime */
  mozilla_has_npruntime = 1;
  if ((moz_funcs->version >> 8) == 0 && (moz_funcs->version & 0xff) < 14)
    mozilla_has_npruntime = 0;
  if (moz_funcs->size < (offsetof(NPNetscapeFuncs, setexception) 
                         + sizeof(NPN_SetExceptionUPP) ))
    mozilla_has_npruntime = 0;
  
  /* call npp_initialize */
  return NPP_Initialize();
}


