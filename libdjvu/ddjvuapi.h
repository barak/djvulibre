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

#ifndef DDJVUAPI_H
#define DDJVUAPI_H

#ifdef __cplusplus
extern "C" { 
#endif
#if 0
}
#endif
#ifndef DDJVUAPI
# define DDJVUAPI /**/
#endif
#ifndef TRUE
# define TRUE  (1)
#endif
#ifndef FALSE
# define FALSE (0)
#endif


/* -------------------------------------------------- */
/* DDJVU API                                          */
/* -------------------------------------------------- */

/* The DDJVU API provides for efficiently decoding and
   displaying DjVu documents.  It provides for displaying
   images without waiting for the complete DjVu data. Images
   can be displayed as soon as sufficient data is
   available. A higher quality image might later be
   displayed when further data is available.  The DjVu
   library achieves this using a complicated scheme
   involving multiple threads. The DDJVU API hides this
   complexity with a familiar event model.
*/

#define DDJVUAPI_VERSION 14

typedef struct ddjvu_context_s  ddjvu_context_t;
typedef union  ddjvu_message_s  ddjvu_message_t;
typedef struct ddjvu_job_s      ddjvu_job_t;
typedef struct ddjvu_document_s ddjvu_document_t;
typedef struct ddjvu_page_s     ddjvu_page_t;
typedef struct ddjvu_format_s   ddjvu_format_t;


/* GENERAL CONVENTIONS:
   Unless specified otherwise:
   - all strings use locale encoding,
   - all filenames are unencoded byte strings,
   - all errors are signaled with error event messages,
   - all functions returning a pointer might return 
     a null pointer (this usually indicates an error 
     condition).

   HEADER:
   Always use the following idiom to include this file.

     #include <libdjvu/ddjvuapi.h>

   This file does not declare functions ddjvu_get_DjVuImage() 
   and djvu_get_DjVuDocument() unless you include files 
   "DjVuImage.h" and "DjVuDocument.h" before this file.

   PREREQUISITES:
   - Please read the djvu man page: <"tools/djvu.1">.
   - Please browse the file format specifications 
     <"doc/djvu3changes.txt"> and <"doc/djvu2spec.djvu">.
*/

  

/* -------------------------------------------------- */
/* DDJVU_CONTEXT_T                                    */
/* -------------------------------------------------- */

/* There is usually only one <ddjvu_context_t> object.  
   This object holds global data structures such as the 
   cache of decoded pages, or the list of pending 
   event messages.
 */


/* ddjvu_context_create ---
   Creates a <ddjvu_context_t> object.
   Argument <programname> is the name of the calling executable. */

DDJVUAPI ddjvu_context_t *
ddjvu_context_create(const char *programname);


/* ddjvu_context_release ---
   Release a reference to a <ddjvu_context_t> object.
   The calling program should no longer reference this object.
   The object itself will be destroyed as soon as no other object
   or thread needs it. */

DDJVUAPI void 
ddjvu_context_release(ddjvu_context_t *context);



/* ------- CACHE ------- */

/* ddjvu_cache_set_size ---
   Sets the maximum size of the cache of decoded page data.
   The argument is expressed in bytes. */

DDJVUAPI void
ddjvu_cache_set_size(ddjvu_context_t *context,
                     unsigned long cachesize);


/* ddjvu_cache_get_size ---
   Returns the maximum size of the cache. */

DDJVUAPI unsigned long
ddjvu_cache_get_size(ddjvu_context_t *context);


/* ddjvu_cache_clear ---
   Clears all cached data. */

DDJVUAPI void
ddjvu_cache_clear(ddjvu_context_t *context);



/* ------- MESSAGE QUEUE ------- */

/* ddjvu_message_peek ---
   Returns a pointer to the next DDJVU message.
   This function returns 0 if no message is available.
   It does not remove the message from the queue. */

DDJVUAPI ddjvu_message_t *
ddjvu_message_peek(ddjvu_context_t *context);


/* ddjvu_message_wait ---
   Returns a pointer to the next DDJVU message.
   This function waits until a message is available.
   It does not remove the message from the queue. */

DDJVUAPI ddjvu_message_t *
ddjvu_message_wait(ddjvu_context_t *context);


