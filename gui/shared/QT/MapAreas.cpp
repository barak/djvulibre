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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "DjVuGlobal.h"
#include "MapAreas.h"
#include "MapDraw.h"
#include "debug.h"
#include "qlib.h"
#include "qxlib.h"
#include "qt_imager.h"
#include "qx_imager.h"

#include <qapplication.h>
#include <qpaintdevicemetrics.h>
#include <qbitmap.h>

#include "vertex.bm"
#include "vertex_mask.bm"

#define MOUSE_SENS		8

GPQCursor::GQCursor::GQCursor(void)
{
  ptr=new QCursor();
}

GPQCursor::GQCursor::GQCursor(QCursor *newptr)
{
  ptr=newptr;
}

GPQCursor::GQCursor::~GQCursor()
{
  if (qApp)
    {
      // The QApplication destructor deletes a number of cursors (but not all of them).
      // Chances are that ptr is a dangling pointer in this case
      delete ptr;
    }
}

GPQCursor::GPQCursor(void)
{
  current=new GQCursor();
}

GPQCursor::GPQCursor(QCursor *ptr)
{
  current=new GQCursor(ptr);
}

GPQCursor::~GPQCursor()
{
  current=0;
}

GPQCursor &
MapArea::top_left_cur(void)
{
  static GPQCursor value;
  return value;
}

GPQCursor &
MapArea::top_cur(void)
{
  static GPQCursor value;
  return value;
}

GPQCursor &
MapArea::top_right_cur(void)
{
  static GPQCursor value;
  return value;
}

GPQCursor &
MapArea::right_cur(void)
{
  static GPQCursor value;
  return value;
}

GPQCursor &
MapArea::bottom_right_cur(void)
{
  static GPQCursor value;
  return value;
}

GPQCursor &
MapArea::bottom_cur(void)
{
  static GPQCursor value;
  return value;
}

GPQCursor &
MapArea::bottom_left_cur(void)
{
  static GPQCursor value;
  return value;
}

GPQCursor &
MapArea::left_cur(void)
{
  static GPQCursor value;
  return value;
}

GPQCursor &
MapArea::inside_cur(void)
{
  static GPQCursor value;
  return value;
}

GPQCursor &
MapArea::ptr_cur(void)
{
  static GPQCursor value;
  return value;
}

GPQCursor &
MapPoly::vertex_cur(void)
{
  static GPQCursor value;
  return value;
}

void
MapPiece::createPixmaps(void)
{
   GRect grect=*this;
   mapper->map(grect);
   
   off_pixmap=QPixmap(grect.width(), grect.height());
   on_pixmap=QPixmap(grect.width(), grect.height());

   off_pixmap.setOptimization(QPixmap::NoOptim);
   on_pixmap.setOptimization(QPixmap::NoOptim);
}

void
MapPiece::clearPixmaps(void)
{
   on_pixmap=QPixmap();
   off_pixmap=QPixmap();
}

/****************************************************************************
***************************** MapArea definition ***************************
****************************************************************************/

MapArea::MapArea(const GP<GMapArea> & area) : gmap_area(area)
{
   pane=0;
   mapper=0;
   in_motion=false;
   enable_edit_controls=false;
   active_outline_mode=inactive_outline_mode=false;
   active=gmap_area->border_always_visible;
   force_always_active=false;
   
   createCursors();
   GUTF8String str=area->check_object();
   if (str.length()) G_THROW(str);
}

void
MapArea::detachWindow(void)
   // This function is called from destructor too.
{
   DEBUG_MSG("MapArea::detachWindow(): detaching the window...\n");
   DEBUG_MAKE_INDENT(3);
   
   mapper=0; pane=0;
   pieces.empty();
}

void
MapArea::attachWindow(QWidget * _pane, GRectMapper * _mapper)
{
   DEBUG_MSG("MapArea::attachWindow(): attaching the window...\n");
   DEBUG_MAKE_INDENT(3);

   pane=_pane; mapper=_mapper;
   
   initBorder();
}

void
MapArea::setOutlineMode(bool inactive_on, bool active_on, bool redraw)
{
   if (inactive_outline_mode!=inactive_on ||
       active_outline_mode!=active_on)
   {
      inactive_outline_mode=inactive_on;
      active_outline_mode=active_on;
      if (inactive_outline_mode) active_outline_mode=true;
      if (!active_outline_mode)
	 for(GPosition pos=pieces;pos;++pos)
	    pieces[pos]->clearPixmaps();
      if (redraw) repaint();
   }
}

void
MapArea::enableEditControls(bool on, bool redraw)
{
   if (enable_edit_controls!=on)
   {
      enable_edit_controls=on;
      if (redraw) repaint();
   }
}

void
MapArea::setInactiveOutlineMode(bool on, bool redraw)
{
   bool inactive_on=on;
   bool active_on=isActiveOutlineMode();
   if (inactive_on) active_on=true;
   setOutlineMode(inactive_on, active_on, redraw);
}

void
MapArea::setActiveOutlineMode(bool on, bool redraw)
{
   setOutlineMode(isInactiveOutlineMode(), on, redraw);
}

void
MapArea::setActive(bool on, bool redraw)
{
  if ((!on && isAlwaysActive()) || active==on)
    return;

  active=on;
  if (redraw && pane && mapper)
    {
      if (isActiveOutlineMode()) drawOutline(Q2G(pane->rect()));
      else if (!isCacheUsed())
	repaint();
      else
	{
	  // Use cache to draw ourselves
	  GRect brect=gmap_area->get_bound_rect();
	  mapper->map(brect);
	  brect.inflate(3, 3);
	  GRect pane_rect=Q2G(pane->rect());
	  GRect urect;
	  if (urect.intersect(pane_rect, brect))
	    {
	      for(GPosition pos=pieces;pos;++pos)
		{
		  GP<MapPiece> piece=pieces[pos];
		  GRect piece_rect=*piece;
		  mapper->map(piece_rect);
		  GRect irect;
		  if (irect.intersect(piece_rect, urect))
		    {
		      QPixmap & pix=active ? piece->getOnPixmap() : piece->getOffPixmap();
		      if (pix.isNull()) repaint(piece_rect);
		      QPainter p(pane);
		      GRect crect=doc_rect;
		      mapper->map(crect);
		      p.setClipRect(G2Q(crect));
		      p.drawPixmap(irect.xmin, irect.ymin, pix,
				   irect.xmin-piece_rect.xmin,
				   irect.ymin-piece_rect.ymin,
				   irect.width(), irect.height());
		    }
		}
	    }
	}
    }
}

