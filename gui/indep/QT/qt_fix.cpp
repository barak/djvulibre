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

#include "debug.h"
#include "exc_msg.h"

#include <qobject.h>
#include <qobjectlist.h>
#include <qlayout.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qwidgetlist.h>
#include <qpainter.h>
#include <qkeycode.h>
#include <qapplication.h>
#include <qlist.h>
#include <qtimer.h>
#include <qwidget.h>

#define QT_FIX_DO_ALL
#include "qt_fix.h"
#include "qlib.h"


#ifdef UNIX
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#endif

#ifdef UNIX
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

QeApplication * qeApp;

void
ActivateLayouts(QWidget * start)
      // This function is damn important. The point is that due to a bug in
      // QT v<=1.41 you have to MANUALLY reactivate all layouts up to the
      // top level one after you change min/max size of a child managed by
      // a layout. QT tries to do smth like this, but due to the bug it doesn't
      // work every the time.
{
#ifdef QT1
   DEBUG2_MSG("ActivateLayouts(): going up and up and reactivating every QLayout...\n");
   DEBUG_MAKE_INDENT(3);
   if (!start) return;
   
   QWidget * parent=start;
   while(!parent->isTopLevel())
   {
      parent=parent->parentWidget();
      
      const QObjectList * childList=parent->children();
      if (childList)
      {
	 QObjectListIt it(*childList);
	 QObject * obj;
	 while((obj=it.current()))
	 {
	    ++it;
	    if (obj->inherits("QLayout"))
	    {
	       QLayout * layout=(QLayout *) obj;
	       int frozen=1;
	       const QObjectList * childList=layout->children();
	       if (childList)
	       {
		  QObjectListIt it(*childList);
		  QObject * obj;
		  while((obj=it.current()))
		  {
		     ++it;
		     if (obj->inherits("QGManager"))
		     {
			frozen=0;
			break;
		     }
		  }
	       }
	       DEBUG2_MSG("parent " << parent->name() << " (" << parent->className() << ") "
			  "contains layout " << obj->name() << " (" << obj->className() << ")\n");
	       if (!frozen)
	       {
		  DEBUG2_MSG("ACTIVATING...\n");
		  ((QLayout *) obj)->activate();
	       } else DEBUG2_MSG("But it's FROZEN => do nothing to avoid SIGSEGV\n");
	    }
	 }
      }
   }
#endif
}

// QEAPPLICATION ----------------------------------------

void
QeApplication::killTimeout(void)
{
   DEBUG_MSG("QeApplication::killTimeout(): deleting widgets commited to death\n");
   DEBUG_MAKE_INDENT(3);

   QList<QWidget> newWidgetsToKill;
   QList<QWidget> activeModalWidgets;

   newWidgetsToKill.setAutoDelete(FALSE);
   activeModalWidgets.setAutoDelete(FALSE);

   while(widgetsToKill.count())
   {
      QWidget * w=widgetsToKill.first();
      DEBUG_MSG("have one candidate to kill: " << w << "\n");
	 
	 // Get the list of all the widgets (to find out if a widget to be deleted
	 // is still alive or not).
      QWidgetList * list=QApplication::allWidgets();
      
      try
      {
	 QWidgetListIt it(*list);	// iterate over the widgets
	 while(it.current())
	 {
	    if (it.current()==w)
	    {
	       DEBUG_MSG("and it's still alive\n");
	       DEBUG_MSG("checking for active modal dialogs\n");
	       QWidget * mw=activeModalWidget();
	       if (mw)
	       {
		  DEBUG_MSG("oops... found one.\n");
		  QWidget * parent=mw;
		  while(parent && parent!=w) parent=parent->parentWidget();
		  if (parent==w)
		  {
		     DEBUG_MSG("and it's child of the widget to be killed => close() it.\n");
		     if (!activeModalWidgets.contains(mw))
			activeModalWidgets.append(mw);
		  }
		  DEBUG_MSG("can't kill anything at this point => reschedule kill\n");
		  newWidgetsToKill.append(w);
	       } else
	       {
		  DEBUG_MSG("deleting widget " << w << "\n");
		  w->close(TRUE);
	       }
	       break;
	    }
	    ++it;
	 }
	 delete list; list=0;
      } catch(...)
      {
	 while(newWidgetsToKill.count())
	 {
	    killWidget(newWidgetsToKill.first());
	    newWidgetsToKill.remove((unsigned int) 0);
	 }
	 while(activeModalWidgets.count())
	 {
	    activeModalWidgets.first()->close();
	    activeModalWidgets.remove((unsigned int) 0);
	 }
	 delete list;
	 throw;
      }
      
      widgetsToKill.remove((unsigned int) 0);
   }
   
   while(newWidgetsToKill.count())
   {
      killWidget(newWidgetsToKill.first());
      newWidgetsToKill.remove((unsigned int) 0);
   }
   while(activeModalWidgets.count())
   {
      activeModalWidgets.first()->close();
      activeModalWidgets.remove((unsigned int) 0);
   }
}

