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

#ifndef HDR_REM_NETSCAPE
#define HDR_REM_NETSCAPE
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <sys/types.h>

// RemoteNetscape - class for controlling Netscape remotely. X11 specific

// We've got pretty weird types here. All the stuff is to avoid using X11
// and QT headers in one file

class RemoteNetscape
{
private:
   void		* displ;			// Actual type is Display *
   u_long	ref_window, top_window;		// Actual type is Window
   
   int		have_lock;

   void		ObtainLock(void);
   void		ReleaseLock(void);
   void		SendCommand(const char * cmd);
public:
   u_long	GetTopWindow(void);		// Actual type is Window
   void		SendPage(void);

   void		SetRefWindow(u_long _ref_window)	// Window
   {
      ref_window=_ref_window; top_window=0;
   };
   
   RemoteNetscape(void) : displ(0), ref_window(0), top_window(0), have_lock(0) {};
   RemoteNetscape(void * _displ, u_long _ref_window) : displ(_displ),
      ref_window(_ref_window), top_window(0), have_lock(0) {};
   ~RemoteNetscape(void);
};

#endif
