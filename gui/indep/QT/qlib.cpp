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
   QHBoxLayout * hlay=new QHBoxLayout(vlay);

      // Creating icon and text
   QeLabel * icon=new QeLabel(this, "exc_icon");
#if defined(QT1) || defined(QT2)
   QPixmap iconpm = QMessageBox::standardIcon(QMessageBox::Critical,
					      QApplication::style() );
#else
   QPixmap iconpm = QMessageBox::standardIcon(QMessageBox::Critical);
#endif
   icon->setPixmap(iconpm);
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
   hlay=new QHBoxLayout(vlay);
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
   
   QHBoxLayout * hlay=new QHBoxLayout(vlay);
   hlay->addStretch(1);
   QePushButton * butt=new QePushButton(QePushButton::tr("&Close"), start, "close_butt");
   butt->setDefault(TRUE);
   hlay->addWidget(butt);

   QWidget::connect(butt, SIGNAL(clicked(void)), dialog, SLOT(accept(void)));
   
   dialog->exec();
   
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