void
QeApplication::construct(void)
{
   qeApp=this;
   connect(&kill_timer, SIGNAL(timeout(void)), this, SLOT(killTimeout(void)));
}

void
QeApplication::slotKillSender(void)
{
   const QObject * obj=sender();
   if (obj->inherits("QWidget"))
      killWidget((QWidget *) obj);
}

void
QeApplication::killWidget(QWidget * widget)
{
  if (widget)
    {
      DEBUG_MSG("QeApplication::killWidgets(): scheduling " << widget << " for kill\n");
      widgetsToKill.append(widget);
      if (!kill_timer.isActive()) kill_timer.start(0, TRUE);
    }
}

bool
QeApplication::x11EventFilter(XEvent * ev)
{
#ifdef UNIX
   if (ev->type==KeyPress &&
       XKeycodeToKeysym(ev->xkey.display, ev->xkey.keycode, 0)==0)
      return TRUE;	// Avoid stupid QT warning due to unassigned keysym
   
   x11EventResult=0;
   emit gotX11Event(ev);
   return x11EventResult;
#endif
}

void
QeApplication::setWidgetGeometry(QWidget * widget)
{
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
}

QeApplication::QeApplication(int & argc, char ** argv) 
  : QApplication(argc, argv)
{
  construct();
  gamma=2.2;
}

// QEFILEDIALOG ----------------------------------------

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

   QString fileName=selectedFile();
#ifdef UNIX
   struct stat st;
   if (::stat(fileName, &st)>=0)
   {
      if (S_ISFIFO(st.st_mode) || S_ISCHR(st.st_mode) ||
	  S_ISBLK(st.st_mode) || S_ISSOCK(st.st_mode))
      {
	 QMessageBox::critical(this, "DjVu",
			       "You should select a regular file,\n"
			       "not a pipe, socket or device file.");
	 return;
      }
      if (!forWriting)
      {
	 int fd=::open(fileName, O_RDONLY);
	 if (fd<0)
	 {
	    QMessageBox::critical(this, "DjVu",
				  "Failed to open file '"+fileName+
				  "' for reading:\n"+strerror(errno));
	    return;
	 } else ::close(fd);
      } else
      {
	 int fd=::open(fileName, O_WRONLY);
	 if (fd<0)
	 {
	    QMessageBox::critical(this, "DjVu",
				  "Failed to open file '"+fileName+
				  "' for writing:\n"+strerror(errno));
	    return;
	 } else ::close(fd);
	 
	 if (QMessageBox::warning(this, "DjVu",
				  "File '"+fileName+"' already exists.\n"
				  "Are you sure you want to overwrite it?",
				  "&Yes", "&No", 0, 0, 1))
	    return;
      }
   } else
      if (!forWriting)
      {
	 QMessageBox::critical(this, "DjVu", "Failed to stat file '"+fileName+"'");
	 return;
      }
#else
   fatal("QeFileDialog::done(): not ported to windows");
#endif

      // Update lastSaveDir and lastLoadDir
   if (rc==Accepted)
   {
      QFileInfo fi = QFileInfo(selectedFile());
      if (fi.isDir())
	 if (forWriting) lastSaveDir=fi.dirPath();
         else lastLoadDir=fi.dirPath();
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
			   QWidget * parent, const char * name, bool modal) :
      QFileDialog(".", filter, parent, name, modal), forWriting(false)
{
   setForWriting(true);
   if (modal) QeDialog::makeTransient(this, parent);
   if ( dirName.isNull() || dirName.isEmpty() || !QFileInfo(dirName).isDir())
      setDir(lastSaveDir);
   else setDir(dirName);
}