/* ddjvu_message_pop ---
   Removes one message from the queue.
   This function must be called after processing the message.
   Pointers returned by previous calls to <ddjvu_message_peek> 
   or <ddjvu_message_wait> are no longer valid after 
   calling <ddjvu_message_pop>. */

DDJVUAPI void
ddjvu_message_pop(ddjvu_context_t *context);


/* ddjvu_message_set_callback ---
   Defines a callback function invoked whenever
   a new message is posted to the ddjvuapi message queue,
   and returns a pointer to the previous callback function.
   This callback function can be called at any time
   while other code is executing. It should simply signal
   the main application event loop that new ddjvuapi messages
   are available.  Under WIN32, this is usually achieved
   by posting a user window message.  Under UNIX, this is
   usually achieved using a pipe: the callback writes 
   a single byte into the pipe; the main application loop
   monitors the reading end of the pipe and detects
   the presence of data. */

typedef void 
(*ddjvu_message_callback_t)(ddjvu_context_t *context, void *closure);

DDJVUAPI void
ddjvu_message_set_callback(ddjvu_context_t *context,
                           ddjvu_message_callback_t callback,
                           void *closure);



/* -------------------------------------------------- */
/* DDJVU_JOB_T                                        */
/* -------------------------------------------------- */


/* Many essential ddjvuapi functions initiate asynchronous operations. 
   These "jobs" run in seperate threads and report their
   progress by posting messages into the ddjvu context event queue. 
   Jobs are sometimes represented by a ddjvu_job_t object. */

/* ddjvu_job_status ---
   Returns the status of the specified job. */

typedef enum {
  DDJVU_JOB_NOTSTARTED, /* operation was not even started */
  DDJVU_JOB_STARTED,    /* operation is in progress */
  DDJVU_JOB_OK,         /* operation terminated successfully */
  DDJVU_JOB_FAILED,     /* operation failed because of an error */
  DDJVU_JOB_STOPPED     /* operation was interrupted by user */
} ddjvu_status_t;

DDJVUAPI ddjvu_status_t
ddjvu_job_status(ddjvu_job_t *job);

#define ddjvu_job_done(job) \
    (ddjvu_job_status(job) >= DDJVU_JOB_OK)
#define ddjvu_job_error(job) \
    (ddjvu_job_status(job) >= DDJVU_JOB_FAILED)


/* ddjvu_job_stop ---
   Attempts to cancel the specified job.
   This is a best effort function. 
   There no guarantee that the job will 
   actually stop.
 */

DDJVUAPI void
ddjvu_job_stop(ddjvu_job_t *job);


/* ddjvu_job_set_user_data ---
   ddjvu_job_get_user_data ---
   Each job can store an arbitray pointer
   that callers can use for any purpose. These two functions
   provide for accessing or setting this pointer. */

DDJVUAPI void
ddjvu_job_set_user_data(ddjvu_job_t *job, void *userdata);

DDJVUAPI void *
ddjvu_job_get_user_data(ddjvu_job_t *job);


/* ddjvu_job_release ---
   Releases a reference to a job object.
   This will not cause the job to stop executing.
   The calling program should no longer reference this object.
   The object itself will be destroyed as soon as no other object
   or thread needs it. */

DDJVUAPI void
ddjvu_job_release(ddjvu_job_t *job);



/* -------------------------------------------------- */
/* DDJVU_MESSAGE_T                                    */
/* -------------------------------------------------- */


/* ddjvu_message_t ---
   This union type represents messages delivered by the
   DDJVU API. Each member of the union pertains to a
   specific kind of message.  Member <m_any> represents the
   information common to all kinds of messages.  Given a
   pointer <p> to a <djvu_message_t>, the message kind can
   be accessed as <"p->m_any.tag">. */


/* ddjvu_message_tag_t ---
   This enumerated type identifies each kind of 
   message delivered by the DDJVU API.  */

typedef enum {
  DDJVU_ERROR,
  DDJVU_INFO,
  DDJVU_NEWSTREAM,
  DDJVU_DOCINFO,
  DDJVU_PAGEINFO,
  DDJVU_RELAYOUT,
  DDJVU_REDISPLAY,
  DDJVU_CHUNK,
  DDJVU_THUMBNAIL,
  DDJVU_PROGRESS,
} ddjvu_message_tag_t;


