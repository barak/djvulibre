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

#ifndef HDR_QT_IMAGER
#define HDR_QT_IMAGER
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "int_types.h"

#include <qcolor.h>
#include <qpixmap.h>

// The purpose of this class is to provide standard interface to Imagers
// doing the real job: like QXImager for X11.

// The point is to use this class (include this header) whenever possible
// and deal with QXImager (or QWImager) only when necessary (e.g. when
// initializing)

class QeImager
{
public:
   virtual u_int32	getGrayXColor(float level)=0;
   virtual u_int32	getXColor(u_char r, u_char g, u_char b)=0;
   virtual u_int32	getXColor(u_int32 color)=0;
   virtual u_int32	getXColor(const char *name)=0;

   virtual QColor	getGrayColor(float level)=0;
   virtual QColor	getColor(u_char r, u_char g, u_char b)=0;
   virtual QColor	getColor(u_int32 color)=0;
   virtual QColor	getColor(const char *name)=0;
   virtual QColor	getColor(const QColor & col)=0;

      // Creates a pixmap of the given size, fills it with the given
      // color, dithers according to the depth and returns it.
   virtual QPixmap	getColorPixmap(int width, int height,
				       u_char r, u_char g, u_char b)=0;
   virtual QPixmap	getColorPixmap(int width, int height,
				       u_int32 color)=0;

   QeImager(void);
   virtual ~QeImager(void);
};

extern QeImager	* qeImager;

#endif
