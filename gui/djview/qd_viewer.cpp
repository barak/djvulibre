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
// $Id: qd_viewer.cpp,v 1.42 2008/08/05 20:53:24 bpearlmutter Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qd_viewer.h"
#include "debug.h"
#include "qlib.h"
#include "qxlib.h"
#include "qx_imager.h"
#include "qd_welcome.h"
#include "GURL.h"
#include "GOS.h"
#include "execdir.h"
#include "names.h"
#include "DjVuFileCache.h"
#include "DjVuMessage.h"

#include "qd_viewer_prefs.h"
#include "djvu_base_res.h"
#include "qd_print_dialog.h"
#include "qd_about_dialog.h"
#include "qd_doc_saver.h"
#include "qd_page_saver.h"
#include "qd_search_dialog.h"
#include "qd_tbar_nav_piece.h"
#include "qd_tbar_print_piece.h"
#include "qd_thumb.h"

#include <qkeycode.h>
#include <qsocketnotifier.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <ctype.h>
#include <errno.h>

#ifdef UNIX
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#ifdef UNIX
#define DJVU_ZOOM_PROP		"_DJVU_ZOOM"
#define DJVU_DETACH_TIME_PROP   "_DJVU_DETACH_TIME"
#endif

bool QDViewer::once_welcomed=false;

bool
QDViewer::PluginData::parseBool(const GUTF8String &qvalue_in)
{
  DEBUG_MSG("QDViewer::PluginData::parseBool()\n");
  GUTF8String value=qvalue_in.downcase();
  return !(value=="false" || value=="no");
}

static inline void
set_reset(int &x, bool plus, bool minus, int y)
{
  if (minus)
    x |= y;
  else if (plus)
    x &= ~y;
}

void
QDViewer::PluginData::parsePair(const GUTF8String &qname_in, 
                                const GUTF8String &qvalue)
{
  DEBUG_MSG("QDViewer::PluginData::parsePair()\n");
  DEBUG_MAKE_INDENT(3);
  DEBUG_MSG("name=" << qname_in << "; " << "value=" << qvalue << "\n");
  GUTF8String name=qname_in.downcase();

  if (name=="passive" || name=="passivestretch")
    {
      toolbar=scrollbars=menu=keyboard=false;
      thumbnails=THUMB_HIDE;
      if (name=="passivestretch") 
        cmd_zoom=IDC_ZOOM_STRETCH;
      else 
        cmd_zoom=IDC_ZOOM_PAGE;
    } 
  else if (name=="rotate") 
    {
      GUTF8String str=qvalue.downcase();
      if (str=="0")
        cmd_rotate = IDC_ROTATE_0;
      else if (str=="90")
        cmd_rotate = IDC_ROTATE_90;
      else if (str=="180")
        cmd_rotate = IDC_ROTATE_180;
      else if (str=="270")
        cmd_rotate = IDC_ROTATE_270;
    }
  else if (name=="toolbar") 
    {
      toolbar = true;
      bool minus = false;
      bool plus = false;
      int pos = 0;
      int npos = pos;
      GUTF8String str=qvalue.downcase();
      while (npos < (int)str.length())
        {
          pos = str.nextNonSpace(npos);
          npos = str.contains(" [],+-", pos);
          if (npos < 0)
            npos = str.length();
          GUTF8String key = str.substr(pos, npos-pos);
          npos = str.nextNonSpace(npos);
          if (key=="no" || key=="false")
            toolbar = minus;
          if (key=="yes" || key=="true")
            toolbar = !minus;
          else if (key=="bottom" && !plus && !minus)
            toolbaropts |= OverrideFlags::TOOLBAR_BOTTOM;
          else if (key == "top" && !plus && !minus)
            toolbaropts |= OverrideFlags::TOOLBAR_TOP;
          else if (key == "auto" && !plus && !minus)
            toolbaropts |= OverrideFlags::TOOLBAR_AUTO;
          else if (key == "always" && !plus && !minus)
            toolbaropts |= OverrideFlags::TOOLBAR_ALWAYS;
          else if (key == "fixed" && !plus && !minus)
            toolbaropts |= OverrideFlags::TOOLBAR_ALWAYS;
          else if (key=="fore" || key=="back" || 
                   key=="color" || key=="bw" ||
                   key=="fore_button" || key=="back_button" || 
                   key=="color_button" || key=="bw_button")
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_DISPCOMBO);
          else if (key=="rescombo")
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_RESCOMBO);
          else if (key=="zoom")
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_ZOOM);
          else if (key=="pan")
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_PAN);
          else if (key=="zoomsel")
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_ZOOMSEL);
          else if (key=="textsel")
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_TEXTSEL);
          else if (key=="rotate")
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_ROTATE);
          else if (key=="search")
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_SEARCH);
          else if (key=="print")
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_PRINT);
          else if (key=="save")
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_SAVE);
          else if (key=="pagecombo")
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_PAGECOMBO);
          else if (key=="backforw")
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_BACKFORW);
          else if (key=="firstlast" || key=="firstlastpage" )
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_FIRSTLAST);
          else if (key=="prevnext" || key=="prevnextpage" ) 
            set_reset(toolbaropts, plus, minus, OverrideFlags::TOOLBAR_NO_PREVNEXT);
          if (npos < (int)str.length())
            {
              if (str[npos] == '-')
                {
                  plus = false;
                  minus = true;
                }
              else if (str[npos] == '+')
                {
                  if (!minus && !plus)
                    toolbaropts |= OverrideFlags::TOOLBAR_NO_BUTTONS;
                  minus = false;
                  plus = true;
                }
              npos += 1;
            }
        }
    }
  else if (name=="notoolbar") 
    toolbar=false;
  else if (name=="scrollbars") 
    scrollbars=parseBool(qvalue);
  else if (name=="noscrollbars")
    scrollbars=false;
  else if (name=="menu")
    menu=parseBool(qvalue);
  else if (name=="nomenu") 
    menu=false;
  else if (name=="frame")
    frame=parseBool(qvalue);
  else if (name=="links")
    links=parseBool(qvalue);
  else if (name=="logo")
    logo=parseBool(qvalue);
  else if (name=="keyboard") 
    keyboard=parseBool(qvalue);
  else if (name=="cache")
    cache=parseBool(qvalue) ? CACHE_ON : CACHE_OFF;
  else if (name=="print") 
    print=parseBool(qvalue);
  else if (name=="thumbnails")
    {
      GUTF8String str=qvalue.downcase();
      if (str=="top") 
        thumbnails=THUMB_TOP;
      else if (str=="bottom") 
        thumbnails=THUMB_BOTTOM;
      else if (str=="left" || str=="yes" || str=="true") 
        thumbnails=THUMB_LEFT;
      else if (str=="right") 
        thumbnails=THUMB_RIGHT;
      else
        thumbnails=THUMB_HIDE;
    } 
  else if (name=="zoom")
    {
      GUTF8String str=qvalue.downcase();
      if (str=="one2one") 
        cmd_zoom=IDC_ZOOM_ONE2ONE;
      else if (str=="width") 
        cmd_zoom=IDC_ZOOM_WIDTH;
      else if (str=="page") 
        cmd_zoom=IDC_ZOOM_PAGE;
      else if (str=="stretch") 
        cmd_zoom=IDC_ZOOM_STRETCH;
      else
        {
          const char * ptr=str;
          int num=strtol(ptr, (char **) &ptr, 10);
          if (!*ptr && num>=5 && num<=999)
	    cmd_zoom=IDC_ZOOM_MIN+num;
        }
    } 
  else if (name=="mode")
    {
      GUTF8String str=qvalue.downcase();
      if (str=="color") 
        cmd_mode=IDC_DISPLAY_COLOR;
      else if (str=="bw") 
        cmd_mode=IDC_DISPLAY_BLACKWHITE;
      else if (str=="fore") 
        cmd_mode=IDC_DISPLAY_FOREGROUND;
      else if (str=="back") 
        cmd_mode=IDC_DISPLAY_BACKGROUND;
   } 
  else if (name=="url") 
    rel_url=qvalue;
  else if (name=="page") 
    page_id=qvalue;
  else if (name=="highlight")
    {
      int x=-1, y=-1, w=-1, h=-1;
      char color_str[1024]; color_str[0]=0;
      const char * const value=qvalue;
      sscanf(value, "%d,%d,%d,%d,%s", &x, &y, &w, &h, color_str);
      color_str[1023]=0;
      GRect rect(x, y, w, h);
      if (rect.xmin<0) 
        rect.xmin=0;
      if (rect.ymin<0) 
        rect.ymin=0;
      if (rect.width()>0 && rect.height()>0)
        {
          u_int32 color=GMapArea::XOR_HILITE;
          if (strlen(color_str))
	    color=DjVuANT::cvt_color(GUTF8String("#")+color_str,GMapArea::XOR_HILITE);
          GP<GMapRect> gmap_rect=GMapRect::create(rect);
          gmap_rect->border_type=GMapArea::NO_BORDER;
          gmap_rect->hilite_color=color;
          hilite_rects.append(gmap_rect);
        }
    } 
  else if (name=="hor_align")
    {
      GUTF8String val=qvalue.downcase();
      if (val=="left")
        hor_align=DjVuANT::ALIGN_LEFT;
      else if (val=="right")
        hor_align=DjVuANT::ALIGN_RIGHT;
      else if (val=="center")
        hor_align=DjVuANT::ALIGN_CENTER;
    } 
  else if (name=="ver_align")
    {
      GUTF8String val=qvalue.downcase();
      if (val=="bottom") 
        ver_align=DjVuANT::ALIGN_BOTTOM;
      else if (val=="top")
        ver_align=DjVuANT::ALIGN_TOP;
      else if (val=="center") 
        ver_align=DjVuANT::ALIGN_CENTER;
    }
}

