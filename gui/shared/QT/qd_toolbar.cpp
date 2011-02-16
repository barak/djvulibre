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

#include "qd_toolbar.h"
#include "debug.h"
#include "qlib.h"

#include <qapplication.h>
#include <qobjectlist.h>



#define margin 2

//****************************************************************************
//******************************** QTBWidget**********************************
//****************************************************************************

class QTBWidget : public QWidget
{
   Q_OBJECT
protected:
   bool		event(QEvent * ev);
public:
   QTBWidget(QWidget * parent=0, const char * name=0) :
	 QWidget(parent, name) {}
};

bool
QTBWidget::event(QEvent * ev)
{
   if (ev->type()==QEvent::LayoutHint ||
       ev->type()==QEvent::ChildInserted ||
       ev->type()==QEvent::ChildRemoved)
   {
      const QObjectList * childList=children();
      if (childList)
      {
	 QObjectListIt it(*childList);
	 QObject * obj;

	 int width=0, height=0;
	 while((obj=it.current()))
	 {
	    ++it;
	    if (obj->isWidgetType())
	    {
              QWidget * w=(QWidget *) obj;
              if (w->isHidden())
                continue;
              QSize ms=w->minimumSize();
              width+=ms.width()+margin;
              if (height<ms.height()) height=ms.height();
	    }
	 }
         width = (width>margin) ? width-margin : 0;
	 setMinimumSize(width, height);
      }
   } 
   else if (ev->type()==QEvent::Resize)
   {
      const QObjectList * childList=children();
      if (childList)
      {
	 QObjectListIt it(*childList);
	 QObject * obj;

	 int x=0;
	 int height=QWidget::height();
	 while((obj=it.current()))
	 {
	    ++it;
	    if (obj->isWidgetType())
	    {
	       QWidget * w=(QWidget *) obj;
               if (w->isHidden())
                 continue;
	       int width=w->minimumSize().width();
	       w->resize(width, height);
	       w->move(x, 0);
	       x+=width+margin;
	    }
	 }
      }
   }
   return QWidget::event(ev);
}

//****************************************************************************
//******************************** QDToolBar *********************************
//****************************************************************************

void
QDToolBar::addPiece(QDTBarPiece * piece)
{
   pieces.append(piece);
}

void
QDToolBar::setEnabled(bool en)
{
   QWidget::setEnabled(en);
   for(GPosition pos=pieces;pos;++pos)
      pieces[pos]->setEnabled(en);
}

bool
QDToolBar::positionWidgets(int width, int rows, bool move, int * height_ptr)
      // Will position widget virtually (if move is FALSE) or in reality
      // (if move is TRUE) using the passed width for the widget's area.
      // The height will be calculated basing on the 'rows'
      // If 'rows' is too small to layout all the widgets, FALSE will be
      // returned. Otherwise - TRUE.
      // Call this function in a loop gradually increasing 'rows' until
      // it returns TRUE. Then call it again with move=TRUE.
      // If 'height_ptr' is not ZERO, it will be set to the calculated
      // height of the widget.
      // 
      // The function does NOT change the toolbar's dimensions. It may
      // reposition the child widgets though.
{
   GPosition pos;
   int max_height=0;	// Maximum height of all widgets
   for(pos=left_list;pos;++pos)
   {
      int height=left_list[pos]->minimumSize().height();
      if (height>max_height) max_height=height;
   }
   for(pos=right_list;pos;++pos)
   {
      int height=right_list[pos]->minimumSize().height();
      if (height>max_height) max_height=height;
   }

   QRect crect=contentsRect();
   crect.setRight(crect.width()+width-rect().width());
   int height=max_height*rows+margin*(rows+1);

      // Position left_list
   int x=crect.left()+margin, y=crect.top()+margin;
   int left_row=0;
   for(pos=left_list;pos;++pos)
   {
      QWidget * w=left_list[pos];
      if (w->isHidden())
        continue;
      int width=w->minimumSize().width();
      if (x+width+margin>crect.right())
      {
	 left_row++;
	 if (left_row>=rows) return false;
	 x=crect.left()+margin;
	 y+=max_height+margin;
      }
      if (move)
      {
	 w->resize(width, max_height);
	 w->move(x, y+(max_height-w->height())/2);
      }
      x+=width+margin;
   }
   int left_x_r=x;

      // Position right_list
   x=crect.right()-margin, y=crect.top()+height-margin;
   int right_row=rows-1;
   for(pos=right_list;pos;++pos)
   {
      QWidget * w=right_list[pos];
      if (w->isHidden())
        continue;
      int width=w->minimumSize().width();
      if (x-width-margin<crect.left())
      {
	 right_row--;
	 if (right_row<left_row) return false;
	 x=crect.right()-margin;
	 y-=max_height+margin;
      }
      if (move)
      {
	 w->resize(width, max_height);
	 w->move(x-width, y-max_height+(max_height-w->height())/2);
      }
      x-=width+margin;
   }
   int right_x_l=x;

   if (height_ptr) *height_ptr=height+crect.top()+rect().bottom()-crect.bottom();
   
      // See if left_list and right_list overlap
   return (left_row<right_row ||
	   (left_row==right_row &&
	    left_x_r+margin<=right_x_l));
}