void
MapArea::setForceAlwaysActive(bool on)
{
   if (!(force_always_active=on))
      for(GPosition pos=pieces;pos;++pos)
	 pieces[pos]->clearPixmaps();
   else setActive(true, true);
}

bool
MapArea::isAlwaysActive(void) const
{
   return force_always_active || gmap_area->border_always_visible ||
      gmap_area->border_type==GMapArea::NO_BORDER;
}

void
MapArea::reset(void)
      // Rereads contents of gmap_area and refreshes display
{
   initBorder();
   repaint();
}

void
MapArea::initBorder(void)
      // Regenerates the boundary and refreshes border on the screen
{
   switch(getBorderType())
   {
      case GMapArea::NO_BORDER:
      case GMapArea::XOR_BORDER:
      case GMapArea::SOLID_BORDER:
	 ma_generatePieces();
	 repaintBorder();
	 break;
      case GMapArea::SHADOW_IN_BORDER:
	 for(int i=0;i<gmap_area->border_width;i++)
	    shadow_pattern[i]=-(100-100*(i+1)/gmap_area->border_width);
	 ma_generatePieces();
	 repaintBorder();
	 break;
      case GMapArea::SHADOW_OUT_BORDER:
	 for(int i=0;i<gmap_area->border_width;i++)
	    shadow_pattern[i]=100-100*(i+1)/gmap_area->border_width;
	 ma_generatePieces();
	 repaintBorder();
	 break;
      case GMapArea::SHADOW_EIN_BORDER:
	 for(int i=0;i<gmap_area->border_width;i++) shadow_pattern[i]=0;
	 shadow_pattern[0]=-100;
	 shadow_pattern[gmap_area->border_width-1]=100;
   	 ma_generatePieces();
	 repaintBorder();
	 break;
      case GMapArea::SHADOW_EOUT_BORDER:
	 for(int i=0;i<gmap_area->border_width;i++) shadow_pattern[i]=0;
	 shadow_pattern[0]=100;
	 shadow_pattern[gmap_area->border_width-1]=-100;
   	 ma_generatePieces();
	 repaintBorder();
	 break;
      default:
	 G_THROW(ERR_MSG("MapArea.unknown_border_type"));
   }
}

static inline void
swap(int & x, int & y)
{
   int tmp=x; x=y; y=tmp;
}

GRect
MapArea::getPiecesBoundRect(void) const
{
   GRect rect=gmap_area->get_bound_rect();
   swap(rect.xmin, rect.xmax);
   swap(rect.ymin, rect.ymax);
   
   for(GPosition pos=pieces;pos;++pos)
   {
      GRect prect=*pieces[pos];
      if (prect.xmin<rect.xmin) rect.xmin=prect.xmin;
      if (prect.ymin<rect.ymin) rect.ymin=prect.ymin;
      if (prect.xmax>rect.xmax) rect.xmax=prect.xmax;
      if (prect.ymax>rect.ymax) rect.ymax=prect.ymax;
   }
   return rect;
}

void
MapArea::repaint(const GRect & rect)
{
   DEBUG_MSG("MapArea::repaint()\n");
      /* QPaintEvent * ev=new QPaintEvent(G2Q(rect));
	 QApplication::postEvent(w, ev); */
   
   QRect qrect=G2Q(rect);
   x11Redraw(pane, &qrect);
}

void
MapArea::repaint(void)
      // Generates PaintEvents for the whole area
{
   if (pane && mapper)
   {
      GRect brect=gmap_area->get_bound_rect();
      mapper->map(brect);
      brect.inflate(3, 3);	// Take into account possible edit controls

      GRect prect=Q2G(pane->rect());
      GRect irect;
      if (irect.intersect(brect, prect)) repaint(irect);
   }
}

void
MapArea::repaintBorder(void)
      // Generated PaintEvents for the MapArea's boundary
{
   for(GPosition pos=pieces;pos;++pos)
      repaintPiece(pieces[pos]);
   
   if (enable_edit_controls &&
       GUTF8String(gmap_area->get_shape_name()) != GMapArea::RECT_TAG)
   {
	 // In this case we also need to update the edit controls
	 // MANUALLY! For rectangles they're included into the "pieces"
      if (pieces.size()>1)
      {
	    // If there is only one pieces list then it covers the whole
	    // polygon/oval including the edit controls => nothing to do
	    // Otherwise: refresh 4 rectangle areas
	 GRect brect=gmap_area->get_bound_rect();
	 mapper->map(brect);
	 brect.inflate(3, 3);
	 
	    // TODO: Maybe send thru X11 to optimize...
	    // Top side
	 repaint(GRect(brect.xmin, brect.ymin, brect.width(), 3));

	    // Right side
	 repaint(GRect(brect.xmax-3, brect.ymin, 3, brect.height()));

	    // Bottom
	 repaint(GRect(brect.xmin, brect.ymax-3, brect.width(), 3));

	    // Left
	 repaint(GRect(brect.xmin, brect.ymin, 3, brect.height()));
      }
   }
}

void
MapArea::repaintPiece(const GP<MapPiece> & piece)
      // Sends PaintEvent to cause repaint of the specified piece
{
   if (pane && mapper)
   {
      GRect grect=*piece;
      mapper->map(grect);
      
      GRect prect=Q2G(pane->rect());
      GRect irect;
      if (irect.intersect(grect, prect)) repaint(irect);
   }
}

void
MapArea::exposeBorder(void)
      // Obsolete. Remove
{
   repaintBorder();
}

void
MapArea::draw(const GRect & pm_rect, const GP<GPixmap> & pm,
	      DRAW_MODE draw_mode)
      // Draws itself into the specified pixmap. The pm_rect should be
      // in the pane's coordinates.
{
   DEBUG_MSG("MapArea::draw(): Highlighting " << gmap_area->url << ":" <<
	     gmap_area->target << "\n");
   DEBUG_MAKE_INDENT(3);

   if (pane)
   {
      if (!isInactiveOutlineMode())
	 if (draw_mode==DRAW_INACTIVE || draw_mode==DRAW_ACTIVE)
	    ma_drawInactive(pm_rect, pm);
      if (!isActiveOutlineMode())
	 if (draw_mode==APPLY_ACTIVE || draw_mode==DRAW_ACTIVE)
	    ma_applyActive(pm_rect, pm);
   }
}

