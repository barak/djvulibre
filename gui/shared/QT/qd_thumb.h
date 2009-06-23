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
// 
// $Id: qd_thumb.h,v 1.10 2007/03/25 20:48:27 leonb Exp $
// $Name: release_3_5_22 $

#ifndef HDR_QD_THUMB
#define HDR_QD_THUMB
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "qd_port.h"
#include "GContainer.h"

#include <qlistbox.h>
#include <qcursor.h>



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
      Pending(int _page, const GP<DataPool> & _pool) 
        : page(_page), pool(_pool) {}
   };
   GP<DjVuDocument>	doc;
   QDPort		port;
   QDMessenger		messenger;
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
   static void	trigger_cb(void * cl_data);
private slots:
   void		slotNotifyFileFlagsChanged(const GP<DjVuFile> & source,
					   long set_mask, long clr_mask);
   void		slotNotifyDocFlagsChanged(const GP<DjVuDocument> & source,
					  long set_mask, long clr_mask);
   void		slotTriggerCB(int);
   void         slotGotoPage(QListBoxItem *);
protected:
   virtual void	resizeEvent(QResizeEvent * ev);
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
