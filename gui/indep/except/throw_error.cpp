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
//C- Library notice by the following text (see doc/lizard2002.djvu):
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

// GCC 2.8.1 on RedHat 4.0 can't compile this file with -DDEBUG_i and -O2
// Disable debug messages locally.
#include "debug.h"
#undef DEBUG_MSG
#define DEBUG_MSG(x)
#undef DEBUG_MSGF
#define DEBUG_MSGF(x)

#include "throw_error.h"
#include "exc_msg.h"
#include "exc_sys.h"

#include <stdio.h>

#define TRY_CASE(name) case E##name: throw EXC_##name(func, message);

void
ThrowError(const char * func, const GUTF8String & msg, int in_errno)
{
   if (in_errno<0) in_errno=errno;

   DEBUG_MSG("ThrowError(): func=" << func << ", errno=" << in_errno << "\n");
   DEBUG_MSG("\tmsg=" << (const char *) msg << "\n");

   // CAUTION 
   // changes for the following must be coordinated with
   // QeExcMessage::QeExcMessage(...) and 
   // QeExcMessage::switchDetails(...)
   GUTF8String message=msg;
   message+=":\n";
   message+=strerror(in_errno);
   message+='\n';
   // END CAUTION
   
   switch(errno)
   {
#ifdef EPERM
      TRY_CASE(PERM);
#endif
#ifdef ENOENT
      TRY_CASE(NOENT);
#endif
#ifdef ESRCH
      TRY_CASE(SRCH);
#endif
#ifdef EINTR
      TRY_CASE(INTR);
#endif
#ifdef EIO
      TRY_CASE(IO);
#endif
#ifdef ENXIO
      TRY_CASE(NXIO);
#endif
#ifdef E2BIG
      TRY_CASE(2BIG);
#endif
#ifdef ENOEXEC
      TRY_CASE(NOEXEC);
#endif
#ifdef EBADF
      TRY_CASE(BADF);
#endif
#ifdef ECHILD
      TRY_CASE(CHILD);
#endif
#ifdef EAGAIN
      TRY_CASE(AGAIN);
#endif
#ifdef ENOMEM
      TRY_CASE(NOMEM);
#endif
#ifdef EACCES
      TRY_CASE(ACCES);
#endif
#ifdef EFAULT
      TRY_CASE(FAULT);
#endif
#ifdef ENOTBLK
      TRY_CASE(NOTBLK);
#endif
#ifdef EBUSY
      TRY_CASE(BUSY);
#endif
#ifdef EEXIST
      TRY_CASE(EXIST);
#endif
#ifdef EXDEV
      TRY_CASE(XDEV);
#endif
#ifdef ENODEV
      TRY_CASE(NODEV);
#endif
#ifdef ENOTDIR
      TRY_CASE(NOTDIR);
#endif
#ifdef EISDIR
      TRY_CASE(ISDIR);
#endif
#ifdef EINVAL
      TRY_CASE(INVAL);
#endif
#ifdef ENFILE
      TRY_CASE(NFILE);
#endif
#ifdef EMFILE
      TRY_CASE(MFILE);
#endif
#ifdef ENOTTY
      TRY_CASE(NOTTY);
#endif
#ifdef ETXTBSY
      TRY_CASE(TXTBSY);
#endif
#ifdef EFBIG
      TRY_CASE(FBIG);
#endif
#ifdef ENOSPC
      TRY_CASE(NOSPC);
#endif
#ifdef ESPIPE
      TRY_CASE(SPIPE);
#endif
#ifdef EDOM
      TRY_CASE(DOM);
#endif
#ifdef ERANGE
      TRY_CASE(RANGE);
#endif
#ifdef ENOMSG
      TRY_CASE(NOMSG);
#endif
#ifdef EDEADLK
      TRY_CASE(DEADLK);
#endif
#ifdef ENOLCK
      TRY_CASE(NOLCK);
#endif
      default: 
        throw SYSTEM_ERROR(func, message);
   }
}