void
MapArea::draw(const GRect & bm_rect, const GP<GBitmap> & bm_in,
	      GRect & pm_rect, GP<GPixmap> & pm_out,
	      DRAW_MODE draw_mode)
      // Draws itself into the specified bitmap. The bm_rect should be
      // in the pane's coordinates. Since GBitmap may not contain color
      // information, we will create a color GPixmap patch (pm_out), where we
      // will actually be drawing. The pm_rect is a rectangle in
      // pane coordinates where this pm_out should go to.
{
   DEBUG_MSG("MapArea::draw(): Highlighting " << gmap_area->url << ":" <<
	     gmap_area->target << "\n");
   DEBUG_MAKE_INDENT(3);

   if (pane)
   {
      GRect brect=gmap_area->get_bound_rect();
      mapper->map(brect);
      brect.inflate(3, 3);
      
      GRect irect;
      if (irect.intersect(bm_rect, brect))
      {
	 pm_rect=irect;
	 irect.translate(-bm_rect.xmin, -bm_rect.ymin);
	 pm_out=GPixmap::create(*bm_in, GRect(irect.xmin, bm_in->rows()-irect.ymax,
					  irect.width(), irect.height()));
	 if (!isInactiveOutlineMode())
	    if (draw_mode==DRAW_INACTIVE || draw_mode==DRAW_ACTIVE)
	       ma_drawInactive(pm_rect, pm_out);
	 if (!isActiveOutlineMode())
	    if (draw_mode==APPLY_ACTIVE || draw_mode==DRAW_ACTIVE)
	       ma_applyActive(pm_rect, pm_out);
      }
   }
}

void
MapArea::drawOutline(const GRect & grect, QPainter * p_in)
{
   if (pane && mapper)
   {
      GRect brect=gmap_area->get_bound_rect();
      mapper->map(brect);
      brect.inflate(3, 3);
      GRect irect;
      if (irect.intersect(brect, grect))
      {
	 QPainter * p=p_in;
	 try
	 {
	    if (!p_in) p=new QPainter(pane);
	    else p_in->save();
	    p->setRasterOp(XorROP);
	    p->setPen(QColor(0xff, 0xff, 0xff));
	    p->setClipRect(G2Q(irect));
	    ma_drawOutline(p, irect);
	    if (editControlsEnabled()) drawEditControls(grect, p);
	    if (!p_in) { delete p; p=0; }
	    else p_in->restore();
	 } catch(...)
	 {
	    if (!p_in) delete p;
	    throw;
	 }
      }
   }
}

void
MapArea::updateCache(const GRect & pm_rect, const GP<GPixmap> & pm,
		     GRectMapper * sdoc_mapper)
      // Takes the passed pixmap and updated the internal cache.
      // The pixmap should already contain the hyperlink draw in the
      // INACTIVE state. We will copy it and apply ACTIVE part here.
      // pm_rect is a rectangle in pane's coordinates where pm is supposed
      // to go to. sdoc_mapper maps screen coordinates to the coordinates
      // of the scaled document (see qd_base_paint.cpp)
{
   DEBUG_MSG("MapArea::updateCache(): updating caches\n");
   DEBUG_MAKE_INDENT(3);

   if (!isCacheUsed() || !pm || !pane) return;
   
   GRect brect=gmap_area->get_bound_rect();
   mapper->map(brect);
   brect.inflate(3, 3);		// To take into account edit controls
      
   GRect urect;
   if (urect.intersect(pm_rect, brect))
   {
      for(GPosition pos=pieces;pos;++pos)
      {
	 GP<MapPiece> piece=pieces[pos];
	 GRect prect=*piece;
	 mapper->map(prect);
	 GRect irect;
	 if (irect.intersect(prect, urect))
	 {
	    if (piece->getOnPixmap().isNull() || piece->getOffPixmap().isNull())
	       piece->createPixmaps();

	    QPixmap & on_pix=piece->getOnPixmap();
	    QPixmap & off_pix=piece->getOffPixmap();

	       // Now I need to make a copy of the area to be cached.
	       // The problem is that I'll need to draw into the GPixmap
	       // and I don't want to spoil the original.
	    GP<GPixmap> ipix_off;
	    GRect pix_rect=irect;
	    pix_rect.translate(-pm_rect.xmin, -pm_rect.ymin);
	    ipix_off=GPixmap::create(*pm, GRect(pix_rect.xmin, pm->rows()-pix_rect.ymax,
					    pix_rect.width(), pix_rect.height()));
	    GP<GPixmap> ipix_on=GPixmap::create(*ipix_off);

	       // Now ipix_off and ipix_on contains the data, which can be modified.
	       // Draw the map area into them
	    draw(irect, ipix_on, APPLY_ACTIVE);

	       // Dither pix_off and pix_on
	    GRect drect=irect;
	    sdoc_mapper->map(drect);
	    if (qxImager)
              qxImager->dither(*ipix_on, drect.xmin, drect.ymin);
	    if (qxImager)
              qxImager->dither(*ipix_off, drect.xmin, drect.ymin);

	       // Now copy the GPixmaps into QPixmaps to be used for caching
	    QDPainter p_off(&off_pix);
	    p_off.drawPixmap(GRect(irect.xmin-prect.xmin,
				   irect.ymin-prect.ymin,
				   irect.width(), irect.height()), ipix_off);
	    p_off.end();
	    
	    QDPainter p_on(&on_pix);
	    p_on.drawPixmap(GRect(irect.xmin-prect.xmin,
				  irect.ymin-prect.ymin,
				  irect.width(), irect.height()), ipix_on);
	    p_on.end();
	 }
      }
   }
}

