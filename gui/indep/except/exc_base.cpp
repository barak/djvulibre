//C-  -*- C++ -*-
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
//C- Library notice by the following text (see etc/PATENT.djvu):
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
// 
// $Id$
// $Name$

#ifdef __GNUG__
#pragma implementation
#endif
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