GUTF8String
QDViewer::PluginData::getQuotedString(const char * & ptr)
{
   DEBUG_MSG("QDViewer::PluginData::getQuotedString()\n");
   GUTF8String val;
   for(ptr++;*ptr;ptr++)
     if (*ptr=='\'') 
       { ptr++; break; }
     else 
       { val+=*ptr; }
   return val;
}

void
QDViewer::PluginData::parse(const DArray<GUTF8String> & argn,
			    const DArray<GUTF8String> & argv)
{
   DEBUG_MSG("QDViewer::PluginData::parse()\n");
   DEBUG_MAKE_INDENT(3);
   
   for(int i=0;i<argn.size();i++)
     if (argn[i].length())
       {
	 GUTF8String name=argn[i].downcase();
	 GUTF8String value;
	 if (i<argv.size())
	   value=argv[i];
	 if (name!="flags")
           {
             parsePair(name, value);
           }
         else
           {
             const char * ptr=value;
             while(*ptr)
               {
                 GUTF8String xname, xvalue;
                 while(true)
                   {
                     while(*ptr && isspace(*ptr)) 
                       ptr++;
                     if (*ptr=='\'') 
                       xname+=getQuotedString(ptr);
                     else if (!*ptr || *ptr=='=' || isspace(*ptr)) 
                       break;
                     else 
                       xname+=*ptr++;
                   }
                 while(*ptr && *ptr=='=') 
                   ptr++;
                 while(true)
                   {
                     if (*ptr=='\'') 
                       xvalue += getQuotedString(ptr);
                     else if (!*ptr || isspace(*ptr)) 
                       break;
                     else 
                       xvalue+=*ptr++;
                   }
                 parsePair(xname, xvalue);
               }
           }
       }
}

QDViewer::PluginData::PluginData(const DArray<GUTF8String> & argn,
				 const DArray<GUTF8String> & argv)
{
  full_mode=true;
  cache=CACHE_UNSPEC;
  parse(argn, argv);
}

QDViewer::PluginData::PluginData(void)
{
   full_mode=true;
   cache=CACHE_UNSPEC;
}

SavedData
QDViewer::getSavedData(void) const
{
   SavedData data;
   data.cmd_mode=getMode(true);	// Disregard mode forced by image structure
   data.cmd_zoom=getCMDZoom();
   data.cmd_rotate=getRotate();
   data.imgx=rectDocument.xmin;
   data.imgy=rectDocument.ymin;
   return data;
}

QDViewer::QDViewer(int _in_netscape,
		   const PluginData & _plugin_data,
		   QWidget * parent, const char * name) :
      QDBase(parent, name), in_netscape(_in_netscape),
      plugin_data(_plugin_data), welcome(0), about(0), page_port(0, 0), doc_port(0, 0)
{
   DEBUG_MSG("QDViewer::QDViewer(): Initializing class...\n");
   DEBUG_MAKE_INDENT(3);

   if (!in_netscape) plugin_data.full_mode=1;

   page_port.getMessenger().setLookAhead(1);

   thumbnails=0;
   nav_tbar=0;
   popup_menu=0;
   start_page_num=0;
   print_win=0;
   
   connect(&hlinks_timer, SIGNAL(timeout(void)),
	   this, SLOT(slotEnableDisplayAllHLinks(void)));

   connect(&page_port, SIGNAL(sigNotifyRedisplay(const GP<DjVuImage> &)),
	   this, SLOT(slotNotifyRedisplay(const GP<DjVuImage> &)));
   connect(&page_port, SIGNAL(sigNotifyRelayout(const GP<DjVuImage> &)),
	   this, SLOT(slotNotifyRelayout(const GP<DjVuImage> &)));
   connect(&page_port, SIGNAL(sigNotifyChunkDone(const GP<DjVuPort> &, const GUTF8String &)),
	   this, SLOT(slotNotifyChunkDone(const GP<DjVuPort> &, const GUTF8String &)));
   connect(&page_port, SIGNAL(sigNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)),
	   this, SLOT(slotNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)));
   connect(&doc_port, SIGNAL(sigNotifyDocFlagsChanged(const GP<DjVuDocument> &, long, long)),
	   this, SLOT(slotNotifyDocFlagsChanged(const GP<DjVuDocument> &, long, long)));

   createPopupMenu();
   createToolBar();		// Recreating, actually

   setOverrideFlags(plugin_data);
   
   if (!plugin_data.saved.isEmpty())
   {
      DEBUG_MSG("cmd_mode=" << plugin_data.saved.cmd_mode << "\n");
      DEBUG_MSG("cmd_zoom=" << plugin_data.saved.cmd_zoom << "\n");
      DEBUG_MSG("imgx=" << plugin_data.saved.imgx << "\n");
      DEBUG_MSG("imgy=" << plugin_data.saved.imgy << "\n");
   
      setRotate(plugin_data.saved.cmd_rotate, 0, SRC_SAVED);	// Don't redraw or layout
      setZoom(plugin_data.saved.cmd_zoom, 0, SRC_SAVED);	// Don't layout
      setMode(plugin_data.saved.cmd_mode, 0, SRC_SAVED);	// Don't redraw
      layout(0);				// Don't redraw
      rectDocument.xmax=rectDocument.xmin=plugin_data.saved.imgx;
      rectDocument.ymax=rectDocument.ymin=plugin_data.saved.imgy;
      saved_data_known=1;
      ignore_ant_mode_zoom=1;
   } else saved_data_known=0;
   
   if (!saved_data_known)
     {
       // Process the zoom and mode specified in the EMBED flags
       if (plugin_data.cmd_rotate>0)
         setRotate(plugin_data.cmd_rotate, true, SRC_TAGS);
       if (plugin_data.cmd_zoom<=0)
         setZoom(prefs.nDefaultZoom, true, SRC_DEFAULT);
       else 
         setZoom(plugin_data.cmd_zoom, true, SRC_TAGS);
       if (plugin_data.cmd_mode<=0) 
         setMode(IDC_DISPLAY_COLOR, true, SRC_DEFAULT);
       else 
         setMode(plugin_data.cmd_mode, true, SRC_TAGS);
     }
   pane->setFocus();
}

