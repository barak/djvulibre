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

#define INCLUDE_MENUITEM_DEF

#include "qlib.h"
#include "qt_n_in_one.h"
#include "debug.h"
#include "exc_msg.h"
#include "exc_misc.h"
#include "exc_res.h"
#include "int_types.h"
#include "qt_painter.h"
#include "GString.h"
#include "DataPool.h"

#include <qmessagebox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qpixmap.h>
#include <qbitmap.h>
#include <qimage.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qmenubar.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "DjVuMessage.h"

#include "qt_fix.h"

//****************************************************************************
//******************************** QeExcMessage ******************************
//****************************************************************************

static GUTF8String 
getExcMsg(const char *exc_cause)
{
   GUTF8String exc_tag;
   const char *exc_sep=0;
   int exc_tag_len=0;
   
   if ( exc_cause )
   {
      exc_sep=strchr(exc_cause, '\n');
      if ( exc_sep )
      {
	 exc_tag_len=exc_sep-exc_cause;
	 int i;
	 for (i=exc_tag_len; i>=0 && (isspace(exc_cause[i]) || exc_cause[i]==':'); --i)
	    ;
	 if ( i>0 ) exc_tag_len=i+1;
	 exc_tag=GUTF8String(exc_cause, exc_tag_len);
      }
      else
	 exc_tag=exc_cause;
   } else
   {
      exc_tag="DjVuMessage.Unrecognized";
   }

   GNativeString exc_msg=DjVuMessage::LookUpUTF8(exc_tag);

   if ( exc_sep )
      exc_msg += GNativeString(exc_cause+exc_tag_len);

   return exc_msg;
}
   

void
QeExcMessage::switchDetails(void)
{
   try
   {
      if (details->isVisible())
      {
	 details_butt->setText(tr("&Details"));
	 details->hide();
      } else
      {
	 details_butt->setText(tr("Hide &Details"));
	 details->show();
      }
      details_butt->setMinimumWidth(details_butt->sizeHint().width());
      ActivateLayouts(details_butt);
   } catch(const GException & exc)
   {
      warning(QStringFromGString(getExcMsg(exc.get_cause())));   
   }
}

QeExcMessage::QeExcMessage(const GException & exc, const char * title,
			   QWidget * parent, const char * pname, bool modal) :
      QeDialog(parent, pname, modal)
{
   if (title) setCaption(title);
   
   QVBoxLayout * vlay=new QVBoxLayout(this, 10, 5);
   QHBoxLayout * hlay=new QHBoxLayout(5);
   vlay->addLayout(hlay);

      // Creating icon and text
   QeLabel * icon=new QeLabel(this, "exc_icon");
   icon->setPixmap(QMessageBox::standardIcon(QMessageBox::Critical,
					     QApplication::style()));
   hlay->addWidget(icon);
      
   QeLabel * text=new QeLabel(QStringFromGString(getExcMsg(exc.get_cause())),
						 this, "exc_text");
   hlay->addWidget(text);

      // Creating "details" of the exception
   details=new QeNInOne(this);
   details->dontResize(FALSE);
   vlay->addWidget(details);
   QWidget * w=new QWidget(details);
   QGridLayout * glay=new QGridLayout(w, 4, 2, 0, 10);
   QFrame * sep=new QFrame(w, "separator");
   sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   sep->setMinimumHeight(sep->sizeHint().height());
   sep->setMaximumHeight(sep->sizeHint().height());
   glay->addMultiCellWidget(sep, 0, 0, 0, 1);
   QeLabel * name, * value;
   QFont font;
   name=new QeLabel(tr("Function name"), w, "func_name");
   glay->addWidget(name, 1, 0);
   QString func_name=exc.get_function();
   if (!func_name) func_name="unknown";
   value=new QeLabel(func_name, w, "func_value");
   value->setFont(QFont("courier", value->font().pointSize()));
   value->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
   glay->addWidget(value, 1, 1);
   name=new QeLabel(tr("File name"), w, "file_name");
   glay->addWidget(name, 2, 0);
   QString file_name=exc.get_file();
   if (!file_name) file_name="unknown";
   value=new QeLabel(file_name, w, "file_value");
   value->setFont(QFont("courier", value->font().pointSize()));
   value->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
   glay->addWidget(value, 2, 1);
   name=new QeLabel(tr("Line number"), w, "line_name");
   glay->addWidget(name, 3, 0);
   char buffer[128];
   sprintf(buffer, "%d", exc.get_line());
   value=new QeLabel(buffer, w, "line_value");
   value->setFont(QFont("courier", value->font().pointSize()));
   value->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
   glay->addWidget(value, 3, 1);
   glay->activate();
   details->hide();
   
      // Creating buttons
   hlay=new QHBoxLayout(5);
   vlay->addLayout(hlay);
   hlay->addStretch(1);
   QePushButton * ok_butt=new QePushButton(tr("&OK"), this, "ok_butt");
   ok_butt->setDefault(TRUE);
   hlay->addWidget(ok_butt);
   details_butt=new QePushButton(tr("&Details"), this, "details_butt");
   hlay->addWidget(details_butt);

   vlay->activate();

   connect(ok_butt, SIGNAL(clicked()), this, SLOT(reject()));
   connect(details_butt, SIGNAL(clicked()), this, SLOT(switchDetails()));
}

