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

#ifndef HDR_QD_SEARCH_DIALOG
#define HDR_QD_SEARCH_DIALOG
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "DjVuDocument.h"
#include "DjVuText.h"

#include <qdialog.h>

#include "qt_fix.h"

class QDSearchDialog : public QeDialog
{
   Q_OBJECT
private:
   static bool	all_pages;
   static bool	case_sensitive;
   static bool	search_backwards;
   static bool	whole_word;

   class QeLineEdit	* text;
   class QeCheckBox	* all_pages_butt;
   class QeCheckBox	* case_butt;
   class QeCheckBox	* back_butt;
   class QeCheckBox	* whole_word_butt;
   class QePushButton	* search_butt;
   class QePushButton	* clear_butt;
   class QLabel		* status_label;

   bool			in_search;
   bool			stop;
   
   int			page_pos;
   int			page_num;
   int			seen_page_num;
   GP<DjVuDocument>	doc;
private slots:
   void		slotSearch(void);
   void		slotTextChanged(const QString &);
protected:
   virtual void	done(int);
signals:
   void		sigDisplaySearchResults(int page_num, const GList<DjVuTXT::Zone *> & zone_list);
public slots:
   void		slotSetPageNum(int page_num);
public:
   QDSearchDialog(int page_num, const GP<DjVuDocument> & doc, QWidget * parent=0,
		  const char * name=0, bool modal=FALSE);
   ~QDSearchDialog(void) {};
};

#endif