QDViewer::~QDViewer(void)
{
   DEBUG_MSG("QDViewer::~QDViewer(): destroying class...\n");
#ifdef UNIX
   Display * displ=x11Display();
   Atom atom_zoom=XInternAtom(displ, DJVU_ZOOM_PROP, False);
   Atom atom_time=XInternAtom(displ, DJVU_DETACH_TIME_PROP, False);
   if (plugin_data.full_mode && atom_zoom && atom_time)
   {
      DEBUG_MSG("saving current zoom into toplevel window properties\n");
      Window shell_win = x11GetTopLevelWindow(x11Display(), winId());
      if (shell_win)
      {
	 char buffer[128];
	 sprintf(buffer, "%d", getCMDZoom());
	 XChangeProperty(displ, shell_win, atom_zoom, XA_STRING,
			 8, PropModeReplace, (u_char *) buffer, strlen(buffer)+1);
	 sprintf(buffer, "%d", (int) time(0));
	 XChangeProperty(displ, shell_win, atom_time, XA_STRING,
			 8, PropModeReplace, (u_char *) buffer, strlen(buffer)+1);
      }
   }
#else
   ::showInfo(this, "DjVu Info",
	      "Do smth with the Netscape's properties in QDViewer::~QDViewer\n");
#endif
      // Don't wait. There were some deadlocks.
   if (dimg) dimg->get_djvu_file()->stop_decode(false);
}

void
QDViewer::layout(bool allow_redraw)
{
   QDBase::layout(allow_redraw);
   if (print_win)
      print_win->setCurZoom(getZoom());
}

void
QDViewer::setCaption(void)
{
   if (dimg)
   {
      QWidget * w=this;
      while(w->parentWidget()) w=w->parentWidget();
      QString mesg=tr("DjView - ")+
	 QStringFromGString((dimg->get_djvu_file()->get_url()).UTF8Filename());
      w->setCaption(mesg);
   }
}

void
QDViewer::savePageAs(void)
{
   DEBUG_MSG("QDViewer::savePageAs(): saving page...\n");
   DEBUG_MAKE_INDENT(3);

      // TODO: Make in another thread
   if (!dimg || !dimg->get_djvu_file()->is_all_data_present())
   {
      DEBUG_MSG("dimg==0 or it's not fully decoded => return\n");
      return;
   }

   GP<DjVuFile> djvu_file=dimg->get_djvu_file();

   QDPageSaver page_saver(djvu_file, this);
   page_saver.save();
}

void
QDViewer::saveDocumentAs(void)
{
   DEBUG_MSG("QDViewer::saveDocumentAs(): saving document...\n");

   QDDocSaver ds(djvu_doc, this);
   ds.save();
}
   
bool
QDViewer::event(QEvent * ev)
{
   try
   {
      if (ev->type()==QEvent::Show)
      {
	 DEBUG_MSG("QDViewer::event(): got QEvent::Show\n");
	 DEBUG_MAKE_INDENT(3);

	 checkWMProps();

	 if (prefs.bBeginner && !once_welcomed && in_netscape)
	 {
	       // In QDWelcome we set TransientFor property. Looks like that
	       // it's too early to do it here. So - delay the QDWelcome
	       // display a bit
	    welcome_timer.start(0, TRUE);
	    connect(&welcome_timer, SIGNAL(timeout(void)), this, SLOT(slotShowWelcome(void)));
	 }
      }	// if (ev->type()==QEvent::Show)
   } catch(const GException & exc)
   {
      showError(this, exc);
   }
   return QDBase::event(ev);
}