/* ddjvu_message_t::m_any ---
   This structure is a member of the union <djvu_message_t>.
   It represents the information common to all kinds of
   messages.  Member <tag> indicates the kind of message.
   Members <context>, <document>, <page>, and <job> indicate 
   the origin of the message.  These fields contain null
   pointers when they are not relevant. */

typedef struct ddjvu_message_any_s {
  ddjvu_message_tag_t   tag;
  ddjvu_context_t      *context;
  ddjvu_document_t     *document;
  ddjvu_page_t         *page;
  ddjvu_job_t          *job;
} ddjvu_message_any_t; 


/* ddjvu_message_t::m_error ---
   Error messages are generated whenever the decoder or the
   DDJVU API encounters an error condition.  All errors are
   reported as error messages because they can occur
   asynchronously.  Member <message> is the error message.
   Members <function>, <filename> and <lineno>
   indicates the place where the error was detected.
*/

struct ddjvu_message_error_s {  /* ddjvu_message_t::m_error */
  ddjvu_message_any_t   any;
  const char           *message;
  const char           *function;
  const char           *filename;
  int                   lineno;
}; 


/* ddjvu_message_t::m_info ---
   This messages provides informational text indicating
   the progress of the decoding process. This might
   be displayed in the browser status bar. */

struct ddjvu_message_info_s {   /* ddjvu_message_t::m_info */
  ddjvu_message_any_t  any;
  const char          *message;
}; 




/* -------------------------------------------------- */
/* DDJVU_DOCUMENT_T                                    */
/* -------------------------------------------------- */


/* ddjvu_document_create ---
   Creates a decoder for a DjVu document and starts
   decoding.  This function returns immediately.  The
   decoding job then generates messages to request the raw
   data and to indicate the state of the decoding process.

   Argument <url> specifies an optional URL for the document.  
   The URL follows the usual syntax (<"protocol://machine/path">). 
   It only serves two purposes:
   - The URL is used as a key for the cache of decoded pages.
   - The URL is used to document <m_newstream> messages.

   Setting argument <cache> to <TRUE> indicates that decoded pages
   should be cached when possible.  This only works when
   argument <url> is not the null pointer.

   It is important to understand that the URL is not used to
   access the data.  The document generates <m_newstream>
   messages to indicate which data is needed.  The caller must 
   then provide the raw data using <ddjvu_stream_write> 
   and <ddjvu_stream_close>.

   Localized characters in argument <url> should be in 
   urlencoded utf-8 (like "%2A"). What is happening for non 
   ascii characters is unclear (probably plain utf8). */

DDJVUAPI ddjvu_document_t *
ddjvu_document_create(ddjvu_context_t *context,
                      const char *url,
                      int cache);

/* ddjvu_document_create_from_file ---
   Creates a document for a DjVu document stored in a file.
   The document will directly access the specified DjVu file 
   or related files without generating <m_newstream> messages. */

DDJVUAPI ddjvu_document_t *
ddjvu_document_create_by_filename(ddjvu_context_t *context,
                                  const char *filename,
                                  int cache);


/* ddjvu_document_job ---
   Access the job object in charge of decoding the document header. 
   In fact <ddjvu_document_t> is a subclass of <ddjvu_job_t>
   and this function is a type cast. */

DDJVUAPI ddjvu_job_t *
ddjvu_document_job(ddjvu_document_t *document);


/* ddjvu_document_release ---
   Release a reference to a <ddjvu_document_t> object.
   The calling program should no longer reference this object.  
   The object itself will be destroyed as soon as no other object
   or thread needs it. */
 
#define ddjvu_document_release(document) \
   ddjvu_job_release(ddjvu_document_job(document))


/* ddjvu_document_set_user_data ---
   ddjvu_document_get_user_data ---
   Each <ddjvu_document_t> object can store an arbitray pointer
   that callers can use for any purpose. These two functions
   provide for accessing or setting this pointer. */

#define ddjvu_document_set_user_data(document,userdata) \
   ddjvu_job_set_user_data(ddjvu_document_job(document),userdata)
#define ddjvu_document_get_user_data(document) \
   ddjvu_job_get_user_data(ddjvu_document_job(document))

/* ddjvu_document_decoding_status ---
   ddjvu_document_decoding_done, ddjvu_document_decoding_error ---
   This function returns the status of the document header decoding job */
   
