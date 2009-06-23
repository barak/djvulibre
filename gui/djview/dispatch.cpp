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
// $Id: dispatch.cpp,v 1.18 2008/08/06 12:21:23 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#ifndef REPARENT
#define REPARENT 1
#endif

#include "DataPool.h"
#include "DjVuMessage.h"
#include "ZPCodec.h" // Hates to be included after QT stuff

#include "io.h"
#include "prefs.h"
#include "init_qt.h"
#include "names.h"
#include "dispatch.h"
#include "netscape.h"
#include "debug.h"
#include "qx_imager.h"
#include "qlib.h"
#include "mime_check.h"

#include <qtimer.h>
#include <sys/time.h>
#include <qapplication.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>


class PluginInstance
{
public:
  GP<DjVuViewer> djvu;
  QWidget *shell;
  bool	init_stream_passed;
  
  ~PluginInstance(void) { shell=0; }
  void attachWindow(QWidget * _shell) { shell=_shell; }
  void	detachWindow(void) { shell=0; }
  PluginInstance(GP<DjVuViewer> _djvu) 
    : djvu(_djvu), shell(0), init_stream_passed(false) {};
  PluginInstance(void) 
    : shell(0), init_stream_passed(false) {};
};

// The only reason why this class is here is to work as a receiver for
// getURL() and showStatus() signals from the DjVuViewer objects
class QDispatchObject : public QObject
{
   Q_OBJECT
private:
public slots:
   void	slotGetURL(const GURL & url, const GUTF8String &qtarget);
   void	slotShowStatus(const QString &qstatus);
   void	slotExitTimeout(void);
public:
   QDispatchObject(void) {}
   ~QDispatchObject(void) {}
};

class PoolDesc
{
public:
   GP<DataPool>	pool;
   bool	receiving;// Tells if we got NewStream for the pool already
   bool	internal; // TRUE if WE requested this stream with empty target
   PoolDesc(const GP<DataPool> & _pool) 
     : pool(_pool), receiving(false), internal(false) {}
};

static QDispatchObject *d_obj;
static QTimer *exit_timer;

static GMap<GURL, void *> pool_descs; // void* is PoolDesc*
static GMap<u_long, PluginInstance> instance;

static GCriticalSection	pool_descs_lock, instance_lock;
static GCriticalSection	rev_pipe_lock;

static void
request_data(const GURL & url, const GUTF8String &target, 
             const DjVuViewer * djvu)
  // WARNING! This function may be called from another thread
{
  DEBUG_MSG("<dispatcher>::request_data(): asking Netscape for data\n");
  DEBUG_MAKE_INDENT(3);
  
  if (rev_pipe>=0)
    {
      GCriticalSectionLock lock(&rev_pipe_lock);
      WriteInteger(rev_pipe, CMD_GET_URL);
      WritePointer(rev_pipe, djvu);
      WriteString(rev_pipe, (const char *)url.get_string());
      WriteString(rev_pipe, target.length() ? target : GUTF8String());
    }
}

static GP<DataPool>
request_data(const GURL & url_in, void * djvu)
  // WARNING! This function may be called from another thread
  // It's set as a request_data_cb() to DjVuViewer.
{
  DEBUG_MSG("<dispatcher>::request_data(): returning DataPool to the DjVuViewer\n");
  DEBUG_MAKE_INDENT(3);
  GURL url=url_in;
  url.clear_hash_argument();
  url.clear_djvu_cgi_arguments();
  GP<DataPool> pool=DataPool::create();
  bool ask_netscape=false;
  {
    GCriticalSectionLock lock(&pool_descs_lock);
    if (pool_descs.contains(url))
      {
        PoolDesc * pd=(PoolDesc *) pool_descs[url];
        DEBUG_MSG("returning cached DataPool\n");
        pool=pd->pool;
        if (!pd->receiving)
          {
            // We do the rerequest here because user might have pressed
            // the "Back" button after the stream was requested last time
            // and before Netscape actually returned it.
            // It won't hurt, anyway. We check for duplications in
            // NewStream().
	    DEBUG_MSG("rerequesting data from Netscape\n");
	    ask_netscape=true;
          }
      } 
    else
      {
        DEBUG_MSG("creating a new DataPool and requesting stream from Netscape\n");
        pool_descs[url]=new PoolDesc(pool=DataPool::create());
        ((PoolDesc *) pool_descs[url])->internal=true;
        ask_netscape=true;
      }
  }
  if (ask_netscape) 
    request_data(url, GUTF8String(), (DjVuViewer *) djvu);
  // Since DataPools stored in pool_descs can be shared between different
  // instances of the plugin, we need to create a private copy here.
  // After all, DjVuFile has a bad habit of stopping DataPools, which is
  // a bad thing to do, if it's shared
  return DataPool::create(pool);
}

