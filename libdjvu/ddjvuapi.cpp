/*C- 
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
//C- Library notice by the following text (see doc/lizard2002.djvu):
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
//C- */ 

/* $Id$ */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation "ddjvuapi.h"
#endif

#include <stdlib.h>
#include <string.h>

#include "ddjvuapi.h"

#include "GException.h"
#include "GSmartPointer.h"
#include "GThreads.h"
#include "GContainer.h"
#include "GString.h"
#include "GBitmap.h"
#include "GPixmap.h"
#include "DjVuPort.h"
#include "DjVuInfo.h"
#include "DjVuImage.h"
#include "DjVuFileCache.h"
#include "DjVuDocument.h"

// ----------------------------------------

struct ddjvu_message_p : public GPEnabled
{
  char *tmp1;
  char *tmp2;
  char *tmp3;
  ddjvu_message_t p;

  ddjvu_message_p() 
    : tmp1(0), tmp2(0), tmp3(0)
  { memset(&p, 0, sizeof(p)); }

  ~ddjvu_message_p() 
  { if (tmp1) free((void*)tmp1);
    if (tmp2) free((void*)tmp2);
    if (tmp3) free((void*)tmp3);
    tmp1 = tmp2 = tmp3 = 0; }
};


// ----------------------------------------

struct ddjvu_context_s : public GPEnabled
{
  GMonitor monitor;
  GP<DjVuFileCache> cache;
  GPList<ddjvu_message_p> mlist;
  ddjvu_message_callback_t callback;
};

struct ddjvu_document_s : public DjVuPort
{
  GP<ddjvu_context_s> myctx;
  GP<DjVuDocument> doc;
  void *userdata;
};

struct ddjvu_page_s : public DjVuPort
{
  GP<ddjvu_document_s> mydoc;
  GP<DjVuImage> img;
  void *userdata;
};

struct ddjvu_format_s : public DjVuPort
{
};

// ----------------------------------------

// Hack to increment counter
static void ref(GPEnabled *p)
{
  GPBase n(p);
  char *gn = (char*)&n;
  *(GPEnabled**)gn = 0;
  n.assign(0);
}

// Hack to decrement counter
static void unref(GPEnabled *p)
{
  GPBase n;
  char *gn = (char*)&n;
  *(GPEnabled**)gn = p;
  n.assign(0);
}

// Allocate stings
static char *xstr(const char *s)
{
  int l = strlen(s);
  char *p = (char*)malloc(l + 1);
  if (p) 
    {
      strcpy(p, s);
      p[l] = 0;
    }
  return p;
}

// Allocate stings
static char *xstr(GNativeString n)
{
  return xstr( (const char*) n );
}

// Allocate stings
static char *xstr(GUTF8String u)
{
  GNativeString n(u);
  return xstr( (const char*) n );
}


// ----------------------------------------

ddjvu_context_t *
ddjvu_context_create(void)
{
  ddjvu_context_t *ctx = new ddjvu_context_t;
  ctx->cache = DjVuFileCache::create();
  ctx->callback = 0;
  ref(ctx);
  return ctx;
}

void 
ddjvu_context_release(ddjvu_context_t *ctx)
{
  if (ctx)
    unref(ctx);
}

// ----------------------------------------


void
ddjvu_cache_set_size(ddjvu_context_t *ctx,
                     unsigned long cachesize)
{
  GMonitorLock(&ctx->monitor);
  if (ctx->cache && cachesize>0)
    ctx->cache->set_max_size(cachesize);
}


DDJVUAPI unsigned long
ddjvu_cache_get_size(ddjvu_context_t *ctx)
{
  GMonitorLock(&ctx->monitor);
  if (ctx->cache)
    return ctx->cache->get_max_size();
  return 0;
}

void
ddjvu_cache_clear(ddjvu_context_t *ctx)
{
  GMonitorLock(&ctx->monitor);
  if (ctx->cache)
    return ctx->cache->clear();
 }


// ----------------------------------------


