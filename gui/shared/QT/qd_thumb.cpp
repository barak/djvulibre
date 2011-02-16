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

#include "qd_thumb.h"
#include "qd_painter.h"
#include "qx_imager.h"
#include "Arrays.h"
#include "IW44Image.h"
#include "GScaler.h"
#include "ByteStream.h"
#include "DataPool.h"

#include "qlib.h"

#include <qpainter.h>
#include <qimage.h>
#include <qstyle.h>
#include <qapplication.h>
#include <qpopupmenu.h>
#include <qsplitter.h>
#include <qvaluelist.h>


#define MIN_ITEM_WIDTH	64
#define MAX_ITEM_WIDTH	256
#define VMARGIN		8
#define HMARGIN		8
#define MAX_IMAGES	15
#define IDC_TH_CLOSE	0

//****************************************************************************
//**************************** QDThumbItems **********************************
//****************************************************************************

// I do not want to allow every item to participate in the notifications
// exchange because this will require <pages_num> pipes for synchronization,
// which may be way too much for the purpose

class QDThumbItem : public QListBoxItem
{
private:
   int		page_width;
   int		page_height;
   
   QDThumbnails	* list;
   TArray<char>	data;
protected:
   virtual void	paint(QPainter * p);
public:
   int		page_num;

   bool		cur_page;
   bool		processed;

      // QListBox service
   virtual int	width(const QListBox *) const { return width(); }
   virtual int	height(const QListBox *) const { return height(); }
   int		width(void) const;
   int		height(void) const { return width(); }

   bool		needsData(void) const { return data.size()==0; }
   
   TArray<char>	getData(void) const { return data; }
   void		setData(const TArray<char> & data);
   void		clearData(void);

   QDThumbItem(int page_num, QDThumbnails * list);
};

int
QDThumbItem::width(void) const
{
   
   int view_width;
   if ( list->rowMajor ) 
      view_width=list->visibleHeight();
   else
      view_width=list->visibleWidth();

   return view_width>MAX_ITEM_WIDTH ? MAX_ITEM_WIDTH :
          view_width<MIN_ITEM_WIDTH ? MIN_ITEM_WIDTH : view_width;
}

void
QDThumbItem::setData(const TArray<char> & d)
{
   if (d.size())
   {
      data=d;

	 // Get the real page proportions
      GP<ByteStream> str=ByteStream::create((const char *) data, data.size());
      GP<IW44Image> iwpix=IW44Image::create_decode(IW44Image::COLOR);
      iwpix->decode_chunk(str);
      page_width=iwpix->get_width();
      page_height=iwpix->get_height();

      list->dataSet(page_num, data);
      list->updateItem(page_num);
   }
}

void
QDThumbItem::clearData(void)
{
   data.empty();
   processed=false;
}

void
QDThumbItem::paint(QPainter * p)
{
   if ( cur_page ) 
     {
      QRect r (HMARGIN/2, VMARGIN/2, width()-HMARGIN, height()-VMARGIN);
      QStyle &style = listBox()->style();
#ifdef QT2
      style.drawFocusRect(p, r, listBox()->colorGroup(), NULL, TRUE );
#else
      style.drawPrimitive(QStyle::PE_FocusRect, p, r, listBox()->colorGroup(),
			  QStyle::Style_HasFocus);
#endif
    }
   p->translate(HMARGIN, VMARGIN);

   // inflation factor used to be 3/2, 10/9 may be enough ?
   int text_height=p->fontMetrics().height()*10/9;
   int w=width()-2*HMARGIN;
   int h=height()-2*VMARGIN;
   QString buffer=QDThumbnails::tr("Page ")+QString::number(page_num+1);
   p->drawText(0, h-text_height, w, text_height,
	       Qt::AlignHCenter | Qt::AlignVCenter, buffer);

   h-=text_height;
   float coeff_x=(float) w/page_width;
   float coeff_y=(float) h/page_height;
   float coeff=coeff_x<coeff_y ? coeff_x : coeff_y;
   GRect prect;
   prect.xmin=(int) (w-page_width*coeff)/2;
   prect.ymin=(int) (h-page_height*coeff)/2;
   prect.xmax=w-prect.xmin;
   prect.ymax=h-prect.ymin;

   QPixmap qpix=list->getImage(page_num, prect.width(), prect.height());

   if (qpix.isNull())
   {
	 // Draw the page border and that's all
      int dc=(int) ((page_width*coeff)/4);
      p->drawLine(prect.xmin+dc, prect.ymin, prect.xmax, prect.ymin);
      p->drawLine(prect.xmax, prect.ymin, prect.xmax, prect.ymax);
      p->drawLine(prect.xmax, prect.ymax, prect.xmin, prect.ymax);
      p->drawLine(prect.xmin, prect.ymax, prect.xmin, prect.ymin+dc);
      p->drawLine(prect.xmin, prect.ymin+dc, prect.xmin+dc, prect.ymin);
      p->drawLine(prect.xmin, prect.ymin+dc, prect.xmin+dc, prect.ymin+dc);
      p->drawLine(prect.xmin+dc, prect.ymin+dc, prect.xmin+dc, prect.ymin);
   } else
   {
	 // Display the page thumbnail. If the list is disables,
	 // modify the thumbnail to look disabled too.
      if (!list->isEnabled())
      {
	 int width=qpix.width();
	 int height=qpix.height();
	 QPixmap qpix1(qpix);
	 if (qxImager) {
	    QPainter p1(&qpix1);
            p1.setPen(qxImager->getGrayColor(0.8));
	    for(int x=-height;x<width;x+=2)
	       p1.drawLine(x, 0, x+height, height);
	    for(int x=0;x<width+height;x+=2)
	       p1.drawLine(x, 0, x-height, height);
	 }
	 p->drawPixmap(prect.xmin, prect.ymin, qpix1);
      } else
	 p->drawPixmap(prect.xmin, prect.ymin, qpix);
	    
      p->setPen(Qt::black);
      p->drawRect(prect.xmin+1, prect.ymin+1, prect.width()-2, prect.height()-2);

   }

   // reverse the translation to move coord to where it was
   p->translate(-HMARGIN, -VMARGIN);
   
}


