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

#ifndef _GOS_H_
#define _GOS_H_
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif

/** @name GOS.h
    Files #"GOS.h"# and #"GOS.cpp"# implement operating system 
    dependent functions with a unified interface.  All these functions
    are implemented as static member of class \Ref{GOS}. 
    Functions are provided for testing the presence of a file or a directory
    (\Ref{GOS::is_file}, \Ref{GOS::is_dir}), for manipulating file and directory names
    (\Ref{GOS::dirname}, \Ref{GOS::basename}, \Ref{GOS::expand_name},
    for obtaining and changing the current directory (\Ref{GOS::cwd}),
    for converting between file names and urls (\Ref{GOS::filename_to_url},
    \Ref{GOS::url_to_filename}), and for counting time (\Ref{GOS::ticks},
    \Ref{GOS::sleep}).
    
    @memo
    Operating System dependent functions.
    @author
    L\'eon Bottou <leonb@research.att.com> -- Initial implementation
    @version
    #$Id$#
*/
//@{

#include "DjVuGlobal.h"
#include "GString.h"

class GURL;

/** Operating System dependent functions. */
class GOS 
{
 public:
  // -----------------------------------------
  // Functions for dealing with filenames
  // -----------------------------------------

  /** Returns the last component of file name #filename#.  If the filename
      suffix matches argument #suffix#, the filename suffix is removed.  This
      function works like the unix command #/bin/basename#, but also supports
      the naming conventions of other operating systems. */
  static GUTF8String basename(const GUTF8String &filename, const char *suffix=0);

  /** Sets and returns the current working directory.
      When argument #dirname# is specified, the current directory is changed
      to #dirname#. This function always returns the fully qualified name
      of the current directory. */
  static GUTF8String cwd(const GUTF8String &dirname=GUTF8String());

  // -----------------------------------------
  // Functions for measuring time
  // -----------------------------------------
  
  /** Returns a number of elapsed milliseconds.  This number counts elapsed
      milliseconds since a operating system dependent date. This function is
      useful for timing code. */
  static unsigned long ticks();

  /** Sleeps during the specified time expressed in milliseconds.
      Other threads can run while the calling thread sleeps. */
  static void sleep(int milliseconds);

  /// Read the named variable from the environment, and converts it to UTF8.
  static GUTF8String getenv(const GUTF8String &name);

#if 0
  // -------------------------------------------
  // Functions for converting filenames and urls
  // -------------------------------------------
  /** Encodes all reserved characters, so that the #filename# can be
      used inside a URL. Every reserved character is encoded using escape
      sequence in the form of #%XX#. The legal characters are alphanumeric and
      #$-_.+!*'(),:#.
      Use \Ref{decode_reserved}() to convert the URL back to the filename. */
//  static GString encode_reserved(const char * filename);
  static GString encode_mbcs_reserved(const char * filename);/*MBCS*/
#endif

};


//@}
// ------------
#endif