/**************************** Editing MapArea *******************************/
void
MapArea::move(const GRect & doc_rect, int x, int y,
	      int vic_code, int vic_data)
{
   if (!mapper) return;

   if (vic_code>=OBJECT_CODES)
   {
      ma_move(doc_rect, x, y, vic_code, vic_data);
      return;
   }
   
   ma_copySaved();
   GMapArea * copy=ma_getCopiedData();
   int xmin=copy->get_xmin(), ymin=copy->get_ymin();
   int xmax=copy->get_xmax(), ymax=copy->get_ymax();
   GRect grect(xmin, ymin, xmax-xmin, ymax-ymin);
   mapper->map(grect);
   xmin=grect.xmin; ymin=grect.ymin; xmax=grect.xmax; ymax=grect.ymax;
   int width=xmax-xmin, height=ymax-ymin;
   int dx=x-start_x, dy=y-start_y;
   switch(vic_code)
   {
      case TOP_LEFT:
	 if (dx>=width || dy>=height) return;
	 if (xmin+dx<doc_rect.xmin) dx=doc_rect.xmin-xmin;
	 if (ymin+dy<doc_rect.ymin) dy=doc_rect.ymin-ymin;
	 grect.xmin+=dx; grect.ymin+=dy;
	 mapper->unmap(grect);
	 copy->transform(grect);
	 break;
      case TOP_RIGHT:
	 if (-dx>=width || dy>=height) return;
	 if (xmax+dx>doc_rect.xmax) dx=doc_rect.xmax-xmax;
	 if (ymin+dy<doc_rect.ymin) dy=doc_rect.ymin-ymin;
	 grect.xmax+=dx; grect.ymin+=dy;
	 mapper->unmap(grect);
	 copy->transform(grect);
	 break;
      case BOTTOM_RIGHT:
	 if (-dx>=width || -dy>=height) return;
	 if (xmax+dx>doc_rect.xmax) dx=doc_rect.xmax-xmax;
	 if (ymax+dy>doc_rect.ymax) dy=doc_rect.ymax-ymax;
	 grect.xmax+=dx; grect.ymax+=dy;
	 mapper->unmap(grect);
	 copy->transform(grect);
	 break;
      case BOTTOM_LEFT:
	 if (dx>=width || -dy>=height) return;
	 if (xmin+dx<doc_rect.xmin) dx=doc_rect.xmin-xmin;
	 if (ymax+dy>doc_rect.ymax) dy=doc_rect.ymax-ymax;
	 grect.xmin+=dx; grect.ymax+=dy;
	 mapper->unmap(grect);
	 copy->transform(grect);
	 break;
      case TOP:
	 if (dy>=height) return;
	 if (ymin+dy<doc_rect.ymin) dy=doc_rect.ymin-ymin;
	 grect.ymin+=dy;
	 mapper->unmap(grect);
	 copy->transform(grect);
	 break;
      case BOTTOM:
	 if (-dy>=height) return;
	 if (ymax+dy>doc_rect.ymax) dy=doc_rect.ymax-ymax;
	 grect.ymax+=dy;
	 mapper->unmap(grect);
	 copy->transform(grect);
	 break;
      case LEFT:
	 if (dx>=width) return;
	 if (xmin+dx<doc_rect.xmin) dx=doc_rect.xmin-xmin;
	 grect.xmin+=dx;
	 mapper->unmap(grect);
	 copy->transform(grect);
	 break;
      case RIGHT:
	 if (-dx>=width) return;
	 if (xmax+dx>doc_rect.xmax) dx=doc_rect.xmax-xmax;
	 grect.xmax+=dx;
	 mapper->unmap(grect);
	 copy->transform(grect);
	 break;
      case INSIDE:
	 if (xmin+dx<doc_rect.xmin) dx=doc_rect.xmin-xmin;
	 if (xmax+dx>doc_rect.xmax) dx=doc_rect.xmax-xmax;
	 if (ymin+dy<doc_rect.ymin) dy=doc_rect.ymin-ymin;
	 if (ymax+dy>doc_rect.ymax) dy=doc_rect.ymax-ymax;
	 grect.xmin+=dx; grect.xmax+=dx;
	 grect.ymin+=dy; grect.ymax+=dy;
	 mapper->unmap(grect);
	 copy->transform(grect);
	 break;
   }
   const char *s=copy->check_object();
   if (!s || !s[0]) ma_loadCopy();
}

void
MapArea::createCursors(void)
{
   DEBUG_MSG("MapArea::createCursors(): creating new cursors\n");
   
      // TODO: Use X11 cursors

   top_left_cur()=new QCursor(SizeFDiagCursor);
   top_cur()=new QCursor(SizeVerCursor);
   top_right_cur()=new QCursor(SizeBDiagCursor);
   right_cur()=new QCursor(SizeHorCursor);
   bottom_left_cur()=new QCursor(SizeBDiagCursor);
   bottom_cur()=new QCursor(SizeVerCursor);
   bottom_right_cur()=new QCursor(SizeFDiagCursor);
   left_cur()=new QCursor(SizeHorCursor);
   inside_cur()=new QCursor(SizeAllCursor);
   ptr_cur()=new QCursor(ArrowCursor);
}

GPQCursor
MapArea::getCursor(int vic_code, int vic_data)
{
   DEBUG_MSG("MapArea::getCursor(): vic_code=" << vic_code <<
	     ", vic_data=" << vic_data << "\n");
   
   if (vic_code>=OBJECT_CODES) return ma_getCursor(vic_code, vic_data);
   else return (vic_code==FAR ? ptr_cur() :
		vic_code==TOP_LEFT ? top_left_cur() :
		vic_code==TOP ? top_cur() :
		vic_code==TOP_RIGHT ? top_right_cur() :
		vic_code==BOTTOM_LEFT ? bottom_left_cur() :
		vic_code==BOTTOM ? bottom_cur() :
		vic_code==BOTTOM_RIGHT ? bottom_right_cur() :
		vic_code==LEFT ? left_cur() :
		vic_code==RIGHT ? right_cur() :
		vic_code==INSIDE ? inside_cur() :
		inside_cur());
}