#define ddjvu_document_decoding_status(document) \
   ddjvu_job_status(ddjvu_document_job(document))
#define ddjvu_document_decoding_done(page) \
    (ddjvu_document_decoding_status(page) >= DDJVU_JOB_OK)
#define ddjvu_document_decoding_error(page) \
    (ddjvu_document_decoding_status(page) >= DDJVU_JOB_FAILED)


/* ------- STREAMS ------- */


/* ddjvu_message_t::m_newstream --- 
   Newstream messages are generated whenever the decoder
   needs to access raw DjVu data.  The caller must then
   provide the requested data using <ddjvu_stream_write> 
   and <ddjvu_stream_close>. 

   In the case of indirect documents, a single decoder 
   might simultaneously request several streams of data.  
   Each stream is identified by a small integer <streamid>.

   The first <m_newstream> message always has member
   <streamid> set to zero and member <name> set to the null
   pointer.  It indicates that the decoder needs to access
   the data in the main DjVu file.  In fact, data can be
   written to stream <0> as soon as the <ddjvu_document_t>
   object is created.

   Further <m_newstream> messages are generated to access
   the auxilliary files of indirect or indexed DjVu
   documents.  Member <name> then provides the basename of
   the auxilliary file.

   Member <url> is set according to the url argument
   provided to function <ddjvu_document_create>.  The first
   newstream message always contain the url passed to
   <ddjvu_document_create>.  Subsequent newstream messages
   contain the url of the auxilliary files for indirect or
   indexed DjVu documents. */
   
struct ddjvu_message_newstream_s { /* ddjvu_message_t::m_newstream */
  ddjvu_message_any_t  any;
  int                  streamid;
  const char          *name;
  const char          *url;
}; 


/* ddjvu_stream_write ---
   Provide raw data to the DjVu decoder.
   This function should be called as soon as the data is available,
   for instance when receiving DjVu data from a network connection.
 */

DDJVUAPI void
ddjvu_stream_write(ddjvu_document_t *document,
                   int streamid,
                   const char *data,
                   unsigned long datalen );

/* ddjvu_stream_close ---
   Indicates that no more data will be provided on a
   particular stream.  Argument <stop> most likely should be
   set to <FALSE>. Setting argument <stop> to <TRUE>
   indicates that the user has interrupted the data transfer
   (for instance by pressing the stop button of a browser)
   and that the decoding threads should be stopped as 
   soon as feasible. */

DDJVUAPI void
ddjvu_stream_close(ddjvu_document_t *document,
                   int streamid,
                   int stop );



/* ------- QUERIES ------- */


/* ddjvu_message_t::m_docinfo ---
   The <m_docinfo> message indicates that basic information
   about the document has been obtained and decoded.
   Not much can be done before this happens.
 */

struct ddjvu_message_docinfo_s {
  ddjvu_message_any_t  any;
};


/* ddjvu_document_get_type ---
   Returns the type of a DjVu document.
   This function might return <DDJVU_DOCTYPE_UNKNOWN>
   when called before receiving a <m_docinfo> message. */

typedef enum {
  DDJVU_DOCTYPE_UNKNOWN=0,
  DDJVU_DOCTYPE_SINGLEPAGE,
  DDJVU_DOCTYPE_BUNDLED, 
  DDJVU_DOCTYPE_INDIRECT,
  DDJVU_DOCTYPE_OLD_BUNDLED, /* obsolete */
  DDJVU_DOCTYPE_OLD_INDEXED, /* obsolete */
} ddjvu_document_type_t;

DDJVUAPI ddjvu_document_type_t
ddjvu_document_get_type(ddjvu_document_t *document);


/* ddjvu_document_get_pagenum ---
   Returns the number of pages in a DjVu document.
   This function might return 1 when called 
   before receiving a <m_docinfo> message */
   
DDJVUAPI int
ddjvu_document_get_pagenum(ddjvu_document_t *document);




/* -------------------------------------------------- */
/* DJVU_PAGE_T                                        */
/* -------------------------------------------------- */


/* ddjvu_page_create_by_pageno ---
   Each page of a document can be accessed by creating a
   <ddjvu_page_t> object with this function.  Argument
   <pageno> indicates the page number, starting with page
   <0> to <pagenum-1>. This function can be called
   immediately after creating the <ddjvu_document_t> object.
   It also initiates the data transfer and the decoding threads 
   for the specified page.  Various messages will document
   the progress of these operations. Error messages will be
   generated if the page does not exists. */

