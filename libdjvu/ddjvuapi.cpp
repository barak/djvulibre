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

#ifdef HAVE_NAMESPACES
namespace DJVU {
  struct ddjvu_context_s;
  struct ddjvu_document_s;
  struct ddjvu_page_s;
  struct ddjvu_format_s;
  struct ddjvu_message_p;
}
using namespace DJVU;
# define DJVUNS DJVU::
#else
# define DJVUNS /**/
#endif

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
#include "IW44Image.h"
#include "DjVuImage.h"
#include "DjVuFileCache.h"
#include "DjVuDocument.h"
#include "DjVuMessageLite.h"

#if HAVE_STDINT_H
# include <stdint.h>
#else
typedef unsigned short uint16_t
typedef unsigned int uint32_t
#endif


// ----------------------------------------


struct DJVUNS ddjvu_message_p : public GPEnabled
{
  GNativeString tmp1;
  GNativeString tmp2;
  ddjvu_message_t p;
  ddjvu_message_p() { memset(&p, 0, sizeof(p)); }
};


// ----------------------------------------


struct DJVUNS ddjvu_context_s : public GPEnabled
{
  GMonitor monitor;
  GP<DjVuFileCache> cache;
  GPList<ddjvu_message_p> mlist;
  ddjvu_message_callback_t callback;
};

struct DJVUNS ddjvu_document_s : public DjVuPort
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

struct DJVUNS ddjvu_page_s : public DjVuPort
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
static void 
ref(GPEnabled *p)
{
  GPBase n(p);
  char *gn = (char*)&n;
  *(GPEnabled**)gn = 0;
  n.assign(0);
}

// Hack to decrement counter
static void 
unref(GPEnabled *p)
{
  GPBase n;
  char *gn = (char*)&n;
  *(GPEnabled**)gn = p;
  n.assign(0);
}

// Allocate strings
static char *
xstr(const char *s)
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

// Allocate strings
static char *
xstr(const GNativeString &n)
{
  return xstr( (const char*) n );
}

// Allocate strings
static char *
xstr(const GUTF8String &u)
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