QeFileDialog::QeFileDialog(QWidget * parent, const char * name, bool modal) :
      QFileDialog(parent, name, modal), forWriting(false)
{
   setForWriting(true);
   if (modal) QeDialog::makeTransient(this, parent);
   setDir(lastSaveDir);
}


// QELABEL ----------------------------------------

void
QeLabel::setMinSize(void)
{
#ifdef QT1
  setMinimumSize(sizeHint());
  ActivateLayouts(this);
#endif
}

void
QeLabel::resizeEvent(QResizeEvent * ev)
{
#ifdef QT1
   if (isText() && (alignment() & WordBreak))
   {
	 // Get the minimum height basing on the width
      int m, fw;
      QRect br;
      {
	 QPainter p(this);
	 br=p.boundingRect(0, 0, ev->size().width(), 1000, alignment(), text());
	 int h=fontMetrics().lineSpacing();
	 br.setHeight(((br.height()+h-1)/h)*h-fontMetrics().leading());
	 m=2*margin();
	 fw=frameWidth();
	 if (m<0)
	 {
	    if (fw>0) m=p.fontMetrics().width("x");
	    else m = 0;
	 }
      }
      int h=br.height()+m+2*fw;
      setMinimumHeight(h);
      ActivateLayouts(this);
   }
#endif
}

QeLabel::QeLabel(QWidget * parent=0, const char * name=0, WFlags f=0) 
  : QLabel(parent, name, f) 
{
}

QeLabel::QeLabel(const QString & text, QWidget * parent=0, const char * name=0, WFlags f=0) 
  : QLabel(text, parent, name, f) 
{
}

QeLabel::QeLabel(QWidget * buddy, const QString & text, 
        QWidget * parent, const char * name=0, WFlags f=0) 
  : QLabel(buddy, text, parent, name, f) 
{
}

void
QeLabel::show(void)   
{
#ifdef QT1
  if (!isText() || !(alignment() & WordBreak)) setMinSize();
#endif
  QLabel::show();
}



// QEPUSHBUTTON ----------------------------------------

void
QePushButton::setMinSize(void)
{
#ifdef QT1
  setMinimumSize(sizeHint());
  ActivateLayouts(this);
#endif
}

QSize
QePushButton::sizeHint(void) const
{
  QSize size=QPushButton::sizeHint();
  return QSize(size.width()+2*infl_w, size.height()+2*infl_h);
}

void 
QePushButton::inflateWidth(int _infl_w) 
{ 
  infl_w=_infl_w; 
#ifndef QT1
  updateGeometry();
#endif
}

void 
QePushButton::inflateHeight(int _infl_h) 
{ 
  infl_h=_infl_h; 
#ifndef QT1
  updateGeometry();
#endif
}

QePushButton::QePushButton(QWidget * parent=0, const char * name=0) 
  : QPushButton(parent, name), infl_w(0), infl_h(0)
{ 
  setAutoResize(TRUE); 
  setMinSize(); 
}

QePushButton::QePushButton(const QString & text, QWidget * parent=0, const char * name=0) 
  : QPushButton(text, parent, name), infl_w(0), infl_h(0)
{ 
  setAutoResize(TRUE); 
  setMinSize(); 
}

void
QePushButton::show(void)
{
  setMinSize();
  QPushButton::show();
}

// QECHECKBOX ----------------------------------------

void 
QeCheckBox::setMinSize(void)
{
#ifdef QT1
  setMinimumSize(sizeHint());
  ActivateLayouts(this);
#endif
}

QeCheckBox::QeCheckBox(QWidget * parent=0, const char * name=0) 
  : QCheckBox(parent, name) 
{
  setMinSize(); 
}

QeCheckBox::QeCheckBox(const QString & text, QWidget * parent, const char * name=0) 
  : QCheckBox(text, parent, name) 
{ 
  setMinSize(); 
}

void 
QeCheckBox::show(void)
{
  setMinSize();
  QCheckBox::show();
}

// QERADIOBUTTON ----------------------------------------

