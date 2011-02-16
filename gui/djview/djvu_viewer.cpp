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

#include "djvu_viewer.h"

#include "debug.h"
#include "GURL.h"
#include "ByteStream.h"
#include "DataPool.h"
#include "DjVuMessage.h"
#include "qlib.h"
#include "djvu_file_cache.h"

#if THREADMODEL==COTHREADS
#include "qd_thr_yielder.h"
#endif

#include <qapplication.h>
#include <qmessagebox.h>


//****************************************************************************
//***************************** DjVuViewerPort *******************************
//****************************************************************************

GP<DataPool>
DjVuViewerPort::request_data(const DjVuPort * source, const GURL & url)
{
   GMonitorLock lock(&disabled);
   if (disabled) return 0;
   else return viewer->requestData(url);
}

bool
DjVuViewerPort::notify_error(const DjVuPort * source, const GUTF8String &qmsg)
{
   const char * const msg=qmsg;
   GMonitorLock lock(&disabled);
   if (!disabled)
      viewer->messenger.showError("DjVu Error", msg);
   return 1;
}

bool
DjVuViewerPort::notify_status(const DjVuPort * source, const GUTF8String &qmsg)
{
  GMonitorLock lock(&disabled);
  if (!disabled)
    viewer->messenger.showStatus(QStringFromGString(qmsg));
  return 1;
}

//****************************************************************************
//******************************* DjVuViewer *********************************
//****************************************************************************

SavedData
DjVuViewer::getSavedData(void)
{
   DEBUG_MSG("DjVuViewer::getSavedData(): returning saved_data:\n");
   DEBUG_MAKE_INDENT(3);
   
   if (viewer) plugin_data.saved=viewer->getSavedData();

   DEBUG_MSG("cmd_mode=" << plugin_data.saved.cmd_mode << "\n");
   DEBUG_MSG("cmd_zoom=" << plugin_data.saved.cmd_zoom << "\n");
   DEBUG_MSG("imgx=" << plugin_data.saved.imgx << "\n");
   DEBUG_MSG("imgy=" << plugin_data.saved.imgy << "\n");
   return plugin_data.saved;
}

DjVuViewer::DjVuViewer(int _in_netscape, const QDViewer::PluginData & _plugin_data) :
      QObject(0, "djvu_viewer"), plugin_data(_plugin_data), in_netscape(_in_netscape)
{
   DEBUG_MSG("DjVuViewer::DjVuViewer(): initializing class...\n");
   DEBUG_MAKE_INDENT(3);

   request_data_cb=0;
   request_data_cl_data=0;

   attach_postpone=false;
   
   port=new DjVuViewerPort(this);
   
   viewer=0;

#if THREADMODEL==COTHREADS
   if (!QDThrYielder::isInitialized()) QDThrYielder::initialize();
#endif

   messenger.setLookAhead(1);
   
      // Connect signals of the QDMessenger
   connect(&messenger, SIGNAL(sigGetURL(const GURL &, const GUTF8String &)),
	   this, SIGNAL(sigGetURL(const GURL &, const GUTF8String &)));
   connect(&messenger, SIGNAL(sigShowError(const GUTF8String &, const GUTF8String &)),
	   this, SLOT(slotShowError(const GUTF8String &, const GUTF8String &)));
   connect(&messenger, SIGNAL(sigShowStatus(const QString &)),
	   this, SIGNAL(sigShowStatus(const QString &)));
}

DjVuViewer::~DjVuViewer(void)
{
   DEBUG_MSG("DjVuViewer::~DjVuViewer(): destroying class\n");
   DEBUG_MAKE_INDENT(3);

   port->disabled=1;
   port=0;
   
   if (document) document->stop_init();
   detach();
}

void
DjVuViewer::slotViewerDestroyed(void)
{
   viewer=0;
}

void
DjVuViewer::slotShowError(const GUTF8String &title, const GUTF8String &msg)
{
  if (!GException::cmp_cause(msg, ByteStream::EndOfFile))
    {
     static const QString mesg=tr("Unexpected end of file encountered");
     emit sigShowStatus(mesg);
    }
  else if (GException::cmp_cause(msg, DataPool::Stop))
    {
      QString qtitle = QStringFromGString(title);
      QString qmsg = QStringFromGString(DjVuMessage::LookUpUTF8(msg));
      ::showError(viewer, qtitle, qmsg);
    }
}

