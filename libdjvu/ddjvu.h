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

typedef struct ddjvu_context_s  ddjvu_context_t;
typedef union  ddjvu_message_s  ddjvu_message_t;
typedef struct ddjvu_document_s ddjvu_document_t;
typedef struct ddjvu_stream_s   ddjvu_stream_t;
typedef struct ddjvu_page_s     ddjvu_page_t;
typedef struct ddjvu_format_s   ddjvu_format_t;


/* General conventions:
   Unless specified otherwise:
   - all strings use locale encoding,
   - all errors are signaled with error event messages.
   - all functions returning a pointer might return 
     a null pointer (this usually indicates an error 
     condition).

   Prerequisites:
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
   Creates a <ddjvu_context_t> object. */

DDJVUAPI ddjvu_context_t *
ddjvu_context_create(void);


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


/* ------- MESSAGES ------- */


/* ddjvu_message_peek ---
   Returns a pointer to the next DDJVU message.
   This function returns 0 if no message is available.
   It never removes a message from the queue. */

DDJVUAPI ddjvu_message_t *
ddjvu_message_peek(ddjvu_context_t *context);


/* ddjvu_message_wait ---
   Returns a pointer to the next DDJVU message.
   This function waits until a message is available.
   It never removes a message from the queue. */

DDJVUAPI ddjvu_message_t *
ddjvu_message_wait(ddjvu_context_t *context);


/* ddjvu_message_pop ---
   Removes the nest message from the queue.
   This function must be called after processing the message.
   Pointers returned by previous calls to <ddjvu_message_peek> 
   or <ddjvu_message_wait> might no longer be valid after 
   calling <ddjvu_message_pop>. */

DDJVUAPI void
ddjvu_message_pop(ddjvu_context_t *context);


/* ddjvu_message_set_callback ---
   Defines a callback function invoked whenever
   a new message is posted to the DDJVU message queue,
   and returns a pointer to the previous callback function.
   This callback function can be called at any time
   while other code is executing. It should simply signal
   the main application event loop that new DDJVU messages
   are available.  Under WIN32, this is usually achieved
   by posting a user window message.  Under UNIX, this is
   usually achieved using a pipe: the callback writes 
   a single byte into the pipe; the main application loop
   monitors the reading end of the pipe and detects
   the presence of data. */

typedef void 
(*ddjvu_message_callback_t)(ddjvu_context_t *context,
                            int howmanymessages);

DDJVUAPI ddjvu_message_callback_t
ddjvu_message_set_callback(ddjvu_context_t *context,
                           ddjvu_message_callback_t callback);


/* ------- MESSAGES ------- */


/* ddjvu_message_t ---
   This union type represents messages delivered by the
   DDJVU API. Each member of the union pertains to a
   specific kind of message.  Member <m_any> represents the
   information common to all kinds of messages.  Given a
   pointer <p> to a <djvu_message_t>, the message kind can
   be accessed as <"p->m_any.kind">. */


/* ddjvu_message_tag_t ---
   This enumerated type identifies each kind of 
   message delivered by the DDJVU API.  */

typedef enum {
  DDJVU_ERROR,
  DDJVU_STATUS,
  DDJVU_NEWSTREAM,
  DDJVU_DOCINFO,
  DDJVU_PAGEINFO,
  DDJVU_RELAYOUT,
  DDJVU_REDISPLAY,
  DDJVU_CHUNK,
} ddjvu_message_tag_t;


/* ddjvu_message_t::m_any ---
   This structure is a member of the union <djvu_message_t>.
   It represents the information common to all kinds of
   messages.  Member <kind> indicates the kind of message.
   Members <context>, <document>, and <page> indicate the
   origin of the message.  These fields contain null
   pointers when they are not relevant. */

typedef struct ddjvu_message_any_s {
  ddjvu_message_tag_t   kind;
  ddjvu_context_t      *context;
  ddjvu_document_t     *document;
  ddjvu_page_t         *page;
} ddjvu_message_any_t; 


/* ddjvu_message_t::m_error ---
   Error messages are generated whenever the decoder or the
   DDJVU API encounters an error condition.  All errors are
   reported as error messages because they can occur
   asynchronously.  Member <category> provides generic
   information about the error.  Member <message> is a
   specific error message.  Members <filename> and <lineno>
   indicates the place where the error was detected.
*/