int
QDToolBar::positionWidgets(void)
      // Will position widgets and return suggested height for the toolbar.
      // It won't resize the toolbar though.
{
   int height=0;
   int rows=1;
   while(!positionWidgets(width(), rows, false, &height))
      rows++;
   positionWidgets(width(), rows, true);
   return height;
}

int
QDToolBar::computeHeight(int width)
{
   int height=0;
   int rows=1;
   while(!positionWidgets(width, rows, false, &height))
      rows++;
   return height;
}

bool
QDToolBar::event(QEvent * ev)
{
   if (ev->type()==QEvent::LayoutHint ||
       ev->type()==QEvent::ChildInserted ||
       ev->type()==QEvent::ChildRemoved ||
       ev->type()==QEvent::Resize)
      adjustPositions();
   return QFrame::event(ev);
}

void
QDToolBar::adjustPositions(void)
{
   int h=positionWidgets();
   setFixedHeight(h);
}

void
QDToolBar::addLeftWidget(QWidget * widget)
{
   if (!left_list.contains(widget))
   {
      widget->resize(widget->minimumSize());
      widget->show();
      left_list.append(widget);
      connect(widget, SIGNAL(destroyed(void)), this, SLOT(slotWidgetDestroyed(void)));
   }
}

void
QDToolBar::addLeftWidgets(const GList<QWidget *> & list)
{
   if (list.size()) 
     {
       QTBWidget * widget=new QTBWidget(this);
       for(GPosition pos=list;pos;++pos)
         list[pos]->reparent(widget, 0, QPoint(0, 0), TRUE);
       addLeftWidget(widget);
     }
}

void
QDToolBar::addLeftWidgets(QWidget * w1, QWidget * w2, QWidget * w3,
			  QWidget * w4, QWidget * w5, QWidget * w6)
{
   GList<QWidget *> list;
   if (w1) list.append(w1);
   if (w2) list.append(w2);
   if (w3) list.append(w3);
   if (w4) list.append(w4);
   if (w5) list.append(w5);
   if (w6) list.append(w6);
   addLeftWidgets(list);
}

void
QDToolBar::addRightWidget(QWidget * widget)
{
   if (!right_list.contains(widget))
   {
      widget->resize(widget->minimumSize());
      widget->show();
      right_list.append(widget);
      connect(widget, SIGNAL(destroyed(void)), this, SLOT(slotWidgetDestroyed(void)));
   }
}

void
QDToolBar::deleteWidget(QWidget * w)
{
   GPosition pos;
   for(pos=left_list;pos;)
      if (left_list[pos]==w)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 left_list.del(this_pos);
      } else ++pos;
   for(pos=right_list;pos;)
      if (right_list[pos]==w)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 right_list.del(this_pos);
      } else ++pos;
}

void
QDToolBar::slotWidgetDestroyed(void)
{
   const QObject * obj=sender();
   deleteWidget((QWidget *) obj);
}

QDToolBar::QDToolBar(QWidget * parent, const char * name) :
      QFrame(parent, name)
{
   being_destroyed=false;
   resize(0, 0);
   setFrameStyle(QFrame::Panel | QFrame::Raised);
}

void 
QDToolBar::setOptions(int opts)
{
  for(GPosition pos=pieces;pos;++pos)
    pieces[pos]->setOptions(opts);
  adjustPositions();
}

void 
QDTBarPiece::setOptions(int opts)
{
}

#include "qd_toolbar_moc.inc"