//****************************************************************************
//********************************* GeoLink **********************************
//****************************************************************************

void
GeoLink::addSlave(QWidget * widget, int link_flags)
{
   if (!widget)
      throw ERROR_MESSAGE("GeoLink::addSlave",
			  "ZERO slave widget passed as input");

   GPosition pos;
   for(pos=slaves;pos;++pos)
      if (slaves[pos]->widget==widget)
      {
	 slaves[pos]->link_flags=link_flags;
	 break;
      }
   if (!pos)
      slaves.append(new Slave(widget, link_flags));
}

void
GeoLink::delSlave(QWidget * widget)
{
   for(GPosition pos=slaves;pos;++pos)
      if (slaves[pos]->widget==widget)
      {
	 slaves.del(pos);
	 break;
      }
}

bool
GeoLink::eventFilter(QObject * obj, QEvent * ev)
{
   if (obj==master && ev->type()==QEvent::Resize)
      for(GPosition pos=slaves;pos;++pos)
      {
	 GP<Slave> slave=slaves[pos];
	 if (slave->link_flags & (LINK_X | LINK_Y | LINK_WIDTH | LINK_HEIGHT))
	 {
	    QRect geo=slave->widget->frameGeometry();
	    if (slave->link_flags & LINK_X)
	       geo.setX(master->x());
	    if (slave->link_flags & LINK_Y)
	       geo.setY(master->y());
	    if (slave->link_flags & LINK_WIDTH)
	       geo.setWidth(master->width());
	    if (slave->link_flags & LINK_HEIGHT)
	       geo.setHeight(master->height());
	    slave->widget->setGeometry(geo);
	 }
	 if (slave->link_flags & (LINK_MIN_WIDTH | LINK_MIN_HEIGHT))
	 {
	    QSize size=slave->widget->minimumSize();
	    if (slave->link_flags & LINK_MIN_WIDTH)
	       size.setWidth(master->width());
	    if (slave->link_flags & LINK_MIN_HEIGHT)
	       size.setHeight(master->height());
	    slave->widget->setMinimumSize(size);
	 }
      }
   return FALSE;
}

GeoLink::GeoLink(QWidget * _master, QObject * parent, const char * name) :
      QObject(parent, name)
{
   if (!_master)
      throw ERROR_MESSAGE("GeoLink::GeoLink",
			  "ZERO master widget passed as input");
   master=_master;
   master->installEventFilter(this);
}

GeoLink::Slave::Slave(QWidget * _widget, int _link_flags)
{
   if (!_widget)
      throw ERROR_MESSAGE("GeoLink::Slave::Slave",
			  "ZERO slave widget passed as input");
   widget=_widget;
   link_flags=_link_flags;
}

//****************************************************************************
//********************************* QeRowColumn ******************************
//****************************************************************************

// This really ugly class will be used to get access to QLabel::drawContents()
// which is protected. We will receive a pointer to a pre-constructed QLabel
// Then we will cast it to LabelFooler and will call draw() function
// Everything should work. In theory.
class LabelFooler : public QLabel
{
public:
#ifndef QT1
   LabelFooler(QWidget *parent,const  char *name=0, WFlags f=0 ) : QLabel(parent,name,f) {}
#endif
   void	draw(QPainter * p)
   {
      QLabel::drawContents(p);
   }
};

