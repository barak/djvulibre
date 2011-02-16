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

#ifndef HDR_QD_BASE
#define HDR_QD_BASE
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "DjVuImage.h"
#include "GMarginCache.h"
#include "MapAreas.h"
#include "prefs.h"
#include "int_types.h"
#include "GException.h"
#include "qd_pane.h"
#include "GURL.h"
#include "DjVuAnno.h"
#include "DjVuText.h"

#include <qscrollbar.h>
#include <qtimer.h>
#include <qtooltip.h>


// QDBase is different from DjVuBase - base of the Motif plugin.
// It doesn't know anything about possible communications with Netscape,
// It doesn't care about NPSavedData, it doesn't even try to read/set
// top level window properties.
// The only exception is the getURL() virtual fn. Alas, I have to process
// hyperlinks events in the same function as scroll events. That's why
// the function is here.

class IncFlag
{
private:
   int          * pointer;
public:
   IncFlag(int * flag) : pointer(flag) { if (pointer) (*pointer)++; }
   ~IncFlag(void) { if (pointer) (*pointer)--; }
};

class QDMapAreaTip : public QToolTip
// I have to play the QToolTip games here (maintain the bounding rectangle
// of the hyperlink) because QT simply doesn't give me any motion events
// while the tip is shown. So I can't decide myself when to hide it =>
// I rely on QT to do it.
{
private:
   QString	text;
   QRect	qrect;
protected:
   virtual void	maybeTip(const QPoint & pnt)
   {
     if (qrect.contains(pnt)) tip(qrect, text);
   }
public:
   QDMapAreaTip(QString _text, const QRect & _qrect, QWidget * _parent) :
	 QToolTip(_parent), text(_text), qrect(_qrect) {}
   virtual ~QDMapAreaTip(void) { clear(); }
};

class QDTBHider : public QObject
{
   Q_OBJECT
private:
   class QDBase	* base;
protected:
   virtual bool eventFilter(QObject *obj, QEvent *ev);
public:
   QDTBHider(class QDBase * base);
   ~QDTBHider(void);
};

class QDBase : public QWidget, public GPEnabled
{
   Q_OBJECT
   friend class QDTBHider;
public:
   enum MODE_SOURCE { 
     MODE_SOURCE_MAX=5, SRC_MANUAL=4, SRC_SAVED=3,
     SRC_TAGS=2, SRC_ANT=1, SRC_DEFAULT=0 
   };
  class OverrideFlags
   {
   public:
     enum THUMB_POS { 
       THUMB_TOP=4, THUMB_BOTTOM=3, THUMB_LEFT=2,
       THUMB_RIGHT=1, THUMB_HIDE=0 
     };
     enum TOOLBAR_OPTS {
       TOOLBAR_AUTO=0x1, TOOLBAR_ALWAYS=0x2,
       TOOLBAR_TOP=0x4, TOOLBAR_BOTTOM=0x8,
       TOOLBAR_NO_DISPCOMBO=0x10, TOOLBAR_NO_RESCOMBO=0x20,
       TOOLBAR_NO_ZOOM=0x40, TOOLBAR_NO_PAN=0x80,
       TOOLBAR_NO_ZOOMSEL=0x100, TOOLBAR_NO_TEXTSEL=0x200,
       TOOLBAR_NO_PRINT=0x400, TOOLBAR_NO_SAVE=0x800,
       TOOLBAR_NO_BACKFORW=0x1000, TOOLBAR_NO_FIRSTLAST=0x2000,
       TOOLBAR_NO_PREVNEXT=0x4000, TOOLBAR_NO_ROTATE=0x8000,
       TOOLBAR_NO_SEARCH=0x10000, TOOLBAR_NO_PAGECOMBO=0x20000,
       TOOLBAR_NO_BUTTONS=0x3FFF0
     };
     THUMB_POS thumbnails;
     int toolbaropts;
     bool toolbar, scrollbars, menu;
     bool frame, links, logo, keyboard;
     bool print;
     int cmd_zoom;
     int cmd_rotate;
     int cmd_mode;
     int hor_align;	// See DjVuANT class for possible values
     int ver_align;	// or hor_align and ver_align flags
     GPList<GMapRect> hilite_rects;
     GURL url;
     OverrideFlags(void);
   };

private:
   bool			toolbar_shown;
   bool			toolbar_enabled;
   bool			showing_toolbar;
   QTimer		toolbar_timer;
   bool			toolbar_asked;
   
