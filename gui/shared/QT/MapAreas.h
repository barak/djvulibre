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
// $Id: MapAreas.h,v 1.8 2007/03/25 20:48:24 leonb Exp $
// $Name: release_3_5_22 $

#ifndef HDR_MAPAREAS
#define HDR_MAPAREAS
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif

#include "GMapAreas.h"
#include "GContainer.h"
#include "GString.h"
#include "GRect.h"
#include "GBitmap.h"
#include "GPixmap.h"
#include "DjVuImage.h"
#include "DjVuDocEditor.h"

#include "qd_painter.h"

#include <qpixmap.h>
#include <qobject.h>
#include <qcursor.h>
#include "int_types.h"


class GPQCursor
{
private:
  class GQCursor : public GPEnabled
  {
  private:
    QCursor *ptr;
    GQCursor(QCursor *);
    GQCursor(void);
    ~GQCursor();
    friend class GPQCursor;
  };
  GP<GQCursor> current;
public:
  GPQCursor(void);
  GPQCursor(QCursor *ptr);
  ~GPQCursor();
  GPQCursor &operator=(QCursor *ptr) {current=new GQCursor(ptr);return *this;}
  operator const QCursor * () const { return current?(current->ptr):0; }
};

// MapArea is a MapArea Object - stuff you need to display map areas
// on the screen. Sometimes it's necessary to cache window parts under
// the hyperlink's boundaries to be able to restore the contents when
// the hyperlink should be hided. MapPiece is the class for this. It's a
// rectangle containing two pixmaps: off_pixmap and on_pixmap. Sometimes
// the whole hyperlink is covered by one MapPiece, sometimes there is a dozen
// of them placed along the hyperlink boundary.

//****************************************************************************
//***************************** MapPiece declaration *************************
//****************************************************************************

/* All classes below use *document* coordinates. But all functions (except
   for constructors) accept screen coordinates
*/
   
class MapPiece : public GRect, public GPEnabled
{
private:
   QPixmap	on_pixmap, off_pixmap;
   GRectMapper	* mapper;
public:
   void		clearPixmaps(void);
   void		createPixmaps(void);
      // DO return references here. Otherwise "clever" QT optimization
      // results in the fact that pixmaps detach right when I don't expect it
   QPixmap &	getOnPixmap(void) { return on_pixmap; }
   QPixmap &	getOffPixmap(void) { return off_pixmap; }

   MapPiece(void) : mapper(0) {}
   MapPiece(const GRect & grect, GRectMapper * _mapper);
   virtual ~MapPiece(void) {}
};

inline
MapPiece::MapPiece(const GRect & grect, GRectMapper * _mapper) :
      GRect(grect), mapper(_mapper)
{
      // Workaround the QT bug, when it fails to draw a rectangle properly
      // in a pixmap of width 1.
   mapper->map(*this);
   if (width()==1) xmax++;
   if (height()==1) ymax++;
   mapper->unmap(*this);
}

/****************************************************************************
**************************** MapObject declaration **************************
****************************************************************************/

class MapArea : public QObject, public GPEnabled
{
   Q_OBJECT
      // MapArea uses screen coordinates, not *document* ones
public:
   enum { EDIT_NOTHING_CHANGED=0, EDIT_SMTH_CHANGED=1, EDIT_CANCELLED=2 };
   enum VicCode { FAR=0, INSIDE=1, TOP_LEFT=2, TOP=3, TOP_RIGHT=4,
		  RIGHT=5, BOTTOM_RIGHT=6, BOTTOM=7, BOTTOM_LEFT=8, LEFT=9,
		  OBJECT_CODES=10 };
   enum DRAW_MODE { DRAW_INACTIVE=1, APPLY_ACTIVE=2, DRAW_ACTIVE=3 };
private:
   QWidget	* pane;
   
