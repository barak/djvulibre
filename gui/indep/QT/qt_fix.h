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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "qwidget.h"

#if (defined(QT_FIX_DO_ALL) || defined(QAPPLICATION_H)) && !defined(QeAPPLICATION_DEFINED)
#define QeAPPLICATION_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qapplication.h>
#endif
#include <qlist.h>
#include <qtimer.h>
#include "debug.h"

extern struct QeApplication * qeApp;

class QeApplication : public QApplication
{
   Q_OBJECT
private:
   QList<QWidget>	widgetsToKill;
   QTimer		kill_timer;

   void		construct(void);
private slots:
   void		killTimeout(void);
signals:
   void		gotX11Event(XEvent *);
public slots:
   void		slotKillSender(void);
public:
   float	gamma;
   QString	geometry;
      // killWidget() kills the mentioned widget when QApplication is about
      // to block. It will not kill a widget if it has already been deleted
      // by someone.
   void		killWidget(QWidget * widget);
      // Resizes and moved widget in accordance with X11 geometry data.
   void		setWidgetGeometry(QWidget * w);
#ifdef UNIX
   bool		x11EventResult;
   virtual bool	x11EventFilter(XEvent * ev);
#endif
   QeApplication(int & argc, char ** argv);
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QDIALOG_H)) && !defined(QeDIALOG_DEFINED)
#define QeDIALOG_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qdialog.h>
#endif
class QeDialog : public QDialog
{
   Q_OBJECT
private:
   bool		modal;
   QWidget	* parent;	// Parent passed to the constructor
   QWidget	* aux_widget;
   bool		wresizable, hresizable;
   
   virtual bool	event(QEvent * ev);	// Used with aux_widget
   virtual bool	eventFilter(QObject * obj, QEvent * ev); // Used w/o aux_widget
   virtual bool	x11Event(XEvent * ev);

   void		setResizeDecorations(bool enabled);
signals:
   void		sigClosed(void);
   void		sigCancelled(void);
public slots:
   void		slotParentDestroyed(void);
public:
   static void	makeTransient(QWidget * dialog, QWidget * parent);

   virtual void	resize(int w, int h);
   virtual void	setGeometry(int x, int y, int w, int h);
   virtual void	show(void);
   virtual void	done(int rc);
   
   QWidget	* startWidget(void) { return aux_widget ? aux_widget : this; }

   void		setResizable(bool wres, bool hres);
   void		setResizable(bool res) { setResizable(res, res); }
   
   QeDialog(QWidget * parent=0, const char * name=0,
	    bool modal=FALSE, WFlags f=0);
};
#endif

#if (defined(QT_FIX_DO_ALL) || defined(QFILEDIALOG_H)) && !defined(QeFILEDIALOG_DEFINED)
#define QeFILEDIALOG_DEFINED
#ifdef QT_FIX_DO_ALL
#include <qfiledialog.h>
#endif
#include "GString.h"
class QeFileDialog : public QFileDialog
{
   Q_OBJECT
private:
   bool		forWriting;
protected slots:
   virtual void	done(int rc);
public:
   static QString	lastSaveDir;
   static QString	lastLoadDir;
   
   void	setForWriting(bool fwr);
   
   QeFileDialog(const QString & dirName, const char * filter=0,
		QWidget * parent=0, const char * name=0, bool modal=FALSE);
   QeFileDialog(QWidget * parent=0, const char * name=0, bool modal=FALSE);
};

#endif