void
QeRadioButton::setMinSize(void)
{
#ifdef QT1
  setMinimumSize(sizeHint());
  ActivateLayouts(this);
#endif
}

QeRadioButton::QeRadioButton(QWidget * parent=0, const char * name=0) 
  : QRadioButton(parent, name) 
{ 
  setMinSize(); 
}

QeRadioButton::QeRadioButton(const QString & text, QWidget * parent, const char * name=0) 
  : QRadioButton(text, parent, name) 
{ 
  setMinSize(); 
}

void
QeRadioButton::show(void)
{
  setMinSize();
  QRadioButton::show();
}


// QESLIDER ----------------------------------------

void
QeSlider::setMinMaxSize(void)
{
#ifdef QT1
  setMinimumSize(sizeHint());
  ActivateLayouts(this);
#endif
}

QeSlider::QeSlider(QWidget * parent=0, const char * name=0) 
  : QSlider(parent, name) 
{ 
  setMinMaxSize(); 
}

QeSlider::QeSlider(Orientation ori, QWidget * parent=0, const char * name=0) 
  : QSlider(ori, parent, name) 
{ 
  setMinMaxSize(); 
}

QeSlider::QeSlider(int min, int max, int step, int value,
                   Orientation ori, QWidget * parent=0, const char * name=0) 
  : QSlider(min, max, step, value, ori, parent, name) 
{ 
  setMinMaxSize(); 
}

void
QeSlider::show(void)
{
  setMinMaxSize();
  QSlider::show();
}


// QELINEEDIT ----------------------------------------

void	
QeLineEdit::setMinMaxSize(void)
{
#ifdef QT1
  setMinimumSize(sizeHint());
  setMaximumHeight(sizeHint().height());
  ActivateLayouts(this);
#endif
}

void
QeLineEdit::show(void)
{
  setMinMaxSize();
  QLineEdit::show();
}

QeLineEdit::QeLineEdit(QWidget * parent=0, const char * name=0) 
  : QLineEdit(parent, name) 
{
  setMinMaxSize(); 
}


// QESPINBOX ----------------------------------------

void 
QeSpinBox::setMinMaxSize(void)
{
#ifdef QT1
  setMinimumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());
  ActivateLayouts(this);
#endif
}

QeSpinBox::QeSpinBox(QWidget * parent=0, const char * name=0)
  : QSpinBox(parent, name) 
{ 
  setMinMaxSize(); 
}

QeSpinBox::QeSpinBox(int min, int max, int step=-1,
                     QWidget * parent=0, const char * name=0)
  : QSpinBox(min, max, step, parent, name) 
{ 
  setMinMaxSize(); 
}

void
QeSpinBox::show(void)
{
  setMinMaxSize();
  QSpinBox::show();
}

bool
QeSpinBox::eventFilter(QObject * obj, QEvent * ev)
{
#ifdef KeyPress
#undef KeyPress
#endif
   if ((obj==editor())&&(ev->type()==(QEvent::KeyPress)))
   {
      int key=((QKeyEvent *) ev)->key();
      if (key==Key_Return || key==Key_Enter)
      {
	 GUTF8String txt1=GStringFromQString(text());
	 interpretText();
	 GUTF8String txt2=GStringFromQString(text());

	    // Now, if the text has been "fixed", stop the "Enter"
	    // event to prevent the dialog from closing.
	 if (txt1!=txt2)
	    return TRUE;
      }
   }
   return QSpinBox::eventFilter(obj, ev);
}

// QEPROGRESSBAR ----------------------------------------

bool
QeProgressBar::setIndicator(QString & string, int progress, int totalSteps)
{
  bool rc = prefix_changed;
  if ( QProgressBar::setIndicator(string, progress, totalSteps) )
    rc = true;
  if (prefix.length()) 
    string = prefix + string;
  prefix_changed = false;
  return rc;
}

void
QeProgressBar::setPrefix(const QString & _prefix)
{
  prefix_changed = (prefix!=_prefix);
  prefix=_prefix;
}

QeProgressBar::QeProgressBar(QWidget * parent=0, const char * name=0) 
  : QProgressBar(parent, name), prefix_changed(false) 
{
}

