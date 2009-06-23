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
// $Id: netscape.cpp,v 1.10 2007/03/25 20:48:23 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "ZPCodec.h"		// Hates to be included after QT stuff

#include "netscape.h"
#include "debug.h"
#include "dispatch.h"
#include "io.h"

//  #ifdef DJVU_EDITOR
//  #include "djvu_terminator.h"
//  #endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <errno.h>

#include <qsocketnotifier.h>
#include <qapplication.h>

#include "init_qt.h"

// This object is here just to provide a way to connect to socket notifiers
class QDWatcher : public QObject
{
   Q_OBJECT
public slots:
   void		slotInput(int);
public:
   QDWatcher(void) {}
   ~QDWatcher(void) {}
};

int	pipe_read=3, pipe_write=4;
int	rev_pipe=5;
struct timeval	* exit_tv=0;

static QSocketNotifier	*input_sn;
static QDWatcher * watcher;

// This is the thread, which will be processing stream-related requests.
// The point is to minimize the chance of dead locks: With this thread the system will
// work find even if the main thread requests data, which is not available.
// There still may be dead locks though because requests like NPP_New/NPP_AttachWindow,
// etc. must be processed by the main thread sychronously.
static GThread	str_thread;

void
PipesAreDead(void)
{
   DEBUG_MSG("PipesAreDead(): Killing "
             "the rest of pipes and removing input handlers\n");
   delete input_sn; input_sn=0;
   close(pipe_read); pipe_read=-1;
   close(pipe_write); pipe_write=-1;
   close(rev_pipe); rev_pipe=-1;
      // Don't close the watcher here: you may be called from its slot.
      // delete watcher; watcher=0;

   if (qApp) qApp->exit(1);
   else exit(1);
}

void
QDWatcher::slotInput(int)
{
   DEBUG_MSG("QDWatcher::slotInput() called: "
             "smth is in the netscape pipe\n");
   try
     {
       Dispatch();
     } 
   catch(const GException & exc)
     {
       exc.perror();
       const char *msg = exc.get_cause();
       if (!GException::cmp_cause(msg, PipeError::Tag))
         PipesAreDead();
     } 
}

void
WorkWithNetscape(void)
{
   DEBUG_MSG("WorkWithNetscape(): "
             "working together with netscape\n");

   WriteString(pipe_write, "Here am I");	// Plugin is waiting for this

//  #ifdef DJVU_EDITOR
//     DjVuTerminator::AddPipe(pipe_read);
//     DjVuTerminator::AddPipe(pipe_write);
//     DjVuTerminator::AddPipe(rev_pipe);
//  #endif

   exit_tv=new timeval;
   exit_tv->tv_sec=INACTIVITY_TIMEOUT/1000;
   exit_tv->tv_usec=0;
   
   while(true)
   {
      if (qApp)
      {
	 if (!watcher)
	 {
	    watcher=new QDWatcher();
	    input_sn=new QSocketNotifier(pipe_read, QSocketNotifier::Read,
					 watcher, "input_sn");
	    input_sn->setEnabled(TRUE);
	    QObject::connect(input_sn, SIGNAL(activated(int)),
			     watcher, SLOT(slotInput(int)));
	 }
	 int rc=qApp->exec();
	 DEBUG_MSG("QApplication::exec() returned rc=" << rc << "\n");
         CleanupQT();
	 exit(rc);
      }
      fd_set read_fds;
      FD_ZERO(&read_fds);
      FD_SET(pipe_read, &read_fds);
      int rc=select(pipe_read+1, &read_fds, 0, 0, exit_tv);
      if (rc<0 && errno!=EINTR)
	 G_THROW(ERR_MSG("WorkWithNetscape.pipe_listen_fail"));
      if (rc==0)
        {
          DEBUG_MSG("Shutdown timeout => exit\n");
          exit(0);
        }
      if (rc>0 && FD_ISSET(pipe_read, &read_fds)) 
        Dispatch();
   }
}

#include "netscape_moc.inc"