bool
QDViewer::eventFilter(QObject *obj, QEvent *e)
{
   OverrideFlags override_flags=getOverrideFlags();

   if (obj!=pane)
      return QDBase::eventFilter(obj, e);

   try
   {
      switch(e->type())
      {
         case QEvent::Enter:
         {
           if (in_netscape)
             {
               DEBUG_MSG("QDViewer::eventFilter(): Got QEvent::Enter\n");
               // Reparenting prevent the window manager to send proper focus events.
               Window w;
               XEvent focusevent;
               focusevent.type = FocusIn;
               focusevent.xfocus.window = w  = topLevelWidget()->winId();
               focusevent.xfocus.mode = NotifyNormal;
               focusevent.xfocus.detail = NotifyInferior;
               XSendEvent(x11Display(), w, False, NoEventMask, &focusevent);
             }
           return FALSE;
         }

	 case QEvent::MouseButtonPress:
	 {
	    DEBUG_MSG("QDViewer::eventFilter(): Got QEvent::MousePress\n");
	    DEBUG_MAKE_INDENT(3);
	    
	    QMouseEvent * ev=(QMouseEvent *) e;
	    if (ev->button()==RightButton)
	    {
	       if (!override_flags.menu) break;

	       if (ev->state() & LeftButton)
	       {
		     // Simulate Btn1Up event
		  QMouseEvent sev(QEvent::MouseButtonRelease,
				  ev->pos(), LeftButton, ev->state());
		  QApplication::sendEvent(pane, &sev);
	       }
	       
	       runPopupMenu(ev);
	       return TRUE;
	    }
	    break;
	 }
#ifdef KeyRelease
#undef KeyRelease
#endif
	 case QEvent::KeyRelease:
	 {
	    DEBUG_MSG("QDViewer::eventFilter(): Got QEvent::KeyRelease\n");
	    DEBUG_MAKE_INDENT(3);

	    hlinks_timer.stop();
	    disableDisplayAllHLinks();
	    
	    break;
	 }
#ifdef KeyPress
#undef KeyPress
#endif	    
	 case QEvent::KeyPress:
	 {
	    DEBUG_MSG("QDViewer::eventFilter(): Got QEvent::KeyPress\n");
	    DEBUG_MAKE_INDENT(3);

	    if (override_flags.keyboard)
	    {
	       int doc_page=0, doc_pages=1;
	       if (dimg && e->type()!=QEvent::MouseMove)
	       {
		  doc_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
		  doc_pages=djvu_doc->get_pages_num();
	       }
      
	       QKeyEvent * ev=(QKeyEvent *) e;

		  // Guess if it's necessary to display all hlinks
	       int display=0;
	       hlinks_timer.stop();
	       disableDisplayAllHLinks();
	       switch(prefs.hlb_num)
	       {
		  case DjVuPrefs::HLB_SHIFT:
		     display=!ev->state() && ev->key()==Key_Shift; break;
		  case DjVuPrefs::HLB_SHIFT_SHIFT:
		     display=(ev->state()==ShiftButton) && ev->key()==Key_Shift; break;
		  case DjVuPrefs::HLB_CTRL:
		     display=!ev->state() && ev->key()==Key_Control; break;
		  case DjVuPrefs::HLB_CTRL_CTRL:
		     display=(ev->state()==ControlButton) && ev->key()==Key_Control; break;
		  case DjVuPrefs::HLB_ALT:
		     display=!ev->state() && ev->key()==Key_Alt; break;
		  case DjVuPrefs::HLB_ALT_ALT:
		     display=(ev->state()==AltButton) && ev->key()==Key_Alt; break;
		  case DjVuPrefs::HLB_ITEMS:
		     break;	// For compiler not to warn
	       }
	       if (display)
	       {
		  DEBUG_MSG("displaying all hlinks in 500 ms...\n");
		  hlinks_timer.start(500, TRUE);
	       }

		  // Go on with key processing
	       switch(ev->key())
	       {
               case Key_F:
               case Key_F3:
                 search();
                 break;
                 
               case Key_G:
                 processCommand(IDC_NAV_GOTO_PAGE);
                 break;
                 
               case Key_Home:
                 if (ev->state() & ControlButton)
                   {
                     gotoPage(0); 
                     return TRUE;
                   }
                 break;
                 
               case Key_End:
                 if (ev->state() & ControlButton)
                   {
                     gotoPage(doc_pages-1);
                     return TRUE;
                   }
                 break;
                 
               case Key_PageUp:
                 if (doc_page>0 && dimg && dimg->get_info())
                   {
                     gotoPage(doc_page-1, false);
                     return TRUE;
                   }
                 break;
                 
               case Key_Backspace:
                 if (rectDocument.ymin>=rectVisible.ymin && doc_page>0 &&
                     dimg && dimg->get_info())
                   {
                     gotoPage(doc_page-1, false);
                     if (dimg && dimg->get_height() && dimg->get_width()) 
                       {
                         // This will only work if the image is cached.
                         scroll(rectDocument.xmax-rectVisible.xmax,
                                rectDocument.ymax-rectVisible.ymax);
                       }
                     return TRUE;
                   }
                 break;
                 
               case Key_PageDown:
                 if (doc_page<doc_pages-1 && dimg && dimg->get_info())
                   {
                     gotoPage(doc_page+1, false);
                     return TRUE;
                   }
                 
               case Key_Space:
               case Key_Return:
               case Key_Enter:
                 if (rectDocument.ymax<=rectVisible.ymax &&
                     doc_page<doc_pages-1 && dimg && dimg->get_info())
                   {
                     gotoPage(doc_page+1, false);
                     return TRUE;
                   }
                 break;
#ifdef QT1
               case Key_Hyper_L:
               case Key_Hyper_R:
               case Key_Pause:
#else
               case Key_Menu:
               case Key_Pause:
#endif
                 runPopupMenu(0);
                 break;
                 
               case '?':
                 if (in_netscape) 
                   slotHelp();
                 break;
                 
               default:
                 break;
	       } // switch(ev->key())
	    } // if (override_flags.keyboard)
	 } // case QEvent::KeyPress
      default:
        break;
      } // switch(e->type())
   } 
   catch(const GException & exc)
   {
     showError(this, exc);
   }
   return QDBase::eventFilter(obj, e);
}

void
QDViewer::setDjVuImage(const GP<DjVuImage> & _dimg, int do_redraw)
{
   DEBUG_MSG("QDViewer::setDjVuImage()\n");
   DEBUG_MAKE_INDENT(3);

   if (dimg)
   {
	 // Save old image into "predecode_" fields for caching purposes
      GURL old_url=dimg->get_djvu_file()->get_url();
      GURL new_url=_dimg->get_djvu_file()->get_url();
      GURL predecode_url1, predecode_url2;
      if (predecode_dimg1)
	 predecode_url1=predecode_dimg1->get_djvu_file()->get_url();
      if (predecode_dimg2)
	 predecode_url2=predecode_dimg2->get_djvu_file()->get_url();
      if (predecode_url1==new_url) { predecode_dimg1=0; predecode_url1=GURL(); }
      if (predecode_url2==new_url) { predecode_dimg2=0; predecode_url2=GURL(); }
      if (!predecode_dimg1) predecode_dimg1=dimg;
      else if (old_url!=predecode_url1) predecode_dimg2=dimg;
   }
   
   DjVuPortcaster * pcaster=DjVuPort::get_portcaster();
   if (dimg) pcaster->del_route(dimg, page_port.getPort());
   if (_dimg) pcaster->add_route(_dimg, page_port.getPort());
   QDBase::setDjVuImage(_dimg, do_redraw);

   if (dimg)
   {
      int page_num=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
      emit sigPageChanged(page_num);

      setCaption();

      if (thumbnails) thumbnails->setCurPageNum(page_num);

	 // If the file's already decoded => predecode next and previos pages.
	 // Otherwise we will initiate it in the notify_file_done() callback
      if (!dimg->get_djvu_file()->is_decoding()) predecode();
   }
}

void
QDViewer::setDjVuDocument(GP<DjVuDocument> & doc, const GUTF8String &qkey_in)
{
   DEBUG_MSG("QDViewer::setDjVuDocument()\n");
   DEBUG_MAKE_INDENT(3);

   DjVuPortcaster * pcaster=DjVuPort::get_portcaster();
   if (djvu_doc) pcaster->del_route(djvu_doc, doc_port.getPort());
   djvu_doc=doc;
   pcaster->add_route(djvu_doc, doc_port.getPort());

   // Check the page key
   GUTF8String key = qkey_in;
   GP<DjVuImage> new_dimg;
   if (key[0] != '$')
     new_dimg = doc->get_page(key, false, page_port.getPort());
   if (! new_dimg)
     {
       if (key[0] == '$')
         key = GUTF8String(((const char*)key)+1);
       if (key.is_int())
         new_dimg=doc->get_page(key.toInt()-1, false, page_port.getPort());
     }
   
   if (!new_dimg) 
     {
       if (doc->is_init_failed())
         G_THROW(ERR_MSG("QDViewer.corrupt"));
       else
         G_THROW(ERR_MSG("QDViewer.pagekey_not_found") 
                 + GUTF8String("\t") 
                 + qkey_in);
     }
   setDjVuImage(new_dimg, 1);

   hundo.empty();
   hredo.empty();
   if (doc->get_pages_num()>1 && plugin_data.thumbnails) 
     showThumbnails();
   if (thumbnails)
     thumbnails->setDjVuDocument(doc);

   if (doc->is_init_complete())
   {
      start_page_num=doc->url_to_page(dimg->get_djvu_file()->get_url());
      if (plugin_data.rel_url.length())
      {
	 plugin_data.url=GURL::UTF8(plugin_data.rel_url,doc->get_init_url().base());
	 setOverrideFlags(plugin_data);
      }
   }
}

