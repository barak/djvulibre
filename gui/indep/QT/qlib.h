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

#ifndef HDR_QLIB
#define HDR_QLIB
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "GException.h"
#include "GRect.h"
#include "Arrays.h"
#include "GPixmap.h"

#include "qt_n_in_one.h"

#include <qdialog.h>
#include <qpushbutton.h>
#include <qrect.h>
#include <qtableview.h>
#include <qmenudata.h>

#include "qt_fix.h"
#include "debug.h"

class QeExcMessage : public QeDialog
{
   Q_OBJECT
private:
   QeNInOne	* details;
   QePushButton	* details_butt;
private slots:
   void		switchDetails(void);
public:
   QeExcMessage(const GException & exc, const char * title=0,
		QWidget * parent=0,
		const char * name=0, bool modal=TRUE);
};

// Links slaves' geometry to the master's geometry. Depending on the
// value of flags (width and height) it will make sure, that width
// and/or height of all slaves is the same as width and/or height
// of the master
//
// NOTE!!!
// When link_flags contains LINK_MIN_WIDTH or LINK_MIN_HEIGHT, the
// corresponding minimum dimension of the slave will be linked to
// respective REGULAR dimension of the master
class GeoLink : public QObject
{
   Q_OBJECT
private:
   class Slave : public GPEnabled
   {
   public:
      enum { LINK_X=1, LINK_Y=2, LINK_WIDTH=4, LINK_HEIGHT=8,
	     LINK_MIN_WIDTH=16, LINK_MIN_HEIGHT=32 };
      int	link_flags;
      QWidget	* widget;

      Slave(QWidget * widget, int link_flags);
   };
      
   QWidget	* master;
   GPList<Slave>slaves;
public:
   enum { LINK_X=1, LINK_Y=2, LINK_WIDTH=4, LINK_HEIGHT=8,
	  LINK_MIN_WIDTH=16, LINK_MIN_HEIGHT=32 };
   
   virtual bool	eventFilter(QObject * obj, QEvent * ev);
   
   void		addSlave(QWidget * widget, int link_flags);
   void		delSlave(QWidget * widget);
   GeoLink(QWidget * master, QObject * parent=0, const char * name=0);
};

class QeGrid : public QWidget
// QLayouts don't allow to create a grid of widgets with the same cell
// sizes. Actually, this *can* be done, but it requires that both
// minsize and stretching are the same, which is hard to achieve.
// This class places its children in the cells of the grid with specified
// alignment.
//
// It watches min/max sizes of the children and resizes/aligns them and
// itself correspondingly
{
   Q_OBJECT
private:
   int		border, autoBorder;
   int		rows, cols;
   int		ignore_resize;
   QWidget 	*** widget;
   int		** alignment;
protected:
   virtual void	resizeEvent(QResizeEvent * ev);
   virtual bool	event(QEvent * ev);

   void		recalcMinMax(void);
public:
   void		addWidget(QWidget * w, int row, int col,
			  int alignment=AlignCenter);
   int		setBorder(int b) { border=b; return b;};
   int		setAutoBorder(int ab) { autoBorder=(ab+1)/2*2; return autoBorder;};
   
   QeGrid(int rows, int cols, QWidget * parent=0, const char * name=0);
   ~QeGrid(void);
};

class QeRowColumn : public QTableView
{
   Q_OBJECT
private:
   bool			do_layout;
   int			hor_interval, ver_interval;
   int			min_visible_rows, min_visible_cols;
   int			alignment;
   TArray<void *>	label;

   void			layout(void);
protected:
   virtual void		show(void);
   virtual bool		event(QEvent * ev);
   virtual void		resizeEvent(QResizeEvent * ev);
   virtual void		paintCell(QPainter * p, int row, int col);
public:
   void			setMinVisibleRows(int _min_visible_rows);
   void			setMinVisibleCols(int _min_visible_cols);
   void			setHorInterval(int _hor_interval);
   void			setVerInterval(int _ver_interval);
   void			setAlignment(int _alignment);
   
   QeRowColumn(QWidget * parent=0, const char * name=0);
   ~QeRowColumn(void) {};
};

inline void
QeRowColumn::setMinVisibleRows(int _min_visible_rows)
{
   min_visible_rows=_min_visible_rows;
   if (min_visible_rows<0) min_visible_rows=0;
   if (isVisible()) layout(); else do_layout=true;
}

inline void
QeRowColumn::setMinVisibleCols(int _min_visible_cols)
{
   min_visible_cols=_min_visible_cols;
   if (min_visible_cols<0) min_visible_cols=0;
   if (isVisible()) layout(); else do_layout=true;
}

inline void
QeRowColumn::setHorInterval(int _hor_interval)
{
   hor_interval=_hor_interval;
   if (hor_interval<0) hor_interval=0;
   if (isVisible()) layout(); else do_layout=true;
}

inline void
QeRowColumn::setVerInterval(int _ver_interval)
{
   ver_interval=_ver_interval;
   if (ver_interval<0) ver_interval=0;
   if (isVisible()) layout(); else do_layout=true;
}

inline void
QeRowColumn::setAlignment(int _alignment)
{
   alignment=_alignment;
   repaint();
}

inline GRect
Q2G(const QRect & r)
{
   return GRect(r.x(), r.y(), r.width(), r.height());
}

inline QRect
G2Q(const GRect & r)
{
   return QRect(r.xmin, r.ymin, r.width(), r.height());
}

void showError(QWidget * parent, const GException & exc);
void showError(QWidget * parent, const QString &title, const GException & exc);
void showError(QWidget * parent, const QString &title, const QString &message);
void showWarning(QWidget * parent, const QString &title, const QString &message);
void showInfo(QWidget * parent, const QString &title, const QString &message);
void showMessage(QWidget * parent, const QString &title, const QString &message,
		 bool draw_frame=false, bool use_fixed=false,
		 bool word_wrap=false);

// Goes down the widget's hierarchy and enables/disables each child
void setEnabled(QWidget * widget, bool flag);

// Goes down the menu's hierarchy and enables/disables each item
// Note! This is not the same as disabling each child.
// Items are not widgets.
void setItemsEnabled(QMenuData * menu, bool flag);

// Goes down the menu's hierarchy until it finds a submenu 'pane'
// After it does it, it will return the ID under which the 'pane' is
// registered. This procedure is useful to learn the ID of the pane, which
// has just been activated. It is not stored in the pane, it's stored in
// its parent in the menu hierarchy (not even widget hierarchy). So we
// have to traverse it.
int paneToID(QMenuData * menu, QMenuData * pane);

QPixmap createIcon(const GPixmap & gpix);

#endif