bool
MapArea::isPointerClose(int x, int y, int * vic_code, int * vic_data)
   // (x, y) are screen coords
{
   if (!mapper) return 0;

   GRect brect=gmap_area->get_bound_rect();
   mapper->map(brect);
   
   int xmin=brect.xmin, ymin=brect.ymin;
   int xmax=brect.xmax, ymax=brect.ymax;
   int dx=xmax-xmin, dy=ymax-ymin;
   
   if (x<xmin-MOUSE_SENS || x>xmax+MOUSE_SENS ||
       y<ymin-MOUSE_SENS || y>ymax+MOUSE_SENS) return 0;
   
   if (ma_isPointerClose(x, y, vic_code, vic_data)) return 1;
   
   if ((xmin-x)*(xmin-x)+(ymin-y)*(ymin-y)<MOUSE_SENS*MOUSE_SENS)
   {
      if (vic_code) *vic_code=TOP_LEFT;
      return 1;
   }
   if ((xmin+dx/2-x)*(xmin+dx/2-x)+(ymin-y)*(ymin-y)<MOUSE_SENS*MOUSE_SENS)
   {
      if (vic_code) *vic_code=TOP;
      return 1;
   }
   if ((xmax-x)*(xmax-x)+(ymin-y)*(ymin-y)<MOUSE_SENS*MOUSE_SENS)
   {
      if (vic_code) *vic_code=TOP_RIGHT;
      return 1;
   }
   if ((xmax-x)*(xmax-x)+(ymin+dy/2-y)*(ymin+dy/2-y)<MOUSE_SENS*MOUSE_SENS)
   {
      if (vic_code) *vic_code=RIGHT;
      return 1;
   }
   if ((xmax-x)*(xmax-x)+(ymax-y)*(ymax-y)<MOUSE_SENS*MOUSE_SENS)
   {
      if (vic_code) *vic_code=BOTTOM_RIGHT;
      return 1;
   }
   if ((xmin+dx/2-x)*(xmin+dx/2-x)+(ymax-y)*(ymax-y)<MOUSE_SENS*MOUSE_SENS)
   {
      if (vic_code) *vic_code=BOTTOM;
      return 1;
   }
   if ((xmin-x)*(xmin-x)+(ymax-y)*(ymax-y)<MOUSE_SENS*MOUSE_SENS)
   {
      if (vic_code) *vic_code=BOTTOM_LEFT;
      return 1;
   }
   if ((xmin-x)*(xmin-x)+(ymin+dy/2-y)*(ymin+dy/2-y)<MOUSE_SENS*MOUSE_SENS)
   {
      if (vic_code) *vic_code=LEFT;
      return 1;
   }
   mapper->unmap(x, y);
   if (is_point_inside(x, y))
   {
      if (vic_code) *vic_code=INSIDE;
      return 1;
   }
   return 0;
}

void
MapArea::ma_drawEditControls(QPainter * painter) {}

static void
_drawFrame(QPainter & painter, int xmin, int ymin, int xmax, int ymax, int length)
{
   int dx=xmax-xmin, dy=ymax-ymin;
   
      // Top left
   painter.drawLine(xmin, ymin, dx<=3*length ? xmax-2 : xmin+length, ymin);
   if (dx>3*length)
   {
	 // Top center
      painter.drawLine(xmin+dx/2-5, ymin, xmin+dx/2+5, ymin);
	 // Top right
      painter.drawLine(xmax-1-1, ymin, xmax-1-length, ymin);
   }
      // Left top
   painter.drawLine(xmax-1, ymin, xmax-1, dy<=3*length ? ymax-2 : ymin+length);
   if (dy>3*length)
   {
	 // Right center
      painter.drawLine(xmax-1, ymin+dy/2-5, xmax-1, ymin+dy/2+5);
	 // Right bottom
      painter.drawLine(xmax-1, ymax-1-1, xmax-1, ymax-1-length);
      
   }
      // Bottom right
   painter.drawLine(xmax-1, ymax-1, dx<=3*length ? xmin+1 : xmax-1-length, ymax-1);
   if (dx>3*length)
   {
	 // Bottom center
      painter.drawLine(xmin+dx/2-5, ymax-1, xmin+dx/2+5, ymax-1);
	 // Bottom left
      painter.drawLine(xmin+1, ymax-1, xmin+length, ymax-1);
   }
      // Left bottom
   painter.drawLine(xmin, ymax-1, xmin, dy<=3*length ? ymin+1 : ymax-1-length);
   if (dy>3*length)
   {
	 // Left center
      painter.drawLine(xmin, ymin+dy/2-5, xmin, ymin+dy/2+5);
	 // Left top
      painter.drawLine(xmin, ymin+1, xmin, ymin+length);
   }
}

void
MapArea::drawEditControls(const GRect & grect, QPainter * p_in)
      // grect is in the screen coord system
      // Correct painter modes WILL be set
{
   GRect brect=gmap_area->get_bound_rect();
   mapper->map(brect);
   brect.inflate(3, 3);
  
   GRect irect;
   if (irect.intersect(grect, brect))
   {
      QPainter * p=p_in;
      try
      {
	    // Prepare
	 if (!p_in) p=new QPainter(pane);
	 else p_in->save();
	 p->setRasterOp(XorROP);
	 p->setPen(QColor(0xff, 0xff, 0xff));
	 p->setClipRect(G2Q(irect));

	    // Draw
	 int xmin=brect.xmin+3, xmax=brect.xmax-3;
	 int ymin=brect.ymin+3, ymax=brect.ymax-3;
	 _drawFrame(*p, xmin-1, ymin-1, xmax+1, ymax+1, 10);
	 _drawFrame(*p, xmin-3, ymin-3, xmax+3, ymax+3, 12);      
	 ma_drawEditControls(p);

	    // Finish
	 if (!p_in) { delete p; p=0; }
	 else p_in->restore();
      } catch(...)
      {
	 if (!p_in) delete p;
	 throw;
      }
   }
}

/****************************************************************************
******************************* MapRect definition ***************************
****************************************************************************/

