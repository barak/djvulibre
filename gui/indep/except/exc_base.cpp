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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "exc_base.h"
#include "debug.h"

#include <string.h>
#include <stdlib.h>

Exception::Exception(void)
{
   type="Exception"; aldel_type=0;
}

Exception::Exception(const char * _func, const char * _msg,
		     const char * _type, const char * _file, int _line) :
      GException(_msg, _file, _line, _func)
{
   DEBUG_MSG("Exception::Exception(func, msg, type, file, line): Initializing class\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!_type) _type="Exception";
   if ((type=strdup(_type))) aldel_type=1;
   else
   {
      type="Exception";
      aldel_type=0;
   }

   DEBUG_MSG("type=" << type << "\n");
}

Exception::Exception(const Exception & exc) : GException(exc)
{
   DEBUG_MSG("Exception::Exception(exc): Initializing class\n");
   DEBUG_MAKE_INDENT(3);
   
   if ((type=strdup(exc.type))) aldel_type=1;
   else
   {
      type="Exception";
      aldel_type=0;
   }

   DEBUG_MSG("type=" << exc.type << "\n");
}

Exception::~Exception(void)
{
   DEBUG3_MSG("Exception::~Exception(): destroying class\n");
   DEBUG_MAKE_INDENT(3);
   
   DEBUG3_MSG("type=" << type << "\n");
   if (type && aldel_type) free(type);
}

void Exception::SetType(const char * _type)
{
   if (type && aldel_type) free(type); type=0;
   if (!_type) _type="Exception";
   if ((type=strdup(_type))) aldel_type=1;
   else
   {
      type="Exception";
      aldel_type=0;
   }
}

Exception & Exception::operator=(const Exception & exc)
{
   if (this!=&exc)
   {
      DEBUG_MSG("Exception::operator=() called\n");
      DEBUG_MAKE_INDENT(3);

      *((GException *) this)=exc;
      
      DEBUG_MSG("type=" << exc.type << "\n");
	 
      if (type && aldel_type) free(type); type=0;

      if ((type=strdup(exc.type))) aldel_type=1;
      else
      {
	 type="Exception";
	 aldel_type=0;
      }
   }
   return *this;
}
