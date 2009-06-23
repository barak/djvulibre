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
// $Id: qd_base_events.cpp,v 1.18 2008/08/05 20:52:26 bpearlmutter Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qd_base_events.h"
#include "djvu_base_res.h"
#include "qd_toolbar.h"
#include "qt_imager.h"
#include "qlib.h"

#include <qclipboard.h>
#include <qkeycode.h>
#include <qsplitter.h>
#include <qapplication.h>

#include "debug.h"

void
QDBase::preScroll(void)
{
   DEBUG_MSG("QDBase::preScroll(): preparing the pane for scroll\n");
}

void
QDBase::postScroll(void)
{
   DEBUG_MSG("QDBase::postScroll(): finishing scroll\n");
}

void
QDBase::slotSliderPressed(void)
{
   try
   {
      const QObject * obj=sender();
      if (obj && obj->isWidgetType() && obj->inherits("QScrollBar"))
	 preScroll();
   } catch(const GException & exc)
   {
      showError(this, exc);
   }
}

void
QDBase::slotSliderMoved(int value)
{
   try
   {
      if (in_layout) return;
   
      const QObject * obj=sender();
      if (obj && obj->isWidgetType() && obj->inherits("QScrollBar"))
      {
	 const QScrollBar * w=(const QScrollBar *) obj;
	 if (w==hscroll)
	    scroll(-(rectVisible.xmin-rectDocument.xmin-value), 0, 0);
	 if (w==vscroll)
	    scroll(0, -(rectVisible.ymin-rectDocument.ymin-value), 0);
      }
   } catch(const GException & exc)
   {
      showError(this, exc);
   }
}

void
QDBase::slotSliderReleased(void)
{
   try
   {
      const QObject * obj=sender();
      if (obj && obj->isWidgetType() && obj->inherits("QScrollBar"))
	 postScroll();
   } catch(const GException & exc)
   {
      showError(this, exc);
   }
}