void
MapRect::makeFrame(const GRect & mod_rect,
		   GPixmap * pm,
		   const GRect & pm_rect,
		   int top_margin, int right_margin,
		   int bottom_margin, int left_margin)
      // pm_rect is the place in pane coordinates where pm is supposed
      // to go. mod_rect is the place in pane coordinates, which should
      // be modified (all or part of it)
      // margins define distances from mod_rect's sides to the sides of
      // the rectangular map area. If they're positive, it means, that
      // mod_rect is completely inside the map area. Otherwise part of
      // it is outside
{
   int dst_x, dst_y;
      // TOP
   for(dst_y=mod_rect.height()-1;dst_y>mod_rect.height()-1-(gmap_area->border_width-top_margin) && dst_y>=0;dst_y--)
   {
      int distance=(mod_rect.height()-1-dst_y)+top_margin;
      int left=-left_margin+distance;
      int right=mod_rect.width()+right_margin-distance;
      if (left<0) left=0;
      if (right>mod_rect.width()) right=mod_rect.width();
      for(dst_x=left;dst_x<right;dst_x++)
      {
	 GPixel & pix=(*pm)[dst_y+pm_rect.ymax-mod_rect.ymax]
			   [dst_x+mod_rect.xmin-pm_rect.xmin];
	 int inc=shadow_pattern[distance];
	 if (inc>0)
	 {
	    pix.r=pix.r<=255-inc ? pix.r+inc : 255;
	    pix.g=pix.g<=255-inc ? pix.g+inc : 255;
	    pix.b=pix.b<=255-inc ? pix.b+inc : 255;
	 } else
	 {
	    pix.r=pix.r>=-inc ? pix.r+inc : 0;
	    pix.g=pix.g>=-inc ? pix.g+inc : 0;
	    pix.b=pix.b>=-inc ? pix.b+inc : 0;
	 }
      }
   }
      // RIGHT
   for(dst_x=mod_rect.width()-1;dst_x>mod_rect.width()-1-(gmap_area->border_width-right_margin) && dst_x>=0;dst_x--)
   {
      int distance=(mod_rect.width()-1-dst_x)+right_margin;
      int bottom=-bottom_margin+distance;
      int top=mod_rect.height()+top_margin-distance;
      if (bottom<0) bottom=0;
      if (top>mod_rect.height()) top=mod_rect.height();
      for(dst_y=bottom;dst_y<top;dst_y++)
      {
	 GPixel & pix=(*pm)[dst_y+pm_rect.ymax-mod_rect.ymax]
			   [dst_x+mod_rect.xmin-pm_rect.xmin];
	 int inc=-shadow_pattern[distance];
	 if (inc>0)
	 {
	    pix.r=pix.r<=255-inc ? pix.r+inc : 255;
	    pix.g=pix.g<=255-inc ? pix.g+inc : 255;
	    pix.b=pix.b<=255-inc ? pix.b+inc : 255;
	 } else
	 {
	    pix.r=pix.r>=-inc ? pix.r+inc : 0;
	    pix.g=pix.g>=-inc ? pix.g+inc : 0;
	    pix.b=pix.b>=-inc ? pix.b+inc : 0;
	 }
      }
   }
      // BOTTOM
   for(dst_y=0;dst_y<gmap_area->border_width-bottom_margin && dst_y<mod_rect.height();dst_y++)
   {
      int distance=dst_y+bottom_margin;
      int left=-left_margin+distance;
      int right=mod_rect.width()+right_margin-distance;
      if (left<0) left=0;
      if (right>mod_rect.width()) right=mod_rect.width();
      for(dst_x=left;dst_x<right;dst_x++)
      {
	 GPixel & pix=(*pm)[dst_y+pm_rect.ymax-mod_rect.ymax]
			   [dst_x+mod_rect.xmin-pm_rect.xmin];
	 int inc=-shadow_pattern[distance];
	 if (inc>0)
	 {
	    pix.r=pix.r<=255-inc ? pix.r+inc : 255;
	    pix.g=pix.g<=255-inc ? pix.g+inc : 255;
	    pix.b=pix.b<=255-inc ? pix.b+inc : 255;
	 } else
	 {
	    pix.r=pix.r>=-inc ? pix.r+inc : 0;
	    pix.g=pix.g>=-inc ? pix.g+inc : 0;
	    pix.b=pix.b>=-inc ? pix.b+inc : 0;
	 }
      }
   }
      // LEFT
   for(dst_x=0;dst_x<gmap_area->border_width-left_margin && dst_x<mod_rect.width();dst_x++)
   {
      int distance=dst_x+left_margin;
      int bottom=-bottom_margin+distance;
      int top=mod_rect.height()+top_margin-distance;
      if (bottom<0) bottom=0;
      if (top>mod_rect.height()) top=mod_rect.height();
      for(dst_y=bottom;dst_y<top;dst_y++)
      {
	 GPixel & pix=(*pm)[dst_y+pm_rect.ymax-mod_rect.ymax]
			   [dst_x+mod_rect.xmin-pm_rect.xmin];
	 int inc=shadow_pattern[distance];
	 if (inc>0)
	 {
	    pix.r=pix.r<=255-inc ? pix.r+inc : 255;
	    pix.g=pix.g<=255-inc ? pix.g+inc : 255;
	    pix.b=pix.b<=255-inc ? pix.b+inc : 255;
	 } else
	 {
	    pix.r=pix.r>=-inc ? pix.r+inc : 0;
	    pix.g=pix.g>=-inc ? pix.g+inc : 0;
	    pix.b=pix.b>=-inc ? pix.b+inc : 0;
	 }
      }
   }
}

void
MapRect::ma_drawInactive(const GRect & pm_rect, const GP<GPixmap> & pm)
      // pm_rect is the rectangle of the pane in screen coordinates
      // where pm is supposed to go to
{
   if (mapper && pm)
   {
      GRect border_rect=gmap_area->get_bound_rect();
      mapper->map(border_rect);

      GRect irect;
      if (irect.intersect(border_rect, pm_rect))
      {
	    // Translate the intersection into the GPixmap's coordinate
	 irect.translate(-pm_rect.xmin, -pm_rect.ymin);

	    // Do hiliting first
	 if (gmap_area->hilite_color!=0xffffffff)
	 {
	    if (gmap_area->hilite_color==0xff000000)
	    {
		  // Do XOR hiliting
	       for(int y=irect.ymin;y<irect.ymax;y++)
	       {
		  GPixel * pix=(*pm)[pm->rows()-1-y]+irect.xmin;
		  for(int x=irect.xmin;x<irect.xmax;x++, pix++)
		  {
		     pix->r^=0xff;
		     pix->g^=0xff;
		     pix->b^=0xff;
		  }
	       }
	    } else
	    {
		  // Do COLOR hiliting
	       int r=(gmap_area->hilite_color & 0xff0000) >> (16+2);
	       int g=(gmap_area->hilite_color & 0xff00) >> (8+2);
	       int b=(gmap_area->hilite_color & 0xff) >> 2;
	       for(int y=irect.ymin;y<irect.ymax;y++)
	       {
		  GPixel * pix=(*pm)[pm->rows()-1-y]+irect.xmin;
		  for(int x=irect.xmin;x<irect.xmax;x++, pix++)
		  {
		     pix->r=((((int) pix->r << 1)+(int) pix->r) >> 2)+r;
		     pix->g=((((int) pix->g << 1)+(int) pix->g) >> 2)+g;
		     pix->b=((((int) pix->b << 1)+(int) pix->b) >> 2)+b;
		  }
	       }
	    }
	 }
      }
   }
}