void
QDViewer::imageUpdated(void)
{
   QDBase::imageUpdated();
   setCaption();
}

void
QDViewer::slotPrintClosed(void)
{
   qeApp->killWidget(print_win);
   print_win=0;
}

void
QDViewer::print(int what)
{
   if (dimg && dimg->get_width() && dimg->get_height())
   {
      if (!getOverrideFlags().print)
	 showError(this, tr("Printing prohibited"),
		   tr("Printing of this image\nhas been disallowed."));
      else
      {
	 if (print_win)
	    print_win->raise();
	 else
	 {
	    GRect prn_rect=rectVisible;
	    ma_mapper.unmap(prn_rect);
	    print_win=new QDPrintDialog(djvu_doc, dimg, &prefs, getMode(true),
					getZoom(), prn_rect, this,
					"djvu_print", FALSE);
	    print_win->setPrint(what==IDC_PRINT_WIN ? QDPrintDialog::PRINT_WIN :
				what==IDC_PRINT_CUSTOM ? QDPrintDialog::PRINT_CUSTOM :
				what==IDC_PRINT_DOC ? QDPrintDialog::PRINT_DOC :
				QDPrintDialog::PRINT_PAGE);
	    connect(print_win, SIGNAL(sigDone(void)),
		    this, SLOT(slotPrintClosed(void)));
	    print_win->show();
	 }
      }
   }
}

void
QDViewer::showUpgradeDialog(void)
{
   if (in_netscape)
   {
     if (!QMessageBox::information(this, tr("Upgrade DjVu now!"),
              tr("You are using an obsolete version of the\nDjVu plugin. "
                 "To install the newest\nversion click \"Download\" and "
                 "follow the\ninstructions.\n"),
              tr("&Download"), tr("&Ignore"), 0, 0, 1))
       getURL(DJVIEW_DOWNLOAD_URL, "_blank");
   } else
     ::showError(this, tr("Upgrade DjVu now!"),
                 tr("You are using an obsolete version of the\nDjVu plugin. "
                    "Please upgrade the viewer to\nview this document.\n"));
}

//************************** QDPort slots ************************************

void
QDViewer::slotNotifyRedisplay(const GP<DjVuImage> &)
      // Connected to the current page only
{
   redraw();
}

void
QDViewer::slotNotifyRelayout(const GP<DjVuImage> &)
      // Connected to the current page only
{
   layout();
}

void
QDViewer::slotNotifyChunkDone(const GP<DjVuPort> &, const GUTF8String &name)
      // Connected to the current page only
{
   DEBUG_MSG("QDViewer::slotNotifyChunkDone\n");
   DEBUG_MAKE_INDENT(3);

   DEBUG_MSG("name=" << name <<"\n");
   
      // Make sure, that hilite rectangles from OverrideFlags get created
   if (name=="INFO")
     createMapAreas(true);
   
      // Process decoded annotations. Note, that we may need to process
      // them later again as new ANTa and ANTz chunks get decoded
   if ((name=="ANTa") || (name=="ANTz") || (name=="FORM:ANNO"))
      decodeAnno(true);
   else
      imageUpdated();
}

void
QDViewer::slotNotifyFileFlagsChanged(const GP<DjVuFile> & source, long set_mask, long)
      // Connected to the current page only
{
   if (set_mask & DjVuFile::DECODE_FAILED)
   {
      DEBUG_MSG("QDViewer::slotNotifyFileFlagsChanged(): decoding failed!\n");
      DEBUG_MAKE_INDENT(3);

      if (dimg && dimg->get_djvu_file()==source)
      {
	    // Complain if plugin version does not match
	 if (dimg->get_version()>DJVUVERSION) showUpgradeDialog();
   
	    // Complain if plugin version does not match
	 if (dimg->get_version()<=DJVUVERSION_TOO_OLD)
	    ::showError(this, tr("Upgrade DjVu now!"),
			tr("This DjVu document uses an inferior\n"
                           "technology, which is no longer supported.\n"
                           "Please send a flame to the webmaster who\n"
                           "uses outdated tools.\n"));
      }
   }
   if (set_mask & DjVuFile::DECODE_STOPPED)
   {
      DEBUG_MSG("QDViewer::slotNotifyFileFlagsChanged(): decoding stopped!\n");
      DEBUG_MAKE_INDENT(3);

      if (dimg && dimg->get_djvu_file()==source)
	 showStatus("DJVU: Transfer Interrupted (STOP)");
   }
   if (set_mask & DjVuFile::DECODE_OK)
   {
      DEBUG_MSG("QDViewer::slotNotifyFileFlagsChanged(): decoding is done!\n");
      DEBUG_MAKE_INDENT(3);

      if (dimg)
      {
	 if (!anno) decodeAnno(true);
	 //decodeAnno(true);
	 if (dimg->get_djvu_file()==source)
	 {
	       // Complain if plugin version does not match
	    if (dimg->get_version()>DJVUVERSION) showUpgradeDialog();
   
	       // Complain if plugin version does not match
	    if (dimg->get_version()<=DJVUVERSION_TOO_OLD)
              ::showError(this, tr("Upgrade DjVu now!"),
                          tr("This DjVu document uses an inferior\n"
                             "technology, which is no longer supported.\n"
                             "Please send a flame to the webmaster who\n"
                             "uses outdated tools.\n"));

            GUTF8String sdesc = 
              DjVuMessage::LookUpUTF8(dimg->get_short_description());
	    QString desc = tr("Page decoding done") + " (" +
              QStringFromGString(sdesc) + ")";
	    showStatus(desc);
            
	    predecode();
	 }
      }
   }
   imageUpdated();
}

void
QDViewer::slotNotifyDocFlagsChanged(const GP<DjVuDocument> & source, 
                                    long set_mask, long)
      // Connected to the whole document
{
   if (set_mask & DjVuDocument::DOC_INIT_FAILED)
     {
       G_THROW(ERR_MSG("QDViewer.corrupt"));
     }

   if (set_mask & DjVuDocument::DOC_INIT_OK)
   {
      int doc_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
      int doc_pages=djvu_doc->get_pages_num();
      
      start_page_num=doc_page;
      
      if (doc_pages>1 && plugin_data.thumbnails)
	 showThumbnails();
      if (thumbnails) thumbnails->setCurPageNum(doc_page);
      if (plugin_data.rel_url.length())
      {
	    // We expand the relative URL here (and don't leave it relative)
	    // because otherwise QDBase will treat it relative to the
	    // PAGE location (inside the same document), while it should
	    // be relative to the DOCUMENT
	 plugin_data.url=GURL::UTF8(plugin_data.rel_url,
                                    djvu_doc->get_init_url().base());
	 setOverrideFlags(plugin_data);
      }

      imageUpdated();
   }
}

