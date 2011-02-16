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

#if defined(HAVE_COTHREAD) || !defined(AUTOCONF)
#include "GThreads.h"
#if THREADMODEL==COTHREADS

/* --------------- begin cothread specific code --------------- */

#include "qd_thr_yielder.h"
#include "GContainer.h"
#include "debug.h"

#include <qsocketnotifier.h>
#include <qtimer.h>
#include <qobject.h>
#include <unistd.h>
#include <fcntl.h>

Helper	*	QDThrYielder::helper=0;
int		QDThrYielder::tasks=1;

//*****************************************************************************
//********************************* Static ************************************
//*****************************************************************************

class Static
{
public:
   Static(void)
   {
      GThread::set_scheduling_callback(QDThrYielder::schedulingCB);
   }
};

static Static s;

//*****************************************************************************
//********************************* Helper ************************************
//*****************************************************************************

class Helper : public QObject
{
   Q_OBJECT
private:
   GMap<int, void *>	read_sockets;	// GMap<int, QSocketNotifier *> in fact
   GMap<int, void *>	write_sockets;	// GMap<int, QSocketNotifier *> in fact
   GMap<int, void *>	except_sockets;	// GMap<int, QSocketNotifier *> in fact
   QTimer timer;
private slots:
   void	slotTimeout(void);
   void	slotSocketNotifier(int);
public:
   void setHooks(bool wait);

   Helper(QObject * parent=0, const char * name=0);
   ~Helper(void);
};

Helper::Helper(QObject * parent, const char * name) :
      QObject(parent, name)
{
   connect(&timer, SIGNAL(timeout(void)), this, SLOT(slotTimeout(void)));

   setHooks(false);
}

Helper::~Helper(void)
{
   GPosition pos;
   for(pos=read_sockets;pos;++pos)
      delete (QSocketNotifier *) read_sockets[pos];
   for(pos=write_sockets;pos;++pos)
      delete (QSocketNotifier *) write_sockets[pos];
   for(pos=except_sockets;pos;++pos)
      delete (QSocketNotifier *) except_sockets[pos];
}

void
Helper::setHooks(bool wait)
{
   if (!wait) timer.start(0, TRUE);
   else
   {
	 // Now check all file descriptors: Since QT is not using
	 // GThread::select(), it has no idea that other threads may have
	 // selected some file descriptors for reading/writing/etc. Here we're
	 // kind of fixing this feature with QSocketNotifiers.
      int nfds;
      unsigned long timeout;
      fd_set read_fds, write_fds, except_fds;
      GThread::get_select(nfds, &read_fds, &write_fds, &except_fds, timeout);
      if (timeout>0 && nfds>0)
      {
	 for(int i=0;i<nfds;i++)
	 {
	    GPosition pos;
	    if (FD_ISSET(i, &read_fds))
	    {
	       if (!read_sockets.contains(i))
	       {
		  QSocketNotifier * sn=new QSocketNotifier(i, QSocketNotifier::Read);
		  connect(sn, SIGNAL(activated(int)), 
                          this, SLOT(slotSocketNotifier(int)));
		  read_sockets[i]=sn;
	       }
	    } else if (read_sockets.contains(i, pos))
	    {
	       delete (QSocketNotifier *) read_sockets[pos];
	       read_sockets.del(pos);
	    }

	    if (FD_ISSET(i, &write_fds))
	    {
	       if (!write_sockets.contains(i))
	       {
		  QSocketNotifier * sn=new QSocketNotifier(i, QSocketNotifier::Write);
		  connect(sn, SIGNAL(activated(int)), 
                          this, SLOT(slotSocketNotifier(int)));
		  write_sockets[i]=sn;
	       }
	    } else if (write_sockets.contains(i, pos))
	    {
	       delete (QSocketNotifier *) write_sockets[pos];
	       write_sockets.del(pos);
	    }

	    if (FD_ISSET(i, &except_fds))
	    {
	       if (!except_sockets.contains(i))
	       {
		  QSocketNotifier * sn 
                    = new QSocketNotifier(i, QSocketNotifier::Exception);
		  connect(sn, SIGNAL(activated(int)), 
                          this, SLOT(slotSocketNotifier(int)));
		  except_sockets[i]=sn;
	       }
	    } else if (except_sockets.contains(i, pos))
	    {
	       delete (QSocketNotifier *) except_sockets[pos];
	       except_sockets.del(pos);
	    }
	 } // for(i=0;i<nfds;i++)
      } // if (timeout>0 && nfds>0)

      timer.start(timeout, TRUE);
   }
}

void
Helper::slotTimeout(void)
{
   setHooks(GThread::yield()!=0);
}

void
Helper::slotSocketNotifier(int)
{
   setHooks(GThread::yield()!=0);
}

#include "qd_thr_yielder_moc.inc"

//*****************************************************************************
//****************************** QDThrYielder **********************************
//*****************************************************************************

void
QDThrYielder::schedulingCB(int cmd)
{
   if (cmd==GThread::CallbackTerminate) tasks--;
   else
   {
      if (cmd==GThread::CallbackCreate) tasks++;
      if (helper) helper->setHooks(false);
   }
}

bool
QDThrYielder::isInitialized(void)
{
   return helper!=0;
}

void
QDThrYielder::initialize(void)
{
      // Note, that QT should be initialized by this moment.
   if (!helper) helper=new Helper(0, "qd_thr_yielder_helper");
}

int
QDThrYielder::getTasksNum(void)
{
   return tasks;
}


/* --------------- end cothread specific code --------------- */
#endif /* THREADMODEL */
#endif /* HAVE_COTHREAD || !ASUTOCONF */