void
MapRect::ma_applyActive(const GRect & pm_rect, const GP<GPixmap> & pm)
      // pm_rect is the rectangle of the pane in screen coordinates
      // where pm is supposed to go to
{
   DEBUG_MSG("MapRect::ma_applyActive() ...\n");

   if (mapper && pm)
   {
      GRect brect=gmap_area->get_bound_rect();
      mapper->map(brect);
      brect.inflate(3, 3);

      GRect irect;
      if (irect.intersect(brect, pm_rect))
      {
	    // Translate the intersection into the GPixmap's coordinate
	 irect.translate(-pm_rect.xmin, -pm_rect.ymin);

	 GRect border_rect=gmap_area->get_bound_rect();
	 mapper->map(border_rect);

	    // Draw the border now
	 switch(getBorderType())
	 {
	    case GMapArea::NO_BORDER: break;
	    case GMapArea::SOLID_BORDER:
	       border_rect.translate(-pm_rect.xmin, -pm_rect.ymin);
	       MapDraw::drawRect(*pm, border_rect, gmap_area->border_color);
	       break;
	    case GMapArea::XOR_BORDER:
	       border_rect.translate(-pm_rect.xmin, -pm_rect.ymin);
	       MapDraw::drawRect(*pm, border_rect, 0xff000000);
	       break;
	    case GMapArea::SHADOW_IN_BORDER:
	    case GMapArea::SHADOW_OUT_BORDER:
	    case GMapArea::SHADOW_EIN_BORDER:
	    case GMapArea::SHADOW_EOUT_BORDER:
	    {
	       int xmin=border_rect.xmin;
	       int ymin=border_rect.ymin;
	       int xmax=border_rect.xmax;
	       int ymax=border_rect.ymax;
	       int width=border_rect.width();
	       int height=border_rect.height();
	    
	       GRect side, iside;
	    
		  // Top side
	       side=GRect(xmin, ymin, width, gmap_area->border_width);
	       if (iside.intersect(side, pm_rect))
		  makeFrame(iside, pm, pm_rect, iside.ymin-ymin,
			    xmax-iside.xmax, ymax-iside.ymax, iside.xmin-xmin);
	    
		  // Bottom side
	       side=GRect(xmin, ymax-gmap_area->border_width,
			  width, gmap_area->border_width);
	       if (iside.intersect(side, pm_rect))
		  makeFrame(iside, pm, pm_rect, iside.ymin-ymin,
			    xmax-iside.xmax, ymax-iside.ymax, iside.xmin-xmin);
	    
		  // Left side
	       side=GRect(xmin, ymin+gmap_area->border_width, gmap_area->border_width,
			  height-2*gmap_area->border_width);
	       if (iside.intersect(side, pm_rect))
		  makeFrame(iside, pm, pm_rect, iside.ymin-ymin,
			    xmax-iside.xmax, ymax-iside.ymax, iside.xmin-xmin);
	    
		  // Right side
	       side=GRect(xmax-gmap_area->border_width, ymin+gmap_area->border_width,
			  gmap_area->border_width, height-2*gmap_area->border_width);
	       if (iside.intersect(side, pm_rect))
		  makeFrame(iside, pm, pm_rect, iside.ymin-ymin,
			    xmax-iside.xmax, ymax-iside.ymax, iside.xmin-xmin);
	    }
	 } // switch(border_type)
      } // if (irect.intersect())
   } // if (mapper ...)
}

void
MapRect::ma_drawOutline(QPainter * p, const GRect & grect)
{
   GRect brect=gmap_rect->get_bound_rect();
   mapper->map(brect);

   p->drawRect(G2Q(brect));
}

MapRect::MapRect(const GP<GMapRect> & rect) :
      MapArea((GMapRect *) rect), gmap_rect(rect)
{
   copy_rect=GMapRect::create();
   saved_rect=GMapRect::create();
   DEBUG_MSG("MapRect::MapRect(): Initializing rectangular hyperlink\n");
}

/****************************************************************************
******************************* MapPoly definition ***************************
****************************************************************************/

void
MapPoly::ma_drawEditControls(QPainter * painter)
      // Correct painter modes are EXPECTED to be set BEFORE
{
   for(int i=0;i<gmap_poly->get_points_num();i++)
   {
      int x=gmap_poly->get_x(i), y=gmap_poly->get_y(i);
      mapper->map(x, y);
      painter->drawPie(x-1, y-1, 3,3, 0, 16*360);
   }
}

bool
MapPoly::ma_isPointerClose(int x, int y, int * vic_code, int * vic_data)
   // (x, y) are screen coords
{
   if (!mapper) return false;
   for(int i=0;i<gmap_poly->get_points_num();i++)
   {
      int vx=gmap_poly->get_x(i), vy=gmap_poly->get_y(i);
      mapper->map(vx, vy);
      int dx=vx-x, dy=vy-y;
      if (dx*dx+dy*dy<MOUSE_SENS*MOUSE_SENS)
      {
	 if (vic_code) *vic_code=VERTEX;
	 if (vic_data) *vic_data=i;
	 return true;
      }
   }
   return false;
}

void
MapPoly::createCursors(void)
{
   QSize vertex_size(vertex_width, vertex_height);
   vertex_cur()=new QCursor(QBitmap(vertex_size, (u_char *) vertex_bits, TRUE),
		      QBitmap(vertex_size, (u_char *) vertex_mask_bits, TRUE),
		      vertex_x, vertex_y);
}

GPQCursor
MapPoly::ma_getCursor(int vic_code, int vic_data)
{
   return vic_code==VERTEX ? vertex_cur() : ptr_cur();
}

void
MapPoly::ma_move(const GRect & doc_rect, int x, int y,
		 int vic_code, int vic_data)
   // doc_rect, (x, y) are in screen coord system
{
   if (!mapper) return;

   if (vic_data<0) vic_data=gmap_poly->get_points_num()-1;
   if (vic_code==VERTEX && vic_data>=0 && vic_data<gmap_poly->get_points_num())
   {
      int dx=x-start_x, dy=y-start_y;
      int vx=saved_poly->get_x(vic_data);
      int vy=saved_poly->get_y(vic_data);
      mapper->map(vx, vy);
      vx+=dx; vy+=dy;
      if (vx<doc_rect.xmin) vx=doc_rect.xmin;
      if (vx>doc_rect.xmax) vx=doc_rect.xmax;
      if (vy<doc_rect.ymin) vy=doc_rect.ymin;
      if (vy>doc_rect.ymax) vy=doc_rect.ymax;
      mapper->unmap(vx, vy);
      
      int sx=gmap_poly->get_x(vic_data);
      int sy=gmap_poly->get_y(vic_data);
      gmap_poly->move_vertex(vic_data, vx, vy);
      const char *s=gmap_poly->check_object();
      if (s&&s[0]) gmap_poly->move_vertex(vic_data, sx, sy);
   }
}