void
QDBase::scroll(int dh, int dv, int update_scrollbars)
{
      // compute new document location
   DEBUG_MSG("DjVuBase::Scroll(): scrolling by " << dh << 'x' << dv << "\n");
   DEBUG_MAKE_INDENT(3);

   if (rectDocument.isempty())
   {
      DEBUG_MSG("rectDocument is empty, nothing to scroll => return.\n");
      return;
   }

   if (!pane->canScrollMore())
   {
      acc_scroll_dh+=dh;
      acc_scroll_dv+=dv;
   } else
   {
      dh+=acc_scroll_dh;
      dv+=acc_scroll_dv;
      acc_scroll_dh=acc_scroll_dv=0;

      if (dh || dv)
      {
	    // adjust document location. Please note: there is a copy of this code in
	    // Layout(). If you make any changes, make sure to do it twice :)
	 int xoff=0;
	 int yoff=0;
	 GRect rectNew=rectDocument;
	 rectNew.translate(-dh, -dv);

	 if (rectNew.width()<=rectVisible.width())
	 {
	    int hor_align=DjVuANT::ALIGN_CENTER;
            if (anno && anno->ant)
              hor_align = anno->ant->hor_align;
            if (override_flags.hor_align!=DjVuANT::ALIGN_UNSPEC)
              hor_align = override_flags.hor_align;
	    switch(hor_align)
	    {
	       case DjVuANT::ALIGN_LEFT:
		  xoff=rectVisible.xmin-rectNew.xmin; break;
	       case DjVuANT::ALIGN_UNSPEC:
	       case DjVuANT::ALIGN_CENTER:
		  xoff=rectVisible.xmin-rectNew.xmin+
		       (rectVisible.width()-rectNew.width())/2; break;
	       case DjVuANT::ALIGN_RIGHT:
		  xoff=rectVisible.xmax-rectNew.xmax; break;
	    }
	 } else if (rectNew.xmin>rectVisible.xmin)
	    xoff=rectVisible.xmin-rectNew.xmin;
	 else if (rectNew.xmax<rectVisible.xmax)
	    xoff=rectVisible.xmax-rectNew.xmax;
      
	 if (rectNew.height()<=rectVisible.height())
	 {
	    int ver_align=DjVuANT::ALIGN_CENTER;
            if (anno && anno->ant)
              ver_align = anno->ant->ver_align;
            if (override_flags.ver_align!=DjVuANT::ALIGN_UNSPEC)
              ver_align = override_flags.ver_align;
	    switch(ver_align)
	    {
	       case DjVuANT::ALIGN_TOP:
		  yoff=rectVisible.ymin-rectNew.ymin; break;
	       case DjVuANT::ALIGN_UNSPEC:
	       case DjVuANT::ALIGN_CENTER:
		  yoff=rectVisible.ymin-rectNew.ymin+
		       (rectVisible.height()-rectNew.height())/2; break;
	       case DjVuANT::ALIGN_BOTTOM:
		  yoff=rectVisible.ymax-rectNew.ymax; break;
	    }
	 } else if (rectNew.ymin>rectVisible.ymin)
	    yoff=rectVisible.ymin-rectNew.ymin;
	 else if (rectNew.ymax<rectVisible.ymax)
	    yoff=rectVisible.ymax-rectNew.ymax;
   
	 rectNew.translate(xoff, yoff);
   
	    // perform scroll
	 int dx=rectNew.xmin - rectDocument.xmin;
	 int dy=rectNew.ymin - rectDocument.ymin;
	 DEBUG_MSG("real displacement: dx=" << dx << ", dy=" << dy << "\n");
	 if (!dx && !dy)
	 {
	    DEBUG_MSG("nothing to do...\n");
	    return;
	 }

	 rectDocument=rectNew;
	 mapper.set_input(rectDocument);
	 ma_mapper.set_output(rectDocument);
	 lens_rect.translate(dx, dy);
	 pane->scroll(dx, dy);
	 if (update_scrollbars)
	 {
	    if (hscroll)
	    {
	       int value=rectVisible.xmin-rectDocument.xmin;
	       if (value<0) value=0;
	       hscroll->setValue(value);
	    }
	    if (vscroll)
	    {
	       int value=rectVisible.ymin-rectDocument.ymin;
	       if (value<0) value=0;
	       vscroll->setValue(value);
	    }
	 }
      }
   }
}

bool
QDBase::isLensVisible(void) const
{
   return !lens_rect.isempty();
}

void
QDBase::showLens(int x, int y)
      // Will position the lens for the first time, or move it to
      // a new location repainting the required regions as needed
{
   if (lens_rect.isempty() ||
       x!=lens_rect.xmin ||
       y!=lens_rect.ymin)
   {
      if (cur_map_area)
      {
	    // We have some problems with lens and active hyperlinks.
	    // Fast workaround is to hide the hyperlink while viewing the lens.
	 cur_map_area->setActive(false, true);
	 delete map_area_tip; map_area_tip=0;
      }

      GRect old_lens_rect=lens_rect;
      lens_rect=GRect(x, y, prefs.magnifierSize, prefs.magnifierSize);

	 // We want to paint the new lens first as otherwise there will be
	 // an unpleasant effect: part of the lens border will be erased by the
	 // image update, part will stay. Try it on a slow machine
      paintLens();

	 // Now see if we need to erase parts of lens at its previous location
      if (!old_lens_rect.isempty())
      {
	    // Update picture where the lens were before
	 GRect irect; irect.intersect(old_lens_rect, lens_rect);
	 if (irect.isempty())
	    paint(old_lens_rect);
	 else
	 {
	    int dx=lens_rect.xmin-old_lens_rect.xmin;
	    int dy=lens_rect.ymin-old_lens_rect.ymin;
	    int adx=abs(dx);
	    int ady=abs(dy);

	    if (dx<0) paint(GRect(lens_rect.xmax, old_lens_rect.ymin,
				  adx, old_lens_rect.height()));
	    else if (dx>0) paint(GRect(old_lens_rect.xmin, old_lens_rect.ymin,
				       adx, old_lens_rect.height()));
	    if (dy<0) paint(GRect(old_lens_rect.xmin, lens_rect.ymax,
				  old_lens_rect.width(), ady));
	    else if (dy>0) paint(GRect(old_lens_rect.xmin, old_lens_rect.ymin,
				       old_lens_rect.width(), ady));
	 }
      }
   }
}

