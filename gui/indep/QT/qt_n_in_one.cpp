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

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef QT1
#include <qgmanager.h>
#else
#include <qabstractlayout.h>
#endif

#include "qt_n_in_one.h"
#include "qlib.h"
#ifndef QT1
#include <q1xcompatibility.h>
#endif

#ifdef QT1
void QeNInOne::recomputeMinMax(void)
{
   int min_w=0, min_h=0;
   int max_w=QGManager::unlimited, max_h=QGManager::unlimited;
   const QObjectList * objectList=children();
   if (objectList)
   {
      QObjectListIt it(*objectList);
      QObject * obj;
      while((obj=it.current()))
      {
	 ++it;
	 if (obj->isWidgetType())
	 {
	    QWidget * w=(QWidget *) obj;
	    QSize min=w->minimumSize();
	    QSize max=w->maximumSize();
	    if (min_w<min.width()) min_w=min.width();
	    if (min_h<min.height()) min_h=min.height();
	    if (max_w>max.width()) max_w=max.width();
	    if (max_h>max.height()) max_h=max.height();
	 }
      }
   }
   QSize min=minimumSize();
   QSize max=maximumSize();
   int done=0;
   if (min_w!=min.width()) { setMinimumWidth(min_w); done=1; }
   if (max_w!=max.width()) { setMaximumWidth(max_w); done=1; }
   if (min_h!=min.height()) { setMinimumHeight(min_h); done=1; }
   if (max_h!=max.height()) { setMaximumHeight(max_h); done=1; }
   if (done) ActivateLayouts(this);
}
#else
QSize QeNInOne::sizeHint(void) const
{
  int sw = -1;
  int sh = -1;
  QWidget *w = checkWidget(activeWidget);
  if (resizable && w)
    return w->sizeHint();
  const QObjectList * objectList=children();
  if (objectList)
    {
      QObjectListIt it(*objectList);
      QObject * obj;
      while((obj=it.current()))
        {
          ++it;
          if (obj->isWidgetType())
            {
              QWidget * w=(QWidget *) obj;
              QSize sz = w->sizeHint();
              if (sw < sz.width()) sw = sz.width();
              if (sh < sz.height()) sh = sz.height();
            }
        }
    }
  return QSize(sw, sh);
}
QSize QeNInOne::minimumSizeHint(void) const
{
  int sw = -1;
  int sh = -1;
  QWidget *w = checkWidget(activeWidget);
  if (resizable && w)
    return w->minimumSizeHint();
  const QObjectList * objectList=children();
  if (objectList)
    {
      QObjectListIt it(*objectList);
      QObject * obj;
      while((obj=it.current()))
        {
          ++it;
          if (obj->isWidgetType())
            {
              QWidget * w=(QWidget *) obj;
              QSize sz = w->minimumSizeHint();
              if (sw < sz.width()) sw = sz.width();
              if (sh < sz.height()) sh = sz.height();
            }
        }
    }
  return QSize(sw, sh);
}
#endif


void QeNInOne::checkActiveWidget(void)
{
  QWidget * w=checkWidget(activeWidget);
  if (w!=activeWidget) setActiveWidget(w);
}

QWidget	* QeNInOne::checkWidget(QWidget * w) const
  // Check and return a legal widget, maybe different from w. If w==0 and
  // there is at least one child - it will be returned
{
  QWidget * last_w=0;
  const QObjectList * objectList=children();
  if (objectList)
     {
       QObjectListIt it(*objectList);
       QObject * obj;
       while((obj=it.current()))
         {
           ++it;
           if (obj->isWidgetType())
             {
               last_w=(QWidget *) obj;
               if (last_w==w) return w;
             }
         }
     }
  return last_w;
}

void QeNInOne::setActiveWidget(QWidget * new_w)
{
   if (new_w==activeWidget) return;

   const QObjectList * objectList=children();
   if (objectList)
   {
      QObjectListIt it(*objectList);
      QObject * obj;
      while((obj=it.current()))
      {
	 ++it;
	 if (obj->isWidgetType())
	 {
	    QWidget * w=(QWidget *) obj;
	    if (w!=new_w) w->hide();
	 }
      }
   }
   activeWidget=new_w;
   if (activeWidget)
   {
      activeWidget->resize(size());
      activeWidget->show();

      if (resizable && isVisible())
      {
	    // Otherwise min/max size is set when a child is added/removed
#ifdef QT1
	 setMinimumSize(activeWidget->minimumSize());
	 setMaximumSize(activeWidget->maximumSize());
	 ActivateLayouts(this);
#else
         updateGeometry();
#endif
      }
   }
}

void QeNInOne::resizeEvent(QResizeEvent * ev)
{
  checkActiveWidget();
  if (activeWidget && ev->size().width() && ev->size().height())
    activeWidget->resize(ev->size());
}

bool QeNInOne::event(QEvent * ev)
{
   try
   {
      if (ev->type()==Event_ChildRemoved) 
      {
         checkActiveWidget();
#ifndef QT1
         updateGeometry();
#endif
      }
      else if (ev->type()==Event_ChildInserted)
      {
#ifdef QT1
	 QWidget * w_ins=((QChildEvent *) ev)->child();
#else
	 QWidget * w_ins=(QWidget *)((QChildEvent *) ev)->child();
#endif
	 w_ins->move(0, 0);
	 checkActiveWidget();
#ifndef QT1
         updateGeometry();
#endif
      } else if (ev->type()==Event_LayoutHint)
      {
	    // Looks like min/max dimensions of a child changed...
	 if (resizable && isVisible())
	 {
	    checkActiveWidget();
#ifdef QT1
	    setMinimumSize(activeWidget->minimumSize());
	    setMaximumSize(activeWidget->maximumSize());
#endif
	 }
#ifndef QT1
         updateGeometry();
#endif
      }
		    
      if (!resizable)
      {
#ifdef QT1
	    // If not resizable, it's necessary to recompute min/max
	    // every time when a child is added.
	    // Otherwise it's done every time a new active widget is chosen
	    // or when LayoutHint event is received.
	 recomputeMinMax();
#endif
      }
   } catch(const GException & exc)
   {
      showError(this, "Error", exc);
   }
   return QWidget::event(ev);
}

void QeNInOne::hide(void)
{
   if (resizable)
   {
#ifdef QT1
      setFixedSize(QSize(1, 1));
      ActivateLayouts(this);
#endif
   }

   QWidget::hide();
}

void QeNInOne::show(void)
{
   if (resizable)
   {
      checkActiveWidget();
      if (activeWidget)
      {
#ifdef QT1
	 setMinimumSize(activeWidget->minimumSize());
	 setMaximumSize(activeWidget->maximumSize());
#endif
      }
   }

   QWidget::show();
}