DDJVUAPI ddjvu_page_t *
ddjvu_page_create_by_pageno(ddjvu_document_t *document,
                            int pageno);

/* ddjvu_page_create_by_pageid ---
   This function is similar to <ddjvu_page_create_by_pageno>
   but identifies the desired page by name instead of page
   number. */

DDJVUAPI ddjvu_page_t *
ddjvu_page_create_by_pageid(ddjvu_document_t *document,
                            const char *pageid);


/* ddjvu_page_job ---
   Access the job object in charge of decoding the document header. 
   In fact <ddjvu_page_t> is a subclass of <ddjvu_job_t>
   and this function is a type cast. */

DDJVUAPI ddjvu_job_t *
ddjvu_page_job(ddjvu_page_t *page);


/* ddjvu_page_release ---
   Release a reference to a <ddjvu_page_t> object.
   The calling program should no longer reference this object.
   The object itself will be destroyed as soon as no other object
   or thread needs it. */

#define ddjvu_page_release(page) \
  ddjvu_job_release(ddjvu_page_job(page))


/* ddjvu_page_set_user_data ---
   ddjvu_page_get_user_data ---
   Each <ddjvu_paqge_t> object can store an arbitray pointer
   that callers can use for any purpose. These two functions
   provide for accessing or setting this pointer. */

#define ddjvu_page_set_user_data(page,userdata) \
   ddjvu_job_set_user_data(ddjvu_page_job(page),userdata)
#define ddjvu_page_get_user_data(page) \
   ddjvu_job_get_user_data(ddjvu_page_job(page))

/* ddjvu_page_decoding_status ---
   ddjvu_page_decoding_done ---
   ddjvu_page_decoding_error ---
   These calls return the status of the page decoding job. */
   
#define ddjvu_page_decoding_status(page) \
   ddjvu_job_status(ddjvu_page_job(page))
#define ddjvu_page_decoding_done(page) \
    (ddjvu_page_decoding_status(page) >= DDJVU_JOB_OK)
#define ddjvu_page_decoding_error(page) \
    (ddjvu_page_decoding_status(page) >= DDJVU_JOB_FAILED)


/* ------- MESSAGES ------- */


/* ddjvu_message_t::m_pageinfo ---
   This message is generated when the basic 
   information about a page is available. */

struct ddjvu_message_pageinfo_s {  /* ddjvu_message_t::m_pageinfo */
  ddjvu_message_any_t  any;
}; 


/* ddjvu_message_t::m_relayout ---
   This message is generated when a DjVu viewer
   should recompute the layout of the page viewer
   because the page size and resolution information has
   been updated. */

struct ddjvu_message_relayout_s {  /* ddjvu_message_t::m_relayout */
  ddjvu_message_any_t  any;
}; 


/* ddjvu_message_t::m_redisplay ---
   This message is generated when a DjVu viewer
   should call <ddjvu_page_render> and redisplay
   the page. This happens, for instance, when newly 
   decoded DjVu data provides a better image. */

struct ddjvu_message_redisplay_s { /* ddjvu_message_t::m_redisplay */
  ddjvu_message_any_t  any;
}; 


/* ddjvu_message_t::m_chunk ---
   This message indicates that an additional chunk
   of DjVu data has been decoded.  Member <chunkid>
   indicates the type of the DjVu chunk. */

struct ddjvu_message_chunk_s {     /* ddjvu_message_t::m_chunk */
  ddjvu_message_any_t  any;
  const char *chunkid;
}; 


/* ------- QUERIES ------- */

/* ddjvu_page_get_width ---
   Returns the page width in pixels. Calling this function 
   before receiving a <m_pageinfo> message always yields <0>. */

DDJVUAPI int
ddjvu_page_get_width(ddjvu_page_t *page);


/* ddjvu_page_get_height---
   Returns the page height in pixels. Calling this function 
   before receiving a <m_pageinfo> message always yields <0>. */

DDJVUAPI int
ddjvu_page_get_height(ddjvu_page_t *page);

/* ddjvu_page_get_resolution ---
   Returns the page resolution in pixels per inche (dpi).
   Calling this function before receiving a <m_pageinfo>
   message yields a meaningless but plausible value. */