   class QDTBarModePiece* mode_tbar;
   class QDTBarRotatePiece* rotate_tbar;

   GP<GBitmap>	djvu_logo_bmp;
   QScrollBar	* hscroll, * vscroll;
   QTimer	cache_timer;

   GRect	lens_rect;

   GPQCursor	cur_wait, cur_hand1, cur_hand2, cur_zoom_select;
   GPQCursor	cur_hand_hl, cur_ptr, cur_blank;
   u_int32	back_color;
   QPixmap	back_pixmap;

   int		displ_dpi;
   int		mode_prio[MODE_SOURCE_MAX];
   int		left_butt_down;
   int		in_hand_scroll, in_paint, in_layout, in_zoom_select;
   OverrideFlags override_flags;

   int		acc_scroll_dh, acc_scroll_dv;
   int		hand_scroll_x, hand_scroll_y;
   int		zoom_select_x0, zoom_select_y0;
   GRect        *lastrect;

   GRectMapper	mapper;

   bool		display_all_hlinks;

   void         drawSelectionRect(const QRect &rect);
   void		createCursors(void);
   void		paint(const GRect & grect);
   void		paint(QPaintDevice * drawable, int dr_x,
		      int dr_y, const GRect & in_rect,
		      bool for_lens=false);
   void		paintLens(const GRect * clip_rect=0);
   void		preScroll(void);
   void		postScroll(void);
   int		getLensHotKey(void) const;
private slots:
      // Slots attached to scroll bars
   void		slotSliderPressed(void);
   void		slotSliderReleased(void);
   void		slotSliderMoved(int);

      // Slots called from the toolbar
   void		slotToolBarTimeout(void);
   void		slotSetZoom(int cmd_zoom);
   void		slotSetMode(int cmd_mode);
   void		slotStickToolBar(bool on);
   void		slotSetRotate(int cmd_rotate);
   void		slotSetPaneMode(int cmd_pane);
   
      // Slot called when the system is idle (to update caches)
   void		slotCheckCache(void);
protected:
   static const int       toolbar_edge;
   static const char	* search_results_name;
   class QSplitter	* splitter;
   QWidget	* main_widget, * thumb_widget;
   GP<QDPane>   pane;
   int		depth;
   class QDToolBar	* toolbar;

   bool		ignore_ant_mode_zoom;
   int		zoom_src, mode_src, rotate_src;
   int		cmd_zoom, cmd_mode, cmd_mode_force, cmd_rotate;
   int          pane_mode;

   GPQCursor	cur_last;
   
   GRect	rectDocument, rectVisible;
   GP<DjVuImage>dimg;
   GP<DjVuAnno>	anno;
   
   DjVuPrefs	prefs;

   QDMapAreaTip	* map_area_tip;
   MapArea	* cur_map_area;
   GPList<MapArea>	map_areas;
   GRectMapper	ma_mapper;

   GMarginCache<GBitmap>	bm_cache;
   GMarginCache<GPixmap>	pm_cache;

   bool		isLensVisible(void) const;
   void		showLens(int x, int y);
   void		hideLens(void);
   
   void		showStatus(const QString &status) { emit sigShowStatus(status); }
   void		scroll(int dh, int dv, int update_scrollbars=1);

   bool		needToShowToolBar(void) const;
   bool		needToHideToolBar(void) const;
   bool		isToolBarEnabled(void) const;
   void		enableToolBar(bool on);
   bool		isToolBarShown(void) const;
   void		showToolBar(bool slow=true);
   void		hideToolBar(bool slow);
   bool		isToolBarStuck(void) const;
   void		stickToolBar(void);
   void		unStickToolBar(void);
   void		addLeftToolBarWidget(QWidget * widget);
   void		addRightToolBarWidget(QWidget * widget);

