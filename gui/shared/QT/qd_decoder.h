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

#ifndef HDR_QD_DECODER
#define HDR_QD_DECODER
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "DjVuDocument.h"
#include "qd_port.h"

#include <qprogressbar.h>
#include <qprogressdialog.h>
#include <qlabel.h>

#include "qt_fix.h"

// QDDecoder: class for decoding DjVuDocuments in the QT environment.
// Blocks execution until the image is completely ready.
// May display progress and status info in externally supplied
// progress bar and status label

class QDDecoder : public QObject
{
   Q_OBJECT
private:
   QProgressBar		* progress_bar;
   QProgressDialog	* progress_dlg;
   QLabel		* status_label;

   QDPort		port;
   GEvent		event;

   GP<DjVuDocument>	document;
   GP<DjVuImage>	image;
   float		last_done;

   void		waitTillDecodingEnds(void);
private slots:
      // Slots to receive translated requests from QDPort
   void		slotNotifyError(const GP<DjVuPort> & source, const GUTF8String &msg);
   void		slotNotifyStatus(const GP<DjVuPort> & source, const QString &msg);
   void		slotNotifyFileFlagsChanged(const GP<DjVuFile> & source,
					   long set_mask, long clr_mask);
   void		slotNotifyDecodeProgress(const GP<DjVuPort> & source, float done);
public:
   GP<DjVuImage>getPageImage(int page_num);

   void		setProgressBar(QProgressBar * progress_bar);
   void		setProgressDialog(QProgressDialog * progress_dlg);
   void		setStatusLabel(QLabel * status_label);
   
   QDDecoder(const GP<DjVuDocument> & doc);
   virtual ~QDDecoder(void) {};
};

inline void
QDDecoder::setProgressBar(QProgressBar * _progress_bar)
{
   progress_bar=_progress_bar;
   progress_bar->setTotalSteps(100);
}

inline void
QDDecoder::setProgressDialog(QProgressDialog * _progress_dlg)
{
   progress_dlg=_progress_dlg;
   progress_dlg->setTotalSteps(100);
}

inline void
QDDecoder::setStatusLabel(QLabel * _status_label)
{
   status_label=_status_label;
}

#endif
 
