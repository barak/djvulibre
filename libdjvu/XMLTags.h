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

#ifndef _LT_XMLTAGS__
#define _LT_XMLTAGS__
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "GContainer.h"
#include "GString.h"

class lt_XMLContents;
class DjVuFile;
class DjVuDocument;
class ByteStream;
class XMLByteStream;
class GURL;

class lt_XMLTags : public GPEnabled
{
protected:
  lt_XMLTags();
  lt_XMLTags(const char n[]);

public:
  /// Empty creator.
  static GP<lt_XMLTags> create(void) { return new lt_XMLTags; }
  /// Default the specified tag.
  static GP<lt_XMLTags> create(const char n[]) { return new lt_XMLTags(n); }
  /// Initialize from the specified URL.
  void init(const GURL & url);
  /// Create from the specified URL.
  static GP<lt_XMLTags> create(const GURL &url);
  /// Initialize from the specified bytestream.
  void init(const GP<ByteStream> &bs);
  /// Create from the specified bytestream.
  static GP<lt_XMLTags> create(const GP<ByteStream> &bs);
  /// Initialize from an XMLByteStream.
  void init(XMLByteStream &xmlbs);
  /// Create from an XML bytestream.
  static GP<lt_XMLTags> create(XMLByteStream &xmlbs);
  /// Non-virtual destructor.
  ~lt_XMLTags();

  inline int get_Line(void) const;
  inline const GUTF8String& get_raw(void) const;
  inline const GUTF8String& get_name(void) const;
  inline const GList<lt_XMLContents>& get_content(void) const;
  inline const GMap<GUTF8String,GUTF8String>& get_args(void) const;
  inline const GMap<GUTF8String,GPList<lt_XMLTags> >& get_allTags(void) const;

  GPList<lt_XMLTags> get_Tags(char const tagname[]) const;
  inline void set_Line(const int xstartline) { startline=xstartline; }

  inline void addtag(GP<lt_XMLTags> x);
  inline void addraw(GUTF8String raw);
  inline GPosition contains(GUTF8String name) const;
  inline const GPList<lt_XMLTags> & operator [] (const GUTF8String name) const;
  inline const GPList<lt_XMLTags> & operator [] (const GPosition &pos) const;
  static void ParseValues(char const *t, GMap<GUTF8String,GUTF8String> &args,bool downcase=true);
  static void get_Maps(char const tagname[],char const argn[],
    GPList<lt_XMLTags> list, GMap<GUTF8String, GP<lt_XMLTags> > &map);
  void write(ByteStream &bs,bool const top=true) const;

protected:
  GUTF8String name;
  GMap<GUTF8String,GUTF8String> args;
  GList<lt_XMLContents> content;
  GUTF8String raw;
  GMap<GUTF8String,GPList<lt_XMLTags> > allTags;
  int startline;
};

class lt_XMLContents
{
public:
  lt_XMLContents(void);
  lt_XMLContents(GP<lt_XMLTags> tag);
  GP<lt_XMLTags> tag;
  GUTF8String raw;
  void write(ByteStream &bs) const;
};

inline GP<lt_XMLTags>
lt_XMLTags::create(const GURL &url)
{
  const GP<lt_XMLTags> retval(new lt_XMLTags);
  retval->init(url);
  return retval;
}

inline GP<lt_XMLTags>
lt_XMLTags::create(const GP<ByteStream> &bs)
{
  const GP<lt_XMLTags> retval(new lt_XMLTags);
  retval->init(bs);
  return retval;
}

inline GP<lt_XMLTags>
lt_XMLTags::create(XMLByteStream &xmlbs)
{
  const GP<lt_XMLTags> retval(new lt_XMLTags);
  retval->init(xmlbs);
  return retval;
}

/// Non-virtual destructor.
inline void
lt_XMLTags::addtag (GP<lt_XMLTags> x)
{
  content.append(lt_XMLContents(x));
  allTags[x->name].append(x);
}

inline void
lt_XMLTags::addraw (GUTF8String r)
{
  GPosition pos=content;
  if(pos)
  {
    content[pos].raw+=r;
  }else
  {
    raw+=r;
  }
}

inline int
lt_XMLTags::get_Line(void) const
{ return startline; }

inline const GUTF8String &
lt_XMLTags::get_name(void) const { return name; }

inline const GUTF8String &
lt_XMLTags::get_raw(void) const { return raw; }

inline const GList<lt_XMLContents> &
lt_XMLTags::get_content(void) const { return content; }

inline const GMap<GUTF8String,GUTF8String> &
lt_XMLTags::get_args(void) const { return args; }

inline const GMap<GUTF8String,GPList<lt_XMLTags> > &
lt_XMLTags::get_allTags(void) const { return allTags; }

inline GPosition
lt_XMLTags::contains(GUTF8String name) const
{
  return allTags.contains(name);
}

inline const GPList<lt_XMLTags> &
lt_XMLTags::operator [] (const GUTF8String name) const
{
  return allTags[name];
}

inline const GPList<lt_XMLTags> &
lt_XMLTags::operator [] (const GPosition &pos) const
{
  return allTags[pos];
}

#endif /* _LT_XMLTAGS__ */


