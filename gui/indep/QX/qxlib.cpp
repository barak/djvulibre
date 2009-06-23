//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either Version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C- 
//C- DjVuLibre-3.5 is derived from the DjVu(r) Reference Library from
//C- Lizardtech Software.  Lizardtech Software has authorized us to
//C- replace the original DjVu(r) Reference Library notice by the following
//C- text (see doc/lizard2002.djvu and doc/lizardtech2007.djvu):
//C-
//C-  ------------------------------------------------------------------
//C- | DjVu (r) Reference Library (v. 3.5)
//C- | Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- | The DjVu Reference Library is protected by U.S. Pat. No.
//C- | 6,058,214 and patents pending.
//C- |
//C- | This software is subject to, and may be distributed under, the
//C- | GNU General Public License, either Version 2 of the license,
//C- | or (at your option) any later version. The license should have
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
// $Id: qxlib.cpp,v 1.10 2007/03/25 20:48:24 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qxlib.h"
#include "debug.h"

#include <X11/Xlib.h>

void 
x11Redraw(QWidget * w, const QRect * rect)
{
   QRect qrect;
   if (rect) qrect=*rect;
   else qrect=w->geometry();
   
   XEvent event;
   memset(&event, 0, sizeof(event));
   event.type=Expose;
   event.xexpose.send_event=True;
   event.xexpose.display=w->x11Display();
   event.xexpose.window=(Window) w->handle();
   event.xexpose.x=qrect.x();
   event.xexpose.y=qrect.y();
   event.xexpose.width=qrect.width();
   event.xexpose.height=qrect.height();
   XSendEvent(w->x11Display(), (Window) w->handle(), False, ExposureMask, &event);
}

unsigned long 
x11GetTopLevelWindow(void * _displ, unsigned long _start)
{
   DEBUG_MSG("x11GetTopLevelWindow(): "
             "traversing the tree up to reach the shell...\n");
   DEBUG_MAKE_INDENT(3);

   Display * displ=(Display *) _displ;
   Window ref_window=(Window) _start;
   
   if (!displ || !ref_window)
     return 0;
   
   Atom atom_wm=XInternAtom(displ, "WM_STATE", True);
   if (!atom_wm) 
     return 0;

   Window shell_win=0;
   Window cur_win=ref_window;
   while(1)
   {
      DEBUG_MSG("looking at window=" << cur_win << "\n");
      
      int props;
      Atom * prop=XListProperties(displ, cur_win, &props);
      if (prop && props)
      {
	 int i;
	 for(i=0;i<props;i++)
	    if (prop[i]==atom_wm) break;
	 XFree(prop);
	 if (i<props)
	 {
	    shell_win=cur_win;
	    break;
	 }
      }
      
      Window root, parent, * child;
      u_int childs;
      if (XQueryTree(displ, cur_win, &root, &parent, &child, &childs))
      {
	 if (child) XFree(child);
	 if (parent==root)
	 {
	    shell_win=cur_win;
	    break;
	 }
	 
	 cur_win=parent;
      }
   }
   DEBUG_MSG("got window=" << shell_win << "\n");
   return shell_win;
}


unsigned long 
x11GetTopLevelWindow(QWidget *w)
{
  return x11GetTopLevelWindow(w->x11Display(), w->winId());
}


void 
x11MakeTransient(QWidget *w, QWidget *fw)
{
  if (fw) 
    {
      unsigned long xfw = x11GetTopLevelWindow(fw);
      if (w && xfw)
        XSetTransientForHint(w->x11Display(), w->winId(), xfw);
    }
}