DDJVUAPI int
ddjvu_page_get_resolution(ddjvu_page_t *page);


/* ddjvu_page_get_gamma ---
   Returns the gamma of the display for which this page was designed.
   Calling this function before receiving a <m_pageinfo>
   message yields a meaningless but plausible value. */

DDJVUAPI double
ddjvu_page_get_gamma(ddjvu_page_t *page);


/* ddjvu_page_get_version ---
   Returns the version of the djvu file format.
   Calling this function before receiving a <m_pageinfo>
   message yields a meaningless but plausible value. */

DDJVUAPI int
ddjvu_page_get_version(ddjvu_page_t *page);


/* ddjvu_page_get_type ---
   Returns the type of the page data.
   Calling this function before the termination of the
   decoding process might returns <DDJVU_PAGETYPE_UNKNOWN>. */

typedef enum {
  DDJVU_PAGETYPE_UNKNOWN,
  DDJVU_PAGETYPE_BITONAL,
  DDJVU_PAGETYPE_PHOTO,
  DDJVU_PAGETYPE_COMPOUND,
} ddjvu_page_type_t;

DDJVUAPI ddjvu_page_type_t
ddjvu_page_get_type(ddjvu_page_t *page);

/* ddjvu_page_get_short_description ---
   ddjvu_page_get_long_description ---
   Return strings describing the DjVu page.
   The returned string must be freed with the C function <free>.
   The short description is a one liner suitable for a status bar.
   The long description lists all the decoded chunks.
   These strings are updated during the decoding process.
   Their value might change until the decoding terminates. */

DDJVUAPI char *
ddjvu_page_get_short_description(ddjvu_page_t *page);

DDJVUAPI char *
ddjvu_page_get_long_description(ddjvu_page_t *page);


/* ddjvu_page_get_rotation ---
   Returns the rotation angle for the DjVu page.
   The rotation is automatically taken into account
   by <ddjvu_page_render>, <ddjvu_page_get_width>
   and <ddjvu_page_get_height>. */

typedef enum {
  DDJVU_ROTATE_0   = 0,
  DDJVU_ROTATE_90  = 1,
  DDJVU_ROTATE_180 = 2,
  DDJVU_ROTATE_270 = 3,
} ddjvu_page_rotation_t;

DDJVUAPI ddjvu_page_rotation_t
ddjvu_page_get_rotation(ddjvu_page_t *page);


/* ddjvu_page_set_rotation ---
   Changes the rotation angle for a DjVu page.
   Calling this function before receiving a <m_pageinfo>
   message has no effect. */

DDJVUAPI void
ddjvu_page_set_rotation(ddjvu_page_t *page,
                        ddjvu_page_rotation_t rot);



/* ------- RENDER ------- */


/* ddjvu_rect_t ---
   This structure specifies the location of a rectangle.
   Coordinates are always expressed in pixels relative to
   the BOTTOM LEFT CORNER of an image.  Members <x> and <y>
   indicate the position of the bottom left corner of the
   rectangle Members <w> and <h> indicate the width and
   height of the rectangle. */

typedef struct ddjvu_rect_s {
  int x, y;
  unsigned int w, h;
} ddjvu_rect_t;


/* ddjvu_render_mode_t ---
   Various ways to render a page. */

typedef enum {
  DDJVU_RENDER_COLOR = 0,       /* color page or stencil */
  DDJVU_RENDER_BLACK,           /* stencil or color page */
  DDJVU_RENDER_COLORONLY,       /* color page or fail */
  DDJVU_RENDER_MASKONLY,        /* stencil or fail */
  DDJVU_RENDER_BACKGROUND,      /* color background layer */
  DDJVU_RENDER_FOREGROUND,      /* color foreground layer */
} ddjvu_render_mode_t;



/* ddjvu_page_render --
   Renders a segment of a page with arbitrary scale.
   Argument <mode> indicates what image layers 
   should be rendered. 

   Conceptually this function renders the full page
   into a rectangle <pagerect> and copies the
   pixels specified by rectangle <renderrect>
   into the buffer starting at position <imagebuffer>.
   The actual code is much more efficient than that.

   The final image is written into buffer <imagebuffer>.  
   Argument <pixelformat> specifies the expected pixel format.  
   Argument <rowsize> specifies the number of BYTES from 
   one row to the next in the buffer. The buffer must be 
   large enough to accomodate the desired image.

   This function makes a best effort to compute an image
   that reflects the most recently decoded data.  It might
   return <FALSE> to indicate that no image could be
   computed at this point, and that nothing was written into
   the buffer. */