void
QDBase::hideLens(void)
{
   if (!lens_rect.isempty())
   {
      GRect rect=lens_rect;
      lens_rect.clear();
      paint(rect);
   }
}

int
QDBase::getLensHotKey(void) const
{
   int key=ControlButton;
   switch(prefs.magnifierHotKey)
   {
      case DjVuPrefs::MAG_ITEMS:	// To avoid warning.
      case DjVuPrefs::MAG_CTRL:
	 key=ControlButton;
	 break;
      case DjVuPrefs::MAG_SHIFT:
	 key=ShiftButton;
	 break;
      case DjVuPrefs::MAG_ALT:
	 key=AltButton;
	 break;
      case DjVuPrefs::MAG_MID:
	 key=MidButton;
	 break;
   }
   return key;
}

void
QDBase::drawSelectionRect(const QRect &rect)
{
   if ( !pane || rect.isEmpty() ) return;
   
   QPainter p(pane);
   p.setRasterOp(XorROP);
   p.setPen(QColor(0xff, 0xff, 0xff));
   p.drawRect(rect);      
}


bool
QDBase::processMouseMoveEvent(QMouseEvent * ev)
{
      // This is a weird check. But unfortunately, there are cases
      // when mouse was pressed in a menu, then moved to the pane,
      // and pane starts receiving Btn1Motion event w/o prior Btn1Down
   if ((ev->state() & LeftButton) && left_butt_down)
   {
      if ( pane_mode == IDC_PANE )
      {
	 if (!in_hand_scroll)
	 {
	    int dx=hand_scroll_x-ev->x();
	    int dy=hand_scroll_y-ev->y();
	    if (dx*dx+dy*dy>=49)
	    {
	       delete map_area_tip; map_area_tip=0;
	       preScroll();
	       in_hand_scroll=1;
	       setCursor();
	    }
	 }
	 if (in_hand_scroll)
	 {
	    scroll(hand_scroll_x-ev->x(), hand_scroll_y-ev->y());
	    hand_scroll_x=ev->x();
	    hand_scroll_y=ev->y();
	 }
      } else //IDC_ZOOM_SELECT || IDC_TEXT_SELECT
      { 
	 if ( pane )
	 {
	    if ( lastrect )
	       drawSelectionRect(G2Q(*lastrect));
	    
	    int rw=ev->x()-zoom_select_x0;
	    int rh=ev->y()-zoom_select_y0;

	    GRect *currect=0;
	    if ( rw < 0 )	    
	       currect= rh<0 ? new GRect(ev->x(),ev->y(),-rw,-rh) :
	       new GRect(ev->x(), zoom_select_y0, -rw, rh);
	    else
	       currect= rh<0 ? new GRect(zoom_select_x0,ev->y(),rw,-rh) :
	       new GRect(zoom_select_x0,zoom_select_y0,rw,rh);

	    drawSelectionRect(G2Q(*currect));

	    if (lastrect) delete lastrect;
	    lastrect=currect;
	 }
      }
   } 
   else if (!(ev->state() & 
              (LeftButton|MidButton|RightButton|getLensHotKey())))
   {
      // Pure motion
      if (needToShowToolBar()) 
        showToolBar();
	       
      if (!override_flags.links || !map_areas.size()) 
        return false;
      
      GRect grect;
      grect.intersect(rectVisible, rectDocument);

      int _xPos=ev->x(), _yPos=ev->y();
      ma_mapper.unmap(_xPos, _yPos);
	       
      MapArea * new_cur_map_area=0;
      for(GPosition pos=map_areas;pos;++pos)
      {
	 GP<MapArea> ma=map_areas[pos];
	 if (ma->getComment() != search_results_name &&
	     ma->is_point_inside(_xPos, _yPos) &&
             ma->isHyperlink() ) 
           new_cur_map_area = ma;
      }
      if (new_cur_map_area != cur_map_area)
      {
	 if (cur_map_area)
	 {
	    cur_map_area->setActive(false, true);
	    delete map_area_tip; 
            map_area_tip=0;
	 }
	 if (new_cur_map_area)
	 {
	    new_cur_map_area->setActive(true, true);
	    GUTF8String comment=new_cur_map_area->getComment();
	    GUTF8String url=new_cur_map_area->getURL();
	    if (url.length())
              showStatus(QStringFromGString(url));
	    else if (comment.length())
              showStatus(QStringFromGString(comment));
	    if (prefs.hlinksPopup)
	    {
	       delete map_area_tip; map_area_tip=0;
	       GRect brect=new_cur_map_area->get_bound_rect();
	       ma_mapper.map(brect);
	       GUTF8String tip=url;
	       if (comment.length()) 
                 tip=comment;
	       if (tip.length())
                 map_area_tip=new QDMapAreaTip(QStringFromGString(tip), G2Q(brect), pane);
	    }
	 } else showStatus(" ");

	 cur_map_area=new_cur_map_area;
	 setCursor();
      }
   }
   
   if (isLensVisible() && (ev->state() & getLensHotKey()))
   {
      if (rectDocument.contains(ev->x(), ev->y()))
	 showLens(ev->x()-prefs.magnifierSize/2,
		  ev->y()-prefs.magnifierSize/2);
      else hideLens();
   }
   
   return true;
}