QeProgressBar::QeProgressBar(int totalSteps, QWidget * parent=0, const char * name=0) 
  : QProgressBar(totalSteps, parent, name), prefix_changed(false) 
{
}

// QEGROUPBOX ----------------------------------------

void	
QeGroupBox::init(void)
{
#ifdef QT1
  QFont fnt=font();
  fnt.setBold(TRUE);
  setFont(fnt);
#endif
}

QeGroupBox::QeGroupBox(QWidget * parent=0, const char * name=0) 
  : QGroupBox(parent, name) 
{ 
  init(); 
}

QeGroupBox::QeGroupBox(const QString & title, QWidget * parent=0, const char * name=0) 
  : QGroupBox(title, parent, name) 
{ 
  init(); 
}


// QEBUTTONGROUP ----------------------------------------

void
QeButtonGroup::init(void)
{
#ifdef QT1
  QFont fnt=font();
  fnt.setBold(TRUE);
  setFont(fnt);
#endif
}

QeButtonGroup::QeButtonGroup(QWidget * parent=0, const char * name=0) 
  : QButtonGroup(parent, name) 
{ 
  init(); 
}

QeButtonGroup::QeButtonGroup(const QString & title, QWidget * parent=0, const char * name=0) 
  : QButtonGroup(title, parent, name) 
{
  init(); 
}


// QEMENUBAR ----------------------------------------

QeMenuBar::QeMenuBar(QWidget * parent=0, const char * name=0)
  : QMenuBar(parent, name)
{
  setSeparator(InWindowsStyle);
}



// QEDIALOG ----------------------------------------

bool
QeDialog::event(QEvent * ev)
  // Useful only when there is an aux_widget
{
#ifdef QT1
   if (aux_widget)
   {
      if (ev->type()==QEvent::Resize)
      {
	 // Resize the aux_widget to respond to manual (mouse) resize
	 aux_widget->resize(width(), height());
      } 
      else if (ev->type()==QEvent::LayoutHint)
      {
	 int min_width=aux_widget->minimumSize().width();
	 int min_height=aux_widget->minimumSize().height();
	 if (!wresizable && min_width) aux_widget->setMaximumWidth(min_width);
	 if (!hresizable && min_height) aux_widget->setMaximumHeight(min_height);
	 setMinimumSize(aux_widget->minimumSize());
	 setMaximumSize(aux_widget->maximumSize());
      }
   }
#endif
   return QDialog::event(ev);
}

void
QeDialog::resize(int w, int h)
{
#ifdef QT1
   if (aux_widget)
   {
	 // Take into account the aux_widget's min/max dimensions a little
	 // bit earlier than we learn about them from an QEvent::LayoutHint
      int minw=aux_widget->minimumSize().width();
      int minh=aux_widget->minimumSize().height();
      int maxw=aux_widget->maximumSize().width();
      int maxh=aux_widget->maximumSize().height();
      if(maxw<minw) maxw=32767;
      if(maxh<minh) maxh=32767;
      if (w<minw) w=minw;
      if (w>maxw) w=maxw;
      if (h<minh) h=minh;
      if (h>maxh) h=maxh;
   }
   if (!isVisible())
   {
      // After I played with min/max sizes, QT will not center the
      // dialog on the screen. Do it ourselves
      QWidget * wid=parent ? parent->topLevelWidget() : QApplication::desktop();
      move(wid->x()+(wid->width()-w)/2,
        wid->y()+(wid->height()-h)/2);
   }
#endif
   QDialog::resize(w, h);
}

void
QeDialog::setGeometry(int x, int y, int w, int h)
{
#ifdef QT1
   if (aux_widget)
   {
	 // Take into account the aux_widget's min/max dimensions a little
	 // bit earlier than we learn about them from an QEvent::LayoutHint
      int minw=aux_widget->minimumSize().width();
      int minh=aux_widget->minimumSize().height();
      int maxw=aux_widget->maximumSize().width();
      int maxh=aux_widget->maximumSize().height();
      if (w<minw) w=minw;
      if (w>maxw) w=maxw;
      if (h<minh) h=minh;
      if (h>maxh) h=maxh;
   }
   if (!isVisible())
     {
       // After I played with min/max sizes, QT will not center the
       // dialog on the screen. Do it ourselves
       QWidget * wid= (parent) ? parent->topLevelWidget() : QApplication::desktop();
       move(wid->x()+(wid->width()-w)/2,
            wid->y()+(wid->height()-h)/2);
   }
#endif
   QDialog::setGeometry(x, y, w, h);
}