void
QDispatchObject::slotGetURL(const GURL & url, const GUTF8String &qtarget)
{
  try
    {
      const QObject * obj=sender();
      if (!obj->inherits("DjVuViewer"))
        G_THROW(ERR_MSG("QDispatchObject.invalid_sender"));
      const DjVuViewer * djvu=(const DjVuViewer *) obj;
      request_data(url, qtarget, djvu);
    } 
  catch(const GException &exc)
    {
      exc.perror();
    }
}

void
QDispatchObject::slotShowStatus(const QString &qstatus)
{
  try
    {
      const QObject * obj=sender();
      if (!obj->inherits("DjVuViewer"))
        G_THROW(ERR_MSG("QDispatchObject.invalid_sender"));
      const DjVuViewer * djvu=(const DjVuViewer *) obj;
      GUTF8String gstatus = GStringFromQString(qstatus);
      gstatus = DjVuMessage::LookUpUTF8(gstatus);
      for (unsigned int i=0; i<gstatus.length(); i++)
        if (gstatus[i]=='\t' || gstatus[i]=='\n')
          gstatus.setat(i, ' ');
      const char *status = (const char*)gstatus;
      if (rev_pipe>=0)
        {
          GCriticalSectionLock lock(&rev_pipe_lock);
          WriteInteger(rev_pipe, CMD_SHOW_STATUS);
          WritePointer(rev_pipe, djvu);
          WriteString(rev_pipe, status);
        }
    } 
  catch(const GException &exc)
    {
      exc.perror();
    }
}

void
QDispatchObject::slotExitTimeout(void)
{
  DEBUG_MSG("QDispatchObject::slotExitTimeout(): quitting the application\n");
  DEBUG_MAKE_INDENT(3);
  qApp->exit(0);
}

static void
Shutdown(void)
{
  DEBUG_MSG("Shutdown() called\n");
  DEBUG_MAKE_INDENT(3);
  DEBUG_MSG("clearing all instances.\n");
  GCriticalSectionLock lock(&instance_lock);
  instance.empty();
  DEBUG_MSG("installing exit timer\n");
  if (!d_obj)
    {
      // select() code in netscape.cpp will take this into account
      delete exit_tv;
      exit_tv=new timeval;
      exit_tv->tv_sec=INACTIVITY_TIMEOUT/1000;
      exit_tv->tv_usec=0;
   } 
  else
    {
      if (!exit_timer)
        {
          exit_timer=new QTimer();
          QObject::connect(exit_timer, SIGNAL(timeout(void)),
                           d_obj, SLOT(slotExitTimeout(void)));
        }
      exit_timer->start(INACTIVITY_TIMEOUT);
    }
}