   OverrideFlags getOverrideFlags(void) { return override_flags; };

   void		displaySearchResults(const GList<DjVuTXT::Zone *> & zones_list);
   void		eraseSearchResults(void);
   virtual void	createMapAreas(bool allow_draw);
   void		eraseMapAreas(bool search_results_too, bool allow_draw);
   virtual void	decodeAnno(bool allow_redraw);
   void		processAnno(bool allow_redraw);
   void		cleanAnno(bool allow_redraw);
   void		setMappers(void);

   virtual QWidget * createThumbnails(bool _rowMajor);
   
   bool		processMouseMoveEvent(QMouseEvent * ev);

   void		createToolBar(void);
   virtual void	updateToolBar(void);
   virtual void	fillToolBar(class QDToolBar * toolbar);
   
   virtual bool eventFilter(QObject *obj, QEvent *ev);
   virtual void	setCursor(void);
   virtual void	getURL(const GUTF8String &, const GUTF8String &) {}
   virtual void	setDjVuImage(const GP<DjVuImage> & _dimg, int do_redraw);

   virtual void	resizeEvent(QResizeEvent * ev);
   virtual void	updateEditToolBar(void) {}
signals:
   void		sigShowStatus(const QString &name);
   void         sigQueryFullScreen(bool &);
public slots:
   void		slotEnableDisplayAllHLinks(void);
   void		slotDisableDisplayAllHLinks(void);
public:
   bool		being_destroyed;
   bool		image_size_known;

   void		enableDisplayAllHLinks(void) { slotEnableDisplayAllHLinks(); }
   void		disableDisplayAllHLinks(void) { slotDisableDisplayAllHLinks(); }

   bool		thumbnailsShown(void) const { return splitter!=0; }
   virtual void	showThumbnails(void);
   void		hideThumbnails(void);

   void		exportToPNM(void);
   
   void		setMode(int cmd_mode, bool do_redraw=1,	
                        int mode_src=SRC_DEFAULT);
   int		getMode(bool disregard_force=false) const;
   void		setZoom(int cmd_zoom, bool do_layout=1,
                        int zoom_src=SRC_DEFAULT);
   int		getCMDZoom(void) const { return cmd_zoom; }
   int		getZoom(void) const;
   void		setRotate(int cmd_rotate, bool do_redraw=1, 
                          int rotate_src=SRC_DEFAULT);
   int          getRotate(void) const;
   void		setBackgroundColor(u_int32 color, int do_redraw);
   u_int32	getBackgroundColor(void) const { return back_color; }
   void		setOverrideFlags(const OverrideFlags & flag);

   GP<DjVuImage>getDjVuImage(void) const { return dimg; }

   virtual void	layout(bool allow_redraw=1);
   virtual void	redraw(const GRect * rect=0);
   //This function should be called when someone changed contents of the
   //image that has been passed to this class via {\Ref setDjVuImage}()
   //before
   virtual void	imageUpdated(void);

   QDBase(QWidget * parent=0, const char * name=0);
   ~QDBase(void) { being_destroyed=true; }
};

// Probably not a good name for 'disregard_force', but here is explanation:
//   The mode can be "forced" by a particular image type. Say, when the image
//   is pure photo or pure b&w, the mode is "forced" to be COLOR or B&W
//   respectively. Most of the calls to getMode() do not pass it any arguments,
//   and getMode will return the net mode (mode, which should be used to display
//   data). Sometimes, though, it's necessary to know the real mode (say, to
//   store it in the NPSavedData). Passing 'false' to getMode() solves this task.
inline int
QDBase::getMode(bool disregard_force) const
{
   return (!disregard_force && cmd_mode_force>=0) ? cmd_mode_force : cmd_mode;
}

#endif
