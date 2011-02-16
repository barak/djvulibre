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

#define INCLUDE_MENUITEM_DEF

#include "qlib.h"
#include "debug.h"
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
#include <qwidgetlist.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "DjVuMessage.h"

#ifdef UNIX
#include "qxlib.h"
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif


//****************************************************************************
//********************************* QeApplication ****************************
//****************************************************************************


QeApplication *qeApp;

QeApplication::QeApplication(int &argc, char **argv) 
  : QApplication(argc, argv)
{
  gamma = 2.2;
  qeApp = this;
  connect(&kill_timer, SIGNAL(timeout(void)), this, SLOT(killTimeout(void)));
}

QeApplication::~QeApplication()
{
  if (qeApp == this)
    qeApp = 0;
}

void
QeApplication::killWidget(QWidget * widget)
{
  if (widget && !widgetsToKill.contains(widget))
    {
      widgetsToKill.append(widget);
      connect(widget, SIGNAL(destroyed(void)), this, SLOT(killDestroy(void)));
      if (! kill_timer.isActive()) 
        kill_timer.start(0, TRUE);
    }
}

void
QeApplication::killDestroy(void)
{
  const QObject *s = sender();
  if (s->isWidgetType())
    widgetsToKill.removeRef((QWidget*)s);
}


void
QeApplication::killTimeout(void)
{
  QList<QWidget> newWidgetsToKill;
  QList<QWidget> widgetsToClose;
  
  
  while(widgetsToKill.count())
    {
      QWidget * w = widgetsToKill.first();
      widgetsToKill.remove((uint)0);
      // The deletion might result in closing a modal dialog being exec'ed.
      // This would cause big trouble in the local loop.
      // Therefore we scan all the toplevel widgets and 
      // inspect all visible modal or popup objects.
      bool okayToKill = TRUE;
      QWidgetList *tops = QApplication::topLevelWidgets();
      QWidgetListIt it(*tops);
      QWidget *top;
      while(( top = it.current() ))
        {
          ++it;
          if (top->isVisible() && (top->isModal() || top->isPopup()))
            {
              QWidget *parent = top;
              while (parent && parent!=w)
                parent = parent->parentWidget();
              if (parent == w)
                {
                  okayToKill = FALSE;
                  widgetsToClose.append(top);
                }
            }
        }
      delete tops;
      if (okayToKill)
        w->close(TRUE);
      else
        newWidgetsToKill.append(w);
    }
  
  // Now proceed
  while (widgetsToClose.count())
    {
      QWidget * w = widgetsToClose.first();
      widgetsToClose.remove((uint)0);
      try { w->close(); } catch(...) { };
    }
  while (widgetsToKill.count())
    {
      QWidget * w = widgetsToKill.first();
      widgetsToKill.remove((uint)0);
      try { w->close(TRUE); } catch(...) { };
    }
  while (newWidgetsToKill.count())
    {
      QWidget * w = newWidgetsToKill.first();
      newWidgetsToKill.remove((uint)0);
      killWidget(w);
    }
}

#ifdef UNIX
bool
QeApplication::x11EventFilter(XEvent * ev)
{
  if (ev->type==KeyPress &&
      XKeycodeToKeysym(ev->xkey.display, ev->xkey.keycode, 0)==0)
    return TRUE;	// Avoid stupid QT warning due to unassigned keysym
  
  x11EventResult=0;
  emit gotX11Event(ev);
   return x11EventResult;
}
#endif