typedef enum  { 
  ddjvu_error_other,            /* generic decoding error */
  ddjvu_error_api,              /* invalid argument */
  ddjvu_error_eof,              /* premature end-of-file */
  ddjvu_error_stop,             /* user initiated interruption */
} ddjvu_error_t;

struct ddjvu_message_error_s {  /* ddjvu_message_t::m_error */
  ddjvu_message_any_t   any;
  ddjvu_error_t         category;
  const char           *message;
  const char           *filename;
  int                   lineno;
}; 


/* ddjvu_message_t::m_status ---
   This messages provides informational text indicating
   the progress of the decoding process. This might
   be displayed in the browser status bar. */

struct ddjvu_message_status_s { /* ddjvu_message_t::m_status */
  ddjvu_message_any_t  any;
  const char          *message;
}; 




/* -------------------------------------------------- */
/* DDJVU_DOCUMENT_T                                    */
/* -------------------------------------------------- */


/* ddjvu_document_create ---
   Creates a decoder for a DjVu document and starts
   decoding.  This function returns immediately.  The
   decoder will later generate messages to request the raw
   data and to indicate the state of the decoding process.

   Argument <url> specifies an optional URL for the document.  
   The URL follows the usual syntax (<"protocol://machine/path">). 
   It only serves two purposes:
   - The URL is used as a key for the cache of decoded pages.
   - The URL is not used used to document <m_newstream> messages.

   Setting argument <cache> to <TRUE> indicates that decoded pages
   should be caches when possible.  This only works when
   argument <url> is not the null pointer.

   It is important to understand that the URL is not used to
   access the data.  The document will generate <m_newstream>
   messages to indicate which data it needs when it needs it.  
   The caller must then provide the raw data using
   <ddjvu_stream_write> and <ddjvu_stream_close>.

   [VERIFY] URL encoding: Argument <url> is encoded
   using the locale encoding. The URL encoding syntax (%2A%40..)
   can be used within an URL to indicate UTF-8 encoded
   characters. */

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


/* ddjvu_document_release ---
   Release a reference to a <ddjvu_document_t> object.
   The calling program should no longer reference this object.  
   The object itself will be destroyed as soon as no other object
   or thread needs it. */
 
DDJVUAPI void
ddjvu_document_release(ddjvu_document_t *document);


/* ------- USERDATA ------- */


/* ddjvu_document_set_user_data ---
   ddjvu_document_get_user_data ---
   Each <ddjvu_document_t> object can store an arbitray pointer
   that callers can use for any purpose. These two functions
   provide for accessing or setting this pointer. */

DDJVUAPI void
ddjvu_document_set_user_data(ddjvu_document_t *document,
                             void *userdata);

DDJVUAPI void *
ddjvu_document_get_user_data(ddjvu_document_t *document);


/* ------- STREAMS ------- */


/* ddjvu_message_t::m_newstream --- 
   Newstream messages are generated whenever the decoder
   needs to access raw DjVu data.  The caller must then
   provide the requested data using <ddjvu_stream_write> 
   and <ddjvu_stream_close>. 

   In the case of indirect documents, a single decoder 
   might simultaneously request several streams of data.  
   Each stream is identified by the opaque handle <streamid>.

   Member <name> in the first <m_newstream> message is
   always the null pointer.  It indicates that the decoder
   needs to access the data in the main DjVu file.  Further
   <m_newstream> messages are generated to access the
   auxilliary files of indirect or indexed DjVu documents.
   Argument <name> then provides the basename of the
   auxilliary file.

   Member <url> is set according to the url argument
   provided to function <ddjvu_document_create>.  The first
   newstream message always contain the url passed to
   <ddjvu_document_create>.  Subsequent newstream messages
   contain the url of the auxilliary files of indirect or
   indexed DjVu documents. */
   
struct ddjvu_message_newstream_s { /* ddjvu_message_t::m_newstream */
  ddjvu_message_any_t  any;
  ddjvu_stream_t      *streamid;
  const char          *name;
  const char          *url;
}; 


/* ddjvu_stream_write ---
   Provide raw data to the DjVu decoder.
   This function should be called as soon as the data is available,
   for instance when receiving DjVu data from a network connection.
 */

