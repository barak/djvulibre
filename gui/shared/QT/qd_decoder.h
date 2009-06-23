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
// $Id: qd_decoder.h,v 1.8 2007/03/25 20:48:25 leonb Exp $
// $Name: release_3_5_22 $

#ifndef HDR_QD_DECODER
#define HDR_QD_DECODER
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "DjVuDocument.h"
#include "qd_port.h"

#include <qprogressbar.h>
#include <qprogressdialog.h>
#include <qlabel.h>



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
 