void
QeRowColumn::layout(void)
{
   DEBUG_MSG("QeRowColumn::layout(): relayouting...\n");
   DEBUG_MAKE_INDENT(3);

   if (label.size()==0)
   {
      DEBUG_MSG("No children => set columns=rows=0\n");
      setNumCols(0);
      setNumRows(0);
   } else
   {
      int i;
      int max_width=0, max_height=0;
      for(i=0;i<label.size();i++)
      {
	 QLabel * l=(QLabel *) label[i];
	 QSize hint=l->sizeHint();
	 if (max_width<hint.width()) max_width=hint.width();
	 if (max_height<hint.height()) max_height=hint.height();
      }
      max_width+=hor_interval/2;
      max_height+=ver_interval/2;
      int columns=width()/max_width;
      if (columns==0) columns=1;
      int rows=(label.size()-1)/columns+1;
   
      DEBUG_MSG("cell_width=" << max_width << "\n");
      DEBUG_MSG("cell_height=" << max_height << "\n");
      DEBUG_MSG("columns=" << columns << "\n");
      DEBUG_MSG("rows=" << rows << "\n");

      setAutoUpdate(FALSE);
      setCellWidth(max_width);
      setCellHeight(max_height);
      setNumCols(columns);
      setAutoUpdate(TRUE);
      setNumRows(rows);

      for(i=0;i<label.size();i++)
	 ((QLabel *) label[i])->resize(max_width, max_height);

         // Take into account "min visible rows/columns" hints
      QRect vrect=viewRect();
      QRect frect=frameRect();
      int dw=frect.width()-vrect.width();
      int dh=frect.height()-vrect.height();
      if (dw<0) dw=0;
      if (dh<0) dh=0;
   
      if (min_visible_rows) setMinimumHeight(min_visible_rows*cellHeight()+dh);
      if (min_visible_cols) setMinimumWidth(min_visible_cols*cellWidth()+dw);
   }
   do_layout=false;
}

bool
QeRowColumn::event(QEvent * ev)
{
   try
   {
      if (ev->type()==QEvent::ChildRemoved)
      {
	 DEBUG_MSG("QeRowColumn::event(): Got one child removed...\n");
	 QWidget * w=(QWidget *)((QChildEvent *) ev)->child();
	 for(int i=0;i<label.size();i++)
	    if (label[i]==w)
	    {
	       for(int j=i;j<label.size()-1;j++)
		  label[j]=label[j+1];
	       label.resize(label.size()-2);
	       break;
	    }
	 if (!isVisible()) do_layout=true;
	 else
	 {
	    layout();
	    repaint(TRUE);
	 }
      } else if (ev->type()==QEvent::ChildInserted)
      {
	 DEBUG_MSG("QeRowColumn::event(): Got one child inserted...\n");
	 QWidget * w=(QWidget *)((QChildEvent *) ev)->child();
	 if (w->inherits("QLabel"))
	 {
	    w->hide();
	    label.resize(label.size());
	    label[label.size()-1]=w;
	    if (!isVisible()) do_layout=true;
	    else
	    {
	       layout();
	       repaint(TRUE);
	    }
	 }
      }
   } catch(const GException & exc)
   {
      showError(this, tr("Error"), exc);
   }
   return QTableView::event(ev);
}

void
QeRowColumn::resizeEvent(QResizeEvent * ev)
{
   DEBUG_MSG("QeRowColumn::resizeEvent(): widget resized\n");
   DEBUG_MAKE_INDENT(3);

   QTableView::resizeEvent(ev);
   layout();
}

void
QeRowColumn::paintCell(QPainter * p, int row, int col)
{
   int label_num=row*numCols()+col;
   if (label_num<label.size())
   {
      QLabel * l=(QLabel *) label[label_num];
      l->setAlignment(alignment);
      ((LabelFooler *) l)->draw(p);
   }
}

void
QeRowColumn::show(void)
{
   if (do_layout) layout();

   QTableView::show();
}

QeRowColumn::QeRowColumn(QWidget * parent, const char * name) :
      QTableView(parent, name), do_layout(true),
      hor_interval(5), ver_interval(5),
      min_visible_rows(0), min_visible_cols(0),
      alignment(Qt::AlignCenter)
{
   setTableFlags(Tbl_autoVScrollBar | Tbl_clipCellPainting);
   setFrameStyle(Panel | Sunken);
   setLineWidth(2);
}

//****************************************************************************
//********************************* show...() ********************************
//****************************************************************************

