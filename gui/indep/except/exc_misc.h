//C-  -*- C++ -*-
//C-
//C- DjVu® Unix Viewer (v. 3.5)
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
//C-
// 
// $Id$
// $Name$

#ifndef HDR_EXC_MISC
#define HDR_EXC_MISC
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "exc_base.h"

#define MISC_EXCEPTION(func, msg) MiscException(func, msg, 0, __FILE__, __LINE__)

class MiscException : public Exception
{
public:
   MiscException(const char * func, const char * msg, const char * type=0,
		 const char * file=0, const int line=0) :
      Exception(func, msg, type ? type : "MiscException", file, line) {};
   virtual ~MiscException(void) {};
};

#define BAD_ARGUMENTS(func, msg) BadArguments(func, msg, 0, __FILE__, __LINE__)

class BadArguments : public MiscException
{
public:
   BadArguments(const char * func, const char * msg, const char * type=0,
		const char * file=0, const int line=0) :
      MiscException(func, msg, type ? type : "BadArguments", file, line) {};
   virtual ~BadArguments(void) {};
};

#define DUMMY Dummy(0, __FILE__, __LINE__)

class Dummy : public MiscException
{
public:
   Dummy(const char * type=0, const char * file=0, const int line=0) :
      MiscException("dummy", "dummy", type ? type : "Dummy", file, line) {};
   virtual ~Dummy(void) {};
};

#define INTERRUPTED(func, msg) Interrupted(func, msg, 0, __FILE__, __LINE__)

class Interrupted : public MiscException
{
public:
   Interrupted(const char * func, const char * msg, const char * type=0,
	       const char * file=0, const int line=0) :
      MiscException(func, msg, type ? type : "Interrupted", file, line) {};
   virtual ~Interrupted(void) {};
};

#endif
