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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qd_messenger.h"
#include "qlib.h"

#include <unistd.h>
#include <qsocketnotifier.h>
#include <sys/time.h>

//****************************************************************************
//*************************** QDMessageQueue *********************************
//****************************************************************************

QDMessageQueue::QDMessageQueue(QObject * parent, const char * name) :
      QObject(parent, name)
{
   DEBUG_MSG("QDMessageQueue::QDMessageQueue(): Initializing class\n");

   queue_head=0;
   
   if (pipe(fd)<0)
     G_THROW("Failed to create pipe");
   QSocketNotifier *sd 
     = new QSocketNotifier(fd[0], QSocketNotifier::Read,
                           this, "djvu_message_queue_socket_notifier");
   sd->setEnabled(TRUE);
   connect(sd, SIGNAL(activated(int)), this, SLOT(notifierActivated(int)));
}

QDMessageQueue::~QDMessageQueue(void)
{
   DEBUG_MSG("QDMessageQueue::~QDMessageQueue(): Destroying class and resources\n");
   ::close(fd[0]);
   ::close(fd[1]);

   GCriticalSectionLock lock(&queue_lock);
   while(queue_head)
   {
      Item * item=queue_head;
      queue_head=item->next;
      delete item;
   }
}

void
QDMessageQueue::addQueueItem(Item * item)
{
   GCriticalSectionLock lock(&queue_lock);
   if (queue_head)
   {
	 // Queue is not empty => smb have already output a byte into
	 // the pipe. Just add one more item to the queue
      Item * tail;
      for(tail=queue_head;tail->next;tail=tail->next);
      tail->next=item;
   } else
   {
	 // Queue is empty => pipe is empty as well => output one byte
      queue_head=item;
      ::write(fd[1], "c", 1);
   }
   item->next=0;
}