static void
New(void)
{
  DEBUG_MSG("New() called\n");
  DEBUG_MAKE_INDENT(3);
  GCriticalSectionLock lock(&instance_lock);
  GP<DjVuViewer> djvu;
  try
    {
      bool full_mode=ReadInteger(pipe_read);
      ReadString(pipe_read); // djvu_dir (now useless)
      int argc=ReadInteger(pipe_read);
      DArray<GUTF8String> argn(argc-1), argv(argc-1);
      for(int i=0;i<argc;i++)
        {
          argn[i]=ReadString(pipe_read);
          argv[i]=ReadString(pipe_read);
        }
      SavedData saved;
      if (ReadInteger(pipe_read))
        {
          saved.cmd_mode=ReadInteger(pipe_read);
          saved.cmd_zoom=ReadInteger(pipe_read);
          saved.imgx=ReadInteger(pipe_read);
          saved.imgy=ReadInteger(pipe_read);
        }
      QDViewer::PluginData plugin_data(argn, argv);
      plugin_data.full_mode=full_mode;
      plugin_data.saved=saved;
      djvu=new DjVuViewer(1, plugin_data); // 1=in_netscape
      if (!d_obj) d_obj=new QDispatchObject();
      QObject::connect(djvu, SIGNAL(sigShowStatus(const QString &)),
                       d_obj, SLOT(slotShowStatus(const QString &)));
      QObject::connect(djvu, SIGNAL(sigGetURL(const GURL &, const GUTF8String &)),
                       d_obj, SLOT(slotGetURL(const GURL &, const GUTF8String &)));
      djvu->setRequestDataCB(request_data, djvu);
      instance[(u_long) (void *) djvu]=PluginInstance(djvu);
      WriteString(pipe_write, OK_STRING);
      WritePointer(pipe_write, djvu);
    } 
  catch(const GException & exc)
    {
      const char *msg = exc.get_cause();
      if (! GException::cmp_cause(msg, PipeError::Tag))
        {
          exc.perror();
          instance.empty();
          PipesAreDead();
        }
      else
        {
          if (djvu)
            if (instance.contains((u_long) (void *) djvu))
              instance.del((u_long) (void *) djvu);
          WriteString(pipe_write, ERR_STRING);
          WriteString(pipe_write, exc.get_cause());
        }
    }
}

static void
DetachWindow(void)
{
  DEBUG_MSG("DetachWindow() called\n");
  DEBUG_MAKE_INDENT(3);
  try
    {
      DjVuViewer * djvu=(DjVuViewer *) ReadPointer(pipe_read);
      if (instance.contains((u_long) djvu))
        {
          PluginInstance & inst=instance[(u_long) djvu];
          if (inst.shell)
            {
              djvu->detach();
#ifdef REPARENT
              Display * displ=inst.shell->x11Display();
              Window shell_win=inst.shell->winId();
              XUnmapWindow(displ, shell_win);
              XReparentWindow(displ, shell_win, 
                              QApplication::desktop()->winId(), 0,0);
              XSync(displ, False);
#endif
              qeApp->killWidget(inst.shell);
            }
          inst.detachWindow();
        }
      WriteString(pipe_write, OK_STRING);
    } 
  catch(const GException & exc)
    {
      const char *msg = exc.get_cause();
      if (! GException::cmp_cause(msg, PipeError::Tag))
        {
          exc.perror();
          instance.empty();
          PipesAreDead();
        }
      else
        {
          WriteString(pipe_write, ERR_STRING);
          WriteString(pipe_write, exc.get_cause());
        }
    }
}