DDJVUAPI int
ddjvu_page_render(ddjvu_page_t *page,
                  const ddjvu_render_mode_t mode,
                  const ddjvu_rect_t *pagerect,
                  const ddjvu_rect_t *renderrect,
                  const ddjvu_format_t *pixelformat,
                  unsigned long rowsize,
                  char *imagebuffer );




/* -------------------------------------------------- */
/* DJVU_FORMAT_T                                      */
/* -------------------------------------------------- */


/* ddjvu_format_style_t ---
   Enumerated type for pixel formats. */

typedef enum {
  DDJVU_FORMAT_BGR24,           /* truecolor 24 bits in BGR order */
  DDJVU_FORMAT_RGB24,           /* truecolor 24 bits in RGB order */
  DDJVU_FORMAT_RGBMASK16,       /* truecolor 16 bits with masks */
  DDJVU_FORMAT_RGBMASK32,       /* truecolor 32 bits with masks */
  DDJVU_FORMAT_GREY8,           /* greylevel 8 bits */
  DDJVU_FORMAT_PALETTE8,        /* paletized 8 bits (6x6x6 color cube) */
  DDJVU_FORMAT_MSBTOLSB,        /* packed bits, msb on the left */
  DDJVU_FORMAT_LSBTOMSB,        /* packed bits, lsb on the left */
} ddjvu_format_style_t;
   

/* ddjvu_format_create ---
   Creates a <ddjvu_format_t> object describing a pixel format.
   Argument <style> describes the generic pixel format.
   Argument <args> is an array of <nargs> unsigned ints
   providing additionnal information:
   - When style is <RGBMASK*>, argument <nargs> must be <3>
     and array <args> contains three contiguous bit masks for 
     the red, green, and blue components of each pixel.
   - When style is <PALETTE*>, argument <nargs> must be <216>
     and array <args> contains the 6*6*6 entries of a web
     color cube.
   - Otherwise <nargs> must be <0>. */

DDJVUAPI ddjvu_format_t *
ddjvu_format_create(ddjvu_format_style_t style, 
                    int nargs, unsigned int *args);


/* ddjvu_format_set_row_order ---
   Sets a flag indicating whether the rows in the pixel buffer
   are stored starting from the top or the bottom of the image.
   Default ordering starts from the bottom of the image.
   This is the opposite of the X11 convention. */

DDJVUAPI void
ddjvu_format_set_row_order(ddjvu_format_t *format, int top_to_bottom);


/* ddjvu_format_set_y_direction ---
   Sets a flag indicating whether the y coordinates in the drawing 
   area are oriented from bottom to top, or from top to botttom.  
   The default is bottom to top, similar to PostScript.
   This is the opposite of the X11 convention. */

DDJVUAPI void
ddjvu_format_set_y_direction(ddjvu_format_t *format, int top_to_bottom);



/* ddjvu_format_set_ditherbits ---
   Specifies the final depth of the image on the screen.
   This is used to decide which dithering algorithm should be used.
   The default is usually appropriate. */

DDJVUAPI void
ddjvu_format_set_ditherbits(ddjvu_format_t *format, int bits);


/* ddjvu_format_set_gamma ---
   Sets the gamma of the display for which the pixels are
   intended.  This will be combined with the gamma stored in
   DjVu documents in order to compute a suitable color
   correction.  The default value is 2.2. */

DDJVUAPI void
ddjvu_format_set_gamma(ddjvu_format_t *format, double gamma);


/* ddjvu_format_release ---
   Release a reference to a <ddjvu_format_t> object.
   The calling program should no longer reference this object. */

DDJVUAPI void
ddjvu_format_release(ddjvu_format_t *format);




/* -------------------------------------------------- */
/* THUMBNAILS                                         */
/* -------------------------------------------------- */


/* ddjvu_thumbnail_status ---
   Determine whether a thumbnail is available for page <pagenum>.
   Calling this function with non zero argument <start> initiates
   a thumbnail calculation job. The completion of the job
   is signalled by a subsequent <m_thumbnail> message. */

