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

#ifndef HDR_QD_VIEWER_SHELL
#define HDR_QD_VIEWER_SHELL
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "DjVuImage.h"		// Must be included before QT

#include <qmenubar.h>
#include <qmainwindow.h>
#include <qlabel.h>
#include <qtimer.h>

#include "qt_n_in_one.h"
#include "prefs.h"
#include "djvu_viewer.h"

#include "qt_fix.h"

// Use this class if you want to create a standalone viewer. It creates
// menu system, wallpaper, DjVuViewer, opens a file - does everything 
// needed to open and display DjVu document without your concern.
// You just need to initialize it and let it live.

class QDViewerShell : public QMainWindow
{
   Q_OBJECT
private:
   GP<DjVuViewer>djvu;
   
   QMenuBar	* menu;
   QFrame       * status_frame;
   QLabel	* status_bar;
   QeNInOne	* main;
   QWidget	* wpaper;
   QWidget	* vparent;

   QTimer	gu_timer;
   const QObject * gu_djvu;
   GURL		gu_url;
   GUTF8String	gu_target;

   bool         fullscreen;
   
   void		getURL(const GURL & url, const GUTF8String &target);
   void		about(void);
   void		help(void);
   QDViewerShell * createNewShell(const GURL & url) const;
private slots:
   void		slotAboutToShowMenu(void);
   void		slotMenuCB(int cmd);
   void		slotGetURLTimeout(void);
   void		slotGetURL(const GURL & url, const GUTF8String &target);
public slots:
   void		slotShowStatus(const QString &msg);
   void         slotToggleFullScreen();
   void         slotQueryFullScreen(bool &);
protected:
   virtual void	closeEvent(QCloseEvent * e);

public:
      // Opens the document with the specified name in the current
      // window. ID serves to identify page to open. This is an obsolete
      // function. openURL() allows to do the same and much more as it
      // understands CGI arguments in the URL. Use that instead.
      // fname='-' means to read data from stdin, in which case the
      // ID is ignored anyway.
   void		openFile(const QString &fname, const QString &id=QString::null);

      // Opens the specified URL in the current window.
      // Empty URL means to read stdin
   void		openURL(const GURL & url);

      // Handle full screen mode
   bool         isFullScreen() const;

   QDViewerShell(QWidget * parent=0, const char * name=0);
   ~QDViewerShell(void);
};

#endif