   bool		in_motion;
   bool		active_outline_mode, inactive_outline_mode;
   bool		active;
   bool		force_always_active;
   bool		enable_edit_controls;
protected:  
   static GPQCursor	&top_left_cur(void);
   static GPQCursor	&top_cur(void);
   static GPQCursor	&top_right_cur(void);
   static GPQCursor	&right_cur(void);
   static GPQCursor	&bottom_right_cur(void);
   static GPQCursor	&bottom_cur(void);
   static GPQCursor	&bottom_left_cur(void);
   static GPQCursor	&left_cur(void);
   static GPQCursor	&inside_cur(void);
   static GPQCursor	&ptr_cur(void);
private:
      // Will generate PaintEvent for the specified piece
   void		repaintPiece(const GP<MapPiece> & piece);
   void		setBorderType(int border_type);
   void		createCursors(void);
   int		max(int x, int y) { return x<y ? y : x; }
   int		min(int x, int y) { return x<y ? x : y; }
   void		initBorder(void);
   GRect	getPiecesBoundRect(void) const;
protected:
   GPList<MapPiece>	pieces;
   GRectMapper		* mapper;
   GRect		doc_rect;
   
   int			start_x, start_y;
   
   signed char		shadow_pattern[32];

   QWidget *		getPane(void) const { return pane; }

   virtual void		ma_applyActive(const GRect & pm_rect, const GP<GPixmap> & pm)=0;
   virtual void		ma_drawInactive(const GRect & pm_rect, const GP<GPixmap> & pm)=0;
   virtual void		ma_drawOutline(QPainter * painter, const GRect & grect)=0;
   virtual void 	ma_drawEditControls(QPainter * painter);
   virtual void		ma_generatePieces(void)=0;
   virtual bool		ma_isPointerClose(int x, int y, int * vic_code=0, int * vic_data=0)=0;
   virtual GPQCursor	ma_getCursor(int vic_code, int vic_data)=0;
   virtual void 	ma_move(const GRect & doc_rect, int x, int y,
				int vic_code, int vic_data)=0;
   virtual void		ma_saveData(void)=0;
   virtual void		ma_restoreSavedData(void)=0;
   virtual GMapArea *	ma_getCopiedData(void)=0;
   virtual void		ma_loadCopy(void)=0;
   virtual void 	ma_copySaved(void)=0;

   MapArea(const GP<GMapArea> & area);
public:
   GP<GMapArea>	gmap_area;

   int		get_xmin(void) { return gmap_area->get_xmin(); }
   int		get_ymin(void) { return gmap_area->get_ymin(); }
   int		get_xmax(void) { return gmap_area->get_xmax(); }
   int		get_ymax(void) { return gmap_area->get_ymax(); }
   
   GUTF8String	getURL(void) const { return gmap_area->url; }
   GUTF8String	getTarget(void) const { return gmap_area->target; }
   GUTF8String	getComment(void) const { return gmap_area->comment; }
   u_int32	getHiliteColor(void) const { return gmap_area->hilite_color; }
   bool		isBorderAlwaysVisible(void) const { return gmap_area->border_always_visible; }
   bool		isHyperlink(void) const { return gmap_area->url.length()!=0; }
   GMapArea::BorderType	getBorderType(void) const { return gmap_area->border_type; }
   bool		isMoving(void) const { return in_motion; }

      // OUTLINE mode is when the hypelink is drawn not into a pixmap
      // but directly to the screen using XOR method. Use drawOutline()
      // to actually draw stuff. If active_outline is set, only the
      // ACTIVE part will be drawn as outline (the one, which may be on or off)
      // is inactive_outline is set too, the whole map area is drawn in the
      // OUTLINE mode (useful for moving in the editor)
   void		setOutlineMode(bool inactive_on, bool active_on, bool redraw);
   void		setActiveOutlineMode(bool on, bool redraw);
   void		setInactiveOutlineMode(bool on, bool redraw);
   bool		isInactiveOutlineMode(void) const { return inactive_outline_mode; }
   bool		isActiveOutlineMode(void) const { return active_outline_mode; }

   void		enableEditControls(bool on, bool redraw);
   bool		editControlsEnabled(void) const { return enable_edit_controls; }