DDJVUAPI void
ddjvu_stream_write(ddjvu_stream_t *streamid,
                   const char *data,
                   unsigned long datalen );

/* ddjvu_stream_close ---
   Indicates that no more data will be provided on a
   particular stream.  Argument <stop> should most likely be
   set to <FALSE>. Setting argument <stop> to <TRUE>
   indicates that the user has interrupted the data transfer
   (for instance by pressing the stop button in a web
   browser) and that the decoding process should be stopped
   as soon as feasible.  This will generate a flurry of 
   <m_error> messages. */

DDJVUAPI void
ddjvu_stream_close(ddjvu_stream_t *streamid,
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
   Calling this function before receiving a <m_docinfo> message
   always returns <DDJVU_DOCTYPE_UNKNOWN>. */

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
   Calling this function before receiving a <m_docinfo> message
   always returns zero. */
   
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
   immediately after creating the <ddjvu_document_t> object,
   will return a valid <ddjvu_page_t> object, and will
   initiate the data transfer and the decoding threads for
   the specified page.  Various DDJVU messages will document
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


/* ddjvu_page_release ---
   Release a reference to a <ddjvu_page_t> object.
   The calling program should no longer reference this object.
   The object itself will be destroyed as soon as no other object
   or thread needs it. */

DDJVUAPI void
ddjvu_page_release(ddjvu_page_t *page);


/* ------- USERDATA ------- */


/* ddjvu_page_set_user_data ---
   ddjvu_page_get_user_data ---
   Each <ddjvu_paqge_t> object can store an arbitray pointer
   that callers can use for any purpose. These two functions
   provide for accessing or setting this pointer. */

DDJVUAPI void
ddjvu_page_set_user_data(ddjvu_page_t *page,
                         void *userdata);

DDJVUAPI void *
ddjvu_page_get_user_data(ddjvu_page_t *page);

/* ------- MESSAGES ------- */


/* ddjvu_message_t::m_pageinfo ---
   This message is generated when the basic 
   information about a page becomes available. */

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
   the page because newly decoded DjVu data
   provides more information. */

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

/* ddjvu_page_status ---
   This function returns the status of the page 
   decoding process. */

typedef enum {
  DDJVU_DECODE_NOTSTARTED, /* decoding was not even started */
  DDJVU_DECODE_STARTED,    /* decoding is in progress */
  DDJVU_DECODE_OK,         /* decoding terminated successfully */
  DDJVU_DECODE_FAILED,     /* decoding failed because of an error */
  DDJVU_DECODE_STOPPED     /* decoding failed because it was stopped by the user*/
} ddjvu_decoding_status_t;

DDJVUAPI ddjvu_decoding_status_t
ddjvu_page_status(ddjvu_page_t *page);


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
   the bottom left corner of an image.  Members <x> and <y>
   indicate the position of the bottom left corner of the
   rectangle.  Members <w> and <h> indicate the width and
   height of the reactangle. */

typedef struct ddjvu_rect_s {
  int x, y;
  unsigned int w, h;
} ddjvu_rect_t;


/* ddjvu_page_render --
   Renders a segment of a page with arbitrary scale.
   Conceptually this function renders the full page
   into a rectangle <pagerect> and copies the
   pixels specified by rectangle <renderrect>
   into the buffer starting at position <imagebuffer>.
   The actual code is much more efficient than that.

   Argument <imageformat> indicates the expected pixel
   format.  Argument <rowsize> indicates the number of bytes
   from one row to the next.
   
   This function makes a best effort to compute an image
   that reflects the most recently decoded data.  It might
   return <FALSE> to indicate that no image could be
   computed at this point, and that nothing was written into
   <imagebuffer>.
*/

DDJVUAPI int
ddjvu_page_render(ddjvu_page_t *page,
                  const ddjvu_format_t *imageformat,
                  const ddjvu_rect_t *pagerect,
                  const ddjvu_rect_t *renderrect,
                  char *imagebuffer,
                  unsigned long rowsize);



/* -------------------------------------------------- */
/* DJVU_FORMAT_T                                      */
/* -------------------------------------------------- */


/* ddjvu_format_create_truecolor ---
   Creates a <ddjvu_format_t> object describing a true color
   pixel format.  Argument <pixelsize> indicates the pixel
   size in bytes and can take values <1>, <2>, <3> or <4>.
   Arguments <redmask>, <greenmask>, and <bluemask> are bit
   masks indicating the pixel bits for each color component.
   Argument <top_to_bottom> is <TRUE> when the image rows
   must be stored from top to bottom, and <FALSE> when the
   image rows must be stored from bottom to top. */

DDJVUAPI ddjvu_format_t *
ddjvu_format_create_truecolor(int pixelsize,
                              unsigned long redmask,
                              unsigned long greenmask,
                              unsigned long bluemask,
                              int top_to_bottom);

/* ddjvu_format_create_palette ---
   Creates a <ddjvu_format_t> object describing a pixel
   format based on a web palette. Argument <pixelsize>
   indicates the pixel size in bytes and can take values
   <1>, <2> or <4>.  Argument <palette> indicate the pixel
   values for the 216 colors of a 6x6x6 web color cube.
   Argument <top_to_bottom> is <TRUE> when the image rows
   must be stored from top to bottom, and <FALSE> when the
   image rows must be stored from bottom to top. */
DDJVUAPI ddjvu_format_t *
ddjvu_format_create_palette(int pixelsize,
                            unsigned long palette[6*6*6],
                            int top_to_bottom);


/* ddjvu_format_create_graylevel ---
   Creates a <ddjvu_format_t> object describing a gray level
   pixel format. Argument <pixelsize> indicates the pixel
   size in bytes and must be <1>. Arguments <pixelwhite> and
   <pixelblack> give the pixel values for the pure white and
   pure black colors.  Argument <top_to_bottom> is <TRUE>
   when the image rows must be stored from top to bottom,
   and <FALSE> when the image rows must be stored from
   bottom to top. */
DDJVUAPI ddjvu_format_t *
ddjvu_format_create_graylevel(int pixelsize,
                              int pixelwhite,
                              int pixelblack,
                              int top_to_bottom);


/* ddjvu_format_create_bitonal ---
   Creates a <ddjvu_format_t> object describing a bit packed
   black&white pixel format. Argument <lsb_to_msb> indicates
   the pixel packing order.  Argument <min_is_white>
   indicates the photometric interpretation. Argument
   <top_to_bottom> is <TRUE> when the image rows must be
   stored from top to bottom, and <FALSE> when the image
   rows must be stored from bottom to top. */

DDJVUAPI ddjvu_format_t *
ddjvu_format_create_bitonal(int lsb_to_msb,
                            int min_is_white,
                            int top_to_bottom);


/* ddjvu_format_set_gamma ---
   Sets the gamma of the display for which the pixels are
   intended.  This will be combined with the gamma stored in
   DjVu documents in order to compute a suitable color
   correction.  The default value is 2.2. */

DDJVUAPI void
ddjvu_format_set_gamma(ddjvu_format_t *format,
                       double gamma);


/* ddjvu_format_release ---
   Release a reference to a <ddjvu_format_t> object.
   The calling program should no longer reference this object.  
   The object itself will be destroyed as soon as no other object
   or thread needs it. */

DDJVUAPI void
ddjvu_format_release(ddjvu_format_t *format);



/* -------------------------------------------------- */
/* MORE                                               */
/* -------------------------------------------------- */


/* More should be defined here:
   - access to annotations, hyperlinks, etc.
   - access to text layer
   - printing
*/
   


/* -------------------------------------------------- */
/* DJVU_MESSAGE_T                                     */
/* -------------------------------------------------- */


/* We can now define the djvu_message_t union */

union djvu_message_s {
  struct ddjvu_message_any_s        m_any;
  struct ddjvu_message_error_s      m_error;
  struct ddjvu_message_status_s     m_status;
  struct ddjvu_message_newstream_s  m_newstream;
  struct ddjvu_message_docinfo_s    m_docinfo;
  struct ddjvu_message_pageinfo_s   m_pageinfo;
  struct ddjvu_message_chunk_s      m_chunk;
  struct ddjvu_message_relayout_s   m_relayout;
  struct ddjvu_message_redisplay_s  m_redisplay;
};





#ifdef __cplusplus
}
#endif
#endif /* DDJVUAPI_H */
