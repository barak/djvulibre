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

#ifndef HDR_QD_PANE
#define HDR_QD_PANE
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "GRect.h"
#include "GContainer.h"

#include <qwidget.h>
#include <qregion.h>
#include <qtimer.h>

#include "qt_fix.h"

// The purpose of this class it to do the following things:
//    1. Override some virtual functions, which cause repaint when I don't
//       need it at all (like when I change backgroundPixmap)
//    2. Provide scroll(dx, dy) for UNIX/X11, which will work better than QT

class QDPane : public QWidget, public GPEnabled
{
   Q_OBJECT
private:
#ifdef UNIX
   GC		gc;
   // Stuff needed to enable smooth scrolling (w/o excessive XSync())
#define SHIFT_EXPOSE_QUEUE_SIZE	512
   
   int		shift_expose_by_dx[SHIFT_EXPOSE_QUEUE_SIZE];
   int		shift_expose_by_dy[SHIFT_EXPOSE_QUEUE_SIZE];
   int		shift_expose_cnt;
#endif
   QRegion	invalid_region;
   GList<GRect>	invalid_rects;
   QTimer	timer;

   void		repaintRegion(void);
private slots:
   void		slotTimeout(void);
protected:
      // These two result in the window update after scrolling when I reset
      // the pane's mode from NoBackground to FixedPixmap
   virtual void	backgroundColorChange(const QColor &) {};
   virtual void	backgroundPixmapChange(const QPixmap &) {};

      // These two repaint the pane when mouse just enters or leaves the window
      // How sweet...
   virtual void	focusInEvent(QFocusEvent *) {};
   virtual void	focusOutEvent(QFocusEvent *) {};

#ifdef UNIX
   virtual bool	x11Event(XEvent * ev);
#endif
public:
#ifdef UNIX
   void		scroll(int dx, int dy);
#endif

   bool		canScrollMore(void) const { return shift_expose_cnt<=1; }
   
   QDPane(QWidget * parent=0, const char * name=0);
   ~QDPane(void);
};

#endif