QDThumbItem::QDThumbItem(int page_num_in, QDThumbnails * list_in) :      
   list(list_in), page_num(page_num_in), cur_page(false), processed(false)
{
   page_width=8500;
   page_height=11000;
}

//****************************************************************************
//**************************** QDThumbnails **********************************
//****************************************************************************

QPixmap
QDThumbnails::getImage(int page_num, int width, int height)
{
   QPixmap pixmap=pixmaps_arr[page_num];
   if (doc && (pixmap.isNull() || pixmap.width()!=width))
   {
      QDThumbItem * it=(QDThumbItem *) item(page_num);
      if (it)
      {
	 TArray<char> data=it->getData();
	 if (!data.size())
	 {
	       // If pending_list is not empty, it means that
	       // an icon data is being retrieved. slotTriggerCB()
	       // will call getDataForNextPage() when it's ready,
	       // and the latter will eventually get data for this
	       // page, if it's still visible
	    if (!pending_list.size()) getDataForPage(page_num);
	       // Data is still not available at this point.
	       // It will be copied later in slotTriggetCB()
	 } else
	 {
	       // Decompress the GPixmap
	    GP<ByteStream> str=ByteStream::create((const char *) data, data.size());
	    GP<IW44Image> iwpix=IW44Image::create_decode(IW44Image::COLOR);
	    iwpix->decode_chunk(str);
	    GP<GPixmap> pm=iwpix->get_pixmap();

	       // Gamma correct it from the DjVuDocument's gamma
	    double gamma=qeApp->gamma/doc->get_thumbnails_gamma();
	    pm->color_correct(gamma);

	       // Scale it
	    GP<GPixmapScaler> scaler=GPixmapScaler::create(pm->columns(), pm->rows(), width, height);
	    GP<GPixmap> pm_scaled=GPixmap::create();
	    scaler->scale(GRect(0, 0, pm->columns(), pm->rows()), *pm,
			 GRect(0, 0, width, height), *pm_scaled);

	       // Dither the scaled pixmap
	    if (qxImager)
              qxImager->dither(*pm_scaled);

	       // Draw it into a QPixmap
	    pixmap=QPixmap(pm_scaled->columns(), pm_scaled->rows(), x11Depth());
	    QePainter p(&pixmap);
	    p.drawPixmap(GRect(0, 0, pm_scaled->columns(),
			       pm_scaled->rows()), pm_scaled);
	    p.end();
	    
	       // Assign to the array for future use
	    pixmaps_arr[page_num]=pixmap;

	       // Now see if we have too many QImages decoded
	    if (!pixmaps_list.contains(page_num)) pixmaps_list.append(page_num);
	    if (pixmaps_list.size()>MAX_IMAGES)
	    {
	       GPosition pos=pixmaps_list.firstpos();
	       pixmaps_arr[pixmaps_list[pos]]=QPixmap();
	       pixmaps_list.del(pos);
	    }
	 }
      }
   }
   return pixmap;
}