static void
AttachWindow(void)
{
  DEBUG_MSG("AttachWindow() called\n");
  DEBUG_MAKE_INDENT(3);
  QWidget *shell=0;
  DjVuViewer *djvu=0;
  try
    {
      GCriticalSectionLock lock(&instance_lock);
      djvu=(DjVuViewer *) ReadPointer(pipe_read);
      GUTF8String displ_name=ReadString(pipe_read);
      GUTF8String back_color=ReadString(pipe_read);
      Window window=(Window) ReadInteger(pipe_read);
      ReadInteger(pipe_read); // colormap (no longer used)
      ReadInteger(pipe_read); // visual_id (no longer used)
      int width=ReadInteger(pipe_read);
      int height=ReadInteger(pipe_read);
      if (instance.contains((u_long) djvu))
        {
          if (!qApp)
            {
              // This is first connection to the display 
              // => Initialize the toolkit
              DEBUG_MSG("checking connection to the display " 
                        << (const char *) displ_name << "\n");
              Display *displ=XOpenDisplay(displ_name);
              if (!displ) 
                G_THROW(ERR_MSG("AttachWindow.open_display_fail") 
                        + GUTF8String("\t") 
                        + GUTF8String(XDisplayName(displ_name)));
              // Build arguments
              int argc = 0;
              static char * argv[10];
              argv[argc++] = (char*)(DJVIEW_NAME);
              // -display
              static char m_display[] = "-display\0";
              static char s_display[128];
              if (displ_name.length()>0 
                  && displ_name.length()<sizeof(s_display)-1)
                {
                  strcpy(s_display, (const char*)displ_name);
                  argv[argc++] = m_display;
                  argv[argc++] = s_display;
                }
#ifdef COPY_BACKGROUND_COLOR
              // -bg
              static char m_bg[] = "-bg\0";
              static char s_bg[128];
              if (back_color.length()>0 && back_color.length()<sizeof(s_bg)-1)
                {
                  strcpy(s_bg, (const char*)back_color);
                  argv[argc++]= m_bg;
                  argv[argc++] = s_bg;
                }
              // - one could check the colormap
              //   and the visual id to add more options.
#endif
              // Initialize Qt
              InitializeQT(argc, argv);
              checkMime();
              // Close display
              XCloseDisplay(displ);
            }
          shell=new QWidget(0, "djvu_shell");
          shell->setGeometry(0, 0, width, height);
#ifdef REPARENT
          Display *displ = qt_xdisplay();
          XSync(displ, False);
          XReparentWindow(displ, shell->winId(), window, 0, 0);
#endif
          djvu->attach(shell);
          shell->show();
          
          PluginInstance & inst=instance[(u_long) djvu];
          inst.attachWindow(shell);
        } // if instance is valid
      WriteString(pipe_write, OK_STRING);
    } 
  catch(const GException & exc)
    {
      const char *msg = exc.get_cause();
      if (! GException::cmp_cause(msg, PipeError::Tag))
        {
          exc.perror();
          instance.empty();
          PipesAreDead();
        }
      else
        {
          if (instance.contains((u_long) djvu))
            {
              djvu->detach();
              instance[(u_long) djvu].detachWindow();
            }
          if (shell) 
            qeApp->killWidget(shell);
          WriteString(pipe_write, ERR_STRING);
          WriteString(pipe_write, exc.get_cause());
        }
    }
}

static void
Resize(void)
{
  DEBUG_MSG("Resize() called\n");
  DEBUG_MAKE_INDENT(3);
  try
    {
      GCriticalSectionLock lock(&instance_lock);
      DjVuViewer * djvu=(DjVuViewer *) ReadPointer(pipe_read);
      int width=ReadInteger(pipe_read);
      int height=ReadInteger(pipe_read);
      if (instance.contains((u_long) djvu))
        {
          PluginInstance & inst=instance[(u_long) djvu];
          if (inst.shell) inst.shell->resize(width, height);
        }
      WriteString(pipe_write, OK_STRING);
    }
  catch(const GException & exc)
    {
      const char *msg = exc.get_cause();
      if (! GException::cmp_cause(msg, PipeError::Tag))
        {
          exc.perror();
          instance.empty();
          PipesAreDead();
        }
      else
        {
          WriteString(pipe_write, ERR_STRING);
          WriteString(pipe_write, exc.get_cause());
        }
    }
}

static void
Destroy(void)
{
   DEBUG_MSG("Destroy() called\n");
   DEBUG_MAKE_INDENT(3);
   try
     {
       GCriticalSectionLock lock(&instance_lock);
       SavedData saved;
       DjVuViewer * djvu=(DjVuViewer *) ReadPointer(pipe_read);
       if (instance.contains((u_long) djvu))
	 {
           PluginInstance & inst=instance[(u_long) djvu];
           saved=djvu->getSavedData();
           if (inst.shell)
             {
	       djvu->detach();
#ifdef REPARENT
	       Display * displ=inst.shell->x11Display();
	       Window shell_win=inst.shell->winId();
	       XUnmapWindow(displ, shell_win);
	       XReparentWindow(displ, shell_win, 
                               QApplication::desktop()->winId(), 0,0);
	       XSync(displ, False);
#endif
	       qeApp->killWidget(inst.shell);
             }
           inst.detachWindow();
           instance.del((u_long) djvu);
	 }
       WriteString(pipe_write, OK_STRING);
       WriteInteger(pipe_write, saved.cmd_mode);
       WriteInteger(pipe_write, saved.cmd_zoom);
       WriteInteger(pipe_write, saved.imgx);
       WriteInteger(pipe_write, saved.imgy);
     }
   catch(const GException & exc)
     {
       const char *msg = exc.get_cause();
      if (! GException::cmp_cause(msg, PipeError::Tag))
        {
          exc.perror();
          instance.empty();
          PipesAreDead();
        }
      else
        {
          WriteString(pipe_write, ERR_STRING);
          WriteString(pipe_write, exc.get_cause());
        }
     }
}

