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

#ifndef _LT_XMLPARSER__
#define _LT_XMLPARSER__
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "GContainer.h"
#include "GURL.h"

class ByteStream;
class lt_XMLTags;
class lt_XMLContents;
class DjVuFile;
class DjVuDocument;
class DjVuImage;
class GBitmap;

// this is the base class for using XML to change DjVu Docs.

class lt_XMLParser : public GPEnabled
{
public:
  class Impl;
  typedef GP<ByteStream> mapOCRcallback(
    void *,const GUTF8String &value,const GP<DjVuImage> &);
protected:
  lt_XMLParser(void);
  virtual ~lt_XMLParser();
public:
  static GP<lt_XMLParser> create(void);
  /// Parse the specified bytestream.
  virtual void parse(const GP<ByteStream> &bs) = 0;
  /// Parse the specified tags - this one does all the work
  virtual void parse(const lt_XMLTags &tags) = 0;
  /// write to disk.
  virtual void save(void) = 0;
  /// erase.
  virtual void empty(void) = 0;

  // helper function for args
  static void setOCRcallback(
    void * const arg,mapOCRcallback * const );
};

#endif /* _LT_XMLPARSER__ */


