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
// $Id: qd_doc_info.h,v 1.9 2007/03/25 20:48:25 leonb Exp $
// $Name: release_3_5_22 $

#ifndef HDR_QD_DOC_INFO
#define HDR_QD_DOC_INFO
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "DjVuDocument.h"
#include "qd_port.h"

#include <qdialog.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qtabdialog.h>



class QDDocInfo : public QTabDialog
{
   Q_OBJECT
private:
   QPushButton		* page_goto_butt;
   QPushButton		* page_info_butt, * thumb_info_butt, * file_info_butt;
   QLabel		* size_label;
   QListView		* page_list, * file_list, * thumb_list;

//   QTabDialog	* tab_dialog;
   
   TArray<void *>	page_items, file_items, thumb_items;

   GP<DjVuDocument>	doc;

   QDPort		port;

   void		preloadNextPage(void);
   void		preloadNextThumb(void);
   void		updatePage(const GP<DjVuFile> & file);
   void		updateThumb(const GP<DjVuFile> & file);
private slots:
   void		slotItemSelected(QListViewItem * item);
   void		slotItemDblClicked(QListViewItem * item);
   void		slotShowInfo(void);
   void		slotGotoPage(void);
   void		slotRightButtonPressed(QListViewItem *, const QPoint &, int);
      // Slots to receive translated requests from QDPort
   void		slotNotifyError(const GP<DjVuPort> &, const GUTF8String &);
   void		slotNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long);
signals:
   void		sigGotoPage(int page_num);
public:
   virtual void	show(void);

      // Note, that structure of DjVuDocument should already be known
   QDDocInfo(const GP<DjVuDocument> & doc, QWidget * parent=0,
	     const char * name=0, bool modal=FALSE);
   ~QDDocInfo(void) {}
};

#endif