void
QeApplication::setWidgetGeometry(QWidget * widget)
{
#ifdef UNIX
  if (widget && geometry.length())
   {
      int x, y;
      int w, h;
      int m=XParseGeometry(geometry, &x, &y, (u_int *) &w, (u_int *) &h);
      QSize minSize=widget->minimumSize();
      QSize maxSize=widget->maximumSize();
      if ((m & XValue)==0) x=widget->geometry().x();
      if ((m & YValue)==0) y=widget->geometry().y();
      if ((m & WidthValue)==0) w=widget->width();
      if ((m & HeightValue)==0) h=widget->height();
      w=QMIN(w, maxSize.width());
      h=QMIN(h, maxSize.height());
      w=QMAX(w, minSize.width());
      h=QMAX(h, minSize.height());
      if ((m & XNegative)) x=desktop()->width()+x-w;
      if ((m & YNegative)) y=desktop()->height()+y-h;
      widget->setGeometry(x, y, w, h);
      // Next widget has same size but different location
      if ((m & WidthValue) && (m & HeightValue))
        geometry = QString("%1x%2").arg(w).arg(h);
      else
        geometry = QString();
   }
#endif
}



//****************************************************************************
//********************************* QeDialog  ********************************
//****************************************************************************


QeDialog::QeDialog(QWidget *parent, const char *name, bool modal, WFlags f)
  : QDialog(parent,name,modal,f)
{
  // This is the only reason to have class QeDialog.
  makeTransient(this,parent);
}

void
QeDialog::makeTransient(QWidget *w, QWidget *fw)
{
#ifdef UNIX
  x11MakeTransient(w, fw);
#endif
}


//****************************************************************************
//********************************* QeFileDialog  ****************************
//****************************************************************************


QString	QeFileDialog::lastSaveDir;
QString QeFileDialog::lastLoadDir;

void
QeFileDialog::done(int rc)
{
  // Check the existance of the file and ask for overwriting...
  if (rc==Rejected || mode()==Directory)
    {
      QFileDialog::done(rc);
      return;
    }

  QString fileName = selectedFile();
  QCString cFileName = fileName.local8Bit();
#ifdef UNIX
  struct stat st;
  if (::stat(cFileName, &st)>=0)
    {
      if (S_ISFIFO(st.st_mode) || S_ISCHR(st.st_mode) ||
	  S_ISBLK(st.st_mode) || S_ISSOCK(st.st_mode))
        {
          QMessageBox::critical(this, "DjVu",
                                tr("You should select a regular file,\n"
                                   "not a pipe, socket or device file."));
          return;
        }
      if (!forWriting)
        {
          int fd=::open(cFileName, O_RDONLY);
          if (fd<0)
            {
              QMessageBox::critical(this, "DjVu",
                                    tr("Cannot open file '%1' for reading.\n%2")
                                    .arg(fileName)
                                    .arg(strerror(errno)) );
              return;
            } 
          ::close(fd);
        } 
      else
        {
          if (QMessageBox::warning(this, "DjVu",
                                   tr("File '%1' already exists.\n"
                                      "Are you sure you want to overwrite it?")
                                   .arg(fileName),
                                   "&Yes", "&No", 0, 0, 1))
            return;
          int fd=::open(cFileName, O_WRONLY);
          if (fd<0)
            {
              QMessageBox::critical(this, "DjVu",
                                    tr("Cannot open file '%1' for writing.\n%2")
                                    .arg(fileName)
                                    .arg(strerror(errno)) );
              return;
            }
          ::close(fd);
        }
    } 
  else if (!forWriting)
    {
      QMessageBox::critical(this, "DjVu", 
                            tr("Failed to stat file '%1'")
                            .arg(fileName) );
      return;
    }
#endif
  // Update lastSaveDir and lastLoadDir
  if (rc==Accepted)
    {
      QFileInfo fi = QFileInfo(selectedFile());
      if (fi.isDir())
	{
	  if (forWriting)
	    lastSaveDir=fi.dirPath();
	  else
	    lastLoadDir=fi.dirPath();
	}
    }
  QFileDialog::done(rc);
}

void	
QeFileDialog::setForWriting(bool fwr)
{
  forWriting=fwr;
  setMode(fwr ? QFileDialog::AnyFile : QFileDialog::ExistingFile);
}