static void
Print(void)
{
   DEBUG_MSG("Print() called\n");
   DEBUG_MAKE_INDENT(3);
   try
     {
       GCriticalSectionLock lock(&instance_lock);
       DjVuViewer * djvu=(DjVuViewer *) ReadPointer(pipe_read);
       ReadInteger(pipe_read);	// full_mode
       if (instance.contains((u_long) djvu))
	 {
           // TODO: use 'full_mode' somehow
           QDViewer * viewer=djvu->getQDViewer();
           if (viewer) viewer->print();
	 }
       WriteString(pipe_write, OK_STRING);
     } 
   catch(const GException & exc)
     {
       const char *msg = exc.get_cause();
       if (! GException::cmp_cause(msg, PipeError::Tag))
         {
           exc.perror();
           instance.empty();
           PipesAreDead();
         }
       else
         {
           WriteString(pipe_write, ERR_STRING);
           WriteString(pipe_write, exc.get_cause());
         }
     }
}

static void
NewStream(void)
{
   DEBUG_MSG("NewStream() called\n");
   DEBUG_MAKE_INDENT(3);
   try
     {
       GCriticalSectionLock plock(&pool_descs_lock);
       GCriticalSectionLock ilock(&instance_lock);
       GP<DataPool> pool;
       DjVuViewer * djvu=(DjVuViewer *) ReadPointer(pipe_read);
       GURL url=GURL::UTF8(ReadString(pipe_read));
       if (instance.contains((u_long) djvu))
	 {
           PluginInstance & inst=instance[(u_long) djvu];
           DEBUG_MSG("url='" << url << "'\n");
           // pool_url uniquely identifies data
           GURL pool_url=url;
           pool_url.clear_hash_argument();
           pool_url.clear_djvu_cgi_arguments();
           if (pool_descs.contains(pool_url))
             {
	       PoolDesc * pd=(PoolDesc *) pool_descs[pool_url];
	       if (pd->receiving)
                 {
                   // We have this DataPool created and it is
                   // already receiving data. The only case when we
                   // need to do anything with it is when it's also an
                   // initial stream for this instance and we MUST pass
                   // it to the DjVuViewer
                   if (!inst.init_stream_passed)
                     {
                       djvu->newStream(url, DataPool::create(pd->pool));
                       inst.init_stream_passed=true;
                     }
                 } 
               else
                 {
                   // The DataPool already exists and is NOT receiving
                   // data. Return ID back to Netscape and prepare for NPP_Write
                   pool=pd->pool;
                 }
             } 
           else
             {
               // Apparently, initial stream (because nobody asked for it) =>
		  // Pass it to the DjVuViewer. Otherwise the PoolDesc should
		  // have been created in slotGetURL()
	       pool_descs[pool_url]=new PoolDesc(pool=DataPool::create());
               
	       djvu->newStream(url, DataPool::create(pool));
	       inst.init_stream_passed=true;
             }
           ((PoolDesc *) pool_descs[pool_url])->receiving=true;
	 } // if instance is legal
       WriteString(pipe_write, OK_STRING);
       WritePointer(pipe_write, pool);
     } 
   catch(const GException & exc)
     {
       const char *msg = exc.get_cause();
       if (! GException::cmp_cause(msg, PipeError::Tag))
         {
           exc.perror();
           instance.empty();
           PipesAreDead();
         }
       else
         {
           WriteString(pipe_write, ERR_STRING);
           WriteString(pipe_write, exc.get_cause());
         }
     }
}