void
QDMessageQueue::notifierActivated(int)
{
   DEBUG_MSG("QDMessageQueue::notifierActivated(): got smth in the pipe\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock(&queue_lock);
   try
   {
	 // We will loop here until all pending requests have been processed
	 // It's important to do so at least for REDRAW events since usually
	 // Decoder works pretty fast and manages to decode everything by the
	 // moment when visualizer finishes redrawing the B&W picture. While
	 // decoding the decoder sends a bunch of REDRAW requests thru this
	 // messenger. Unfortunately, if we process it one-by-one then no event
	 // compression will be observed as QT seems to pay much more attention
	 // to XEvents (generated by x11Redraw) than to active SocketNotifiers.
	 // Thus, instead of processing another pipe REDRAW command (and thus
	 // sending another Expose event, which could be merged with the previous
	 // one), QT notices Expose events returned from XServer and processes
	 // it immediately. So we don't let QT return to event loop and do it.
	 // We process all pending commands at once

	 // CORRECTION: Now there is a flag "look_ahead" that tells if to
	 // look for pending events messenger events or not. Specially for
	 // cases where QT interface should have a priority.

	 // It's OFF by default (QT rules)

      char ch;
      ::read(fd[0], &ch, 1);

	 // Pick up the first item from the list and store 'messenger'
      QDMessenger * messenger=0;
      if (queue_head) messenger=queue_head->messenger;
      if (messenger)
      {
	 bool la=messenger->getLookAhead();
	 DEBUG_MSG("messenger=" << messenger << ", look_ahead=" << la << "\n");

	    // Now look thru all the queue in search for items designated for
	    // messenger (if "look ahead" is on), or consider just the first one
	 while(true)
	 {
	    Item * item=0;
	    for(Item ** item_ptr=&queue_head;*item_ptr;item_ptr=&(*item_ptr)->next)
	       if ((*item_ptr)->messenger==messenger)
	       {
		  item=*item_ptr;
		  *item_ptr=item->next;
		  break;
	       }
	    if (item)
	    {
		 // We don't want the queue to be locked while we're
		 // in messenger->sendSignal(). Too easy to get a dead lock
	       queue_lock.unlock();
	       try {
		     // The messenger is guaranteed not to be destroyed until
		     // we enter into sendSignal(). sendSignal() may call
		     // smth, which will destroy the messenger, so we shouldn't
                     // use 'messenger' pointer again.
		  messenger->sendSignal(item);
	       } catch(...) {
		  queue_lock.lock();	// Lock it back
		  if (queue_head) ::write(fd[1], "c", 1);
		  delete item;
		  throw;
	       }
	       queue_lock.lock();	// Lock it back
	       delete item;
	       if (!la) break;
	    } else break;
	 }
      }

	 // Now output one more byte into the pipe to make sure that
	 // QT will ask us to process queue again
      if (queue_head) ::write(fd[1], "c", 1);
   } catch(const GException & exc)
   {
      ::showError(0, tr("DjVu Error"), exc);
   }
}

void
QDMessageQueue::clearMessenger(QDMessenger * messenger)
{
   GCriticalSectionLock lock(&queue_lock);
   for(Item ** item_ptr=&queue_head;*item_ptr;)
      if ((*item_ptr)->messenger==messenger)
      {
	 Item * item=*item_ptr;
	 *item_ptr=item->next;
	 delete item;
      } else item_ptr=&(*item_ptr)->next;
}

//****************************************************************************
//****************************** QDMessenger *********************************
//****************************************************************************

QDMessageQueue	* QDMessenger::queue;

void
QDMessenger::sendSignal(QDMessageQueue::Item * item)
{
   if (item)
   {
      switch(item->cmd)
      {
	 case SHOW_STATUS:
            DEBUG_MSG("SHOW_STATUS request read by messenger\n");
            emit sigShowStatus(QStringFromGString(item->msg)); 
            break;
         case LAYOUT:
	    DEBUG_MSG("LAYOUT request read by messenger\n");
	    emit sigLayout();
	    break;
	 case REDRAW:
	    DEBUG_MSG("REDRAW request read by messenger\n");
	    emit sigRedraw();
	    break;
	 case SHOW_ERROR:
	    DEBUG_MSG("SHOW_ERROR request read by messenger\n");
	    emit sigShowError(item->title, item->msg);
	    break;
	 case GET_URL:
           {
             DEBUG_MSG("GET_URL request read by messenger, url='" <<
                       item->url << "'\n");
             emit sigGetURL(GURL::UTF8(item->url), item->target.length() ?
                            item->target : GUTF8String()); 
             break;
           }
	 case GENERAL_REQ:
	    DEBUG_MSG("GENERAL_REQ request read by messenger, req='" <<
		      item->req << "'\n");
	    emit sigGeneralReq(item->req);
	    break;
	 case GENERAL_MSG:
	    DEBUG_MSG("GENERAL_MSG request read by messenger, msg='" <<
		      item->msg << "'\n");
	    emit sigGeneralMsg(item->msg);
	    break;
         default:
	    G_THROW("Unknown command read from the pipe.");
      }
   }
}

void
QDMessenger::showStatus(const QString &qstatus)
{
   const char * const status=qstatus;
   DEBUG_MSG("QDMessenger::showStatus(): status='" << status << "'\n");

   QDMessageQueue::Item * item=new QDMessageQueue::Item(this, SHOW_STATUS);
   item->msg=status;
   queue->addQueueItem(item);
}

void
QDMessenger::layout(void)
{
   DEBUG_MSG("QDMessenger::layout() called...\n");

   QDMessageQueue::Item * item=new QDMessageQueue::Item(this, LAYOUT);
   queue->addQueueItem(item);
}

void
QDMessenger::redraw(void)
{
   DEBUG_MSG("QDMessenger::redraw() called...\n");

   QDMessageQueue::Item * item=new QDMessageQueue::Item(this, REDRAW);
   queue->addQueueItem(item);
}

void
QDMessenger::showError(const GUTF8String &title, const GUTF8String &msg)
{
   DEBUG_MSG("QDMessenger::showError(): msg='" << msg << "'...\n");

   QDMessageQueue::Item * item=new QDMessageQueue::Item(this, SHOW_ERROR);
   item->title=title;
   item->msg=msg;
   queue->addQueueItem(item);
}

void
QDMessenger::getURL(const GURL & url, const GUTF8String &target)
{
   DEBUG_MSG("QDMessenger::getURL(): url='" << url <<
	     "', target='" << target << "'...\n");

   QDMessageQueue::Item * item=new QDMessageQueue::Item(this, GET_URL);
   item->url=url.get_string();
   item->target=target.length() ? target : GUTF8String();
   queue->addQueueItem(item);
}

void
QDMessenger::generalReq(int req)
{
   DEBUG_MSG("QDMessenger::generalReq(): req=" << req << "...\n");

   QDMessageQueue::Item * item=new QDMessageQueue::Item(this, GENERAL_REQ);
   item->req=req;
   queue->addQueueItem(item);
}

void
QDMessenger::generalMsg(const GUTF8String &qmsg)
{
   const char * const msg=qmsg;
   DEBUG_MSG("QDMessenger::generalMsg(): msg='" << msg << "'...\n");

   QDMessageQueue::Item * item=new QDMessageQueue::Item(this, GENERAL_MSG);
   item->msg=msg;
   queue->addQueueItem(item);
}

QDMessenger::QDMessenger(QWidget * parent, const char * name) :
      QObject(parent, name), look_ahead(false)
{
   DEBUG_MSG("QDMessenger::QDMessenger(): Initializing class\n");

   if (!queue) queue=new QDMessageQueue(0, "qd_message_queue");
}

QDMessenger::~QDMessenger(void)
{
   DEBUG_MSG("QDMessenger::~QDMessenger(): Destroying class and resources\n");
   queue->clearMessenger(this);
}