void
showError(QWidget * parent, const GException & exc)
{
#ifdef NO_DEBUG
   if (!exc.cmp_cause(DataPool::Stop)) return;
#endif
   QeExcMessage * msg=new QeExcMessage(exc, QeExcMessage::tr("Error"), parent, "exc_message");
   msg->exec();
   delete msg;
}

void
showError(QWidget * parent, const QString &qtitle, const GException & exc)
{
   const char *title=qtitle;
#ifdef NO_DEBUG
   if (!exc.cmp_cause(DataPool::Stop)) return;
#endif
   if (!title) title=QeExcMessage::tr("Error");
   QeExcMessage * msg=new QeExcMessage(exc, title, parent, "exc_message");
   msg->exec();
   delete msg;
}
   
void
showError(
  QWidget * parent, const QString &qtitle, const QString &qmessage)
{
#ifdef NO_DEBUG
   const char *message=qmessage;
   if (!GException::cmp_cause(message, DataPool::Stop)) return;
#endif
   QMessageBox::critical(parent, qtitle, qmessage);
}

void
showWarning(
  QWidget * parent, const QString &qtitle, const QString &qmessage)
{
#ifdef NO_DEBUG
   const char *message=qmessage;
   if (!GException::cmp_cause(message, DataPool::Stop)) return;
#endif
   QMessageBox::warning(parent, qtitle, qmessage);
}

void
showInfo(QWidget * parent, const QString &qtitle, const QString &qmessage)
{
#ifdef NO_DEBUG
   const char *message=qmessage;
   if (!GException::cmp_cause(message, DataPool::Stop)) return;
#endif
   QMessageBox::information(parent, qtitle, qmessage);
}

void
showMessage(QWidget * parent, const QString &qtitle, const QString &qmessage,
	    bool draw_frame, bool use_fixed, bool word_wrap)
{
   const char *title=qtitle;
   const char *message=qmessage;
   QeDialog * dialog=new QeDialog(parent, "msg_dialog", TRUE);
   dialog->setCaption(title);
   QWidget * start=dialog->startWidget();
   QVBoxLayout * vlay=new QVBoxLayout(start, 20, 10, "vlay");
   QeLabel * label=new QeLabel(message, start, "msg_label");
   if (draw_frame)
   {
      label->setFrameStyle(QFrame::Box | QFrame::Sunken);
      label->setMargin(10);
   }
   if (use_fixed)
   {
      QFont font=label->font();
      font.setFamily("Courier");
      label->setFont(font);
   }
   if (word_wrap)
   {
      const char * ptr;
      for(ptr=message;*ptr;ptr++)
	 if (*ptr=='\n') break;
      {
        GUTF8String mesg=GUTF8String(message,ptr-message);
        label->setText(QStringFromGString(mesg));
      }
      label->setMinimumWidth(label->sizeHint().width());
      label->setText(message);
      label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter | Qt::WordBreak);
   }
   vlay->addWidget(label);
   
   QHBoxLayout * hlay=new QHBoxLayout(5, "hlay");
   vlay->addLayout(hlay);
   hlay->addStretch(1);
   QePushButton * butt=new QePushButton(QePushButton::tr("&Close"), start, "close_butt");
   butt->setDefault(TRUE);
   hlay->addWidget(butt);

   QWidget::connect(butt, SIGNAL(clicked(void)), dialog, SLOT(accept(void)));
   
   dialog->exec();
   
}

void
setEnabled(QWidget * widget, bool flag)
{
   const QObjectList * childList=widget->children();
   if (childList)
   {
      QObjectListIt it(*childList);
      QObject * obj;
      while((obj=it.current()))
      {
	 ++it;
	 if (obj->isWidgetType()) setEnabled((QWidget *) obj, flag);
      }
   }
   widget->setEnabled(flag);
}

void
setItemsEnabled(QMenuData * menu, bool flag)
{
      // Disable all items in this menu
   for(u_int i=0;i<menu->count();i++)
   {
      int id=menu->idAt(i);
      if (id>=0)		// There can also be separators
      {
	 menu->setItemEnabled(id, flag);
	 QMenuItem * item=menu->findItem(id);
	 if (item && item->popup())
	    setItemsEnabled(item->popup(), flag);
      }
   }
}

int
paneToID(QMenuData * menu, QMenuData * pane)
{
   for(u_int i=0;i<menu->count();i++)
   {
      int id=menu->idAt(i);
      if (id>=0)		// There can also be separators
      {
	 QMenuItem * item=menu->findItem(id);
	 if (item && item->popup())
	 {
	    if (item->popup()==pane) return id;
	    id=paneToID(item->popup(), pane);
	    if (id>=0) return id;
	 }
      }
   }
   return -1;
}