//***************************** GoTo stuff ***********************************

void
QDViewer::history_add(int pageno)
{
  if (hundo.size()==0 || hundo[hundo]!=pageno)
    hundo.prepend(pageno);
  hredo.empty();
}

int
QDViewer::history_undo(void)
{
  int res = -1;
  int doc_page = djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
  if (hundo.size())
    {
      GPosition pos = hundo;
      res = hundo[pos];
      hundo.del(pos);
      hredo.prepend(doc_page);
    }
  return res;
}

int
QDViewer::history_redo(void)
{
  int res = -1;
  int doc_page = djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
  if (hredo.size())
    {
      GPosition pos = hredo;
      res = hredo[pos];
      hredo.del(pos);
      hundo.prepend(doc_page);
    }
  return res;
}


void
QDViewer::gotoPage(int new_page, bool history)
{
   DEBUG_MSG("QDViewer::gotoPage(): "
            "Request to go to page #" << new_page << "\n");
   DEBUG_MAKE_INDENT(3);

   if (dimg)
   {
      int doc_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
      int doc_pages=djvu_doc->get_pages_num();
      if (new_page<0) new_page=doc_pages-1;
      if (new_page>=doc_pages) new_page=doc_pages-1;

      if (new_page!=doc_page)
      {
	 if (plugin_data.hilite_rects.size())
	 {
	    if (doc_page==start_page_num)
	    {
		  // Get rid of hilited areas
	       OverrideFlags flags=getOverrideFlags();
	       flags.hilite_rects.empty();
	       setOverrideFlags(flags);
	    } else if (new_page==start_page_num)
	    {
		  // Restore hilited areas
	       OverrideFlags flags=getOverrideFlags();
	       flags.hilite_rects=plugin_data.hilite_rects;
	       setOverrideFlags(flags);
	    }
	 }
	 GURL url=djvu_doc->page_to_url(new_page);
	 getURL(url.get_string(), "_self", history);
      }
   }
}

void
QDViewer::slotGotoPage(int new_page)
{
   try
     {
       gotoPage(new_page);
     } 
   catch(const GException & exc)
     {
       ::showError(this, exc);
     }
}

void
QDViewer::getURL(const GUTF8String &url_in, 
                 const GUTF8String &target)
{
  getURL(url_in, target, true);
}

void
QDViewer::getURL(const GUTF8String &url_in, 
                 const GUTF8String &target, 
                 bool history)
{
   DEBUG_MSG("QDViewer::getURL(): url_in='" << url_in << 
             "', target='" << target << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!url_in.length()) return;
   
   if (print_win)
     {
       QMessageBox::information(this, tr("Finish printing first"),
                                tr("Please close the Print dialog first."),
                                "&OK", 0, 0, 0, 0);
       return;
     }
   
   GURL url;
   int doc_page = djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
   int doc_pages = djvu_doc->get_pages_num();
   
   // See if it starts with '#'
   if (url_in[0]=='#')
     {
       try
         {
           char base = 0;
           const char *s = (const char*)url_in + 1;
           const char *q = s;
           if (*q == '+' || *q == '-' || *q == '=')
             base = *q++;
           if (! base)
             url = djvu_doc->id_to_url(s);
           if (url.is_empty())
             {
               GUTF8String str = q;
               if (str.is_int())
                 {
                   int n = str.toInt();
                   if (base=='+')
                     n = doc_page + n;
                   else if (base=='-')
                     n = doc_page - n;
                   else
                     n = n - 1;
                   if (n >= doc_pages)
                     n = doc_pages - 1;
                   if (n < 0)
                     n = 0;
                   url = djvu_doc->page_to_url(n);
                 }
             }
         } 
       catch(const GException & exc)
         {
           showError(this, exc);
         }
     } 
   else
     {
       GURL base = djvu_doc->get_init_url().base();
       base.clear_all_arguments();
       url=GURL::UTF8(url_in,base);
     }
   
   if (!url.is_empty())
     {
       if ( !target.length() || (target=="_self") )
         {
           GP<DjVuImage> new_dimg;
        
           // Check: maybe it's predecoded
           if (predecode_dimg1 && 
               predecode_dimg1->get_djvu_file()->get_url()==url)
             new_dimg=predecode_dimg1;
           else if (predecode_dimg2 && 
                    predecode_dimg2->get_djvu_file()->get_url()==url)
             new_dimg=predecode_dimg2;
           else
             {
               // Check: maybe it's in the same document
               int page_num = djvu_doc->url_to_page(url);
               if (page_num == doc_page)
                 new_dimg = dimg;
               else if (page_num >= 0) 
                 new_dimg = djvu_doc->get_page(page_num, false, 
                                               page_port.getPort());
             }
           if (new_dimg && new_dimg != dimg)
             {
               if (history)
                 history_add(doc_page);
               setDjVuImage(new_dimg, 1);
               setCaption();
             }
           if (new_dimg)
             return;
         }
       QString mesg=tr("Requesting URL ")+QStringFromGString(url.get_string());
       showStatus(mesg);
       emit sigGetURL(url, target);
     }
}

void
QDViewer::predecode(void)
{
   DEBUG_MSG("QDViewer::predecode(): Seeing if we need to predecode smth...\n");
   DEBUG_MAKE_INDENT(3);

   bool done=false;

   GURL cur_url=dimg->get_djvu_file()->get_url();
   GURL predecode_url1, predecode_url2;
   if (predecode_dimg1)
      predecode_url1=predecode_dimg1->get_djvu_file()->get_url();
   if (predecode_dimg2)
      predecode_url2=predecode_dimg2->get_djvu_file()->get_url();
   if (predecode_url1==cur_url) { predecode_dimg1=0; predecode_url1=GURL(); }
   if (predecode_url2==cur_url) { predecode_dimg2=0; predecode_url2=GURL(); }
   
      // Predecode the next and the previous pages
   int page=djvu_doc->url_to_page(cur_url);
   int pages=djvu_doc->get_pages_num();
   GP<DjVuImage> next_dimg, prev_dimg;
   if (page+1<pages)
   {
      GURL next_url=djvu_doc->page_to_url(page+1);
      if (predecode_url1==next_url) next_dimg=predecode_dimg1;
      else if (predecode_url2==next_url) next_dimg=predecode_dimg2;
      else
      {
	 DEBUG_MSG("predecoding page " << (page+1+1) << "\n");
	 next_dimg=djvu_doc->get_page(page+1, false);

	    // We don't want to predecode two pages at the same time.
	 done=next_dimg->get_djvu_file()->is_decoding();
      }
   }
   if (page>0)
   {
      GURL prev_url=djvu_doc->page_to_url(page-1);
      if (predecode_url1==prev_url) prev_dimg=predecode_dimg1;
      else if (predecode_url2==prev_url) prev_dimg=predecode_dimg2;
      else
      {
	 DEBUG_MSG("predecoding page " << (page+1-1) << "\n");
	 if (!done) prev_dimg=djvu_doc->get_page(page-1, false);
	 else
	 {
	    GP<DjVuFile> file=djvu_doc->get_djvu_file(page-1);
	    if (file->is_decode_ok())
	    {
	       prev_dimg=DjVuImage::create();
	       prev_dimg->connect(file);
	    }
	 }
      }
   }
   if (next_dimg && prev_dimg)
   {
      predecode_dimg1=prev_dimg;
      predecode_dimg2=next_dimg;
   } else
   {
      GP<DjVuImage> dimg=prev_dimg ? prev_dimg : next_dimg;
      if (!predecode_dimg1) predecode_dimg1=dimg;
      else if (predecode_url1!=dimg->get_djvu_file()->get_url())
	 predecode_dimg2=dimg;
   }
}

