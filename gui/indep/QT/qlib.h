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

#ifndef HDR_QLIB
#define HDR_QLIB
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "GException.h"
#include "GRect.h"
#include "Arrays.h"
#include "GPixmap.h"
#include "GString.h"

#include <qapplication.h>
#include <qdialog.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qrect.h>
#include <qmenudata.h>
#include <qstring.h>
#include <qtimer.h>

#include "debug.h"

// ------------------------------------------------

inline GUTF8String
GStringFromQString(const QString & x)
{
  GUTF8String retval=(const char *)x.utf8();
  return retval;
}

inline QString
QStringFromGString(const GUTF8String & x)
{
  QString retval=QString::fromUtf8((const char *)x);
  return retval;
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


// ------------------------------------------------


class QeApplication : public QApplication
{
  Q_OBJECT
public:
  QeApplication(int &argc, char **argv);
  ~QeApplication();

  // Function killWidget() schedules a 
  // widget for destruction.
public:
  void           killWidget(QWidget * widget);
private:
  QList<QWidget> widgetsToKill;
  QTimer         kill_timer;
private slots:
  void           killDestroy(void);
  void           killTimeout(void);
  
  // Signal gotX11Event tracks X11 events
#ifdef UNIX
signals:
   void         gotX11Event(XEvent *);
public:
   bool         x11EventResult;
   virtual bool x11EventFilter(XEvent *);
#endif

  // Contain starting gamma and geometry
public:
   float	gamma;
   QString	geometry;
   void		setWidgetGeometry(QWidget * w);
};

extern QeApplication * qeApp;


// ------------------------------------------------

class QeDialog : public QDialog
{
  Q_OBJECT
public:
  QeDialog(QWidget * parent=0, const char * name=0, bool modal=FALSE, WFlags f=0);
  QWidget *startWidget() { return this; }
  static void makeTransient(QWidget *w, QWidget *fw);
};

// ------------------------------------------------

class QeExcMessage : public QeDialog
{
   Q_OBJECT
private:
   QWidget	* details;
   QPushButton	* details_butt;
private slots:
   void		switchDetails(void);
public:
   QeExcMessage(const GException &exc, QString title,
		QWidget *parent=0, const char *name=0, bool modal=TRUE);
};

void showError(QWidget * parent, const GException & exc);
void showError(QWidget * parent, const QString &title, const GException & exc);
void showError(QWidget * parent, const QString &title, const QString &message);
void showWarning(QWidget * parent, const QString &title, const QString &message);
void showInfo(QWidget * parent, const QString &title, const QString &message);
void showMessage(QWidget * parent, const QString &title, const QString &message,
		 bool draw_frame=false, bool use_fixed=false,
		 bool word_wrap=false);

// ------------------------------------------------

class QeFileDialog : public QFileDialog
{
   Q_OBJECT
private:
   bool	forWriting;
protected slots:
   virtual void done(int rc);
public:
  static QString lastSaveDir;
  static QString lastLoadDir;
  void setForWriting(bool fwr);
  QeFileDialog(const QString &dirName, const char *filter=0,
               QWidget *parent=0, const char * name=0, bool modal=FALSE);
  QeFileDialog(QWidget *parent=0, const char * name=0, bool modal=FALSE);
};


// ------------------------------------------------

QPixmap createIcon(const GPixmap & gpix);
void setItemsEnabled(QMenuData * menu, bool flag);
bool setComboBoxCurrentItem(QComboBox *combo, QString item);
void showOrHide(QWidget *w, bool show);

#endif