bool
QeDialog::eventFilter(QObject * obj, QEvent * ev)
	 // Called when there is a valid parent
{
#ifdef QT1
   if (ev->type()==Event_LayoutHint)
   {
      int width, height;
      if (!wresizable && (width=minimumSize().width())) setMaximumWidth(width);
      if (!hresizable && (height=minimumSize().height())) setMaximumHeight(height);
   }
#endif
   return QDialog::eventFilter(obj, ev);
}

void
QeDialog::setResizable(bool wres, bool hres)
{
   wresizable = wres;
   hresizable = hres;
#ifdef QT1
   static const int qcoord_max=(QCOORD_MAX>32767)?32767:QCOORD_MAX;
   if (wresizable=wres)
   {
      if (aux_widget) aux_widget->setMaximumWidth(qcoord_max);
      setMaximumWidth(qcoord_max);
   } else
   {
      int width;
      if (aux_widget)
	 if ((width=aux_widget->minimumSize().width()))
	    aux_widget->setMaximumWidth(width);
      if ((width=minimumSize().width()))
	 setMaximumWidth(width);
   }
   if (hresizable=hres)
   {
      if (aux_widget) aux_widget->setMaximumHeight(qcoord_max);
      setMaximumHeight(qcoord_max);
   } else
   {
      int height;
      if (aux_widget)
      {
	 if ((height=aux_widget->minimumSize().height()))
         {
	    aux_widget->setMaximumHeight(height);
         }
      }
      if ((height=minimumSize().height()))
	 setMaximumHeight(height);
   }
#endif
   setResizeDecorations(wresizable || hresizable);
}


void
QeDialog::show(void)
{
#ifdef QT1
      // The following operation is not that useless as one may think
      // We override resize() to fix the width and height in accordance
      // with the aux_widget's min/max dimensions. I'm more than sure, that
      // width() and height() at this moment are 1, but aux_widget already
      // has valid min/max dimensions. So, before we show anything, we
      // can take advantage of them and eliminate some flicking.
      // QWidget::resize() sends XConfigureWindow() event immediately
      // Thus, the window's size will be adjusted before it's actually
      // shown.
      //
      // This is useful for the case with the aux_widget only because only
      // in this case it takes some time to deliver QEvent::LayoutHint from
      // the aux_widget to this dialog. Note, that QLayouts modify the
      // aux_widget's min/max size directly, while the dialog learns about it
      // only via a posted (delayed) event.
   resize(width(), height());
#else
   if (parent)
     {
       QSize sz = sizeHint();
       QWidget * wid= parent->topLevelWidget();
       move( wid->x() + (wid->width() - sz.width())/2,
             wid->y() + (wid->height() - sz.height())/2 );
     }
#endif
   QDialog::show();
}

void
QeDialog::done(int rc)
{
  if (rc==Accepted) emit sigClosed();
  else emit sigCancelled();
  QDialog::done(rc);
}

void
QeDialog::slotParentDestroyed(void)
{
      // This slot is called when we have a "virtual" parent (passed to
      // the constructor of a non-modal dialog). It's not a real parent
      // from the QT's viewpoint because in this case we pass NULL to
      // QDialog's constructor. So QT will not destroy us when this parent
      // passes away. So we have to watch for this and commit a suicide.
   qeApp->killWidget(this);
}