// post a new message
static void
msg_push(const ddjvu_message_any_t &head,
         GP<ddjvu_message_p> msg = 0)
{
  ddjvu_context_t *ctx = head.context;
  if (! msg) msg = new ddjvu_message_p;
  msg->p.m_any = head;
  int size = 0;
  {
    GMonitorLock lock(&ctx->monitor);
    ctx->mlist.append(msg);
    ctx->monitor.broadcast();
    size = ctx->mlist.size();
  }
  if (ctx->callback) 
    (ctx->callback)(ctx, size);
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
msg_prep_info(GUTF8String message)
{
  GP<ddjvu_message_p> p = new ddjvu_message_p;
  p->tmp1 = DjVuMessageLite::LookUpUTF8(message); // i18n nonsense!
  p->p.m_info.message = (const char*)(p->tmp1);
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
  msg_push(xhead(DDJVU_INFO, this), msg_prep_info(m));
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
      streamid += 1;
      {
        GMonitorLock lock(&monitor);        
        if (streamid > 0)
          streams[streamid] = pool = DataPool::create();
        else
          pool = streams[(streamid = 0)];
      }
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
          document->userdata = 0;
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
      GP<DataPool> pool;
      { 
        GMonitorLock lock(&doc->monitor); 
        pool = doc->streams[p];
      }
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
      GP<DataPool> pool;
      { 
        GMonitorLock lock(&doc->monitor); 
        pool = doc->streams[p];
      }
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


ddjvu_status_t
ddjvu_document_decoding_status(ddjvu_document_t *document)
{
  G_TRY
    {
      DjVuDocument *doc = document->doc;
      if (doc)
        {
          long flags = doc->get_doc_flags();
          if (flags & DjVuDocument::DOC_INIT_OK)
            return DDJVU_OPERATION_OK;
          else if (flags & DjVuDocument::DOC_INIT_FAILED)
            return DDJVU_OPERATION_FAILED;
          else
            return DDJVU_OPERATION_STARTED;
        }
    }
  G_CATCH(ex)
    {
      ERROR(document,ex);
    }
  G_ENDCATCH;
  return DDJVU_OPERATION_NOTSTARTED;
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
          page->userdata = 0;
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
  msg_push(xhead(DDJVU_INFO, this), msg_prep_info(m));
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
  if (! img) return;
  if (! pageinfoflag) msg_push(xhead(DDJVU_PAGEINFO, this));
  pageinfoflag = true;
  msg_push(xhead(DDJVU_RELAYOUT, this));
}

void 
ddjvu_page_s::notify_redisplay(const DjVuImage *dimg)
{
  if (! img) return;
  if (! pageinfoflag) msg_push(xhead(DDJVU_PAGEINFO, this));
  pageinfoflag = true;
  msg_push(xhead(DDJVU_REDISPLAY, this));
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


ddjvu_status_t
ddjvu_page_decoding_status(ddjvu_page_t *page)
{
  G_TRY
    {
      if (! page->img)
        return DDJVU_OPERATION_NOTSTARTED;        
      DjVuFile *file = page->img->get_djvu_file();
      if (! file)
        return DDJVU_OPERATION_NOTSTARTED;
      else if (file->is_decode_stopped())
        return DDJVU_OPERATION_STOPPED;
      else if (file->is_decode_failed())
        return DDJVU_OPERATION_FAILED;
      else if (file->is_decode_ok())
        return DDJVU_OPERATION_OK;
      else if (file->is_decoding())
        return DDJVU_OPERATION_STARTED;
    }
  G_CATCH(ex)
    {
      ERROR(page, ex);
    }
  G_ENDCATCH;
  return DDJVU_OPERATION_NOTSTARTED;
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


struct DJVUNS ddjvu_format_s
{
  ddjvu_format_style_t style;
  uint32_t rgb[3][256];
  uint32_t palette[6*6*6];
  double gamma;
  char ditherbits;
  bool toptobottom;
};

static ddjvu_format_t *
fmt_error(ddjvu_format_t *fmt)
{
  delete fmt;
  return 0;
}

ddjvu_format_t *
ddjvu_format_create(ddjvu_format_style_t style,
                    int nargs, unsigned int *args)
{
  ddjvu_format_t *fmt = new ddjvu_format_s;
  memset(fmt, 0, sizeof(ddjvu_format_t));
  fmt->style = style;  
  fmt->toptobottom = false;
  fmt->gamma = 2.2;
  // Ditherbits
  fmt->ditherbits = 32;
  if (style==DDJVU_FORMAT_RGBMASK16)
    fmt->ditherbits = 16;
  else if (style==DDJVU_FORMAT_PALETTE8)
    fmt->ditherbits = 8;
  else if (style==DDJVU_FORMAT_MSBTOLSB || style==DDJVU_FORMAT_LSBTOMSB)
    fmt->ditherbits = 1;
  // Args
  switch(style)
    {
    case DDJVU_FORMAT_RGBMASK16:
    case DDJVU_FORMAT_RGBMASK32: 
      {
        if (sizeof(uint16_t)!=2 || sizeof(uint32_t)!=4)
          return fmt_error(fmt);
        if (nargs!=3 || !args)
          return fmt_error(fmt);
        for (int j=0; j<3; j++)
          {
            int shift = 0;
            uint32_t mask = args[j];
            for (shift=0; shift<32 && !(mask & 1); shift++)
              mask >>= 1;
            if ((shift>=32) || (mask&(mask+1)))
              return fmt_error(fmt);
            for (int i=0; i<256; i++)
              fmt->rgb[j][i] = (mask & ((int)((i*mask+127.0)/255.0)))<<shift;
          }
        break;
      }
    case DDJVU_FORMAT_PALETTE8:
      {
        if (nargs!=6*6*6 || !args)
          return fmt_error(fmt);
        for (int k=0; k<6*6*6; k++)
          fmt->palette[k] = args[k];
        for(int i=0; i<6; i++)
          for(int j=0; j<(i+1)*0x33 && j<256; j++)
            {
              fmt->rgb[0][j] = i * 6 * 6;
              fmt->rgb[1][j] = i * 6;
              fmt->rgb[2][j] = i;
            }
        break;
      }
    case DDJVU_FORMAT_RGB24:
    case DDJVU_FORMAT_BGR24:
    case DDJVU_FORMAT_GREY8:
    case DDJVU_FORMAT_LSBTOMSB:
    case DDJVU_FORMAT_MSBTOLSB:
      if (!nargs) 
        break;
    default:
      return fmt_error(fmt);
    }
  return fmt;
}

void
ddjvu_format_set_row_order(ddjvu_format_t *format, int top_to_bottom)
{
  format->toptobottom = !! top_to_bottom;
}

void
ddjvu_format_set_ditherbits(ddjvu_format_t *format, int bits)
{
  if (bits>0 && bits<=64)
    format->ditherbits = bits;
}

void
ddjvu_format_set_gamma(ddjvu_format_t *format, double gamma)
{
  if (gamma>=0.5 && gamma<=5.0)
    format->gamma = gamma;
}

void
ddjvu_format_release(ddjvu_format_t *format)
{
  delete format;
}

static void
fmt_convert_row(const GPixel *p, int w, 
                const ddjvu_format_t *fmt, char *buf)
{
  const uint32_t (*r)[256] = fmt->rgb;
  switch(fmt->style)
    {
    case DDJVU_FORMAT_BGR24:    /* truecolor 24 bits in BGR order */
      {
        memcpy(buf, (const char*)p, 3*w);
        break;
      }
    case DDJVU_FORMAT_RGB24:    /* truecolor 24 bits in RGB order */
      { 
        while (--w >= 0) { 
          buf[0]=p->r; buf[1]=p->g; buf[2]=p->b; 
          buf+=3; p+=1; 
        }
        break;
      }
    case DDJVU_FORMAT_RGBMASK16: /* truecolor 16 bits with masks */
      {
        uint16_t *b = (uint16_t*)buf;
        while (--w >= 0) {
          b[0]=(r[0][p->b]+r[1][p->b]+r[2][p->b]); 
          b+=1; p+=1; 
        }
        break;
      }
    case DDJVU_FORMAT_RGBMASK32: /* truecolor 32 bits with masks */
      {
        uint32_t *b = (uint32_t*)buf;
        while (--w >= 0) {
          b[0]=(r[0][p->b]+r[1][p->b]+r[2][p->b]); 
          b+=1; p+=1; 
        }
        break;
      }
    case DDJVU_FORMAT_GREY8:    /* greylevel 8 bits */
      {
        while (--w >= 0) { 
          buf[0]=(5*p->b + 9*p->g + 2*p->b)>>4; 
          buf+=1; p+=1; 
        }
        break;
      }
    case DDJVU_FORMAT_PALETTE8: /* paletized 8 bits (6x6x6 color cube) */
      {
        const uint32_t *u = fmt->palette;
        while (--w >= 0) {
          buf[0] = u[r[0][p->b]+r[1][p->b]+r[2][p->b]]; 
          buf+=1; p+=1; 
        }
        break;
      }
    case DDJVU_FORMAT_MSBTOLSB: /* packed bits, msb on the left */
      {
        unsigned char s=0, m=0x80;
        while (--w >= 0) {
          if ( 5*p->b + 9*p->g + 2*p->b < 128*16 ) { m |= s; }
          if (! (m >>= 1)) { *buf++ = s; s=0; m=0x80; }
        }
        if (s < 0x80) { *buf++ = s; }
        break;
      }
    case DDJVU_FORMAT_LSBTOMSB: /* packed bits, lsb on the left */
      {
        unsigned char s=0, m=0x1;
        while (--w >= 0) {
          if ( 5*p->b + 9*p->g + 2*p->b < 128*16 ) { m |= s; }
          if (! (m <<= 1)) { *buf++ = s; s=0; m=0x1; }
        }
        if (s > 0x1) { *buf++ = s; }
        break;
      }
    }
}

static void
fmt_convert(GPixmap *pm, const ddjvu_format_t *fmt, char *buffer, int rowsize)
{
  int w = pm->columns();
  int h = pm->rows();
  // Loop on rows
  if (fmt->toptobottom)
    {
      for(int r=h-1; r>=0; r--, buffer+=rowsize)
        fmt_convert_row((*pm)[r], w, fmt, buffer);
    }
  else
    {
      for(int r=0; r<h; r++, buffer+=rowsize)
        fmt_convert_row((*pm)[r], w, fmt, buffer);
    }
}

static void
fmt_convert_row(unsigned char *p, unsigned char *g, int w, 
                const ddjvu_format_t *fmt, char *buf)
{
  const uint32_t (*r)[256] = fmt->rgb;
  switch(fmt->style)
    {
    case DDJVU_FORMAT_BGR24:    /* truecolor 24 bits in BGR order */
    case DDJVU_FORMAT_RGB24:    /* truecolor 24 bits in RGB order */
      { 
        while (--w >= 0) { 
          buf[0]=buf[1]=buf[2]=g[*p];
          buf+=3; p+=1; 
        }
        break;
      }
    case DDJVU_FORMAT_RGBMASK16: /* truecolor 16 bits with masks */
      {
        uint16_t *b = (uint16_t*)buf;
        while (--w >= 0) {
          unsigned char x = g[*p];
          b[0]=(r[0][x]+r[1][x]+r[2][x]); 
          b+=1; p+=1; 
        }
        break;
      }
    case DDJVU_FORMAT_RGBMASK32: /* truecolor 32 bits with masks */
      {
        uint32_t *b = (uint32_t*)buf;
        while (--w >= 0) {
          unsigned char x = g[*p];
          b[0]=(r[0][x]+r[1][x]+r[2][x]); 
          b+=1; p+=1; 
        }
        break;
      }
    case DDJVU_FORMAT_GREY8:    /* greylevel 8 bits */
      {
        while (--w >= 0) { 
          buf[0]=g[*p];
          buf+=1; p+=1; 
        }
        break;
      }
    case DDJVU_FORMAT_PALETTE8: /* paletized 8 bits (6x6x6 color cube) */
      {
        const uint32_t *u = fmt->palette;
        while (--w >= 0) {
          buf[0] = u[g[*p]*(1+6+36)];
          buf+=1; p+=1; 
        }
        break;
      }
    case DDJVU_FORMAT_MSBTOLSB: /* packed bits, msb on the left */
      {
        unsigned char s=0, m=0x80;
        while (--w >= 0) {
          if (g[*p] < 128) { m |= s; }
          if (! (m >>= 1)) { *buf++ = s; s=0; m=0x80; }
        }
        if (s < 0x80) { *buf++ = s; }
        break;
      }
    case DDJVU_FORMAT_LSBTOMSB: /* packed bits, lsb on the left */
      {
        unsigned char s=0, m=0x1;
        while (--w >= 0) {
          if (g[*p] < 128) { m |= s; }
          if (! (m <<= 1)) { *buf++ = s; s=0; m=0x1; }
        }
        if (s > 0x1) { *buf++ = s; }
        break;
      }
    }
}

static void
fmt_convert(GBitmap *bm, const ddjvu_format_t *fmt, char *buffer, int rowsize)
{
  int w = bm->columns();
  int h = bm->rows();
  int m = bm->get_grays();
  // Gray levels
  int i;
  unsigned char g[256];
  for (i=0; i<m; i++)
    g[i] = 255 - ( i * 255 + (m - 1)/2 ) / (m - 1);
  for (i=m; i<256; i++)
    g[i] = 0;
  // Loop on rows
  if (fmt->toptobottom)
    {
      for(int r=h-1; r>=0; r--, buffer+=rowsize)
        fmt_convert_row((*bm)[r], g, w, fmt, buffer);
    }
  else
    {
      for(int r=0; r<h; r++, buffer+=rowsize)
        fmt_convert_row((*bm)[r], g, w, fmt, buffer);
    }
}

static void
fmt_dither(GPixmap *pm, const ddjvu_format_t *fmt, int x, int y)
{
  if (fmt->ditherbits < 15)
    pm->ordered_666_dither(x, y);
  else if (fmt->ditherbits < 24)
    pm->ordered_32k_dither(x, y);
}


// ----------------------------------------

static void
rect2rect(const ddjvu_rect_t *r, GRect &g)
{
  g.xmin = r->x;
  g.xmax = g.xmin + r->w;
  g.ymin = r->x;
  g.ymax = g.ymin + r->h;
}

int
ddjvu_page_render(ddjvu_page_t *page,
                  const ddjvu_render_mode_t mode,
                  const ddjvu_rect_t *pagerect,
                  const ddjvu_rect_t *renderrect,
                  const ddjvu_format_t *pixelformat,
                  unsigned long rowsize,
                  char *imagebuffer )
{
  G_TRY
    {
      GP<GPixmap> pm;
      GP<GBitmap> bm;
      GRect prect;
      GRect rrect;
      rect2rect(pagerect, prect);
      rect2rect(renderrect, rrect);
      DjVuImage *img = page->img;
      if (img) 
        {
          switch (mode)
            {
            case DDJVU_RENDER_DEFAULT:
              if (pixelformat->ditherbits < 8)
                pm = img->get_pixmap(rrect, prect, pixelformat->gamma);
              if (! pm) 
                bm = img->get_bitmap(rrect, prect);
              break;
            case DDJVU_RENDER_COLORONLY:
              pm = img->get_pixmap(rrect, prect, pixelformat->gamma);
              break;
            case DDJVU_RENDER_BACKGROUND:
              pm = img->get_bg_pixmap(rrect, prect, pixelformat->gamma);
              break;
            case DDJVU_RENDER_FOREGROUND:
              pm = img->get_fg_pixmap(rrect, prect, pixelformat->gamma);
              break;
            case DDJVU_RENDER_MASK:
              bm = img->get_bitmap(rrect, prect);
              break;
            }
        }
      if (pm)
        {
          int dx = rrect.xmin - prect.xmin;
          int dy = rrect.ymin - prect.xmin;
          fmt_dither(pm, pixelformat, dx, dy);
          fmt_convert(pm, pixelformat, imagebuffer, rowsize);
          return 2;
        }
      else if (bm)
        {
          fmt_convert(bm, pixelformat, imagebuffer, rowsize);
          return 1;
        }
    }
  G_CATCH(ex)
    {
      ERROR(page, ex);
    }
  G_ENDCATCH;
  return 0;
}


// ----------------------------------------


ddjvu_status_t
ddjvu_thumbnail_status(ddjvu_document_t *document, int pagenum, int start)
{
  return DDJVU_OPERATION_NOTSTARTED;
}

int
ddjvu_thumbnail_render(ddjvu_document_t *document, int pagenum, 
                       int *wptr, int *hptr,
                       const ddjvu_format_t *pixelformat,
                       unsigned long rowsize,
                       char *imagebuffer)
{
  return 0;
}
