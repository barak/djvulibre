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
#include "DataPool.h"
#include "DjVuInfo.h"
#include "DjVuImage.h"
#include "DjVuFileCache.h"
#include "DjVuDocument.h"
#include "DjVuMessageLite.h"


// ----------------------------------------

struct ddjvu_message_p : public GPEnabled
{
  GNativeString tmp1;
  GNativeString tmp2;
  ddjvu_message_t p;
  ddjvu_message_p() { memset(&p, 0, sizeof(p)); }
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
  GMonitor monitor;
  GPMap<int,DataPool> streams;
  int streamid;
  void *userdata;
  bool fileflag;
  bool urlflag;
  bool docinfoflag;
  // virtual port functions
  virtual bool inherits(const GUTF8String&);
  virtual bool notify_error(const DjVuPort*, const GUTF8String&);  
  virtual bool notify_status(const DjVuPort*, const GUTF8String&);
  virtual void notify_doc_flags_changed(const DjVuDocument*, long, long);
  virtual GP<DataPool> request_data(const DjVuPort*, const GURL&);
};

struct ddjvu_page_s : public DjVuPort
{
  GP<ddjvu_document_s> mydoc;
  GP<DjVuImage> img;
  void *userdata;
  bool pageinfoflag;
  // virtual port functions
  virtual bool inherits(const GUTF8String&);
  virtual bool notify_error(const DjVuPort*, const GUTF8String&);  
  virtual bool notify_status(const DjVuPort*, const GUTF8String&);
  virtual void notify_file_flags_changed(const DjVuFile*, long, long);
  virtual void notify_relayout(const class DjVuImage*);
  virtual void notify_redisplay(const class DjVuImage*);
  virtual void notify_chunk_done(const DjVuPort*, const GUTF8String &);
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

// Allocate strings
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
static char *xstr(const GNativeString &n)
{
  return xstr( (const char*) n );
}
static char *xstr(const GUTF8String &u)
{
  GNativeString n(u);
  return xstr( n );
}

// Fill a message head
static ddjvu_message_any_t 
xhead(ddjvu_message_tag_t tag,
      ddjvu_context_t *ctx)
{
  ddjvu_message_any_t any;
  any.tag = tag;
  any.page = 0;
  any.document = 0;
  any.context = ctx;
  return any;
}
static ddjvu_message_any_t 
xhead(ddjvu_message_tag_t tag,
      ddjvu_document_t *doc)
{
  ddjvu_message_any_t any;
  any.tag = tag;
  any.page = 0;
  any.document = doc;
  any.context = any.document->myctx;
  return any;
}
static ddjvu_message_any_t 
xhead(ddjvu_message_tag_t tag,
      ddjvu_page_t *pag)
{
  ddjvu_message_any_t any;
  any.tag = tag;
  any.page = pag;
  any.document = any.page->mydoc;
  any.context = any.document->myctx;
  return any;
}


// ----------------------------------------


ddjvu_context_t *
ddjvu_context_create(void)
{
  G_TRY
    {
      ddjvu_context_t *ctx = new ddjvu_context_s;
      ctx->cache = DjVuFileCache::create();
      ctx->callback = 0;
      ref(ctx);
      return ctx;
    }
  G_CATCH(ex)
    {
    }
  G_ENDCATCH;
  return 0;
}

void 
ddjvu_context_release(ddjvu_context_t *ctx)
{
  G_TRY
    {
      if (ctx)
        unref(ctx);
    }
  G_CATCH(ex)
    {
    }
  G_ENDCATCH;
}


// ----------------------------------------


// remove messages
static void 
msg_remove(ddjvu_context_t *ctx, 
           ddjvu_message_tag_t tag,
           ddjvu_page_t *pag)
{
  ctx->monitor.enter();
  GPosition p = ctx->mlist;
  while (p)
    {
      ddjvu_message_any_t &any = ctx->mlist[p]->p.m_any;
      if (any.tag == tag && any.page == pag)
        {
          GPosition c = p; --p;
          ctx->mlist.del(c);
        }
      ++p;
    }
  ctx->monitor.leave();
}

// post a new message
static void
msg_push(const ddjvu_message_any_t &head,
         GP<ddjvu_message_p> msg = 0)
{
  ddjvu_context_t *ctx = head.context;
  if (! msg) msg = new ddjvu_message_p;
  msg->p.m_any = head;
  ctx->monitor.enter();
  ctx->mlist.append(msg);
  ctx->monitor.broadcast();
  ctx->monitor.leave();
  if (ctx->callback)
    (ctx->callback)(ctx, ctx->mlist.size());
}

static void
msg_push_nothrow(const ddjvu_message_any_t &head,
                 GP<ddjvu_message_p> msg = 0)
{
  G_TRY
    {
      msg_push(head, msg);
    }
  G_CATCH(ex)
    {
    }
  G_ENDCATCH;
}


// ----------------------------------------


// prepare error message from string
static GP<ddjvu_message_p>
msg_prep_error(GUTF8String message,
               const char *function=0, 
               const char *filename=0, 
               int lineno=0)
{
  GP<ddjvu_message_p> p = new ddjvu_message_p;
  p->p.m_error.message = 0;
  p->p.m_error.function = function;
  p->p.m_error.filename = filename;
  p->p.m_error.lineno = lineno;
  G_TRY 
    { 
      p->tmp1 = DjVuMessageLite::LookUpUTF8(message);
      p->p.m_error.message = (const char*)(p->tmp1);
    }
  G_CATCH(ex) 
    {
    } 
  G_ENDCATCH;
  return p;
}

// prepare error message from exception
static GP<ddjvu_message_p>
msg_prep_error(const GException &ex,
               const char *function=0, 
               const char *filename=0, 
               int lineno=0)
{
  GP<ddjvu_message_p> p = new ddjvu_message_p;
  p->p.m_error.message = 0;
  p->p.m_error.function = function;
  p->p.m_error.filename = filename;
  p->p.m_error.lineno = lineno;
  G_TRY 
    { 
      p->tmp1 = DjVuMessageLite::LookUpUTF8(ex.get_cause());
      p->p.m_error.message = (const char*)(p->tmp1);
      p->p.m_error.function = ex.get_function();
      p->p.m_error.filename = ex.get_file();
      p->p.m_error.lineno = ex.get_line();
    }
  G_CATCH(exc) 
    {
    } 
  G_ENDCATCH;
  return p;
}

// prepare status message
static GP<ddjvu_message_p>
msg_prep_status(GUTF8String message)
{
  GP<ddjvu_message_p> p = new ddjvu_message_p;
  p->tmp1 = DjVuMessageLite::LookUpUTF8(message); // i18n nonsense!
  p->p.m_status.message = (const char*)(p->tmp1);
  return p;
}

// ----------------------------------------


#ifdef __GNUG__
# define ERROR(x, m) \
    msg_push_nothrow(xhead(DDJVU_ERROR,x),\
                     msg_prep_error(m,__func__,__FILE__,__LINE__))
#else
# define ERROR(x, m) \
    msg_push_nothrow(xhead(DDJVU_ERROR,x),\
                     msg_prep_error(m,0,__FILE__,__LINE__))
#endif

// ----------------------------------------


void
ddjvu_cache_set_size(ddjvu_context_t *ctx,
                     unsigned long cachesize)
{
  G_TRY
    {
      GMonitorLock(&ctx->monitor);
      if (ctx->cache && cachesize>0)
        ctx->cache->set_max_size(cachesize);
    }
  G_CATCH(ex) 
    {
      ERROR(ctx, ex);
    }
  G_ENDCATCH;
}

DDJVUAPI unsigned long
ddjvu_cache_get_size(ddjvu_context_t *ctx)
{
  G_TRY
    {
      GMonitorLock(&ctx->monitor);
      if (ctx->cache)
        return ctx->cache->get_max_size();
    }
  G_CATCH(ex) 
    { 
      ERROR(ctx, ex);
    }
  G_ENDCATCH;
  return 0;
}

void
ddjvu_cache_clear(ddjvu_context_t *ctx)
{
  G_TRY
    {
      GMonitorLock(&ctx->monitor);
      if (ctx->cache)
        return ctx->cache->clear();
    }
  G_CATCH(ex)
    {
      ERROR(ctx, ex);
    }
  G_ENDCATCH;
 }


// ----------------------------------------


ddjvu_message_t *
ddjvu_message_peek(ddjvu_context_t *ctx)
{
  G_TRY
    {
      GMonitorLock(&ctx->monitor);
      if (ctx->mlist.size())
        return &ctx->mlist[ctx->mlist]->p;
    }
  G_CATCH(ex)
    {
    }
  G_ENDCATCH;
  return 0;
}

ddjvu_message_t *
ddjvu_message_wait(ddjvu_context_t *ctx)
{
  G_TRY
    {
      GMonitorLock(&ctx->monitor);
      while (! ctx->mlist.size())
        ctx->monitor.wait();
      return &ctx->mlist[ctx->mlist]->p;
    }
  G_CATCH(ex)
    {
    }
  G_ENDCATCH;
  return 0;
}

void
ddjvu_message_pop(ddjvu_context_t *ctx)
{
  G_TRY
    {
      GMonitorLock(&ctx->monitor);
      GPosition p = ctx->mlist;
      if (p) 
        ctx->mlist.del(p);
    }
  G_CATCH(ex)
    {
    }
  G_ENDCATCH;
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


// ----------------------------------------


bool
ddjvu_document_s::inherits(const GUTF8String &classname)
{
  return (classname == "ddjvu_document_s");
}

bool 
ddjvu_document_s::notify_error(const DjVuPort *, const GUTF8String &m)
{
  if (!doc) return 0;
  msg_push(xhead(DDJVU_ERROR, this), msg_prep_error(m));
  return 1;
}
 
bool 
ddjvu_document_s::notify_status(const DjVuPort *p, const GUTF8String &m)
{
  if (!doc) return 0;
  msg_push(xhead(DDJVU_STATUS, this), msg_prep_status(m));
  return 1;
}

void 
ddjvu_document_s::notify_doc_flags_changed(const DjVuDocument *, long, long)
{
  if (docinfoflag || !doc) return;
  long flags = doc->get_doc_flags();
  if ((flags & DjVuDocument::DOC_INIT_OK) ||
      (flags & DjVuDocument::DOC_INIT_FAILED) )
  {
    msg_push(xhead(DDJVU_DOCINFO, this));
    docinfoflag = true;
  }
}

GP<DataPool> 
ddjvu_document_s::request_data(const DjVuPort*p, const GURL &url)
{
  GP<DataPool> pool;
  if (doc && !fileflag)
    {
      monitor.enter();
      if (streamid < 0)
        {
          streamid = 0;
          pool = streams[streamid];
        }
      else
        {
          streamid += 1;
          pool = streams[streamid] = DataPool::create();
        }
      monitor.leave();
      // build message
      GP<ddjvu_message_p> p = new ddjvu_message_p;
      p->p.m_newstream.streamid = streamid;
      // Note: the following line try to restore
      //       the bytes stored in the djvu file
      //       despite LT's i18n and gurl classes.
      p->tmp1 = (const char*)url.fname(); 
      p->p.m_newstream.name = (const char*)(p->tmp1);
      p->p.m_newstream.url = 0;
      if (urlflag)
        {
          // Should be urlencoded.
          p->tmp2 = (const char*)url.get_string();
          p->p.m_newstream.url = (const char*)(p->tmp2);
        }
    }
  return pool;
}


// ----------------------------------------


ddjvu_document_t *
ddjvu_document_create(ddjvu_context_t *ctx,
                      const char *url,
                      int cache)
{
  G_TRY
    {
      DjVuFileCache *xcache = ctx->cache;
      if (! cache) xcache = 0;
      ddjvu_document_t *d = new ddjvu_document_s;
      ref(d);
      d->streams[0] = DataPool::create();
      d->streamid = -1;
      d->fileflag = false;
      d->docinfoflag = false;
      d->myctx = ctx;
      if (url)
        {
          GURL gurl = GUTF8String(url);
          d->doc = DjVuDocument::create(gurl, d, xcache);
          d->urlflag = true;
        }
      else
        {
          GUTF8String s;
          s.format("ddjvu:///%p/index.djvu", d);
          GURL gurl = s;
          d->doc = DjVuDocument::create(gurl, d, xcache);
          d->urlflag = false;
        }
      return d;
    }
  G_CATCH(ex)
    {
      ERROR(ctx, ex);
    }
  G_ENDCATCH;
  return 0;
}

ddjvu_document_t *
ddjvu_document_create_by_filename(ddjvu_context_t *ctx,
                                  const char *filename,
                                  int cache)
{
  G_TRY
    {
      DjVuFileCache *xcache = ctx->cache;
      if (! cache) xcache = 0;
      GURL gurl = GURL::Filename::UTF8(filename);
      ddjvu_document_t *d = new ddjvu_document_s;
      ref(d);
      d->streamid = -1;
      d->fileflag = true;
      d->urlflag = false;
      d->docinfoflag = false;
      d->myctx = ctx;
      d->doc = DjVuDocument::create(gurl, d, xcache);
      return d;
    }
  G_CATCH(ex)
    {
      ERROR(ctx, ex);
    }
  G_ENDCATCH;
  return 0;
}

void
ddjvu_document_release(ddjvu_document_t *document)
{
  G_TRY
    {
      if (document)
        {
          document->doc = 0;
          unref(document);
        }
    }
  G_CATCH(ex)
    {
    }
  G_ENDCATCH;
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
  G_TRY
    {
      GPosition p = doc->streams.contains(streamid);
      if (!p) G_THROW("Unknown stream ID");
      doc->monitor.enter();
      GP<DataPool> pool = doc->streams[p];
      doc->monitor.leave();
      pool->add_data(data, datalen);
    }
  G_CATCH(ex)
    {
      ERROR(doc,ex);
    }
  G_ENDCATCH;
}

void
ddjvu_stream_close(ddjvu_document_t *doc,
                   int streamid,
                   int stop )
{
  G_TRY
    {
      GPosition p = doc->streams.contains(streamid);
      if (!p) G_THROW("Unknown stream ID");
      doc->monitor.enter();
      GP<DataPool> pool = doc->streams[p];
      doc->monitor.leave();
      if (stop)
        pool->stop();
      else
        pool->set_eof();
      doc->streams.del(p);
    }
  G_CATCH(ex)
    {
      ERROR(doc, ex);
    }
  G_ENDCATCH;
}


// ----------------------------------------


ddjvu_decoding_status_t
ddjvu_document_decoding_status(ddjvu_document_t *document)
{
  G_TRY
    {
      DjVuDocument *doc = document->doc;
      if (doc)
        {
          long flags = doc->get_doc_flags();
          if (flags & DjVuDocument::DOC_INIT_OK)
            return DDJVU_DECODE_OK;
          else if (flags & DjVuDocument::DOC_INIT_FAILED)
            return DDJVU_DECODE_FAILED;
          else
            return DDJVU_DECODE_STARTED;
        }
    }
  G_CATCH(ex)
    {
      ERROR(document,ex);
    }
  G_ENDCATCH;
  return DDJVU_DECODE_NOTSTARTED;
}

ddjvu_document_type_t
ddjvu_document_get_type(ddjvu_document_t *document)
{
  G_TRY
    {
      DjVuDocument *doc = document->doc;
      if (doc)
        {
          switch (doc->get_doc_type())
            {
            case DjVuDocument::OLD_BUNDLED:
              return DDJVU_DOCTYPE_OLD_BUNDLED;
            case DjVuDocument::OLD_INDEXED:
              return DDJVU_DOCTYPE_OLD_INDEXED;
            case DjVuDocument::BUNDLED:
              return DDJVU_DOCTYPE_BUNDLED;
            case DjVuDocument::INDIRECT:
              return DDJVU_DOCTYPE_INDIRECT;
            case DjVuDocument::SINGLE_PAGE:
              return DDJVU_DOCTYPE_SINGLEPAGE;
            default:
              break;
            }
        }
    }
  G_CATCH(ex)
    {
      ERROR(document,ex);
    }
  G_ENDCATCH;
  return DDJVU_DOCTYPE_UNKNOWN;
}

int
ddjvu_document_get_pagenum(ddjvu_document_t *document)
{
  G_TRY
    {
      DjVuDocument *doc = document->doc;
      if (doc)
        return doc->get_pages_num();
    }
  G_CATCH(ex)
    {
      ERROR(document,ex);
    }
  G_ENDCATCH;
  return 1;
}


// ----------------------------------------


ddjvu_page_t *
ddjvu_page_create_by_pageno(ddjvu_document_t *document,
                            int pageno)
{
  G_TRY
    {
      DjVuDocument *doc = document->doc;
      if (! doc) return 0;
      ddjvu_page_t *p = new ddjvu_page_s;
      ref(p);
      p->mydoc = document;
      p->img = doc->get_page(pageno, false, p);
      p->userdata = 0;
      p->pageinfoflag = false;
      return p;
    }
  G_CATCH(ex)
    {
      ERROR(document, ex);
    }
  G_ENDCATCH;
  return 0;
}

ddjvu_page_t *
ddjvu_page_create_by_pageid(ddjvu_document_t *document,
                            const char *pageid)
{
  G_TRY
    {
      DjVuDocument *doc = document->doc;
      if (! doc) return 0;
      ddjvu_page_t *p = new ddjvu_page_s;
      ref(p);
      p->mydoc = document;
      p->img = doc->get_page(GNativeString(pageid), false, p);
      p->userdata = 0;
      p->pageinfoflag = false;
      return p;
    }
  G_CATCH(ex)
    {
      ERROR(document, ex);
    }
  G_ENDCATCH;
  return 0;
}

void
ddjvu_page_release(ddjvu_page_t *page)
{
  G_TRY
    {
      if (page)
        {
          page->img = 0;
          unref(page);
        }
    }
  G_CATCH(ex)
    {
    }
  G_ENDCATCH;
}


// ----------------------------------------


bool
ddjvu_page_s::inherits(const GUTF8String &classname)
{
  return (classname == "ddjvu_page_s");
}

bool 
ddjvu_page_s::notify_error(const DjVuPort *, const GUTF8String &m)
{
  if (!img) return 0;
  msg_push(xhead(DDJVU_ERROR, this), msg_prep_error(m));
  return 1;
}
 
bool 
ddjvu_page_s::notify_status(const DjVuPort *p, const GUTF8String &m)
{
  if (!img) return 0;
  msg_push(xhead(DDJVU_STATUS, this), msg_prep_status(m));
  return 1;
}

void 
ddjvu_page_s::notify_file_flags_changed(const DjVuFile*, long, long)
{
  if (pageinfoflag || !img) return;
  DjVuFile *file = img->get_djvu_file();
  if (!file) return;
  long flags = file->get_flags();
  if ((flags && DjVuFile::DECODE_OK) ||
      (flags && DjVuFile::DECODE_FAILED) ||
      (flags && DjVuFile::DECODE_STOPPED) )
    {
      msg_push(xhead(DDJVU_PAGEINFO, this));
      pageinfoflag = true;
    }
}

void 
ddjvu_page_s::notify_relayout(const DjVuImage *dimg)
{
  if (img!=dimg || !img) return;
  ddjvu_context_t *ctx = mydoc->myctx;
  ctx->monitor.enter();
  bool needpageinfo = !pageinfoflag;
  pageinfoflag = true;
  msg_remove(ctx, DDJVU_RELAYOUT, this);
  msg_remove(ctx, DDJVU_REDISPLAY, this);
  if (needpageinfo) msg_push(xhead(DDJVU_PAGEINFO, this));
  msg_push(xhead(DDJVU_RELAYOUT, this));
  ctx->monitor.leave();
}

void 
ddjvu_page_s::notify_redisplay(const DjVuImage *dimg)
{
  if (img!=dimg || !img) return;
  ddjvu_context_t *ctx = mydoc->myctx;
  ctx->monitor.enter();
  msg_remove(ctx, DDJVU_REDISPLAY, this);
  msg_push(xhead(DDJVU_REDISPLAY, this));
  ctx->monitor.leave();
}

void 
ddjvu_page_s::notify_chunk_done(const DjVuPort*, const GUTF8String &name)
{
  if (! img) return;
  GP<ddjvu_message_p> p = new ddjvu_message_p;
  p->tmp1 = name;
  p->p.m_chunk.chunkid = (const char*)(p->tmp1);
  msg_push(xhead(DDJVU_CHUNK,this), p);
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
ddjvu_page_decoding_status(ddjvu_page_t *page)
{
  G_TRY
    {
      if (! page->img)
        return DDJVU_DECODE_NOTSTARTED;        
      DjVuFile *file = page->img->get_djvu_file();
      if (! file)
        return DDJVU_DECODE_NOTSTARTED;
      else if (file->is_decode_stopped())
        return DDJVU_DECODE_STOPPED;
      else if (file->is_decode_failed())
        return DDJVU_DECODE_FAILED;
      else if (file->is_decode_ok())
        return DDJVU_DECODE_OK;
      else if (file->is_decoding())
        return DDJVU_DECODE_STARTED;
    }
  G_CATCH(ex)
    {
      ERROR(page, ex);
    }
  G_ENDCATCH;
  return DDJVU_DECODE_NOTSTARTED;
}

int
ddjvu_page_get_width(ddjvu_page_t *page)
{
  G_TRY
    {
      if (page->img)
        return page->img->get_width();
    }
  G_CATCH(ex)
    {
      ERROR(page, ex);
    }
  G_ENDCATCH;
  return 0;
}

int
ddjvu_page_get_height(ddjvu_page_t *page)
{
  G_TRY
    {
      if (page->img)
        return page->img->get_height();
    }
  G_CATCH(ex)
    {
      ERROR(page, ex);
    }
  G_ENDCATCH;
  return 0;
}

int
ddjvu_page_get_resolution(ddjvu_page_t *page)
{
  G_TRY
    {
      if (page->img)
        return page->img->get_dpi();
    }
  G_CATCH(ex)
    {
      ERROR(page, ex);
    }
  G_ENDCATCH;
  return 0;
}

double
ddjvu_page_get_gamma(ddjvu_page_t *page)
{
  G_TRY
    {
      if (page->img)
        return page->img->get_gamma();
    }
  G_CATCH(ex)
    {
      ERROR(page, ex);
    }
  G_ENDCATCH;
  return 2.2;
}

int
ddjvu_page_get_version(ddjvu_page_t *page)
{
  G_TRY
    {
      if (page->img)
        return page->img->get_version();
    }
  G_CATCH(ex)
    {
      ERROR(page, ex);
    }
  G_ENDCATCH;
  return DJVUVERSION;
}

ddjvu_page_type_t
ddjvu_page_get_type(ddjvu_page_t *page)
{
  G_TRY
    {
      if (! page->img)
        return DDJVU_PAGETYPE_UNKNOWN;
      else if (page->img->is_legal_bilevel())
        return DDJVU_PAGETYPE_BITONAL;
      else if (page->img->is_legal_photo())
        return DDJVU_PAGETYPE_PHOTO;
      else if (page->img->is_legal_compound())
        return DDJVU_PAGETYPE_COMPOUND;
    }
  G_CATCH(ex)
    {
      ERROR(page, ex);
    }
  G_ENDCATCH;
  return DDJVU_PAGETYPE_UNKNOWN;
}

char *
ddjvu_page_get_short_description(ddjvu_page_t *page)
{
  G_TRY
    {
      if (page && page->img)
        return xstr(page->img->get_short_description());
    }
  G_CATCH(ex)
    {
      ERROR(page, ex);
    }
  G_ENDCATCH;
  return 0;
}

char *
ddjvu_page_get_long_description(ddjvu_page_t *page)
{
  G_TRY
    {
      if (page && page->img)
        return xstr(page->img->get_long_description());
    }
  G_CATCH(ex)
    {
      ERROR(page, ex);
    }
  G_ENDCATCH;
  return 0;
}


// ----------------------------------------


ddjvu_page_rotation_t
ddjvu_page_get_rotation(ddjvu_page_t *page)
{
  ddjvu_page_rotation_t rot = DDJVU_ROTATE_0;
  G_TRY
    {
      if (page->img)
        rot = (ddjvu_page_rotation_t)page->img->get_rotate();
    }
  G_CATCH(ex)
    {
      ERROR(page, ex);
    }
  G_ENDCATCH;
  return rot;
}

void
ddjvu_page_set_rotation(ddjvu_page_t *page,
                        ddjvu_page_rotation_t rot)
{
  G_TRY
    {
      switch(rot)
        {
        case DDJVU_ROTATE_0:
        case DDJVU_ROTATE_90:
        case DDJVU_ROTATE_180:
        case DDJVU_ROTATE_270:
          if (page->img)
            page->img->set_rotate((int)rot);
          break;
        default:
          G_THROW("Illegal ddjvu rotation code");
          break;
        }
    }
  G_CATCH(ex)
    {
      ERROR(page, ex);
    }
  G_ENDCATCH;
}


// ----------------------------------------


struct ddjvu_format_s
{
  int rdata[256];
  int gdata[256];
  int bdata[256];
  int palette[6*6*6];
  int pixelsize;
  bool toptobottom;
  bool haspalette;
  bool lsbtomsb;
  bool msbtolsb;
  double gamma;
  ddjvu_format_s() { 
    memset(this, 0, sizeof(ddjvu_format_s)); 
    gamma=2.2; 
  }
};

ddjvu_format_t *
ddjvu_format_create_truecolor(int pixelsize,
                              unsigned long rmask,
                              unsigned long gmask,
                              unsigned long bmask,
                              int toptobottom)
{
  ddjvu_format_t *fmt = new ddjvu_format_s;
  fmt->pixelsize = pixelsize;
  fmt->toptobottom = !!toptobottom;
  fmt->haspalette = false;
  fmt->msbtolsb = false;
  fmt->lsbtomsb = false;
  for (int i=0; i<255; i++)
    {
      fmt->rdata[i] = rmask & (int)(( rmask * i + 127.0 ) / 255.0);
      fmt->gdata[i] = gmask & (int)(( gmask * i + 127.0 ) / 255.0);
      fmt->bdata[i] = bmask & (int)(( bmask * i + 127.0 ) / 255.0);
    }
  return fmt;
}

ddjvu_format_t *
ddjvu_format_create_palette(int pixelsize,
                            unsigned long palette[6*6*6],
                            int toptobottom)
{
  ddjvu_format_t *fmt = new ddjvu_format_s;
  fmt->pixelsize = pixelsize;
  fmt->toptobottom = !!toptobottom;
  fmt->haspalette = true;
  fmt->msbtolsb = false;
  fmt->lsbtomsb = false;
  for(int i=0; i<6; i++)
    for(int j=0; j<(i+1)*0x33 && j<256; j++)
      {
        fmt->rdata[j] = i * 6 * 6;
        fmt->gdata[j] = i * 6;
        fmt->bdata[j] = i;
    }
  for (int k=0; k<6*6*6; k++)
    fmt->palette[k] = palette[k];
  return fmt;
}

ddjvu_format_t *
ddjvu_format_create_graylevel(int pixelsize,
                              int pixelwhite,
                              int pixelblack,
                              int toptobottom)
{
  ddjvu_format_t *fmt = new ddjvu_format_s;
  fmt->pixelsize = pixelsize;
  fmt->toptobottom = !!toptobottom;
  fmt->haspalette = false;
  fmt->msbtolsb = false;
  fmt->lsbtomsb = false;
  int r = pixelwhite - pixelblack;
  for (int i=0; i<255; i++)
    {
      int g = pixelblack + (int)((r * i + 127.0) /  255.0);
      fmt->rdata[i] = 5 * g / 16;
      fmt->gdata[i] = 9 * g / 16;
      fmt->bdata[i] = g - fmt->rdata[i] - fmt->gdata[i];
    }
  return 0;
}

ddjvu_format_t *
ddjvu_format_create_bitonal(int lsbtomsb,
                            int toptobottom)
{
  ddjvu_format_t *fmt = new ddjvu_format_s;
  fmt->pixelsize = 0;
  fmt->toptobottom = !!toptobottom;
  fmt->haspalette = false;
  fmt->msbtolsb = !lsbtomsb;
  fmt->lsbtomsb = !!lsbtomsb;
  return 0;
}

void
ddjvu_format_set_gamma(ddjvu_format_t *format,
                       double gamma)
{
  if (gamma>=0.5 && gamma<=5.0)
    format->gamma = gamma;
}

void
ddjvu_format_release(ddjvu_format_t *format)
{
  delete format;
}


// ----------------------------------------


int
ddjvu_page_render(ddjvu_page_t *page,
                  const ddjvu_format_t *fmt,
                  const ddjvu_rect_t *prect,
                  const ddjvu_rect_t *rrect,
                  char *img,
                  unsigned long rowsize)
{
  return 0;
}


// ----------------------------------------