//********************************** SLOTS ***********************************

void
QDViewer::checkWMProps(void)
{
   if (!in_netscape) return;
   DEBUG_MSG("QDViewer::checkWMProps(): searching for DjVu properties\n");
#ifdef UNIX
   Display * displ=x11Display();
   XSync(displ, False);
      
   Atom atom_zoom=XInternAtom(displ, DJVU_ZOOM_PROP, True);
   Atom atom_time=XInternAtom(displ, DJVU_DETACH_TIME_PROP, True);
   if (plugin_data.full_mode && !saved_data_known && atom_zoom && atom_time)
   {
      Window shell_win = x11GetTopLevelWindow(x11Display(), winId());
      DEBUG_MSG("shell_win=" << shell_win << "\n");
      if (shell_win)
      {
	 Atom type;
	 int format;
	 u_long items, bytes_to_go;
	 u_char * prop;
	 DEBUG_MSG("seeking for " << DJVU_ZOOM_PROP << " property\n");
	 if (XGetWindowProperty(displ, shell_win, atom_zoom, 0, 128/4,
				False, XA_STRING, &type, &format, &items,
				&bytes_to_go, &prop)==Success && prop && type)
	 {
	    int zoom=atol((char *) prop);
	    XFree(prop);
	    DEBUG_MSG("retrieved zoom " << zoom << "\n");
	    DEBUG_MSG("seeking for " << DJVU_DETACH_TIME_PROP << " property\n");
	    if (XGetWindowProperty(displ, shell_win, atom_time, 0, 128/4,
				   False, XA_STRING, &type, &format, &items,
				   &bytes_to_go, &prop)==Success && prop && type)
	    {
	       time_t dtime=(time_t) atol((char *) prop);
	       XFree(prop);
	       DEBUG_MSG("elapsed " << (time(0)-dtime) 
                         << " seconds since last detach.\n");
	       if (time(0)-dtime<10 && zoom>0) setZoom(zoom, 1, SRC_DEFAULT);
	    } else DEBUG_MSG("oops. It's not there.\n");
	 } else
	 {
	    DEBUG_MSG("found toplevel window, but the property isn't there.\n");
	 }
      }
   }
#else
   ::showInfo(this, "DjVu Info",
	      "Do smth with the Netscape's properties in QDViewer::checkWMProps\n");
#endif
}

void
QDViewer::slotWelcomeClosed(void)
{
   DEBUG_MSG("QDViewer::slotWelcomeClosed(): closing welcome dialog\n");

   if (!welcome) return;

   try
   {
      prefs.bBeginner=!((QDWelcome *) welcome)->neverShowAgain();
      DjVuPrefs disk_prefs;
      disk_prefs.bBeginner=prefs.bBeginner;
      disk_prefs.save();
      qeApp->killWidget(welcome); welcome=0;
   } catch(const GException & exc)
   {
      showError(this, tr("DjVu Error"), exc);
   }
}

void
QDViewer::slotShowPrefs(void)
{
   DEBUG_MSG("QDViewer::slotShowPrefs(): showing preferences\n");

   try
   {
      double dScreenGamma=prefs.dScreenGamma;
      int fastZoom=prefs.fastZoom;
      bool optimizeLCD=prefs.optimizeLCD;
      bool hlinksBorder=prefs.hlinksBorder;
      QDViewerPrefs pd(&prefs, this, "djvu_prefs", TRUE);
      if (plugin_data.cache==PluginData::CACHE_OFF)
	 pd.disableCache();
      if (pd.exec()==QDialog::Accepted)
      {
	 bool do_redraw=prefs.dScreenGamma!=dScreenGamma ||
			prefs.optimizeLCD!=optimizeLCD;
	 bool do_layout=prefs.fastZoom!=fastZoom;
	 bool repaint_thumb=prefs.dScreenGamma!=dScreenGamma;

	 bm_cache.setMaxSize(prefs.mcacheSize*1024*1024);
	 pm_cache.setMaxSize(prefs.mcacheSize*1024*1024);

	 if (qxImager)
           qxImager->setOptimizeLCD(prefs.optimizeLCD);
	 setBackgroundColor(getBackgroundColor(), false);
	 
	 if (prefs.toolBarOn) 
           enableToolBar(true);
	 else 
           enableToolBar(false);
         if (prefs.toolBarAlwaysVisible) 
           stickToolBar();
	 else 
           unStickToolBar();

	 if (hlinksBorder!=prefs.hlinksBorder)
	 {
	    do_redraw=true;
	    for(GPosition pos=map_areas;pos;++pos)
	    {
	       GP<MapArea> ma=map_areas[pos];
	       ma->setActiveOutlineMode(!ma->isAlwaysActive() &&
					prefs.hlinksBorder, true);
	    }
	 }
	 
	 DjVuFileCache * cache=djvu_doc->get_cache();
	 if (cache) cache->set_max_size(prefs.pcacheSize*1024*1024);

	 if (thumbnailsShown())
	    thumbnails->setFastMode(prefs.fastThumb);
	 
	 qeApp->gamma=prefs.dScreenGamma;
	 if (do_layout) 
           layout();
	 else if (do_redraw) 
           redraw();
	 if (repaint_thumb && thumbnails) 
           thumbnails->rereadGamma();
      }
   } catch(const GException & exc)
   {
      showError(this, tr("DjVu Error"), exc);
   }
}

void
QDViewer::slotHelp(void)
{
   DEBUG_MSG("QDViewer::slotHelp(): showing help\n");
   if (in_netscape)
     {
       try
         {
           GURL helpurl;
           helpurl = getDjViewDataFile(DJVIEW_HELP_DJVU);
           if ( helpurl.is_empty() )
             helpurl = getDjViewDataFile(DJVIEW_HELP_HTML);
           if ( helpurl.is_empty() )
             helpurl = GURL::UTF8(GUTF8String(DJVIEW_HELP_URL));
           
           getURL(helpurl.get_string(), "_blank");
         } 
       catch(const GException & exc)
         {
           showError(this, tr("DjVu Error"), exc);
         }
     }
}

void
QDViewer::slotAbout(void)
{
  if (! about)
    about = new QDAboutDialog(this, "about", FALSE);
  about->show();
  about->raise();
}