      // Affects drawing of some stuff. Currently, inactive MapArea will
      // not draw its border.
   void		setActive(bool on, bool redraw);
   bool		isActive(void) const { return active; }

      // Returns TRUE either if border is always visible or always invisible
   bool		isAlwaysActive(void) const;
   void		setForceAlwaysActive(bool on);

   bool		isCacheUsed(void) const { return !isAlwaysActive() && !isActiveOutlineMode(); }

      // Drawing stuff
      // Draws all contents of the hyperlink into the specified
      // GPixmap/GBitmap unless ACTIVE_OUTLINE mode is on
   void		draw(const GRect & pm_rect, const GP<GPixmap> & pm,
		     DRAW_MODE draw_mode);
   void		draw(const GRect & bm_rect, const GP<GBitmap> & bm_in,
		     GRect & pm_rect, GP<GPixmap> & pm_out,
		     DRAW_MODE draw_mode);
      // Will draw outline of the map area using XOR mode in the pane
   void		drawOutline(const GRect & grect, QPainter * p=0);
      // Will draw edit controls of the map area directly into the pane
   void		drawEditControls(const GRect & grect, QPainter * p=0);
   void		repaint(const GRect & rect);
      // Causes PaintEvent to be generated for the whole hyperlink area
   void		repaint(void);
      // Causes PaintEvent to be generated for the area's boundary
   void		repaintBorder(void);
   
   void		reset(void);
   void		exposeBorder(void);
   void		layout(const GRect & doc_rect);
   void		updateCache(const GRect & pm_rect, const GP<GPixmap> & pm,
			    GRectMapper * sdoc_mapper);
   
   void		detachWindow(void);
   void		attachWindow(QWidget * _pane, GRectMapper * _mapper);

   int		edit(GP<DjVuDocEditor> & doc,
		     const GP<DjVuImage> & dimg, float gamma);

   bool		is_point_inside(int x, int y);
   GRect	get_bound_rect(void);

   bool		isPointerClose(int x, int y, int * vic_code=0, int * vic_data=0);
   GPQCursor	getCursor(int vic_code, int vic_data);
   void		startMoving(int x, int y);
   void		abortMoving(void) { in_motion=false; ma_restoreSavedData(); }
   void		move(const GRect & doc_rect, int x, int y,
		     int vic_code, int vic_data);
   void		finishMoving(void) { in_motion=false; ma_generatePieces(); }
   
   virtual ~MapArea(void)
   {
      // Whatever you want to destroy here, which has smth to do with
      // graphics - don't forget to destroy it in DetachWindow() too.
      detachWindow();
   }
};

inline bool
MapArea::is_point_inside(int x, int y)
{
   return gmap_area->is_point_inside(x, y);
}

inline GRect
MapArea::get_bound_rect(void)
{
   return gmap_area->get_bound_rect();
}

inline void
MapArea::startMoving(int x, int y)
{
   start_x=x; start_y=y;
   in_motion=true;
   ma_saveData();
}

inline void
MapArea::layout(const GRect & _doc_rect)
{
   doc_rect=_doc_rect;
   ma_generatePieces();
}

/****************************************************************************
******************************* MapRect declaration *************************
****************************************************************************/

class MapRect : public MapArea
{
private:
   GP<GMapRect>	gmap_rect;
   GP<GMapRect>	saved_rect, copy_rect;