QeFileDialog::QeFileDialog(const QString & dirName, const char * filter,
			   QWidget * parent, const char * name, bool modal) 
  : QFileDialog(".", filter, parent, name, modal), forWriting(false)
{
  setForWriting(true);
  if (modal) 
    QeDialog::makeTransient(this, parent);
  if ( dirName.isNull() || dirName.isEmpty() || !QFileInfo(dirName).isDir())
    setDir(lastSaveDir);
  else 
    setDir(dirName);
}

QeFileDialog::QeFileDialog(QWidget * parent, const char * name, bool modal) 
  : QFileDialog(parent, name, modal), forWriting(false)
{
  setForWriting(true);
  if (modal) 
    QeDialog::makeTransient(this, parent);
  setDir(lastSaveDir);
}



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
	 int i = exc_tag_len; 
         while (i>=0 && (isspace(exc_cause[i]) || exc_cause[i]==':'))
           i -= 1;
	 if ( i>0 ) exc_tag_len=i+1;
	 exc_tag=GUTF8String(exc_cause, exc_tag_len);
      }
      else
	 exc_tag=exc_cause;
   } else
   {
      exc_tag="DjVuMessage.Unrecognized";
   }

   GUTF8String exc_msg=DjVuMessage::LookUpUTF8(exc_tag);
   
   if ( exc_sep )
      exc_msg += GUTF8String(exc_cause+exc_tag_len);

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
          layout()->activate();
          resize(1,1);
        } 
      else
        {
          details_butt->setText(tr("Hide &Details"));
          details->show();
        }
    }
  catch(const GException & exc)
    {
      warning(QStringFromGString(getExcMsg(exc.get_cause())));   
    }
}

QeExcMessage::QeExcMessage(const GException & exc, QString title,
			   QWidget * parent, const char * pname, bool modal) :
      QeDialog(parent, pname, modal)
{
   if (!title.isNull()) 
     setCaption(title);
   
   QVBoxLayout * vlay=new QVBoxLayout(this, 10, 5);
   QHBoxLayout * hlay=new QHBoxLayout(vlay);

      // Creating icon and text
   QLabel * icon=new QLabel(this, "exc_icon");
#if defined(QT1) || defined(QT2)
   QPixmap iconpm = QMessageBox::standardIcon(QMessageBox::Critical,
					      QApplication::style() );
#else
   QPixmap iconpm = QMessageBox::standardIcon(QMessageBox::Critical);
#endif
   icon->setPixmap(iconpm);
   hlay->addWidget(icon);
      
   QLabel * text=new QLabel(QStringFromGString(getExcMsg(exc.get_cause())),
						 this, "exc_text");
   hlay->addWidget(text);

      // Creating "details" of the exception
   details = new QWidget(this);
   vlay->addWidget(details);
   QGridLayout * glay=new QGridLayout(details, 4, 2, 0, 10);
   QFrame * sep=new QFrame(details, "separator");
   sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   glay->addMultiCellWidget(sep, 0, 0, 0, 1);
   QLabel * name, * value;
   QFont font;
   name=new QLabel(tr("Function name"), details, "func_name");
   glay->addWidget(name, 1, 0);
   QString func_name=exc.get_function();
   if (!func_name) func_name="unknown";
   value=new QLabel(func_name, details, "func_value");
   value->setFont(QFont("courier", value->font().pointSize()));
   value->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
   glay->addWidget(value, 1, 1);
   name=new QLabel(tr("File name"), details, "file_name");
   glay->addWidget(name, 2, 0);
   QString file_name=exc.get_file();
   if (!file_name) file_name="unknown";
   value=new QLabel(file_name, details, "file_value");
   value->setFont(QFont("courier", value->font().pointSize()));
   value->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
   glay->addWidget(value, 2, 1);
   name=new QLabel(tr("Line number"), details, "line_name");
   glay->addWidget(name, 3, 0);
   char buffer[128];
   sprintf(buffer, "%d", exc.get_line());
   value=new QLabel(buffer, details, "line_value");
   value->setFont(QFont("courier", value->font().pointSize()));
   value->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
   glay->addWidget(value, 3, 1);
   details->hide();
   
      // Creating buttons
   hlay=new QHBoxLayout(vlay);
   hlay->addStretch(1);
   QPushButton * ok_butt=new QPushButton(tr("&OK"), this, "ok_butt");
   ok_butt->setDefault(TRUE);
   hlay->addWidget(ok_butt);
   details_butt=new QPushButton(tr("&Details"), this, "details_butt");
   hlay->addWidget(details_butt);

   connect(ok_butt, SIGNAL(clicked()), this, SLOT(reject()));
   connect(details_butt, SIGNAL(clicked()), this, SLOT(switchDetails()));
}