//***************************************************************************
//********************************* QeGrid **********************************
//***************************************************************************

void QeGrid::recalcMinMax(void)
{
   DEBUG2_MSG("QeGrid::recalcMinMax(): finding new min/max values\n");
   DEBUG_MAKE_INDENT(3);

   int max_width=0;
   int min_width=0;
   int i;
   for(i=0;i<rows;i++)
   {
      for(int j=0;j<cols;j++)
      {
	 QWidget * w=widget[i][j];
	 if (w)
	 {
	    int max_w=w->maximumSize().width();
	    if (max_w>max_width) max_width=max_w;

	    int min_w=w->minimumSize().width();
	    if (min_w>min_width) min_width=min_w;
	 }
      }
   }

   min_width*=cols; max_width*=cols;

   int max_height=0;
   int min_height=0;
   for(i=0;i<rows;i++)
   {
      for(int j=0;j<cols;j++)
      {
	 QWidget * w=widget[i][j];
	 if (w)
	 {
	    int max_h=w->maximumSize().height();
	    if (max_h>max_height) max_height=max_h;

	    int min_h=w->minimumSize().height();
	    if (min_h>min_height) min_height=min_h;
	 }
      }
   }

   min_height*=rows; max_height*=rows;

   max_width+=2*border+(cols-1)*autoBorder;
   if (max_width>QCOORD_MAX) max_width=QCOORD_MAX;
   max_height+=2*border+(rows-1)*autoBorder;
   if (max_height>QCOORD_MAX) max_height=QCOORD_MAX;

   DEBUG2_MSG("setting min_w=" << min_width << ", min_h=" << min_height << "\n");
   ignore_resize=1;
   QSize old_size=size(); 
   setMinimumSize(min_width+2*border+(cols-1)*autoBorder,
		  min_height+2*border+(rows-1)*autoBorder);
   setMaximumSize(max_width, max_height);
   ActivateLayouts(this);
   ignore_resize=0;
   QEvent * e=new QResizeEvent(size(), old_size);
   QApplication::postEvent(this, e);
   DEBUG2_MSG("recalcMinMax() finished\n");
}

void
QeGrid::resizeEvent(QResizeEvent * ev)
{
   DEBUG2_MSG("QeGrid::resizeEvent(): got Resize event:" << width() << "x" << height() << "\n");
   DEBUG_MAKE_INDENT(3);
   
   if (ignore_resize) return;

   try
   {
      int w=width();
      int h=height();

      int cw=(w-2*border)/cols;
      int ch=(h-2*border)/rows;
      for(int i=0;i<rows;i++)
	 for(int j=0;j<cols;j++)
	 {
	    QWidget * wid=widget[i][j];
	    if (wid)
	    {
	       int cx=border+(w-2*border)*j/cols;
	       int cy=border+(h-2*border)*i/rows;

	       int min_w=wid->minimumSize().width();
	       int min_h=wid->minimumSize().height();
	       int max_w=wid->maximumSize().width();
	       int max_h=wid->maximumSize().height();
	    
	       int wid_x=cx; if (j) wid_x+=autoBorder/2;
	       int wid_y=cy; if (i) wid_y+=autoBorder/2;
	       int wid_w=cw; if (j>0) wid_w-=autoBorder/2;
	       if (j<cols-1) wid_w-=autoBorder/2;
	       int wid_h=ch; if (i>0) wid_h-=autoBorder/2;
	       if (i<rows-1) wid_h-=autoBorder/2;

	       if (wid_w<min_w || wid_h<min_h)
	       {
		  DEBUG2_MSG("QeGrid::resizeEvent(): Not enough space: wid_w=" << wid_w <<
			     ", min_w=" << min_w << ", wid_h=" << wid_h <<
			     ", min_h=" << min_h << "\n");
		  continue;
	       }

	       int align=alignment[i][j];
	       if (wid_w>=max_w)
	       {
		  if (align & AlignHCenter) wid_x+=(wid_w-max_w)/2;
		  else if (align & AlignRight) wid_x+=wid_w-max_w;
		  wid_w=max_w;
	       }
	       if (wid_h>=max_h)
	       {
		  if (align & AlignVCenter) wid_y+=(wid_h-max_h)/2;
		  else if (align & AlignBottom) wid_y+=wid_h-max_h;
		  wid_h=max_h;
	       }
	       DEBUG2_MSG("resizing " << wid_x << " to " << wid_w << "x" << wid_h <<
			  ", min=" << min_w << "x" << min_h <<
			  ", max=" << max_w << "x" << max_h << "\n");
	       wid->resize(wid_w, wid_h);
	       wid->move(wid_x, wid_y);
	    }
	 }
   } catch(const GException & exc)
   {
      showError(this, tr("Error"), exc);
   }
}