void
MapPoly::ma_drawInactive(const GRect & pm_rect, const GP<GPixmap> & pm)
      // pm_rect is the rectangle of the pane in screen coordinates
      // where pm is supposed to go to
{
}

void
MapPoly::ma_applyActive(const GRect & pm_rect, const GP<GPixmap> & pm)
      // pm_rect is the rectangle of the pane in screen coordinates
      // where pm is supposed to go to
{
   DEBUG_MSG("MapPoly::ma_applyActive() ...\n");

   if (mapper && pm &&
       (getBorderType()==GMapArea::XOR_BORDER ||
	getBorderType()==GMapArea::SOLID_BORDER))
   {
      GRect brect=gmap_area->get_bound_rect();
      mapper->map(brect);
      brect.inflate(3, 3);

      GRect irect;
      if (irect.intersect(brect, pm_rect))
      {
	 GRect drect=irect;
	 drect.inflate(2, 2);		// Just in case :)
	 mapper->unmap(drect);

	 for(int i=0;i<gmap_poly->get_sides_num();i++)
	    if (gmap_poly->does_side_cross_rect(drect, i))
	    {
	       int x1=gmap_poly->get_x(i), y1=gmap_poly->get_y(i);
	       int x2=gmap_poly->get_x((i+1) % gmap_poly->get_points_num());
	       int y2=gmap_poly->get_y((i+1) % gmap_poly->get_points_num());
	       mapper->map(x1, y1);
	       mapper->map(x2, y2);

	       if (getBorderType()==GMapArea::XOR_BORDER)
		  MapDraw::drawLine(*pm, x1-pm_rect.xmin, y1-pm_rect.ymin,
				    x2-pm_rect.xmin, y2-pm_rect.ymin, 0xff000000);
	       else
		  MapDraw::drawLine(*pm, x1-pm_rect.xmin, y1-pm_rect.ymin,
				    x2-pm_rect.xmin, y2-pm_rect.ymin, gmap_area->border_color);
	    }
      }
      
	 /* Uncomment this if you want pieces drawn
      for(GPosition pos=pieces;pos;++pos)
      {
	 MapPiece & p=*pieces[pos];
	 GRect grect=p;
	 mapper->map(grect);
	 grect.translate(-pm_rect.xmin, -pm_rect.ymin);
	 MapDraw::drawRect(*pm, grect, 0xff0000);
      } */
   }
}

void
MapPoly::ma_drawOutline(QPainter * p, const GRect & grect)
{
   for(int i=0;i<gmap_poly->get_sides_num();i++)
   {
      int x1=gmap_poly->get_x(i), y1=gmap_poly->get_y(i);
      int x2=gmap_poly->get_x((i+1) % gmap_poly->get_points_num());
      int y2=gmap_poly->get_y((i+1) % gmap_poly->get_points_num());
      mapper->map(x1, y1);
      mapper->map(x2, y2);

      p->drawLine(x1, y1, x2, y2);
   }
}

MapPoly::MapPoly(const GP<GMapPoly> & poly) :
      MapArea((GMapPoly *) poly), gmap_poly(poly)
{
   DEBUG_MSG("MapPoly::MapPoly(): Initializing rectangular hyperlink\n");
   copy_poly=GMapPoly::create();
   saved_poly=GMapPoly::create();
   createCursors();
}

/****************************************************************************
******************************* MapOval definition ***************************
****************************************************************************/

void
MapOval::ma_drawInactive(const GRect & grect, const GP<GPixmap> & pm)
      // pm_rect is the rectangle of the pane in screen coordinates
      // where pm is supposed to go to
{
}

void
MapOval::ma_applyActive(const GRect & pm_rect, const GP<GPixmap> & pm)
      // pm_rect is the rectangle of the pane in screen coordinates
      // where pm is supposed to go to
{
   DEBUG_MSG("MapOval::ma_applyActive() called\n");
   DEBUG_MAKE_INDENT(3);

   if (mapper && pm)
   {
      GRect brect=gmap_area->get_bound_rect();
      mapper->map(brect);
      brect.inflate(3, 3);

      GRect irect;
      if (irect.intersect(brect, pm_rect))
      {
	    // Translate the intersection into the GPixmap's coordinate
	 irect.translate(-pm_rect.xmin, -pm_rect.ymin);

	 GRect border_rect=gmap_area->get_bound_rect();
	 mapper->map(border_rect);
	 border_rect.translate(-pm_rect.xmin, -pm_rect.ymin);

	    // Draw the border now
	 switch(getBorderType())
	 {
	    case GMapArea::NO_BORDER: break;
	    case GMapArea::SOLID_BORDER:
	       MapDraw::drawOval(*pm, border_rect, gmap_area->border_color);
	       break;
	    case GMapArea::XOR_BORDER:
	       MapDraw::drawOval(*pm, border_rect, 0xff000000);
	       break;
	    case GMapArea::SHADOW_IN_BORDER:
	    case GMapArea::SHADOW_OUT_BORDER:
	    case GMapArea::SHADOW_EIN_BORDER:
	    case GMapArea::SHADOW_EOUT_BORDER:
		  // For compiler not to warn
	       break;
	 } // switch(border_type)
      } // if (irect.intersect())

      /* Uncomment this if you want pieces drawn 
      for(GPosition pos=pieces;pos;++pos)
      {
	 MapPiece & p=*pieces[pos];
	 GRect grect=p;
	 mapper->map(grect);
	 grect.translate(-pm_rect.xmin, -pm_rect.ymin);
	 MapDraw::drawRect(*pm, grect, 0xff0000);
      }*/
   } // if (mapper ...)

   DEBUG_MSG("MapOval::ma_applyActive(): DONE\n");
}

void
MapOval::ma_drawOutline(QPainter * p, const GRect & grect)
{
   GRect brect=gmap_oval->get_bound_rect();
   mapper->map(brect);

   p->drawEllipse(G2Q(brect));
}

MapOval::MapOval(const GP<GMapOval> & oval) :
      MapArea((GMapOval *) oval), gmap_oval(oval)
{
   DEBUG_MSG("MapOval::MapOval(): Initializing rectangular hyperlink\n");
   copy_oval=GMapOval::create();
   saved_oval=GMapOval::create();
}