   void		makeFrame(const GRect & dst_rect,
			  GPixmap * pm,
			  const GRect & src_rect,
			  int top_margin, int right_margin,
			  int bottom_margin, int left_margin);
protected:
   virtual void		ma_applyActive(const GRect & pm_rect, const GP<GPixmap> & pm);
   virtual void		ma_drawInactive(const GRect & pm_rect, const GP<GPixmap> & pm);
   virtual void		ma_drawOutline(QPainter * painter, const GRect & grect);
   virtual void		ma_generatePieces(void);
   virtual bool		ma_isPointerClose(int x, int y, int * vic_code=0, int * vic_data=0) { return 0; }
   virtual GPQCursor	ma_getCursor(int vic_code, int vic_data) { QCursor *q=new QCursor(ArrowCursor); return GPQCursor(q);}
   virtual void		ma_move(const GRect & doc_rect, int x, int y,
				int vic_code, int vic_data) {}
   virtual void		ma_saveData(void) { *saved_rect=*gmap_rect; }
   virtual void		ma_restoreSavedData(void) { *gmap_rect=*saved_rect; }
   virtual GMapArea *	ma_getCopiedData(void) { return copy_rect; }
   virtual void		ma_loadCopy(void) { *gmap_rect=*copy_rect; }
   virtual void		ma_copySaved(void) { *copy_rect=*saved_rect; }
public:
   MapRect(const GP<GMapRect> & rect);
   virtual ~MapRect(void) {}
};

/****************************************************************************
***************************** MapPoly declaration ***************************
****************************************************************************/

class MapPoly : public MapArea
{
public:
   enum VicType { VERTEX=OBJECT_CODES+1, POLY_CODES=OBJECT_CODES+2 };
private:
   GP<GMapPoly>		gmap_poly;
   GP<GMapPoly>		saved_poly, copy_poly;

   static GPQCursor&	vertex_cur(void);
   
   void		createCursors(void);
protected:
   virtual void		ma_applyActive(const GRect & pm_rect, const GP<GPixmap> & pm);
   virtual void		ma_drawInactive(const GRect & pm_rect, const GP<GPixmap> & pm);
   virtual void		ma_drawOutline(QPainter * painter, const GRect & grect);
   virtual void		ma_drawEditControls(QPainter * painter);
   virtual void		ma_generatePieces(void);
   virtual bool		ma_isPointerClose(int x, int y, int * vic_code=0, int * vic_data=0);
   virtual GPQCursor	ma_getCursor(int vic_code, int vic_data);
   virtual void 	ma_move(const GRect & doc_rect, int x, int y,
				int vic_code, int vic_data);
   virtual void		ma_saveData(void) { *saved_poly=*gmap_poly; }
   virtual void		ma_restoreSavedData(void) { *gmap_poly=*saved_poly; }
   virtual GMapArea *	ma_getCopiedData(void) { return copy_poly; }
   virtual void		ma_loadCopy(void) { *gmap_poly=*copy_poly; }
   virtual void		ma_copySaved(void) { *copy_poly=*saved_poly; }
public:
   MapPoly(const GP<GMapPoly> & poly);
   virtual ~MapPoly(void) {}
};

/****************************************************************************
***************************** MapOval declaration ***************************
****************************************************************************/

class MapOval : public MapArea
{
private:
   GP<GMapOval>	gmap_oval;
   GP<GMapOval>	saved_oval, copy_oval;
protected:
   virtual void		ma_applyActive(const GRect & pm_rect, const GP<GPixmap> & pm);
   virtual void		ma_drawInactive(const GRect & pm_rect, const GP<GPixmap> & pm);
   virtual void		ma_drawOutline(QPainter * painter, const GRect & grect);
   virtual void		ma_generatePieces(void);
   virtual bool		ma_isPointerClose(int x, int y, int * vic_code=0, int * vic_data=0) { return 0; }
   virtual GPQCursor	ma_getCursor(int vic_code, int vic_data) { return new QCursor(ArrowCursor);}
   virtual void 	ma_move(const GRect & doc_rect, int x, int y,
				int vic_code, int vic_data) {}
   virtual void		ma_saveData(void) { *saved_oval=*gmap_oval; }
   virtual void		ma_restoreSavedData(void) { *gmap_oval=*saved_oval; }
   virtual GMapArea *	ma_getCopiedData(void) { return copy_oval; }
   virtual void		ma_loadCopy(void) { *gmap_oval=*copy_oval; }
   virtual void 	ma_copySaved(void) { *copy_oval=*saved_oval; }
public:
   MapOval(const GP<GMapOval> & oval);
   virtual ~MapOval(void) {}
};

#endif
