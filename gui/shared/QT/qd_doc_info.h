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

#ifndef HDR_QD_DOC_INFO
#define HDR_QD_DOC_INFO
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
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