DDJVUAPI ddjvu_status_t
ddjvu_thumbnail_status(ddjvu_document_t *document, int pagenum, int start);


/* ddjvu_message_t::m_thumbnail ---
   This message is sent when additional thumbnails are available. */

struct ddjvu_message_thumbnail_s { /* ddjvu_message_t::m_thumbnail */
  ddjvu_message_any_t  any;
  int pagenum;
}; 


/* ddjvu_thumbnail_render ---
   Renders a thumbnail for page <pagenum>.
   Argument <imagebuffer> must be large enough to contain
   an image of size <*wptr> by <*hptr> using pixel format
   <pixelformat>. Argument <rowsize> specifies the number 
   of BYTES from one row to the next in the buffer.
   This function returns <FALSE> when no thumbnail is available.
   Otherwise it returns <TRUE>, adjusts <*wptr> and <*hptr> to 
   reflect the thumbnail size, and writes the pixel data into 
   the image buffer. */

DDJVUAPI int
ddjvu_thumbnail_render(ddjvu_document_t *document, int pagenum, 
                       int *wptr, int *hptr,
                       const ddjvu_format_t *pixelformat,
                       unsigned long rowsize,
                       char *imagebuffer);



/* -------------------------------------------------- */
/* SAVE AND PRINT JOBS                                */
/* -------------------------------------------------- */

#ifndef DDJVU_WITHOUT_STDIO
# include <stdlib.h>
# include <stdio.h>
#else 
# define FILE void
#endif

/* Not yet implemented */

struct ddjvu_message_progress_s {
  ddjvu_message_any_t any;
  ddjvu_status_t status;
  int percent;
};


/* Options like ddjvu ? */
DDJVUAPI ddjvu_job_t *
ddjvu_document_export(ddjvu_document_t *document, FILE *output,
                      int optc, const char * const * optv);


/* Options like djvups ? */
DDJVUAPI ddjvu_job_t *
ddjvu_document_print(ddjvu_document_t *document, FILE *output,
                     int optc, const char * const * optv);


/* Options to be defined */
DDJVUAPI ddjvu_job_t *
ddjvu_document_save(ddjvu_document_t *document, FILE *output, 
                    int optc, const char * const * optv);



/* -------------------------------------------------- */
/* ANNOTATIONS AND HIDDEN_TEXT                        */
/* -------------------------------------------------- */

/* Not yet defined */
/* Not yet implemented */


/* -------------------------------------------------- */
/* DJVU_MESSAGE_T                                     */
/* -------------------------------------------------- */


/* We can now define the djvu_message_t union */

union ddjvu_message_s {
  struct ddjvu_message_any_s        m_any;
  struct ddjvu_message_error_s      m_error;
  struct ddjvu_message_info_s       m_info;
  struct ddjvu_message_newstream_s  m_newstream;
  struct ddjvu_message_docinfo_s    m_docinfo;
  struct ddjvu_message_pageinfo_s   m_pageinfo;
  struct ddjvu_message_chunk_s      m_chunk;
  struct ddjvu_message_relayout_s   m_relayout;
  struct ddjvu_message_redisplay_s  m_redisplay;
  struct ddjvu_message_thumbnail_s  m_thumbnail;
};



/* -------------------------------------------------- */
/* BACKDOORS                                          */
/* -------------------------------------------------- */

#ifdef __cplusplus
} // extern "C"
#endif

/* ddjvu_get_DjVuImage ---
   ddjvu_get_DjVuDocument ---
   These functions provide an access to the libdjvu objects 
   associated with the ddjvuapi objects.  These backdoors can
   be useful for advanced manipulations.  These two functions 
   are declared in C++ when file "ddjvuapi.h" is included 
   after the libdjvu header files "DjVuDocument.h". */

#ifdef __cplusplus
# ifndef NOT_USING_DJVU_NAMESPACE
#  ifdef _DJVUIMAGE_H
DDJVUAPI GP<DjVuImage>
ddjvu_get_DjVuImage(ddjvu_page_t *page);
#  endif
#  ifdef _DJVUDOCUMENT_H
DDJVUAPI GP<DjVuDocument>
ddjvu_get_DjVuDocument(ddjvu_document_t *document);
#  endif
# endif
#endif

#endif /* DDJVUAPI_H */
