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

#ifndef HDR_QD_THUMB
#define HDR_QD_THUMB
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "qd_port.h"
#include "GContainer.h"

#include <qlistbox.h>
#include <qcursor.h>

#include "qt_fix.h"

class QDThumbnails : public QListBox
{
   Q_OBJECT
   friend class QDThumbItem;
private:
   class Pending : public GPEnabled
   {
   public:
      int		page;
      GP<DataPool>	pool;
      Pending(int _page, const GP<DataPool> & _pool) :
	    page(_page), pool(_pool) {}
   };
   GP<DjVuDocument>	doc;
   QDPort		port;
   QDMessenger		messenger;
   class QPopupMenu	* popup_menu;
   int			popup_menu_id;

   bool			need_rescan;

   GArray<QPixmap>	pixmaps_arr;
   GList<int>		pixmaps_list;

   GPList<Pending>	pending_list;

   QCursor	normalCursor;

   bool		fast_mode;
   int		cur_page_num;

   bool         rowMajor;
   int          min_list_width;
   int          max_list_width;
   
   QPixmap	getImage(int page_num, int width, int height);
   void		processCommand(int cmd);

   static void	trigger_cb(void * cl_data);
private slots:
   void		slotPopup(int);
   void		slotNotifyFileFlagsChanged(const GP<DjVuFile> & source,
					   long set_mask, long clr_mask);
   void		slotNotifyDocFlagsChanged(const GP<DjVuDocument> & source,
					  long set_mask, long clr_mask);
   void		slotTriggerCB(int);
   void         slotGotoPage(QListBoxItem *);
protected:
   virtual void	resizeEvent(QResizeEvent * ev);
   virtual void	mousePressEvent(QMouseEvent * ev);
   virtual void	dataSet(int page_num, const TArray<char> & data) {}
   void		setData(int page_num, const TArray<char> & data);
   bool		getDataForPage(int page_num);
   void		getDataForNextPage(void);
signals:
   void		sigGotoPage(int page_num);
   void		sigCloseThumbnails(void);
public slots:
   void		slotReloadPage(int page_num) { reloadPage(page_num); }
public:
   virtual QSize sizeHint(void) const;
   virtual void	show(void);
   
   void		rescan(void);
      // Will clear data for page 'page_num' and request it from DjVuDocument
      // again. fast_mode will be used to determine if DjVuDocument should
      // decode the file or just return predecoded data (if any)
   void		reloadPage(int page_num);

   void		rereadGamma(void);
   void		setFastMode(bool en);
   void		setDjVuDocument(GP<DjVuDocument> & doc);
   virtual void	setCurPageNum(int cur_page_num);

   QDThumbnails(QWidget * parent=0, const char * name=0, bool _rowMajor=FALSE);
   ~QDThumbnails(void) {}
};

#endif
