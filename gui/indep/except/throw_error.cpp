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