ddjvu_message_t *
ddjvu_message_peek(ddjvu_context_t *ctx)
{
  GMonitorLock(&ctx->monitor);
  if (ctx->mlist.size())
    return &ctx->mlist[ctx->mlist]->p;
  return 0;
}

ddjvu_message_t *
ddjvu_message_wait(ddjvu_context_t *ctx)
{
  GMonitorLock(&ctx->monitor);
  while (! ctx->mlist.size())
    ctx->monitor.wait();
  return &ctx->mlist[ctx->mlist]->p;
}

void
ddjvu_message_pop(ddjvu_context_t *ctx)
{
  GMonitorLock(&ctx->monitor);
  GPosition p = ctx->mlist;
  if (p) 
    ctx->mlist.del(p);
}

ddjvu_message_callback_t
ddjvu_message_set_callback(ddjvu_context_t *ctx,
                           ddjvu_message_callback_t callback)
{
  GMonitorLock(&ctx->monitor);
  ddjvu_message_callback_t oldcallback = ctx->callback;
  ctx->callback = callback;
  return oldcallback;
}

static void
ddjvu_message_push(ddjvu_context_t *ctx, GP<ddjvu_message_p> msg)
{
  // lock
  ctx->monitor.enter();
  // special treatment
  ddjvu_message_tag_t k = msg->p.m_any.kind;
  if (k == DDJVU_RELAYOUT || k == DDJVU_REDISPLAY)
    {
      for (GPosition p = ctx->mlist; p; ++p)
        {
          ddjvu_message_tag_t s = ctx->mlist[p]->p.m_any.kind;
          if ((k==DDJVU_REDISPLAY && (s==k)) || 
              (k==DDJVU_RELAYOUT  && (s==k || s==DDJVU_REDISPLAY)) )
            {
              GPosition c = p; --p;
              ctx->mlist.del(c);
            }
        }
    }
  // append
  ctx->mlist.append(msg);
  // signal
  ctx->monitor.broadcast();
  ctx->monitor.leave();
  // callback
  if (ctx->callback)
    (ctx->callback)(ctx, ctx->mlist.size());
}

static void
ddjvu_error(const char *msg,
            ddjvu_context_t *ctx, 
            ddjvu_document_t *doc = 0,
            ddjvu_page_t *pag = 0,
            ddjvu_error_t cat = ddjvu_error_api,
            const char *filename = 0,
            int lineno = 0)
{
  GP<ddjvu_message_p> p = new ddjvu_message_p;
  p->tmp1 = xstr(msg);
  p->p.m_any.kind = DDJVU_ERROR;
  p->p.m_any.context = ctx;
  p->p.m_any.document = doc;
  p->p.m_any.page = pag;
  p->p.m_error.message = p->tmp1;
  p->p.m_error.filename = filename;
  p->p.m_error.lineno = lineno;
  ddjvu_message_push(ctx, p);
}

#define APIERROR(m,c,d,p) \
  ddjvu_error(m, c, d, p, ddjvu_error_api, __FILE__, __LINE__)

#define APIASSERT(cond, m,c,d,p) \
  if (!(cond)) ddjvu_error(m, c, d, p, ddjvu_error_api, __FILE__, __LINE__)


// ----------------------------------------


ddjvu_document_t *
ddjvu_document_create(ddjvu_context_t *ctx,
                      const char *url,
                      int cache)
{
  return 0;
}

ddjvu_document_t *
ddjvu_document_create_by_filename(ddjvu_context_t *ctx,
                                  const char *filename,
                                  int cache)
{
  return 0;
}

void
ddjvu_document_release(ddjvu_document_t *doc)
{
  if (doc)
    unref(doc);
}


// ----------------------------------------


void
ddjvu_document_set_user_data(ddjvu_document_t *doc,
                             void *userdata)
{
  if (doc)
    doc->userdata = userdata;
}

void *
ddjvu_document_get_user_data(ddjvu_document_t *doc)
{
  if (doc)
    return doc->userdata;
  return 0;
}


// ----------------------------------------