void
QDThumbnails::rereadGamma(void)
{
      // Remove all QPixmaps and repaint the list
      // When every item repaints itself, it will as for QPixmap again
      // and we will regenerate it from compressed data with correct gamma.
   for(int i=0;i<pixmaps_arr.size();i++)
      pixmaps_arr[i]=QPixmap();
   pixmaps_list.empty();
   repaint();
}

//***************************** QDPort stuff *********************************

void
QDThumbnails::slotNotifyFileFlagsChanged(const GP<DjVuFile> & f,
					 long set_mask, long clr_mask)
{
   if (doc && (set_mask & DjVuFile::DECODE_OK))
   {
      GURL url=f->get_url();
      int page_num=doc->url_to_page(url);
      // the pending list needs updating
      for (GPosition pos=pending_list;pos;++pos)
      {
	 if (pending_list[pos]->page==page_num)
	 {
	    pending_list.del(pos);
	    break;
	 }
      }
      if (page_num>=0 && page_num<(int) count())
      {
	 QDThumbItem * it=(QDThumbItem *) item(page_num);
	 if (it && it->needsData())
	 {
	    GP<DataPool> pool=doc->get_thumbnail(page_num, fast_mode);
	    if (pool)
	    {
	       pending_list.append(new Pending(page_num, pool));
	       pool->add_trigger(0, -1, trigger_cb, this);
	       setCursor(waitCursor);
	    }
	 }
      }
   }
}

void
QDThumbnails::slotNotifyDocFlagsChanged(const GP<DjVuDocument> &,
					long set_mask, long)
{
   if (set_mask & DjVuDocument::DOC_INIT_OK) rescan();
}

void
QDThumbnails::trigger_cb(void * cl_data)
{
   QDThumbnails * th=(QDThumbnails *) cl_data;
   th->messenger.generalReq(0);
}

void
QDThumbnails::slotTriggerCB(int)
      // Called when DjVuDocument supplied enough data for the
      // currently update thumbnail.
{
      // Look thru the list of "pending" DataPools and find the one, which
      // has all data. Process as many as possible
   for(GPosition pos=pending_list;pos;)
   {
      GP<Pending> p=pending_list[pos];
      if (p->pool && p->pool->is_eof())
      {
	 GPosition this_pos=pos;
	 ++pos;
	 pending_list.del(this_pos);
	 QDThumbItem * it=(QDThumbItem *) item(p->page);
	 if (it && it->needsData())
	 {
	    TArray<char> data(p->pool->get_size()-1);
	    p->pool->get_data(data, 0, data.size());
	    it->setData(data);
	 }
      } else ++pos;
   }

   if (pending_list.size()==0)
      setCursor(normalCursor);

      // Now see if there is another visible icon, which has not
      // been processed yet.
      // We're not afraid of recursion here, because there is this
      // synchronizing pipe. The request has to go thru this pipe,
      // and we're reading from it only when the control is in the
      // QT's event loop
   getDataForNextPage();
}

void
QDThumbnails::getDataForNextPage(void)
      // Here we will try to find a visible thumbnail icon w/o
      // data associated with it. We make sure not to process
      // the same icon twice (DjVuDocument may have no data for some
      // pages, so we mustn't ask twice).
      // We also disregard invisible icons, as getting data for
      // them right now may hang the program w/o any good reason.
{
   if (!isVisible() || !doc || pending_list.size()) return;
   
   for(int page_num=0;page_num<(int) count();page_num++)
   {
      QDThumbItem * it=(QDThumbItem *) item(page_num);
      if (it && !it->processed &&
	  itemVisible(page_num) &&
	  it->needsData())
      {
	 if (getDataForPage(page_num))
	    break;
      }
   }
}

bool
QDThumbnails::getDataForPage(int page_num)
      // Will attempt to get a thumbnail for the specified page.
      //
      // First we will check if the thumbnail data for the specified
      // page is already known
      //
      // Then we will append another item to the pending_list, add
      // trigger and return. Trigger will be responsible for actually
      // setting the data. Trigger can be called immediately (from
      // add_trigger()) if all data is available right now. This is
      // not a problem as trigger_cb() will just output one byte into
      // a synchronizing pipe. The real work will be done later (after
      // QT acquires the control back)
      //
      // The function will return TRUE, if data's been requested
      // successfully, and FALSE, if due to any reason the request
      // shouldn't or couldn't be made.
{
   if (!doc) return false;
   // if page is still pending, then back off
   for (GPosition pos=pending_list;pos;++pos)
   {
      if (pending_list[pos]->page==page_num)
	 return true;
   }
   
   QDThumbItem * it=(QDThumbItem *) item(page_num);
   if (it)
   {
      it->processed=true;
      if (it->needsData())
      {
	 GP<DataPool> pool=doc->get_thumbnail(page_num, fast_mode);
	 if (pool)
	 {
	    pending_list.append(new Pending(page_num, pool));
	    pool->add_trigger(0, -1, trigger_cb, this);
	    setCursor(waitCursor);
	    return true;
	 }
      }
   }
   return false;
}