static void
Write(void)
{
   DEBUG_MSG("Write() called\n");
   DEBUG_MAKE_INDENT(3);
   try
     {
       DataPool * pool=(DataPool *) ReadPointer(pipe_read);
       TArray<char> array=ReadArray(pipe_read);
       GURL url;
       bool found=false;
       bool internal=false;
       {
         GCriticalSectionLock lock(&pool_descs_lock);
         for(GPosition pos=pool_descs;pos;++pos)
           if (((PoolDesc *) pool_descs[pos])->pool==pool)
             {
               found=true;
               url=pool_descs.key(pos);
               internal=((PoolDesc *) pool_descs[pos])->internal;
               break;
             }
       }
       // If this is a stream, that we have requested, it's a good idea to
       // check its contents. If it's not a DjVu file, we will redirect the
       // stream to the Netscape. Most often the file is in invalid format
       // because it's not found and Netscape returns us just an HTML page
       // saying, that the file is not found. If this is the case, the best
       // thing is to ask Netscape load it again in frame '_self', so that the
       // user will see this HTML message.  One problem is that if this file
       // has DjVu extension then Netscape will attempt to use the plugin
       // (again) to display it.  So we check for the "internal" flag,
       // which is true if:
       //   1. The file was requested by us
       //   2. It was requested with empty "target"
       bool check_signature=false;
       char signature[8];
       if (found && internal && pool->get_size()<8)
	 {
           if (pool->get_size()+array.size()>=8)
             {
	       check_signature=true;
	       int s=pool->get_size();
	       if (s) pool->get_data(signature, 0, s);
	       if (s<8) memcpy(signature+s, (const char *) array, 8-s);
             }
	 }
       if (check_signature)
	 {
           // Check if it's DjVu data
           if (!strncmp(signature, "FORM", 4))
             {
               d_obj->slotShowStatus("Missing signature in DjVu file.");
             }
           else if (strncmp(signature, "AT&TFORM", 8))
             {
               // Not a DjVu file :(
               // First - STOP the DataPool
	       GP<DjVuViewer> djvu;
	       {
                 GCriticalSectionLock lock(&instance_lock);
                 djvu=instance[instance].djvu;
	       }
	       {
                 GCriticalSectionLock lock(&pool_descs_lock);
                 delete (PoolDesc *) pool_descs[url];
                 pool_descs.del(url);
                 pool->stop();
	       }
               // Set the 'found' to FASLSE. This will tell
               // the nsdejavu.so to stop sending the data.
	       found=false;
	       bool redirect=true;
               // Now see, if we need to display the error, or we have to keep
               // compatibility with npdjvu-2.0, which silently ignored the
               // absence of DjVu document directory. We will get a pointer to
               // the DjVuDocument, check if it's an OLD_INDEXED document, and
               // will ignore the error if so.
	       GP<DjVuDocument> djvu_doc=djvu->getDjVuDocument();
	       int doc_type=djvu_doc->get_doc_type();
	       if (doc_type==DjVuDocument::SINGLE_PAGE ||
		   doc_type==DjVuDocument::OLD_BUNDLED)
                 {
                   // Compatibility mode. Just make sure that the
                   // name of the stream is not a page name,
                   // in which case we still need to issue an error
                   int page_num=djvu_doc->url_to_page(url);
                   if (page_num<0)
                     {
                       // Yep. It's not a page. So it must be a directory.
                       // IGNORE the fact that it does not exist.
                       redirect=false;
                     }
                 }
	       if (redirect)
                 request_data(url, "_self", djvu);
             }
	 }
       if (found)
         pool->add_data((const char *) array, array.size());
       WriteString(pipe_write, OK_STRING);
       WriteInteger(pipe_write, found ? array.size() : 0);
       // When nsdejavu.so sees ZERO return value, it will get rid
       // of this stream and will never try to pipe its data again
     } 
   catch(const GException & exc)
     {
       const char *msg = exc.get_cause();
       if (! GException::cmp_cause(msg, PipeError::Tag))
         {
           exc.perror();
           GCriticalSectionLock lock(&instance_lock);
           instance.empty();
           PipesAreDead();
         }
       else
         {
           WriteString(pipe_write, ERR_STRING);
           WriteString(pipe_write, exc.get_cause());
         }
     }
}