bool
QeGrid::event(QEvent * ev)
{
   try
   {
      if (ev->type()==QEvent::LayoutHint)
      {
	 DEBUG2_MSG("QeGrid::event(): one of the child changed its min/max dims\n");
	 recalcMinMax();
      }
   } catch(const GException & exc)
   {
      showError(this, tr("Error"), exc);
   }
   return QWidget::event(ev);
}

void
QeGrid::addWidget(QWidget * w, int row, int col, int align)
{
   DEBUG2_MSG("QeGrid::addWidget(): adding new widget\n");
   if (row<0 || row>=rows)
      throw BAD_ARGUMENTS("QeGrid::addWidget", "Invalid row number.");
   if (col<0 || col>=cols)
      throw BAD_ARGUMENTS("QeGrid::addWidget", "Invalid column number.");

   if (widget[row][col])
      throw ERROR_MESSAGE("QeGrid::addWidget", "Cell is already occupied.");

   widget[row][col]=w;
   alignment[row][col]=align;

   recalcMinMax();
}

QeGrid::QeGrid(int _rows, int _cols, QWidget * parent, const char * name) :
      QWidget(parent, name), border(10), autoBorder(10),
      rows(_rows), cols(_cols), ignore_resize(0), widget(0), alignment(0)
{
   if (rows<=0 || cols<=0)
      throw BAD_ARGUMENTS("QeGrid::QeGrid", "Invalid grid dimensions.");

   try
   {
      if (!(widget=new QWidget**[rows]) ||
	  !(alignment=new int*[rows]))
	 throw OUT_OF_MEMORY("QeGrid::QeGrid", "Not enough memory to initialize.");
      for(int i=0;i<rows;i++)
      {
	 if (!(widget[i]=new QWidget*[cols]) ||
	     !(alignment[i]=new int[cols]))
	    throw OUT_OF_MEMORY("QeGrid::QeGrid", "Not enough memory to initialize.");
	 memset(widget[i], 0, sizeof(QWidget*)*cols);
      }
   } catch(...)
   {
      if (widget)
      {
	 for(int i=0;i<rows;i++) delete widget[i];
	 delete widget;
      }
      if (alignment)
      {
	 for(int i=0;i<rows;i++) delete alignment[i];
	 delete alignment;
      }
      throw;
   }
}

QeGrid::~QeGrid(void)
{
   if (widget)
   {
      for(int i=0;i<rows;i++) delete widget[i];
      delete widget;
   }
   if (alignment)
   {
      for(int i=0;i<rows;i++) delete alignment[i];
      delete alignment;
   }
}

QPixmap
createIcon(const GPixmap & gpix_in)
{
      // Make a copy (we're going to dither (=modify) it)
   GP<GPixmap> ggpix=GPixmap::create(gpix_in);
   GPixmap &gpix=*ggpix;
   
   int width=gpix.columns();
   int height=gpix.rows();

      // Create mask by hands (heuristic mask offered by QT is ugly)
   QImage qimg(width, height, 1, 2, QImage::LittleEndian);
   qimg.fill(0);
   GPixel trans_pix=gpix[0][0];
   for(int i=0;i<height;i++)
      for(int j=0;j<width;j++)
      {
	 const GPixel & pixel=gpix[i][j];
	 if (pixel!=trans_pix) qimg.setPixel(j, height-i-1, 1);
      }
   QBitmap qbmp(width, height);
   qbmp.convertFromImage(qimg, Qt::MonoOnly);

   QPixmap qpix(width, height);
   
   int depth=qpix.depth();
   if (depth<15) gpix.ordered_666_dither();
   else if (depth<24) gpix.ordered_32k_dither();

   QePainter p(&qpix);
   p.drawPixmap(GRect(0, 0, width, height), &gpix);
   p.end();
   qpix.setMask(qbmp);

   return qpix;
}