void
QDThumbnails::setData(int page_num, const TArray<char> & data)
{
   if (page_num>=0 && page_num<(int) count())
   {
      QDThumbItem * it=(QDThumbItem *) item(page_num);
      if (it) it->setData(data);
   }
}

void
QDThumbnails::resizeEvent(QResizeEvent * ev)
{
   // We want to keep the top cell on the screen.
   // It's especially important to do it in our case since
   // the cells' size changes during the list resize.
   int top_item=topItem();
   QListBox::resizeEvent(ev);
   if (top_item>=0) setTopItem(top_item);
   // the layout has to be redone 
   triggerUpdate(TRUE);
   
}

void
QDThumbnails::setFastMode(bool en)
{
   if (fast_mode!=en)
   {
      pending_list.empty();		// Just in case. Won't hurt
      setCursor(normalCursor);
      for(int page_num=0;page_num<(int) count();page_num++)
	 ((QDThumbItem *) item(page_num))->processed=false;
      
      fast_mode=en;
      if (isVisible()) getDataForNextPage();
      repaint();
   }
}

void
QDThumbnails::setCurPageNum(int cur_page_num_in)
{
   if (cur_page_num!=cur_page_num_in)
   {
      cur_page_num=cur_page_num_in;
      if (count()>0)
      {
	 if (cur_page_num<0) cur_page_num=0;
	 if (cur_page_num>=(int) count()) cur_page_num=count()-1;
	 for(int page_num=0;page_num<(int) count();page_num++)
	 {
	    QDThumbItem * it=(QDThumbItem *) item(page_num);
	    bool on=(page_num==cur_page_num);
	    if (it && it->cur_page!=on)
	    {
	       it->cur_page=on;
	       updateItem(page_num);
	    }
	 }
	 if (!itemVisible(cur_page_num))
	    setTopItem(cur_page_num);
      }
   }
}

void
QDThumbnails::reloadPage(int page_num)
{
   QDThumbItem * it=(QDThumbItem *) item(page_num);
   if (it)
   {
      it->clearData();
      pixmaps_arr[page_num]=QPixmap();
      GPosition pos;
      if (pixmaps_list.search(page_num, pos))
	 pixmaps_list.del(pos);

      // no need to check pending list. it will be updated in
      // slotTriggerCB
      GP<DataPool> pool=doc->get_thumbnail(page_num, fast_mode);
      if (pool)
      {
	 pending_list.append(new Pending(page_num, pool));
	 pool->add_trigger(0, -1, trigger_cb, this);
	 setCursor(waitCursor);
      }
   }
}

void
QDThumbnails::rescan(void)
{
   if (doc && doc->is_init_complete())
   {
      setAutoUpdate(FALSE);
      clear();

      int pages=doc->get_pages_num();
      pixmaps_arr.empty();
      pixmaps_arr.resize(pages-1);
      pixmaps_list.empty();
      pending_list.empty();

      for(int page_num=0;page_num<pages;page_num++)
	 insertItem(new QDThumbItem(page_num, this), -1);
      if (cur_page_num>=0)
      {
	 if (cur_page_num<0) cur_page_num=0;
	 if (cur_page_num>=(int) count()) cur_page_num=count()-1;
	 for(int page_num=0;page_num<(int) count();page_num++)
	 {
	    QDThumbItem * it=(QDThumbItem *) item(page_num);
	    if (it) it->cur_page=(page_num==cur_page_num);
	 }
	 if (!itemVisible(cur_page_num))
           setTopItem(cur_page_num);
      }
      setAutoUpdate(TRUE);
      repaint();
      if (isVisible()) 
        getDataForNextPage();
      need_rescan=false;
   }
}

void
QDThumbnails::setDjVuDocument(GP<DjVuDocument> & doc_in)
{
   DjVuPortcaster * pcaster=DjVuPort::get_portcaster();
   if (doc) pcaster->del_route(doc, port.getPort());
   doc=doc_in;
   if (doc)
   {
      pcaster->add_route(doc, port.getPort());
   
      need_rescan=true;
      if (isVisible()) rescan();
   }
}

