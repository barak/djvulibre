//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.5)
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
// 
// $Id$
// $Name$

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
/** This file impliments the DjVuProgressTask elements.  The memory
    functions are implimented in a separate file, because only the memory
    functions should be compiled with out overloading of the memory functions.
 */
  

#ifdef NEED_DJVU_PROGRESS
#include "DjVuGlobal.h"


// ----------------------------------------

#include "GOS.h"
#include "GThreads.h"
#include "GException.h"
#include "GContainer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INITIAL  500
#define INTERVAL 250

class DjVuProgressTask::Data : public GPEnabled
{
public:
  djvu_progress_callback *callback;
  DjVuProgressTask *head;
  const char *gtask;
  unsigned long lastsigdate;
  Data(djvu_progress_callback *_callback):
    callback(_callback), head(0), gtask(0), lastsigdate(0) {}
};

  
static GMap<void *,GP<DjVuProgressTask::Data> > &
get_map(void)
{
  static GMap<void *,GP<DjVuProgressTask::Data> > &map=
    GMap<void *,GP<DjVuProgressTask::Data> >::static_reference(); 
  return map;
}

djvu_progress_callback *
DjVuProgressTask::set_callback(djvu_progress_callback *_callback)
{ 
  djvu_progress_callback *retval=0;
  if(_callback)
  {
    GMap<void *,GP<DjVuProgressTask::Data> > &map=get_map();
    void *threadID=GThread::current();
    if(map.contains(threadID))
    {
      DjVuProgressTask::Data &data=*(map[threadID]);
      retval=data.callback;
      data.callback=_callback;
      data.head=0;
      data.gtask=0;
      data.lastsigdate=0;
    }else
    {
      map[threadID]=new Data(_callback);
    }
  }else
  {
    GMap<void *,GP<DjVuProgressTask::Data> > &map=get_map();
    void *threadID=GThread::current();
    if(map.contains(threadID))
    {
      DjVuProgressTask::Data &data=*(map[threadID]);
      retval=data.callback;
      data.callback=0;
      data.head=0;
      data.gtask=0;
      data.lastsigdate=0;
      map.del(threadID);
    }
  }
  return retval;
}

DjVuProgressTask::DjVuProgressTask(const char *xtask,int nsteps)
  : task(xtask),parent(0), nsteps(nsteps), runtostep(0), gdata(0), data(0)
{
  //  gtask=task;
  {
    GMap<void *,GP<DjVuProgressTask::Data> > &map=get_map();
    void *threadID=GThread::current();
    if(map.contains(threadID))
    {
      gdata=new GP<Data>;
      Data &d=*(data=((*(GP<Data> *)gdata)=map[threadID]));
      if(d.callback)
      {
        unsigned long curdate = GOS::ticks();
        startdate = curdate;
        if (!d.head)
          d.lastsigdate = curdate + INITIAL;
        parent = d.head;
        d.head = this;
      }
    }
  }
}

DjVuProgressTask::~DjVuProgressTask()
{
  if (data && data->callback)
  {
    if (data->head != this)
      G_THROW( ERR_MSG("DjVuGlobal.not_compatible") );
    data->head = parent;
    if (!parent)
    {
      unsigned long curdate = GOS::ticks();
      if((*(data->callback))(data->gtask?data->gtask:"",curdate-startdate, curdate-startdate))
      {
        G_THROW("INTERRUPT");
      }
    }
  }
  delete (GP<Data> *)gdata;
}

void
DjVuProgressTask::run(int tostep)
{
  if(data)
  {
    data->gtask=task;
    if ((data->callback)&&(tostep>runtostep))
    {
      unsigned long curdate = GOS::ticks();
      if (curdate > data->lastsigdate + INTERVAL)
        signal(curdate, curdate);
      runtostep = tostep;
    }
  }
}

void
DjVuProgressTask::signal(unsigned long curdate, unsigned long estdate)
{
  int inprogress = runtostep;
  if (inprogress > nsteps)
    inprogress = nsteps;
  if (inprogress > 0)
    {
      const unsigned long enddate = startdate+
        (unsigned long)(((float)(estdate-startdate) * (float)nsteps) / (float)inprogress);
      if (parent)
      {
        parent->signal(curdate, enddate);
      }
      else if (data && data->callback && curdate<enddate)
      {
        if((*(data->callback))(data->gtask?data->gtask:"",curdate-startdate, enddate-startdate))
        {
          G_THROW("INTERRUPT");
        }
        data->lastsigdate = curdate;
      }
    }
}

// Progress callback
//
djvu_progress_callback *
djvu_set_progress_callback( djvu_progress_callback *callback )
{
   return DjVuProgressTask::set_callback(callback);
}

int djvu_supports_progress_callback(void) {return 1;}

#else

#ifndef HAS_DJVU_PROGRESS_TYPEDEF
extern "C"
{
  void *djvu_set_progress_callback(void *);
  int djvu_supports_progress_callback(void);
}
void *djvu_set_progress_callback(void *) { return 0; }
int djvu_supports_progress_callback(void) {return 0;}
#else
int djvu_supports_progress_callback(void) {return 0;}
djvu_progress_callback *
djvu_set_progress_callback( djvu_progress_callback *) { return 0; }
#endif

#endif