static void
DestroyStream(void)
{
  DEBUG_MSG("DestroyStream() called\n");
  DEBUG_MAKE_INDENT(3);
  try
    {
      DataPool * pool=(DataPool *) ReadPointer(pipe_read);
      bool ok = ReadInteger(pipe_read);
      GP<DataPool> data_pool;
      {
        GCriticalSectionLock lock(&pool_descs_lock);
        for(GPosition pos=pool_descs;pos;++pos)
          if (((PoolDesc *) pool_descs[pos])->pool==pool)
            {
              data_pool=pool;
              delete (PoolDesc *) pool_descs[pos];
              pool_descs.del(pos);
              break;
            }
      }
      if (data_pool)
        {
          if (ok) 
            data_pool->set_eof();
          else 
            data_pool->stop(true); 
        }
      WriteString(pipe_write, OK_STRING);
    }  
  catch(const GException & exc)
    {
      const char *msg = exc.get_cause();
      if (! GException::cmp_cause(msg, PipeError::Tag))
        {
	 exc.perror();
	 GCriticalSectionLock lock(&instance_lock);
	 instance.empty();
	 PipesAreDead();
        }
      else
        {
          WriteString(pipe_write, ERR_STRING);
          WriteString(pipe_write, exc.get_cause());
        }
    }
}

static void
URLNotify(void)
{
  DEBUG_MSG("URLNotify() called\n");
  DEBUG_MAKE_INDENT(3);
  try
    {
      GURL url=GURL::UTF8(ReadString(pipe_read));
      ReadInteger(pipe_read);	// status (not used)
      GCriticalSectionLock lock(&pool_descs_lock);
      GPosition pos;
      if (pool_descs.contains(url, pos))
        {
          ((PoolDesc *) pool_descs[pos])->pool->set_eof();
          delete (PoolDesc *) pool_descs[pos];
          pool_descs.del(pos);
        }
      WriteString(pipe_write, OK_STRING);
    }
  catch(const GException & exc)
    {
      const char *msg = exc.get_cause();
      if (! GException::cmp_cause(msg, PipeError::Tag))
        {
          exc.perror();
          GCriticalSectionLock lock(&instance_lock);
          instance.empty();
          PipesAreDead();
        }
      else
        {
          WriteString(pipe_write, ERR_STRING);
          WriteString(pipe_write, exc.get_cause());
        }
    }
}

static void
Handshake(void)
{
  DEBUG_MSG("Handshake() called\n");
  DEBUG_MAKE_INDENT(3);
  WriteString(pipe_write, OK_STRING);
}

void
Dispatch(void)
{
   DEBUG_MSG("Dispatch(): Reading request and data from stdin\n");
   DEBUG_MAKE_INDENT(3);
   try
     {
       // Stop the timer since we have some activity in the pipe.
       if (exit_timer) 
         exit_timer->stop();
       delete exit_tv; 
       exit_tv=0;
       int req=ReadInteger(pipe_read);
       switch(req)
         {
         case CMD_SHUTDOWN:
           Shutdown();
           break;
         case CMD_NEW:
           New();
           break;
         case CMD_DETACH_WINDOW:
           DetachWindow();
           break;
         case CMD_ATTACH_WINDOW:
           AttachWindow();
           break;
         case CMD_RESIZE:
           Resize();
           break;
         case CMD_DESTROY:
           Destroy();
           break;
         case CMD_PRINT:
           Print();
           break;
         case CMD_NEW_STREAM:
           NewStream();
           break;
         case CMD_WRITE:
           Write();
           break;
         case CMD_DESTROY_STREAM:
           DestroyStream();
           break;
         case CMD_URL_NOTIFY:
           URLNotify();
           break;
         case CMD_HANDSHAKE:
           Handshake();
           break;
         default:
           G_THROW(ERR_MSG("Dispatch.unknown_pipe_read_request"));
         }
     } 
   catch(GException &exc)
     {
       const char *msg = exc.get_cause();
       if (! GException::cmp_cause(msg, PipeError::Tag))
         {
           instance.empty();
           PipesAreDead();
         }
       else
         {
           G_RETHROW;
         }
     }
}

#include "dispatch_moc.inc"
