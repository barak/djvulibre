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

#ifndef HDR_QD_VIEWER
#define HDR_QD_VIEWER
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "qd_base.h"
#ifdef UNIX
#include "rem_netscape.h"
#endif
#include "saved_data.h"
#include "DjVuDocument.h"
#include "qd_port.h"
#include "djvu_base_res.h"

#include <qpopupmenu.h>
#ifndef QT1
#include <qtimer.h>
#endif

#include "qt_fix.h"

// QDViewer defines popup menu (QDANTBAse doesn't have one since it's
// inherited by both QDViewer and QDEditor which may override it) and
// does everything left undone by QDANTBase, but which can do now because
// QDViewer is AWARE of possible Netscape connection. Actually, QDANTBase
// and QDBase don't know about netscape, they're just visualizers, plain
// widgets. QDViewer has a flag "in_netscape" so it can do smth, which can
// not be done by QDANTBase. It also saves/reads current resolution to
// the top-level window's properties.

class QDViewer : public QDBase
{
   Q_OBJECT
public:
   class PluginData : public OverrideFlags
   {
   private:
      void	parsePair(const GUTF8String &name, const GUTF8String &value);
      
      static bool	parseBool(const GUTF8String &value);
      static GUTF8String getQuotedString(const char *& ptr);
   public:
      enum CACHE { CACHE_UNSPEC=-1, CACHE_OFF=0, CACHE_ON=1 };
      SavedData	saved;
      bool	full_mode;
      int	cache;
      GUTF8String rel_url;
      GUTF8String page_id;
      void	parse(const DArray<GUTF8String> & argn, const DArray<GUTF8String> & argv);
      PluginData(void);
      PluginData(const DArray<GUTF8String> & argn, const DArray<GUTF8String> & argv);
   };
private:
     // Operation modes
   int		saved_data_known;
   int		in_netscape;
   PluginData	plugin_data;

   static bool	once_welcomed;

   QPopupMenu	* popup_menu;
   int		popup_menu_id;
   QWidget	* welcome;
   QDialog      * about;

   class QDPrintDialog	* print_win;

   class QDThumbnails	* thumbnails;

   class QDTBarNavPiece	* nav_tbar;

   QTimer	hlinks_timer, welcome_timer;

      // The number of the first page viewed in this class. Initially
      // ZERO. Is set to smth real after document init is over.
   int		start_page_num;
   
   GP<DjVuDocument>	djvu_doc;
   QDPort		page_port, doc_port;
   GP<DjVuImage>	predecode_dimg1, predecode_dimg2;

#ifdef UNIX
   // Platform dependent class used to issue remote commands to netscape
   RemoteNetscape	rem_netscape;

      // These pipe used to catch errors coming from a child process
      // They're passed to children for substituting their stderr stream
      // Make sure you close them in the ~QDViewer destructor
   GList<int>	child_error_pipes;
#endif

   void		createPopupMenu(void);
   void		runPopupMenu(QMouseEvent * ev);
   void		checkWMProps(void);
   void		showUpgradeDialog(void);
   void		setCaption(void);
   void		predecode(void);
   void		print(int what);
private slots:
      // QDPort connected slots
   void		slotNotifyRedisplay(const GP<DjVuImage> & source);
   void		slotNotifyRelayout(const GP<DjVuImage> & source);
   void		slotNotifyChunkDone(const GP<DjVuPort> & source, const GUTF8String &name);
   void		slotNotifyFileFlagsChanged(const GP<DjVuFile> & source, long, long);
   void		slotNotifyDocFlagsChanged(const GP<DjVuDocument> & source, long, long);

   void		slotSearchClosed(void);
   void		slotDisplaySearchResults(int page_num, const GList<DjVuTXT::Zone *> & zones_list);
   void		slotPrintClosed(void);
   
   void		slotPopupCB(int id) { popup_menu_id=id; };
   void		slotWelcomeClosed(void);
   void		slotShowPrefs(void);
   void		slotHelp(void);
   void		slotAbout(void);
   void		slotShowWelcome(void);
   void		slotChildError(int pipe);
   void		slotGotoPage(int page);
   void		slotDoCmd(int cmd);
   void		slotPrint(void) { print(); }
   void		slotSave(void) { saveDocumentAs(); }
   void		slotFind(void) { search(); }
   void		slotCloseThumbnails(void) { hideThumbnails(); }
protected:
   virtual bool	event(QEvent * ev);
   virtual bool eventFilter(QObject *obj, QEvent *ev);

   virtual void	setDjVuImage(const GP<DjVuImage> & _dimg, int do_redraw);

   virtual QWidget * createThumbnails(bool _rowMajor);
   
   virtual void	fillToolBar(class QDToolBar * toolbar);
   virtual void	updateToolBar(void);
   virtual void	getURL(const GUTF8String &url, const GUTF8String &target);
signals:
   void		sigGetURL(const GURL & url, const GUTF8String &target);
   void		sigPageChanged(int page_num);
   void         sigToggleFullScreen(void);
public:
   SavedData	getSavedData(void) const;
   void		gotoPage(int page_num);
   void		print(void);
   void		savePageAs(void);
   void		saveDocumentAs(void);
   void		setDjVuDocument(GP<DjVuDocument> & doc,	const GUTF8String &key=(const char *)0);
   void		search(void);
   virtual void	imageUpdated(void);
   void		processCommand(int cmd);
   void		setupMenu(QMenuData * menu);

   virtual void	layout(bool allow_redraw=1);

      // Just checks, that the number of pages is greater than 1
   virtual void	showThumbnails(void);
   
   QDViewer(int in_netscape, const PluginData & pdata,
	    QWidget * parent=0, const char * name=0);
   ~QDViewer(void);
};

inline void
QDViewer::print(void)
{
   print(IDC_PRINT_DOC);
}

#endif
