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

#ifndef _JPEGDECODER_H_
#define _JPEGDECODER_H_
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "GSmartPointer.h"

class ByteStream;
class GPixmap;

#ifdef NEED_JPEG_DECODER

#include <string.h>
#include <setjmp.h>

/** @name JPEGDecoder.h
    Files #"JPEGDecoder.h"# and #"JPEGDecoder.cpp"# implement an
    interface to the decoding subset of the IJG JPEG library.
    @memo
    Decoding interface to the IJG JPEG library.
    @version
    #$Id$#
    @author
    Parag Deshmukh <parag@sanskrit.lz.att.com> 
*/
//@{

class GUTF8String;

/** This class ensures namespace isolation. */
class JPEGDecoder
{
public:
  class Impl;

  /** Decodes the JPEG formated ByteStream */ 
  static GP<GPixmap> decode(ByteStream & bs);
  static void decode(ByteStream & bs,GPixmap &pix);
#ifdef LIBJPEGNAME
  static void *jpeg_lookup(const GUTF8String &name);
  static jpeg_error_mgr *jpeg_std_error(jpeg_error_mgr *x);
  static void jpeg_CreateDecompress(jpeg_decompress_struct *x,int v, size_t s);
  static void jpeg_destroy_decompress(j_decompress_ptr x);
  static int jpeg_read_header(j_decompress_ptr x,boolean y);
  static JDIMENSION jpeg_read_scanlines(j_decompress_ptr x,JSAMPARRAY y,JDIMENSION z);
  static boolean jpeg_finish_decompress(j_decompress_ptr x);
  static boolean jpeg_resync_to_restart(jpeg_decompress_struct *x,int d);
  static boolean jpeg_start_decompress(j_decompress_ptr x);
#endif // LIBJPEGNAME
};


//@}

#endif // NEED_JPEG_DECODER
#endif // _JPEGDECODER_H_

