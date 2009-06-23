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
// $Id: qd_decoder.cpp,v 1.9 2007/03/25 20:48:25 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qd_decoder.h"
#include "debug.h"
#include "qlib.h"

#include <qapplication.h>



//***************************** Decoder callbacks **************************

void
QDDecoder::slotNotifyError(const GP<DjVuPort> &, const GUTF8String &qmsg)
{
   ::showError(0, "DjVu decoder error", QStringFromGString(qmsg));
   event.set();
}

void
QDDecoder::slotNotifyStatus(const GP<DjVuPort> &, const QString &qmsg)
{
   const char * const msg=qmsg;
   if (status_label)
   {
      GUTF8String text;
      for(const char * ptr=msg;*ptr && *ptr!='\n';ptr++)
      {
	 if (*ptr!='\t')
         {
           text+=*ptr;
	 }else
         {
           text+=' ';
         }
      }
      status_label->setText(QStringFromGString(text));
   }
   event.set();
}

void
QDDecoder::slotNotifyFileFlagsChanged(const GP<DjVuFile> &, long set_mask, long)
{
   if (set_mask & (DjVuFile::DECODE_OK | DjVuFile::DECODE_FAILED | DjVuFile::DECODE_STOPPED))
   {
      if (status_label) status_label->setText("");
      if (progress_bar) progress_bar->reset();
      if (progress_dlg) progress_dlg->setProgress(100);
      event.set();
   }
}

void
QDDecoder::slotNotifyDecodeProgress(const GP<DjVuPort> &, float done)
{
   if (progress_bar || progress_dlg)
      if ((int) (done*50)!=(int) (last_done*50))
      {
	 if (progress_bar) progress_bar->setProgress((int) (done*100));
	 if (progress_dlg) progress_dlg->setProgress((int) (done*100));
	 last_done=done;
      }
   event.set();
}

void
QDDecoder::waitTillDecodingEnds(void)
{
   DEBUG_MSG("QDDecoder::waitTillDecodingEnds(): blocking till decode is over\n");
   DEBUG_MAKE_INDENT(3);
   
   while(image->get_djvu_file()->is_decoding())
   {
      event.wait(100);
      qApp->processEvents(100);
      if (progress_dlg && progress_dlg->wasCancelled())
	 image->get_djvu_file()->stop_decode(true);
   }
   if (progress_bar) progress_bar->reset();
   if (progress_dlg) progress_dlg->setProgress(100);
}

GP<DjVuImage>
QDDecoder::getPageImage(int page_num)
{
   DEBUG_MSG("QDDecoder::getPageImage(): page_num=" << page_num << "\n");
   DEBUG_MAKE_INDENT(3);

   last_done=0;
   if (progress_dlg) progress_dlg->setProgress(0);

   image=document->get_page(page_num, false, port.getPort());
   waitTillDecodingEnds();

   if (image->get_djvu_file()->is_decode_ok()) return image;
   else return 0;
}

QDDecoder::QDDecoder(const GP<DjVuDocument> & doc) :
      port(true, true), document(doc)
{
   DEBUG_MSG("QDDecoder::QDDecoder(): initializing class...\n");
   DEBUG_MAKE_INDENT(3);
   
   progress_bar=0;
   progress_dlg=0;
   status_label=0;
   last_done=0;

      // Connect slots & signals
   connect(&port, SIGNAL(sigNotifyError(const GP<DjVuPort> &, const GUTF8String &)),
	   this, SLOT(slotNotifyError(const GP<DjVuPort> &, const GUTF8String &)));
   connect(&port, SIGNAL(sigNotifyStatus(const GP<DjVuPort> &, const QString &)),
	   this, SLOT(slotNotifyStatus(const GP<DjVuPort> &, const QString &)));
   connect(&port, SIGNAL(sigNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)),
	   this, SLOT(slotNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)));
   connect(&port, SIGNAL(sigNotifyDecodeProgress(const GP<DjVuPort> &, float)),
	   this, SLOT(slotNotifyDecodeProgress(const GP<DjVuPort> &, float)));
}