QSize
QDThumbnails::sizeHint(void) const
{
   int ideal_size;
   QSplitter *parent = dynamic_cast<QSplitter *>(parentWidget());
   
   if ( parent )
   {
      if ( rowMajor ) 
	 ideal_size=parent->parentWidget()->height()/5;
      else
	 ideal_size=parent->parentWidget()->width()/7;
	 
      if ( ideal_size < min_list_width )
	 ideal_size=min_list_width;
      else if ( ideal_size > max_list_width )
	 ideal_size=max_list_width;
   }
   else
      ideal_size=MAX_ITEM_WIDTH/2;
   
   QSize size=QListBox::sizeHint();

   if ( rowMajor ) 
      size.setHeight(ideal_size);
   else 
      size.setWidth(ideal_size);
   
   return size;
}

void
QDThumbnails::show(void)
{
   pending_list.empty();		// Just in case. Won't hurt
   setCursor(normalCursor);
   if (need_rescan) rescan();
   else getDataForNextPage();
   QListBox::show();
}

QDThumbnails::QDThumbnails(QWidget * parent, const char * name, bool _rowMajor) :
   QListBox(parent, name), port(false, false), rowMajor(_rowMajor)
{

   if ( rowMajor )
   {
      min_list_width=MIN_ITEM_WIDTH+HMARGIN;
      max_list_width=MAX_ITEM_WIDTH+HMARGIN;
      setRowMode(1);
      setVScrollBarMode(AlwaysOff);
      setHScrollBarMode(AlwaysOn);
   }
   else
   {
      min_list_width=MIN_ITEM_WIDTH+VMARGIN;
      max_list_width=MAX_ITEM_WIDTH+VMARGIN;
      setColumnMode(1);
      setHScrollBarMode(AlwaysOff);
      setVScrollBarMode(AlwaysOn);
   }
   
   need_rescan=true;
   cur_page_num=-1;
   pending_list.empty();
   normalCursor=cursor();
   
      // Now set minimum and maximum width
   
   if ( rowMajor ) 
   {
      setMinimumHeight(min_list_width);
      setMaximumHeight(max_list_width);
   }
   else
   {
      setMinimumWidth(min_list_width);
      setMaximumWidth(max_list_width);
   }

      // Connect slots
   connect(&port, SIGNAL(sigNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)),
           this, SLOT(slotNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)));
   connect(&port, SIGNAL(sigNotifyDocFlagsChanged(const GP<DjVuDocument> &, long, long)),
           this, SLOT(slotNotifyDocFlagsChanged(const GP<DjVuDocument> &, long, long)));
   connect(&messenger, SIGNAL(sigGeneralReq(int)), this, SLOT(slotTriggerCB(int)));
   connect(this, SIGNAL(clicked(QListBoxItem *)), this, SLOT(slotGotoPage(QListBoxItem *)));
   
#ifdef QT2
      //********************************************************************
      // The following is necessary because of the QT bug.
      // They're trying to make QListBox use palette().base() for
      // drawing the cells' background (white). Unfortunately different
      // pieces of code apparently disagree what color should actually
      // be used, and the result is that among white color there are
      // big gray (palette().background()) blocks. All I can do is to force
      // both the background() and base() be the same.
      // To make things worse, setBackgroundColor(white) doesn't fix the
      // problem and I have to change the palette.
      // To make things much worse, QTableView intercepts setPalette() request
      // and tries to pass it to the scrollbars as well, which I do not need.
      // So I have to call QWidget::setPalette().
      // Finally, even QWidget can spoil my scrollbars, and I force it
      // do not propagate the palette using setPalettePropagation()
      // Create scrollbars before changing the palette's background
   verticalScrollBar();
   horizontalScrollBar();
      // Modify the palette
   QPalette p=palette().copy();
   QColorGroup g=p.normal();
   p.setNormal(QColorGroup(g.foreground(), g.base(), g.light(),
			   g.dark(), g.mid(), g.text(), g.base()));
   setPalettePropagation(NoChildren);
   QWidget::setPalette(p);	// QTableView will otherwise change scrollbars
      //********************************************************************
#endif
}

void
QDThumbnails::slotGotoPage(QListBoxItem *it)
{
   QDThumbItem *tit=dynamic_cast<QDThumbItem *>(it);
   if ( tit )
      emit sigGotoPage( tit->page_num );
}