QeDialog::QeDialog(QWidget * parent_, const char * name,
		   bool modal_, WFlags f) 
  :  QDialog(modal_ ? parent_ : 0, name, modal_, f),
     modal(modal_), parent(parent_), aux_widget(0),
     wresizable(false), hresizable(false)
{
#ifdef QT1
      // Note: parent here *may* be non-zero even when QDialog ate
      // zero parent. This is why we make this strange check
   if (!parentWidget())
   {
      aux_widget=new QWidget(this, "aux_dialog_widget");
      aux_widget->setGeometry(0, 0, width(), height());
      if (parent) connect(parent, SIGNAL(destroyed(void)),
			  this, SLOT(slotParentDestroyed(void)));
   } else
   {
      aux_widget=0;
      parentWidget()->installEventFilter(this);
   }
      // We need to do it both when the dialog is not modal (when
      // QT does nothing) or when the dialog IS modal because in the
      // latter case QT doesn't care to search for the top level widget
   if (parent) makeTransient(this, parent->topLevelWidget());
   setResizable(0);
   resize(0, 0);
#else
   if (!parentWidget() && parent)
     {
       makeTransient(this, parent->topLevelWidget());
       connect(parent, SIGNAL(destroyed(void)),
	       this, SLOT(slotParentDestroyed(void)));     
     }
#endif
}

#ifdef UNIX

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include "qxlib.h"

//
// Contents of the _MWM_HINTS property.
//

typedef struct
{
      // These correspond to XmRInt resources. (VendorSE.c)
   int	flags;
   int	functions;
   int	decorations;
   int	input_mode;
   int	status;
} MotifWmHints;

typedef MotifWmHints    MwmHints;

// bit definitions for MwmHints.flags 
#define MWM_HINTS_FUNCTIONS     (1L << 0)
#define MWM_HINTS_DECORATIONS   (1L << 1)
#define MWM_HINTS_INPUT_MODE    (1L << 2)
#define MWM_HINTS_STATUS        (1L << 3)

// bit definitions for MwmHints.functions
#define MWM_FUNC_ALL            (1L << 0)
#define MWM_FUNC_RESIZE         (1L << 1)
#define MWM_FUNC_MOVE           (1L << 2)
#define MWM_FUNC_MINIMIZE       (1L << 3)
#define MWM_FUNC_MAXIMIZE       (1L << 4)
#define MWM_FUNC_CLOSE          (1L << 5)
#define MWM_FUNC_QUIT_APP       (1L << 23)      // for 4Dwm

// bit definitions for MwmHints.decorations
#define MWM_DECOR_ALL           (1L << 0)
#define MWM_DECOR_BORDER        (1L << 1)
#define MWM_DECOR_RESIZEH       (1L << 2)
#define MWM_DECOR_TITLE         (1L << 3)
#define MWM_DECOR_MENU          (1L << 4)
#define MWM_DECOR_MINIMIZE      (1L << 5)
#define MWM_DECOR_MAXIMIZE      (1L << 6)

#endif /* UNIX */

void
QeDialog::makeTransient(QWidget * dialog, QWidget * parent)
{
#ifdef UNIX
   if (parent && dialog)
   {
	 // We want to find the real top-level X11 window, which may
	 // be different from parent->topLevelWidget()->winId() because
	 // we may have remapped it (as it IS in the case of Netscape plugin)
      Window top_level=x11GetTopLevelWindow(parent->x11Display(),
					    (u_long) parent->winId());
      XSetTransientForHint(dialog->x11Display(), dialog->winId(), top_level);
   }

      // Transient windows are not supposed to be in iconic state
      // Some "GURUS" though misconfigure their window managers, which
      // will surely result in troubles with transient modal dialogs
   XWindowAttributes attr;
   if (XGetWindowAttributes(dialog->x11Display(), dialog->winId(), &attr))
      XSelectInput(dialog->x11Display(), dialog->winId(),
		   attr.your_event_mask | PropertyChangeMask);
#endif /* UNIX */
}

bool
QeDialog::x11Event(XEvent * ev)
{
#ifdef UNIX
      // Look for changes in the window state (iconified) and deiconify it back
   if (ev->type==PropertyNotify &&
       ev->xproperty.window==winId())
   {
      Atom atom=XInternAtom(x11Display(), "WM_STATE", True);
      if (atom==ev->xproperty.atom)
      {
	 bool dialog_iconified=false, toplevel_iconified=false;
	 
	 Atom ret_type;
	 int ret_format;
	 unsigned long nitems, bytes_left;
	 unsigned char * prop=0;
	 if (XGetWindowProperty(x11Display(), winId(), atom, 0, 3, False,
				atom, &ret_type, &ret_format, &nitems,
				&bytes_left, &prop)==Success && prop && ret_type)
	 {
	    dialog_iconified=(*(long *) prop==IconicState);
	    XFree(prop); prop=0;

	    Window parent=0;
	    if (XGetTransientForHint(x11Display(), winId(), &parent) && parent!=0)
	    {
	       Window top_level=x11GetTopLevelWindow(x11Display(), parent);
	       if (XGetWindowProperty(x11Display(), top_level, atom, 0, 3, False,
				      atom, &ret_type, &ret_format, &nitems,
				      &bytes_left, &prop)==Success && prop && ret_type)
	       {
		  toplevel_iconified=(*(long *) prop==IconicState);
		  XFree(prop); prop=0;
	       }
	    }
	 }

	 if (dialog_iconified && !toplevel_iconified)
	 {
	    raise();
	    XMapWindow(x11Display(), winId());
	 }
      }
   }
#endif /* UNIX */
   return QDialog::x11Event(ev);
}