bool
QDBase::eventFilter(QObject *obj, QEvent *e)
{
   try
   {
      if (obj==toolbar)
      {
	 switch(e->type())
	 {
	    case QEvent::Enter:
	       showToolBar();
	       break;
            default:
               break;
	 }
      } 
      else if (obj==main_widget)
      {
	 switch(e->type())
	 {
	    case QEvent::Resize:
	       layout(true);
	       break;

	    case QEvent::LayoutHint:
	       if (isToolBarEnabled() && isToolBarStuck() && toolbar->isVisible())
		  if (toolbar->isVisible()) layout(true);
	       break;

            default:
               break;
	 }
      } 
      else if (obj==pane)
      {
	 if (e->type()!=QEvent::MouseMove)
	 {
	    DEBUG_MSG("QDBase::eventFilter(): Got event...");
	    DEBUG_MAKE_INDENT(3);
	 }

	 switch(e->type())
	 {
	    case QEvent::MouseButtonPress:
	    {     
	       DEBUG_MSGN("Event_MousePress\n");
	       QMouseEvent * ev=(QMouseEvent *) e;

	       hideToolBar(0);
	       if (ev->button()==LeftButton)
	       {
		  left_butt_down=1;
		  if ( pane_mode==IDC_PANE )
		  {
		     hand_scroll_x=ev->x();
		     hand_scroll_y=ev->y();

		     // Unfortunately, in "focus follows mouse" mode KWM
		     // issues Event_Leave and Event_Enter right before this
		     // Event_MouseButtonPress. This effectively deselects
		     // the current hyperlink. So we want it back.
		     if (ev->state()==0) 
                       processMouseMoveEvent(ev);
	       
		     if (!cur_map_area)
		     {
			preScroll();
			in_hand_scroll=1;
			setCursor();
		     }  
                     // If we're over a hyperlink - wait before we either
		     // release the mouse button (and go to URL)
                     // or displace far enough
		  } 
                  else
		  {
		     in_zoom_select=1;
		     zoom_select_x0=ev->x();
		     zoom_select_y0=ev->y();
		     setCursor();
		  }   
		  return TRUE;
	       } 
               else if (ev->button()==MidButton)
	       {
		     // Unfortunately, in "focus follows mouse" mode KWM
		     // issues Event_Leave and Event_Enter right before this
		     // Event_MouseButtonPress. This effectively deselects
		     // the current hyperlink. So we want it back.
		  if (ev->state()==0) 
                    processMouseMoveEvent(ev);

		     // The order is right. First we should call
		     // processMouseMoveEvent() above
		  if (getLensHotKey()==MidButton)
		  {
		     QPoint cursor=pane->mapFromGlobal(QCursor::pos());
		     if (rectDocument.contains(cursor.x(), cursor.y()))
			showLens(cursor.x()-prefs.magnifierSize/2,
				 cursor.y()-prefs.magnifierSize/2);
		  }
	       }
	       break;
	    }
	    case QEvent::MouseButtonRelease:
	    {
	       DEBUG_MSGN("QEvent::MouseRelease\n");
	       QMouseEvent * ev=(QMouseEvent *) e;
	       hideToolBar(0);
	       if (ev->button()==LeftButton)
	       {
		  left_butt_down=0;
		  if (in_hand_scroll)
		  {
		     postScroll();
		     in_hand_scroll=0;
		     setCursor();
		  } 
                  else if (in_zoom_select)
		  {
		     in_zoom_select=0;
		     setCursor();
		     
		     if (pane_mode==IDC_TEXT_SELECT)
			eraseSearchResults();
            
		     if(lastrect && !lastrect->isempty())
		     {
			if ( pane_mode == IDC_ZOOM_SELECT )
			{
			   GRect zoom_rect=*lastrect;

			   zoom_rect.intersect(zoom_rect, rectDocument);

			   int cur_zoom_factor=getZoom();
			   int hzoom = int(cur_zoom_factor * 
                                           ((float)rectVisible.height() /
                                            zoom_rect.height()));
			   int wzoom = int(cur_zoom_factor * 
                                           ((float)rectVisible.width() / 
                                            zoom_rect.width()));
			
			   int new_zoom_factor=hzoom<wzoom?hzoom:wzoom;
			   int new_zoom_cmd=new_zoom_factor+IDC_ZOOM_MIN;

			   if ( new_zoom_cmd < IDC_ZOOM_MAX )
			   {
			      setZoom(new_zoom_cmd, true, SRC_MANUAL);

			      // centering the new zoomed area 
			      int dx = (zoom_rect.xmax+zoom_rect.xmin
                                        -rectVisible.xmax-rectVisible.xmin)/2;
			      int dy = (zoom_rect.ymax+zoom_rect.ymin
                                        -rectVisible.ymax-rectVisible.ymin)/2;
			      float zf = new_zoom_factor / 
                                (float)cur_zoom_factor;
			      dx = (int)(dx*zf); 
                              dy = (int)(dy*zf);
			      scroll(dx,dy);
			   } 
                           else
			   {
			      drawSelectionRect(G2Q(zoom_rect));
			   }
			} 
                        else // IDC_TEXT_SELECT
			{
			   drawSelectionRect(G2Q(*lastrect));
			   GRect text_rect=*lastrect;

			   GRect rect=rectDocument;
			   text_rect.intersect(text_rect, rect);
			   
			   ma_mapper.unmap(text_rect);
  			   dimg->map(text_rect);			

			   static GUTF8String UTF8selectedtext;
			   GP<ByteStream> bs=dimg->get_djvu_file()->get_text();
			   if( bs )
			   {
			      GP<DjVuText> djvutext = DjVuText::create();
			      djvutext->decode(bs);
			      if( djvutext->txt )
			      {
				 GList<GRect> rects=djvutext->txt->
                                   find_text_with_rect(text_rect,
                                                       UTF8selectedtext);
				 search_results_name="Selected Text";
				 for(GPosition pos=rects;pos;++pos)
				 {
				    GRect irect=rects[pos];
                                    dimg->unmap(irect);
				    GP<GMapRect> gma=GMapRect::create(irect);
				    gma->comment=search_results_name;
				    gma->hilite_color=0xff000000;
				    gma->border_type=GMapArea::NO_BORDER;
				    gma->url="";
				    GP<MapArea> ma=new MapRect(gma);
				    map_areas.append(ma);
				    ma->attachWindow(pane, &ma_mapper);
				    ma->layout(GRect(0, 0, 
                                                     dimg->get_width(),
                                                     dimg->get_height()));
				    ma->repaint();
				 }
                                 
				 QApplication::clipboard()->setText(QStringFromGString(UTF8selectedtext));
			      }
			   }
			}
			
			delete lastrect;
			lastrect=0;
		     }
		  } 
                  else if ( pane_mode == IDC_PANE )
		     if (cur_map_area && cur_map_area->isHyperlink())
			getURL(cur_map_area->getURL(), 
                               cur_map_area->getTarget());
		  return TRUE;
	       } 
               else if (ev->button()==MidButton)
	       {
		  if (getLensHotKey()==MidButton)
		     hideLens();
		  
		  if (cur_map_area && cur_map_area->isHyperlink())
		  {
		     GUTF8String gurl=cur_map_area->getURL();
		     getURL(gurl, "_blank");
		  }
		  return TRUE;
	       }
	       break;
	    }
	    case QEvent::MouseMove:
	    {
	       if (processMouseMoveEvent((QMouseEvent *) e)) return TRUE;
	       break;
	    }
	    case QEvent::Paint:
	    {
	       DEBUG_MSGN("QEvent::Paint\n");
	       QPaintEvent * ev=(QPaintEvent *) e;
	       paint(Q2G(ev->rect()));
	       return TRUE;
	    }
	    case QEvent::Leave:
	    {
	       DEBUG_MSGN("QEvent::Leave\n");
	       showStatus(" ");

	       if (cur_map_area)
	       {
		 if (!((display_all_hlinks && cur_map_area->isHyperlink()) ||
		       cur_map_area->isBorderAlwaysVisible()))
		  {
		     GRect grect;
		     grect.intersect(rectVisible, rectDocument);
		     cur_map_area->setActive(false, true);
			// cur_map_area->eraseBorder(grect);
		  }
		  cur_map_area=0;
		  delete map_area_tip; map_area_tip=0;
	       }
	       hideLens();
	       return TRUE;
	    }
	    case QEvent::Enter:
	    {
	       DEBUG_MSGN("QEvent::Enter\n");
	       if (needToShowToolBar()) showToolBar();
	       return TRUE;
	    }
#if QT_VERSION > 210
            case QEvent::Wheel:
            {
                QWheelEvent *ev = (QWheelEvent *)e;
                int delta = ev->delta() / 5;
                if (delta != 0)
                  scroll(0, -delta);
                ev->accept();
            }
#endif
	    case QEvent::Resize:
	    {
	       DEBUG_MSGN("QEvent::Resize\n");
	       return TRUE;	// Don't want automatic QEvent::Paint
	    }
	    case QEvent::KeyPress:
	    {
	       DEBUG_MSGN("QEvent::KeyPress\n");

	       if (override_flags.keyboard)
	       {
		  QKeyEvent * ev=(QKeyEvent *) e;
		  hideToolBar(0);
		  switch(ev->key())
		  {
		     case Key_Control:
		     case Key_Shift:
		     case Key_Alt:
		       if ((ev->key()==Key_Control && getLensHotKey()==ControlButton) ||
			   (ev->key()==Key_Shift && getLensHotKey()==ShiftButton) ||
			   (ev->key()==Key_Alt && getLensHotKey()==AltButton))
			{
			   QPoint cursor=pane->mapFromGlobal(QCursor::pos());
			   if (rectDocument.contains(cursor.x(), cursor.y()))
			      showLens(cursor.x()-prefs.magnifierSize/2,
				       cursor.y()-prefs.magnifierSize/2);
			}
			break;

		     case Key_Up:
			scroll(0, -rectVisible.height()/25);
			break;

		     case Key_Down:
			scroll(0, +rectVisible.height()/25);
			break;

		     case Key_Left:
			scroll(-rectVisible.width()/25, 0);
			break;

		     case Key_Right:
			scroll(+rectVisible.width()/25, 0);
			break;

		     case Key_Home:
			scroll(rectDocument.xmin-rectVisible.xmin,
			       rectDocument.ymin-rectVisible.ymin);
			break;

		     case Key_End:
			scroll(rectDocument.xmax-rectVisible.xmax,
			       rectDocument.ymax-rectVisible.ymax);
			break;

		     case Key_PageUp:
		     case Key_Backspace:
			if (rectDocument.ymin<rectVisible.ymin)
			   scroll(0, -rectVisible.height()*9/10);
			break;

		     case Key_PageDown:
		     case Key_Return:
		     case Key_Enter:
		     case Key_Space:
			if (rectDocument.ymax>rectVisible.ymax)
			   scroll(0, rectVisible.height()*9/10);
			break;
	       
		     case Key_W:
			setZoom(IDC_ZOOM_WIDTH, 1, SRC_MANUAL);
			break;

		     case Key_P:
			setZoom(IDC_ZOOM_PAGE, 1, SRC_MANUAL);
			break;

		     case Key_1:
			setZoom(IDC_ZOOM_MIN+100, 1, SRC_MANUAL);
			break;

		     case Key_2:
			setZoom(IDC_ZOOM_MIN+200, 1, SRC_MANUAL);
			break;

		     case Key_3:
			setZoom(IDC_ZOOM_MIN+300, 1, SRC_MANUAL);
			break;

		     case Key_Equal:
		     case Key_Plus:
			setZoom(IDC_ZOOM_ZOOMIN, 1, SRC_MANUAL);
			break;

		     case Key_Minus:
		     case Key_F24:	// Thanks to Solaris
			setZoom(IDC_ZOOM_ZOOMOUT, 1, SRC_MANUAL);
			break;
			
		     case Key_Escape:
			if ( in_zoom_select && lastrect)
			{
			   drawSelectionRect(G2Q(*lastrect));
			   delete lastrect;
			   lastrect=0;
			}
			break;
		  } // switch(ev->key())
		  return TRUE;
	       } // if (override_flags.keyboard)
	    } // case QEvent::KeyPress:
	    case QEvent::KeyRelease:
	    {
	       DEBUG_MSGN("QEvent::KeyRelease\n");

	       if (override_flags.keyboard)
	       {
		  QKeyEvent * ev=(QKeyEvent *) e;
		  switch(ev->key())
		    {
		    case Key_Control:
		    case Key_Shift:
		    case Key_Alt:
		      if ((ev->key()==Key_Control && getLensHotKey()==ControlButton) ||
			  (ev->key()==Key_Shift && getLensHotKey()==ShiftButton) ||
			  (ev->key()==Key_Alt && getLensHotKey()==AltButton))
			hideLens();
		      break;
		    }
	       }
	    } // case QEvent::KeyRelease:
	    default:
	       DEBUG_MSGN("number " << e->type() << "\n");
	 } // switch(e->type())
      } // if (obj==pane)
   } catch(const GException & exc)
   {
      showError(this, exc);
   }
   return QWidget::eventFilter(obj, e);
}

