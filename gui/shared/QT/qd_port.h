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

#ifndef HDR_QD_PORT
#define HDR_QD_PORT
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "DjVuPort.h"
#include "DjVuImage.h"
#include "DjVuFile.h"
#include "DjVuDocument.h"
#include "qd_messenger.h"


#include <qobject.h>

class QDPort : public QObject
{
   Q_OBJECT
private:
   class Port : public DjVuPort
   {
   public:
      class QDPort	* port;

      GSafeFlags	disabled;
      
	 // Functions inherited from DjVuPort
	 // request_data() can not be implemented here :(
	 // The reason is that it should RETURN the DataRange IMMEDIATELY
	 // while the main purpose of QDPort is to tunnel requests thru
	 // a pipe for thread synchronization
      virtual bool	inherits(const GUTF8String &class_name) const;
      virtual bool	notify_error(const DjVuPort * source, const GUTF8String &msg);
      virtual bool	notify_status(const DjVuPort * source, const GUTF8String &msg);
      virtual void	notify_redisplay(const DjVuImage * source);
      virtual void	notify_relayout(const DjVuImage * source);
      virtual void	notify_chunk_done(const DjVuPort * source, const GUTF8String &name);
      virtual void	notify_file_flags_changed(const DjVuFile * source,
						  long set_mask, long clr_mask);
      virtual void	notify_doc_flags_changed(const DjVuDocument * source,
						 long set_mask, long clr_mask);
      virtual void	notify_decode_progress(const DjVuPort * source, float done);

      Port(class QDPort * p) : port(p) {}
   };
   friend class QDPort::Port;

   GP<Port>		port;
   QDMessenger		messenger;
   GPList<DjVuPort>	src_list;
   GCriticalSection	src_lock;
   bool			watch_errors, watch_status;

   bool		sig_error_on, sig_status_on;
   bool		sig_redisplay_on, sig_relayout_on;
   bool		sig_chunk_done_on, sig_file_flags_changed_on;
   bool		sig_doc_flags_changed_on, sig_decode_progress_on;
private slots:
   void		slotGeneralMsg(const GUTF8String &msg);
protected:
   virtual void	connectNotify(const char *);
   virtual void	disconnectNotify(const char *);
signals:
   void		sigNotifyError(const GP<DjVuPort> & source, const GUTF8String &msg);
   void		sigNotifyStatus(const GP<DjVuPort> & source, const QString &msg);
   void		sigNotifyRedisplay(const GP<DjVuImage> & source);
   void		sigNotifyRelayout(const GP<DjVuImage> & source);
   void		sigNotifyChunkDone(const GP<DjVuPort> & source, const GUTF8String &name);
   void		sigNotifyFileFlagsChanged(const GP<DjVuFile> & source,
					  long set_mask, long clr_mask);
   void		sigNotifyDocFlagsChanged(const GP<DjVuDocument> & source,
					 long set_mask, long clr_mask);
   void		sigNotifyDecodeProgress(const GP<DjVuPort> & source, float done);
public:
      // Functions called by QDPort::Port
   bool		notify_error(const DjVuPort * source, const GUTF8String &msg);
   bool		notify_status(const DjVuPort * source, const GUTF8String &msg);
   void		notify_redisplay(const DjVuImage * source);
   void		notify_relayout(const DjVuImage * source);
   void		notify_chunk_done(const DjVuPort * source, const GUTF8String &name);
   void		notify_file_flags_changed(const DjVuFile * source,
					  long set_mask, long clr_mask);
   void		notify_doc_flags_changed(const DjVuDocument * source,
					 long set_mask, long clr_mask);
   void		notify_decode_progress(const DjVuPort * source, float done);
   
   QDMessenger &	getMessenger(void);
   GP<DjVuPort>		getPort(void);
   QDPort(bool watch_errors, bool watch_status,
	  QObject * parent=0, const char * name=0);
   virtual ~QDPort(void);
};

inline QDMessenger &
QDPort::getMessenger(void) { return messenger; }

inline GP<DjVuPort>
QDPort::getPort(void) { return (DjVuPort *) port; }

inline bool
QDPort::Port::inherits(const GUTF8String &class_name) const
{
   return (class_name=="QDPort") || DjVuPort::inherits(class_name);
}

#endif