void
QDViewer::slotShowWelcome(void)
{
   try
   {
      if (!once_welcomed)
      {
	 welcome=new QDWelcome(this, "djvu_welcome");
	 welcome->show();
	 connect(welcome, SIGNAL(closed(void)), this, SLOT(slotWelcomeClosed(void)));
	 connect(welcome, SIGNAL(preferences(void)), this, SLOT(slotShowPrefs(void)));
	 connect(welcome, SIGNAL(help(void)), this, SLOT(slotHelp(void)));
	 connect(welcome, SIGNAL(about(void)), this, SLOT(slotAbout(void)));
	 once_welcomed=true;
      }
   } catch(const GException & exc)
   {
      showError(this, "DjVu Error", exc);
   }
}

void
QDViewer::slotChildError(int pipe)
{
      // Whenever we start a new child (like djedit) we create one pipe
      // connected to stderr of that child. We add our end of this pipe
      // to child_error_pipes list (to close in destructor) and create
      // QSocketNotifier connected to this slot function.
   try
   {
      GUTF8String message;
      const QObject * obj=sender();
      if (obj && obj->inherits("QSocketNotifier"))
      {
	 QSocketNotifier * notif=(QSocketNotifier *) obj;
	 while(1)
	 {
	   timeval tv; tv.tv_sec=tv.tv_usec=0;
	   fd_set read_fds, except_fds;
	   FD_ZERO(&read_fds); FD_SET(pipe, &read_fds);
	   FD_ZERO(&except_fds); FD_SET(pipe, &except_fds);
	   int rc=select(pipe, &read_fds, 0, &except_fds, &tv);
	   if (rc<0 && errno==EINTR) continue;
	   if (rc<0)
	     G_THROW(ERR_MSG("QDViewer.no_child_errmsg"));
	   if (rc>=0)
	     {
	       if (FD_ISSET(pipe, &read_fds))
		 {
		   char ch;
		   int rc=::read(pipe, &ch, 1);
		   if (rc<0)
		     G_THROW(ERR_MSG("QDViewer.no_child_errmsg"));
		   if (rc==0) break;
		   message+=ch;
		 }
	       else if (FD_ISSET(pipe, &except_fds))
		 {
		   notif->setEnabled(FALSE);
		   break;
		 }
	       else
		 break;
	     }
	 }
      }
      if (message.length())
        showError(this, tr("Subprocess diagnostics"), QStringFromGString(message));
   } catch(const GException & exc)
   {
      showError(this, exc);
   }
}

//****************************************************************************
//******************************** Search ************************************
//****************************************************************************

void
QDViewer::slotSearchClosed(void)
{
   eraseSearchResults();
   qeApp->killWidget((QWidget *) sender());
}

void
QDViewer::slotDisplaySearchResults(int page_num, 
                                   const GList<DjVuTXT::Zone *> & zones_list)
{
   eraseSearchResults();

   if (zones_list.size())
   {
	 // Switch the page
      gotoPage(page_num);

	 // Wait until we have the rectDocument non-empty...
	 // The data is already there and decoding is done in another thread
	 // So this shouldn't take a lot of time...
      if (rectDocument.isempty())
      {
	 while(rectDocument.isempty())
	    qApp->processEvents(100);
      }
      
	 // Create the map areas
      search_results_name="Search results";
      for(GPosition pos=zones_list;pos;++pos)
      {
         GRect irect=zones_list[pos]->rect;
         dimg->unmap(irect);
	 GP<GMapRect> gma=GMapRect::create(irect);
	 gma->comment=search_results_name;
	 gma->hilite_color=0xff000000;
	 gma->border_type=GMapArea::NO_BORDER;
	 gma->url="";
	 GP<MapArea> ma=new MapRect(gma);
	 map_areas.append(ma);
	 ma->attachWindow(pane, &ma_mapper);
	 ma->layout(GRect(0, 0, dimg->get_width(), dimg->get_height()));
	 ma->repaint();
      }

	 // Position the page to see the first map area
      DjVuTXT::Zone * first_zone=zones_list[zones_list];
      int xc=(first_zone->rect.xmin+first_zone->rect.xmax)/2;
      int yc=(first_zone->rect.ymin+first_zone->rect.ymax)/2;
      ma_mapper.map(xc, yc);

      scroll(xc-pane->width()/2, yc-pane->height()/2, true);
   }
}

void
QDViewer::search(void)
{
   QDSearchDialog * d;
   d=new QDSearchDialog(djvu_doc->url_to_page(dimg->get_djvu_file()->get_url()),
			djvu_doc, this, "search", FALSE);
   connect(d, SIGNAL(sigDone(void)), 
           this, SLOT(slotSearchClosed(void)));
   connect(d, SIGNAL(sigDisplaySearchResults(int, const GList<DjVuTXT::Zone *> &)),
	   this, SLOT(slotDisplaySearchResults(int, const GList<DjVuTXT::Zone *> &)));
   connect(this, SIGNAL(sigPageChanged(int)), 
           d, SLOT(slotSetPageNum(int)));
   d->show();
}

//****************************************************************************
//******************************* Toolbar ************************************
//****************************************************************************

void
QDViewer::slotDoCmd(int cmd)
{
   try
     {
       processCommand(cmd);
     } 
   catch(const GException & exc)
     {
       ::showError(this, exc);
     }
}

void
QDViewer::fillToolBar(QDToolBar * toolbar)
{
   QDBase::fillToolBar(toolbar);

   QDTBarPrintPiece * print_tbar=new QDTBarPrintPiece(toolbar);
   nav_tbar=new QDTBarNavPiece(toolbar);

   connect(print_tbar, SIGNAL(sigPrint(void)), this, SLOT(slotPrint(void)));
   connect(print_tbar, SIGNAL(sigFind(void)), this, SLOT(slotFind(void)));
   connect(print_tbar, SIGNAL(sigSave(void)), this, SLOT(slotSave(void)));
   connect(nav_tbar, SIGNAL(sigGotoPage(int)), this, SLOT(slotGotoPage(int)));
   connect(nav_tbar, SIGNAL(sigDoCmd(int)), this, SLOT(slotDoCmd(int)));
}

void
QDViewer::updateToolBar(void)
{
  QDBase::updateToolBar();

  if (toolbar && nav_tbar)
    {
      if (dimg)
	{
	  int doc_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
	  nav_tbar->update(doc_page, djvu_doc, hundo.size(), hredo.size());
	}
      else
	{
	  nav_tbar->update(-1, djvu_doc, false, false);
	}
    }
}

void
QDViewer::showThumbnails(void)
{
   if (djvu_doc && djvu_doc->get_pages_num()>1)
      QDBase::showThumbnails();
}

QWidget *
QDViewer::createThumbnails(bool _rowMajor)
{
   thumbnails=new QDThumbnails(this, "thumbnails", _rowMajor);
   thumbnails->setDjVuDocument(djvu_doc);
   thumbnails->setFastMode(prefs.fastThumb);
   thumbnails->setCurPageNum(djvu_doc->url_to_page(dimg->get_djvu_file()->get_url()));
   connect(thumbnails, SIGNAL(sigGotoPage(int)), 
           this, SLOT(slotGotoPage(int)));
   connect(thumbnails, SIGNAL(sigCloseThumbnails(void)), 
           this, SLOT(slotCloseThumbnails(void)));
   return thumbnails;
}
