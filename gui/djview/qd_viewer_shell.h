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

#ifndef HDR_QD_VIEWER_SHELL
#define HDR_QD_VIEWER_SHELL
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "DjVuImage.h"		// Must be included before QT

#include <qmenubar.h>
#include <qmainwindow.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qwidgetstack.h>

#include "prefs.h"
#include "djvu_viewer.h"

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
   QWidgetStack	* main;
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
