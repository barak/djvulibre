//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.5)
//C- 
//C- Copyright © 2000-2001 LizardTech, Inc. All Rights Reserved.
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

#ifndef __DJVU_MESSAGE_H__
#define __DJVU_MESSAGE_H__
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "DjVuMessageLite.h"

class GURL;

class DjVuMessage : public DjVuMessageLite
{
protected:
  void init(void);
  DjVuMessage(void);

public:
  /// Use the locale info and find the XML files on disk.
  static void use_language(void);

  /// Set the program name used when searching for XML files on disk.
  static void set_programname(const GUTF8String &programname);
  static GUTF8String &programname(void);

  /// creates this class specifically.
  static const DjVuMessageLite &create_full(void);

  /** Adds a byte stream to be parsed whenever the next DjVuMessage::create()
      call is made. */
  static void AddByteStreamLater(const GP<ByteStream> &bs)
  { use_language(); DjVuMessageLite::AddByteStreamLater(bs); }

  /** Destructor: Does any necessary cleanup. Actions depend on how the message
      file is implemented. */
  ~DjVuMessage();

  //// Same as LookUp, but this is a static method.
  static GUTF8String LookUpUTF8( const GUTF8String & MessageList )
  { use_language();return DjVuMessageLite::LookUpUTF8(MessageList); }

  /** Same as Lookup, but returns a multibyte character string in the
      current locale. */
  static GNativeString LookUpNative( const GUTF8String & MessageList )
  { use_language();return DjVuMessageLite::LookUpNative(MessageList); }

  /// This is a simple alias to the above class, but does an fprintf to stderr.
  static void perror( const GUTF8String & MessageList )
  { use_language();DjVuMessageLite::perror(MessageList); }

  static GList<GURL> GetProfilePaths(void);
};


#endif /* __DJVU_MESSAGE_H__ */

