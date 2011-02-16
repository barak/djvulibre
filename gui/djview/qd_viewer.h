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

#ifndef HDR_QD_VIEWER
#define HDR_QD_VIEWER
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "qd_base.h"
#include "saved_data.h"
#include "DjVuDocument.h"
#include "qd_port.h"
#include "djvu_base_res.h"

#include <qpopupmenu.h>
#include <qtimer.h>


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
   public:
     enum CACHE { CACHE_UNSPEC=-1, CACHE_OFF=0, CACHE_ON=1 };
     SavedData	 saved;
     bool	 full_mode;
     int	 cache;
     GUTF8String rel_url;
     GUTF8String page_id;
     // helpers
     void parse(const DArray<GUTF8String> & argn, const DArray<GUTF8String> & argv);
     PluginData(void);
     PluginData(const DArray<GUTF8String> & argn, const DArray<GUTF8String> & argv);
   private:
     void parsePair(const GUTF8String &name, const GUTF8String &value);
     static bool parseBool(const GUTF8String &value);
     static GUTF8String getQuotedString(const char *& ptr);
   };

private:
     // Operation modes
   int		          saved_data_known;
   int		          in_netscape;
   PluginData	          plugin_data;
   static bool	          once_welcomed;
   QPopupMenu	        * popup_menu;
   int		          popup_menu_id;
   QWidget	        * welcome;
   QDialog              * about;

   class QDPrintDialog	* print_win;
   class QDThumbnails	* thumbnails;
   class QDTBarNavPiece	* nav_tbar;
   QTimer	          hlinks_timer, welcome_timer;
   int		          start_page_num;
   GP<DjVuDocument>	  djvu_doc;
   QDPort		  page_port, doc_port;
   GP<DjVuImage>	  predecode_dimg1, predecode_dimg2;
   GList<int>             hundo, hredo;

   void         history_add(int page);
   int          history_undo(void);
   int          history_redo(void);
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
   void		slotNotifyChunkDone(const GP<DjVuPort> & source, 
                                    const GUTF8String &name);
   void		slotNotifyFileFlagsChanged(const GP<DjVuFile> & source, 
                                           long, long);
   void		slotNotifyDocFlagsChanged(const GP<DjVuDocument> & source, 
                                          long, long);

   void		slotSearchClosed(void);
   void		slotDisplaySearchResults(int page_num, 
                                         const GList<DjVuTXT::Zone *> & zones_list);
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
   virtual void	getURL(const GUTF8String &url, const GUTF8String &target, 
                       bool history );
signals:
   void		sigGetURL(const GURL & url, const GUTF8String &target);
   void		sigPageChanged(int page_num);
   void         sigToggleFullScreen(void);
public:
   SavedData	getSavedData(void) const;
   void		gotoPage(int page_num, bool history=true);
   void		print(void);
   void		savePageAs(void);
   void		saveDocumentAs(void);
   void		setDjVuDocument(GP<DjVuDocument> & doc,	
                                const GUTF8String &key=(const char *)0);
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
