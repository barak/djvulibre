//C-  -*- C++ -*-
//C- -----------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- DjVuLibre-3.5 is derived from the DjVu (r) Reference Library.
//C- 
//C- DjVu (r) Reference Library (v. 3.5)
//C- Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
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
//C- -----------------------------------------------------------
// 
// $Id$
// $Name$

#ifndef _DJVMDIR0_H
#define _DJVMDIR0_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "GString.h"

class ByteStream;

/** @name DjVmDir0.h

    Files #"DjVmDir0.h"# and #"DjVmDir0.cpp"# contain implementation of
    \Ref{DjVmDir0} class responsible for reading and writing the directory
    of a multipage all-in-one-file DjVu document (usually referred to as
    {\bf DjVm} document.

    This is {\bf not} a navigation directory, which lists all the pages
    in a multipage document. The navigation directory is supported by class
    \Ref{DjVuNavDir}. This is the directory of a DjVm archive.
    

    @memo Directory of DjVu all-in-one-file DjVu documents.
    @author Andrei Erofeev <eaf@geocities.com>
    @version #$Id$# */

//@{

/** Directory of all-in-one-file DjVu documents (also known as DjVm documents).
    This class can read and write the directory (table of contents, in other
    words) of a DjVm document. This table of contents lists all {\bf files}
    included into the document, not {\bf pages} like \Ref{DjVuNavDir} does.
    It is normally stored in the document inside {\bf DIR0} chunk where
    {\bf "0"} refers to the version number.

    An object of this class can be created either as a result of the decoding
    of an existing DjVm file, or manually by calling the default constructor
    and adding later directory entries by means of \Ref{add_file}() function.

    You normally will not want to create or decode this directory manually.
    \Ref{DjVmFile} class will do it for you. */
class DjVmDir0 : public GPEnabled
{
public:
      /** Describes a file record inside a DjVm document (archive) */
   class FileRec;
private:
   GMap<GUTF8String, GP<FileRec> >	name2file;
   GPArray<FileRec>		num2file;
protected:
      /// Default constructor
   DjVmDir0(void) {};
public:
      /// Copy constructor
   DjVmDir0(const DjVmDir0 & d);

   static GP<DjVmDir0> create(void) {return new DjVmDir0;}

   virtual ~DjVmDir0(void) {};

      /// Returns the number of files in the DjVm archive
   int		get_files_num(void) const;
   
      /// Returns the file record with name #name#
   GP<FileRec>	get_file(const GUTF8String &name);

      /// Returns the file record number #file_num#
   GP<FileRec>	get_file(int file_num);

      /** Creates a new file record with name #name# at offset
	  #offset# and size #size#, which is in IFF format if
	  #iff_file# is #TRUE#. */
   void		add_file(const GUTF8String &name, bool iff_file,
			 int offset=-1, int size=-1);

      /// Returns the size of the directory if it were encoded in #DIR0# chunk
   int		get_size(void) const;

      /** Encodes the directory in #DIR0# chunk into the specified
	  \Ref{ByteStream} */
   void		encode(ByteStream & bs);

      /** Decodes the directory from #DIR0# chunk from the specified
	  \Ref{ByteStream} */
   void		decode(ByteStream & bs);

};

      /** Describes a file record inside a DjVm document (archive) */
class DjVmDir0::FileRec : public GPEnabled
{
public:
  /// Name of the file.
  GUTF8String		name;
  /// 1 if the file is in IFF format.
  bool		iff_file;
  /// Offset of the file in the archive.
  int		offset;
  /// Size of the file
  int		size;

  friend int	operator==(const FileRec & f1, const FileRec & f2);

  /// Constructs the #FileRec# object
  FileRec(const GUTF8String &name, bool iff_file,
	      int offset=-1, int size=-1);
  /// Default constructor
  FileRec(void);
  virtual ~FileRec(void);
};

inline
DjVmDir0::FileRec::FileRec(
  const GUTF8String &name_in, bool iff_file_in, int offset_in, int size_in)
: name(name_in), iff_file(iff_file_in), offset(offset_in), size(size_in)
{
}

inline
DjVmDir0::FileRec::FileRec(void) : iff_file(0), offset(-1), size(-1)
{
}

inline
DjVmDir0::FileRec::~FileRec(void)
{
}

inline int
DjVmDir0::get_files_num(void) const
{
   return num2file.size();
}

inline
DjVmDir0::DjVmDir0(const DjVmDir0 & d) :
      name2file(d.name2file), num2file(d.num2file)
{
}

//@}

#endif
