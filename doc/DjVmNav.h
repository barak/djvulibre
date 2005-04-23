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

#ifndef _DjVmNav_H
#define _DjVmNav_H

#include "DjVuGlobal.h"
#include "GString.h"
#include "GThreads.h"

class ByteStream;

/** The NAVM chunk.

    The optional #"NAVM"# chunk which follows the DIRM chunk describes
	how the user can navigate the document.

	This is a list of DjVuBookMarks.
**/

class DjVmNav : public GPEnabled
{
public:
  /** Class \Ref{DjVmNav::DjVuBookMark} represents a entry in the heirarchy
  of contents. */
  class DjVuBookMark;
 
   DJVUREFAPI static GP<DjVmNav> create(void);
      /** Decodes the directory from the specified stream. */
   DJVUREFAPI void decode(const GP<ByteStream> &stream);
      /** Encodes the directory into the specified stream. */
   DJVUREFAPI void encode(const GP<ByteStream> &stream) ;
   DJVUREFAPI void dump(const GP<ByteStream> &stream) ;
      /** Return bookmark at zero-based position i */
   DJVUREFAPI bool getBookMark(GP<DjVuBookMark> &gpBookMark, int i) ;
   DJVUREFAPI int getBookMarkCount();
      /** append the BookMark to the end of the list */
   DJVUREFAPI void append (const GP<DjVuBookMark> &gpBookMark) ;
      /** Remove the BookMark from the list */
//   DJVUREFAPI void remove (const GP<DjVuBookMark> &gpBookMark) ;

   /**This function will check the given bookmark is valid or not*/
   DJVUREFAPI bool isValidBookmark();
   /**This function determines if the given  count_array is a tree sequence.
   that is: if it fits a tree. The definition of tree sequence and the correctness of this
   function is given in paper: ../validate/src/treesequence.djvu*/
   DJVUREFAPI int get_tree(int index, int* count_array, int count_array_size);



      /** Tests if directory defines an {\em indirect} document. */
protected:
      /** Class \Ref{DjVmNav::DjVuBookMark} represents the directory records
          managed by class \Ref{DjVmNav}. */
   DJVUREFAPI DjVmNav(void) { } ;
   //DJVUREFAPI ~DjVmNav(void) { } ;

private:
   GCriticalSection class_lock;
   GPList<DjVuBookMark>	bookmark_list;
};

/** The DjVuBookMark.

	Each entry in the Navigation chunk (NAVM) is a bookmark.  A bookmark contains
	a count of immediate children, a display string and a url.
**/
class DjVmNav::DjVuBookMark : public GPEnabled
{
public:

protected:
  /** Default constructor. */
  DJVUREFAPI DjVuBookMark(void);
  //DJVUREFAPI ~DjVuBookMark(void);
public:
  DJVUREFAPI static GP<DjVuBookMark> create(void);
  DJVUREFAPI static GP<DjVuBookMark> create(const unsigned short count,
     const GUTF8String &displayname, const GUTF8String &url);
  DJVUREFAPI void encode(const GP<ByteStream> &stream);
  DJVUREFAPI void dump(const GP<ByteStream> &stream);
  DJVUREFAPI void decode(const GP<ByteStream> &stream);

  int count;	// count of immediate children.
  GUTF8String displayname;	// example:  "Section 3.5 - Encryption"
  GUTF8String url;	// url, may be blank or relative.
};


#endif // _DjVmNav_H