//****************************************************************************
//********************************* show...() ********************************
//****************************************************************************

void
showError(QWidget * parent, const GException & exc)
{
#ifdef NDEBUG
   if (!exc.cmp_cause(DataPool::Stop)) return;
#endif
   QeExcMessage * msg=new QeExcMessage(exc, QeExcMessage::tr("Error"), 
                                       parent, "exc_message");
   msg->exec();
   delete msg;
}

void
showError(QWidget * parent, const QString &qtitle, const GException & exc)
{
   const char *title=qtitle;
#ifdef NDEBUG
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
#ifdef NDEBUG
   const char *message=qmessage;
   if (!GException::cmp_cause(message, DataPool::Stop)) return;
#endif
   QMessageBox::critical(parent, qtitle, qmessage);
}

void
showWarning(
  QWidget * parent, const QString &qtitle, const QString &qmessage)
{
#ifdef NDEBUG
   const char *message=qmessage;
   if (!GException::cmp_cause(message, DataPool::Stop)) return;
#endif
   QMessageBox::warning(parent, qtitle, qmessage);
}

void
showInfo(QWidget * parent, const QString &qtitle, const QString &qmessage)
{
#ifdef NDEBUG
   const char *message=qmessage;
   if (!GException::cmp_cause(message, DataPool::Stop)) return;
#endif
   QMessageBox::information(parent, qtitle, qmessage);
}

void
showMessage(QWidget * parent, const QString &qtitle, const QString &qmessage,
	    bool draw_frame, bool use_fixed, bool word_wrap)
{
   QeDialog * dialog=new QeDialog(parent, "msg_dialog", TRUE);
   dialog->setCaption(qtitle);
   QWidget * start=dialog->startWidget();
   QVBoxLayout * vlay=new QVBoxLayout(start, 20, 10, "vlay");
   QLabel * label=new QLabel(qmessage, start, "msg_label");
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
       int i;
       for (i=0; i<(int)qmessage.length(); i++)
         if (qmessage[i] == QChar('\n'))
           break;
       label->setText(qmessage.left(i));
       label->setMinimumWidth(label->sizeHint().width());
       label->setText(qmessage);
       label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter | Qt::WordBreak);
   }
   vlay->addWidget(label);
   
   QHBoxLayout * hlay=new QHBoxLayout(vlay);
   hlay->addStretch(1);
   QPushButton * butt=new QPushButton(QPushButton::tr("&Close"), start, "close_butt");
   butt->setDefault(TRUE);
   hlay->addWidget(butt);

   QWidget::connect(butt, SIGNAL(clicked(void)), dialog, SLOT(accept(void)));
   
   dialog->exec();
   
}



//****************************************************************************
//************************* QT helper functions ******************************
//****************************************************************************




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

bool
setComboBoxCurrentItem(QComboBox *combo, QString item)
{
  for(int i=0; i<combo->count(); i++)
    if (combo->text(i) == item)
      {
	combo->setCurrentItem(i);
	return TRUE;
      }
  return FALSE;
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

void
showOrHide(QWidget *w, bool b)
{
  if (w)
    {
      bool h = w->isHidden();
      if (b && h)
        w->show();
      else if (!b && !h)
        w->hide();
    }
}