void
QeDialog::setResizeDecorations(bool enabled)
{
#ifdef QT1
      // We remove "resize" WM decorations and functions for Motif Window
      // Manager ONLY. Let's hope, that QT-2.0 will be smart enough to do it
      // itself for any WM.
      // Removal of the decorations is not critical, as the dialog is still
      // not resizable due to minSize==maxSize imposed by QeDialog handlers.

   Atom atom=XInternAtom(x11Display(), "_MOTIF_WM_HINTS", False);
   if (atom)
   {
      MwmHints hints;
      memset(&hints, 0, sizeof(hints));
      
      Atom real_type;
      int real_format;
      u_long real_items;
      u_long bytes_left;
      u_char * data;
      if (XGetWindowProperty(x11Display(), winId(), atom,
			     0, sizeof(MwmHints)/4, False, atom,
			     &real_type, &real_format, &real_items,
			     &bytes_left, &data)==Success &&
	  data && real_type)
      {
	 hints=*((MwmHints *) data);
	 XFree(data);
      }
      if (!(hints.flags & MWM_HINTS_FUNCTIONS))
	 hints.functions=MWM_FUNC_ALL;
      if (!(hints.flags & MWM_HINTS_DECORATIONS))
	 hints.decorations=MWM_DECOR_ALL;
      hints.flags|=MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;

      if (hints.functions & MWM_FUNC_ALL)
	 if (enabled) hints.functions&=~MWM_FUNC_RESIZE;
	 else hints.functions|=MWM_FUNC_RESIZE;
      else
	 if (enabled) hints.functions|=MWM_FUNC_RESIZE;
	 else hints.functions&=~MWM_FUNC_RESIZE;
	 
      if (hints.decorations & MWM_DECOR_ALL)
	 if (enabled) hints.decorations&=~MWM_DECOR_RESIZEH;
	 else hints.decorations|=MWM_DECOR_RESIZEH;
      else
	 if (enabled) hints.decorations|=MWM_DECOR_RESIZEH;
	 else hints.decorations&=~MWM_DECOR_RESIZEH;

      XChangeProperty(x11Display(), winId(), atom, atom,
		      32, PropModeReplace, (u_char *) &hints,
		      sizeof(MwmHints)/4);
   }
#endif
}

// QECOMBOBOX ----------------------------------------

void
QeComboBox::setMinMaxSize(void)
{
#ifdef QT1
  setMinimumWidth(sizeHint().width());
  setMinimumHeight(sizeHint().height());
  setMaximumHeight(sizeHint().height());
  ActivateLayouts(this);
#endif
}

void
QeComboBox::setCurrentItem(const QString & qtext)
{
   const char * const txt=qtext;
   for(int i=count()-1;i>=0;i--)
     if (!strcmp(text(i), txt))
       {
        setCurrentItem(i);
        break;
       }
}

void
QeComboBox::setCurrentItem(int item_num)
{ 
  QComboBox::setCurrentItem(item_num); 
}

QeComboBox::QeComboBox(QWidget * parent=0, const char * name=0)
  : QComboBox(parent, name) 
{ 
  setMinMaxSize(); 
}

QeComboBox::QeComboBox(bool rw, QWidget * parent=0, const char * name=0) 
  : QComboBox(rw, parent, name) 
{ 
  setMinMaxSize(); 
}

void
QeComboBox::show(void)
{
  setMinMaxSize();
  QComboBox::show();
}