static GRect
invRect(const GRect & rect)
{
   return GRect(rect.xmin, -rect.ymax, rect.width(), rect.height());
}

void
QDBase::slotCheckCache(void)
{
   DEBUG_MSG("QDBase::checkCache(): seeing if cached need updating\n");
   DEBUG_MAKE_INDENT(3);

   if (rectDocument.isempty())
   {
      DEBUG_MSG("but rectDocument is empty => return.\n");
      return;
   }

   try
   {
      bool do_again=true;

      if (dimg && dimg->get_width() && dimg->get_height())
      {
	 if (bm_cache.isAllocated())
	 {
	    if (getMode()!=IDC_DISPLAY_BLACKWHITE)
	    {
		  // Try to get a piece of pixmap. If it works out =>
		  // disable bm_cache
	       GRect grect=rectDocument;
	       if (grect.xmax-grect.xmin>10) grect.xmax=grect.xmin+10;
	       if (grect.ymax-grect.ymin>10) grect.ymax=grect.ymin+10;
	 
	       GP<GPixmap> pm=dimg->get_pixmap(invRect(grect),
					       invRect(rectDocument),
					       prefs.dScreenGamma);
	       if (pm)
	       {
		  DEBUG_MSG("switching from BM to PM cache as image and mode are not BW\n");
		  bm_cache.disable();
		  pm_cache.enable();
	       }
	    } // if (getMode()!=IDC_DISPLAY_BLACKWHITE)
      
	    if (bm_cache.isAllocated())
	    {
	       DEBUG_MSG("dealing with bm_cache as either image or mode is BW\n");
	       GRect grect;
	       if (bm_cache.nextRectToCache(100000, grect))
	       {
		  DEBUG_MSG("caching rect=(" << grect.xmin << ", " <<
			    grect.ymin << ", " << grect.width() << ", "
			    << grect.height() << ")\n");
	    
		  GRectMapper doc_mapper;
		  doc_mapper.set_input(rectDocument);
		  doc_mapper.set_output(GRect(0, 0, rectDocument.width(),
					      rectDocument.height()));
		  doc_mapper.mirrory();
	    
		  GRect grectImg=grect; doc_mapper.unmap(grectImg);
	    
		  GP<GBitmap> bm=dimg->get_bitmap(invRect(grectImg),
						  invRect(rectDocument),
						  sizeof(int));
		  if (bm) bm_cache.addRect(grect, bm);
		  else do_again=false;
	       }
	    } // if (bm_cache.isAllocated())
	 } // if (bm_cache.isAllocated())
	 if (pm_cache.isAllocated())
	 {
	    DEBUG_MSG("dealing with pm_cache\n");
	    GRect grect;
	    if (pm_cache.nextRectToCache(100000, grect))
	    {
	       DEBUG_MSG("caching rect=(" << grect.xmin << ", "
			 << grect.ymin << ", " << grect.width() << ", "
			 << grect.height() << ")\n");

	       GRectMapper doc_mapper;
	       doc_mapper.set_input(rectDocument);
	       doc_mapper.set_output(GRect(0, 0, rectDocument.width(),
					   rectDocument.height()));
	       doc_mapper.mirrory();
	 
	       GRect grectImg=grect; doc_mapper.unmap(grectImg);
	 
	       GP<GPixmap> pm;
	       switch (getMode())
	       {
		  case IDC_DISPLAY_COLOR:
		     pm=dimg->get_pixmap(invRect(grectImg),
					 invRect(rectDocument),
					 prefs.dScreenGamma);
		     break;
		  case IDC_DISPLAY_BACKGROUND:
		     pm=dimg->get_bg_pixmap(invRect(grectImg),
					    invRect(rectDocument),
					    prefs.dScreenGamma);
		     break;
		  case IDC_DISPLAY_FOREGROUND:
		     pm=dimg->get_fg_pixmap(invRect(grectImg),
					    invRect(rectDocument),
					    prefs.dScreenGamma);
		     break;
	       }
	       if (pm)
	       {
		     // All dithering is now done in paint()
		     /*
		       mapper.map(grectImg);
		       if (depth<15) pm->ordered_666_dither(grectImg.xmin, grectImg.ymin);
		       else if (depth<24) pm->ordered_32k_dither(grectImg.xmin, grectImg.ymin);
		     */
		  pm_cache.addRect(grect, pm);
	       } else
	       {
		  DEBUG_MSG("switching from PM to BM cache as image is BW\n");
		  pm_cache.disable();
		  bm_cache.enable();
	       }
	    } // if (pm_cache.nextRectToCache(100000, grect))
	 } // if (pm_cache.isAllocated())
      } // if (dimg && dimg->get_width() && dimg->get_height())

      GRect dummy_rect;
      do_again=do_again && pm_cache.isAllocated() &&
	       pm_cache.nextRectToCache(100000, dummy_rect);
      do_again=do_again && bm_cache.isAllocated() &&
	       bm_cache.nextRectToCache(100000, dummy_rect);
      if (do_again) cache_timer.start(0, TRUE);
   } catch(const GException & exc)
   {
      showError(this, "DjVu Error", exc);
   }
}

void
QDBase::resizeEvent(QResizeEvent * ev)
{
   DEBUG_MSG("QDBase::resizeEvent()\n");
   // order matters 
   if (splitter)
   {
      // I'm not sure why I have to resize thumb_widget here
      thumb_widget->resize(thumb_widget->sizeHint());
      splitter->resize(ev->size());
   }
   else main_widget->resize(ev->size());
   QWidget::resizeEvent(ev);

}