bool
DjVuViewer::eventFilter(QObject * obj, QEvent * ev)
{
   try
   {
      if (obj->isWidgetType())
      {
	 QWidget * w=(QWidget *) obj;
	 if (ev->type()==QEvent::Resize && viewer)
	 {
	    DEBUG_MSG("DjVuViewer::eventFilter(): resizing the viewer...\n");
	    viewer->resize(w->size());
	 };
      };
   } catch(const GException & exc)
   {
      showError("DjVu Error", exc.get_cause());
   };
   return FALSE;
}

void
DjVuViewer::attach(QWidget * parent)
{
   DEBUG_MSG("DjVuViewer::attach(): attaching to parent=" << parent << "\n");
   DEBUG_MAKE_INDENT(3);

   qeApp->killWidget(viewer); viewer=0;
   
   if (!doc_pool)
   {
	 // Postpone attachment until the data is known.
      attach_postpone=true;
      attach_parent=parent;
   } else
   {
      viewer=new QDViewer(in_netscape, plugin_data, parent, "qd_viewer");
      connect(viewer, SIGNAL(destroyed(void)), this, SLOT(slotViewerDestroyed(void)));
      connect(viewer, SIGNAL(sigShowStatus(const QString &)),
	      this, SIGNAL(sigShowStatus(const QString &)));
      connect(viewer, SIGNAL(sigGetURL(const GURL &, const GUTF8String &)),
	      this, SIGNAL(sigGetURL(const GURL &, const GUTF8String &)));

      viewer->resize(parent->size());
      viewer->show();
      parent->installEventFilter(this);

	 // What if we're being reattached by Netscape? Set DjVuImage immediately.
      if (document)
        viewer->setDjVuDocument(document, plugin_data.page_id);
   }
}

void
DjVuViewer::detach(void)
{
   DEBUG_MSG("DjVuViewer::detach(): detaching viewer\n");
   DEBUG_MAKE_INDENT(3);

      // Finally destroying the viewer
   if (viewer)
   {
      GP<DjVuImage> dimg=viewer->getDjVuImage();
      if (dimg && document && document->is_init_complete())
        {
          GURL url=dimg->get_djvu_file()->get_url();
          int page_num=document->url_to_page(url);
          if (page_num>=0) 
            plugin_data.page_id.format("#$%d", page_num+1);
        }
      plugin_data.saved=viewer->getSavedData();
      viewer->parentWidget()->removeEventFilter(this);
      qeApp->killWidget(viewer); viewer=0;
   }
}

GP<DataPool>
DjVuViewer::requestData(const GURL & url)
{
   if (url==doc_url) return doc_pool;
   else if (request_data_cb)
      return request_data_cb(url, request_data_cl_data);
   return 0;
}

void
DjVuViewer::newStream(const GURL & _url, const GP<DataPool> & pool)
      // This function should be called from the main thread ONLY
{
   DEBUG_MSG("DjVuViewer::newStream(): Got stream for URL=" << _url << "\n");
   DEBUG_MAKE_INDENT(3);

   static const QString qspace(" ");
   showStatus(qspace);	// To kill messages like "Requesting directory..."

   GURL url=_url;

   if (!document)
   {
      DEBUG_MSG("First stream => decode thread should be started...\n");

      GUTF8String key=url.hash_argument();
      if (key.length()) plugin_data.page_id=key;
      else if (url.cgi_arguments())
      {
	 DArray<GUTF8String> cgi_names=url.djvu_cgi_names();
	 DArray<GUTF8String> cgi_values=url.djvu_cgi_values();
	 if (cgi_names.size()>0)
	 {
	    plugin_data.parse(cgi_names, cgi_values);
	 }
      }

      DjVuFileCache * cache=get_file_cache();

      GUTF8String ext=url.extension();
      
      if (ext!="djvu" && ext!="djv")
	 if (plugin_data.cache!=QDViewer::PluginData::CACHE_ON)
	    plugin_data.cache=QDViewer::PluginData::CACHE_OFF;
      
      if (plugin_data.cache==QDViewer::PluginData::CACHE_OFF) cache=0;
      
      url.clear_all_arguments();
      doc_pool=pool;
      doc_url=url;

      document=DjVuDocument::create(url, (DjVuPort *) port, cache);
      
      if (attach_postpone)
      {
	 attach_postpone=false;
	 attach(attach_parent);
      } else if (viewer)
      {
	 viewer->setDjVuDocument(document, plugin_data.page_id);
      }
   } else
   {
      DEBUG_MSG("Unexpected stream for url='" << url << "' :(\n");
   }
}