void
ddjvu_stream_write(ddjvu_document_t *doc,
                   int streamid,
                   const char *data,
                   unsigned long datalen )
{
}

void
ddjvu_stream_close(ddjvu_document_t *doc,
                   int streamid,
                   int stop )
{
}


// ----------------------------------------


ddjvu_document_type_t
ddjvu_document_get_type(ddjvu_document_t *doc)
{
  return DDJVU_DOCTYPE_UNKNOWN;
}

int
ddjvu_document_get_pagenum(ddjvu_document_t *doc)
{
  return 0;
}


// ----------------------------------------


ddjvu_page_t *
ddjvu_page_create_by_pageno(ddjvu_document_t *doc,
                             int pageno)
{
  return 0;
}

ddjvu_page_t *
ddjvu_page_create_by_pageid(ddjvu_document_t *doc,
                            const char *pageid)
{
  return 0;
}

void
ddjvu_page_release(ddjvu_page_t *pag)
{
}


// ----------------------------------------


void
ddjvu_page_set_user_data(ddjvu_page_t *pag,
                         void *userdata)
{
  if (pag)
    pag->userdata = userdata;
}

void *
ddjvu_page_get_user_data(ddjvu_page_t *pag)
{
  if (pag)
    return pag->userdata;
  return 0;
}


// ----------------------------------------


ddjvu_decoding_status_t
ddjvu_page_decoding_status(ddjvu_page_t *pag)
{
  return DDJVU_DECODE_NOTSTARTED;
}

int
ddjvu_page_get_width(ddjvu_page_t *pag)
{
  return 0;
}

int
ddjvu_page_get_height(ddjvu_page_t *pag)
{
  return 0;
}

int
ddjvu_page_get_resolution(ddjvu_page_t *pag)
{
  return 0;
}

double
ddjvu_page_get_gamma(ddjvu_page_t *pag)
{
  return 2.2;
}

int
ddjvu_page_get_version(ddjvu_page_t *pag)
{
  return DJVUVERSION;
}

ddjvu_page_type_t
ddjvu_page_get_type(ddjvu_page_t *pag)
{
  return DDJVU_PAGETYPE_UNKNOWN;
}

char *
ddjvu_page_get_short_description(ddjvu_page_t *pag)
{
  return 0;
}

char *
ddjvu_page_get_long_description(ddjvu_page_t *pag)
{
  return 0;
}


// ----------------------------------------


ddjvu_page_rotation_t
ddjvu_page_get_rotation(ddjvu_page_t *pag)
{
  return DDJVU_ROTATE_0;
}


void
ddjvu_page_set_rotation(ddjvu_page_t *pag,
                        ddjvu_page_rotation_t rot)
{
}


// ----------------------------------------


int
ddjvu_page_render(ddjvu_page_t *pag,
                  const ddjvu_format_t *fmt,
                  const ddjvu_rect_t *prect,
                  const ddjvu_rect_t *rrect,
                  char *img,
                  unsigned long rowsize)
{
  return 0;
}


// ----------------------------------------


ddjvu_format_t *
ddjvu_format_create_truecolor(int pixelsize,
                              unsigned long redmask,
                              unsigned long greenmask,
                              unsigned long bluemask,
                              int top_to_bottom)
{
  return 0;
}

ddjvu_format_t *
ddjvu_format_create_palette(int pixelsize,
                            unsigned long palette[6*6*6],
                            int top_to_bottom)
{
  return 0;
}

ddjvu_format_t *
ddjvu_format_create_graylevel(int pixelsize,
                              int pixelwhite,
                              int pixelblack,
                              int top_to_bottom)
{
  return 0;
}

ddjvu_format_t *
ddjvu_format_create_bitonal(int lsb_to_msb,
                            int min_is_white,
                            int top_to_bottom)
{
  return 0;
}

void
ddjvu_format_set_gamma(ddjvu_format_t *format,
                       double gamma)
{
}

void
ddjvu_format_release(ddjvu_format_t *format)
{
}


// ----------------------------------------

