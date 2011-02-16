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

#include "DjVuGlobal.h"
#include "qd_base.h"
#include "debug.h"
#include "djvu_base_res.h"
#include "qd_painter.h"
#include "qlib.h"
#include "GOS.h"
#include "cin_data.h"
#include "qd_tbar_mode_piece.h"
#include "qd_tbar_rotate_piece.h"

#include "wait.bm"
#include "wait_mask.bm"
#include "hand1.bm"
#include "hand1_mask.bm"
#include "hand2.bm"
#include "hand2_mask.bm"
#include "zoom_select.bm"
#include "zoom_select_mask.bm"

#include <math.h>
#include <qbitmap.h>
#include <qpaintdevicemetrics.h>
#include <qapplication.h>
#include <qsplitter.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qfiledialog.h>



const char *
QDBase::search_results_name="Search results";

const int 
QDBase::toolbar_edge = 3;

static inline
int round(float v)
{
   return (int) (v+0.5);
}

QDBase::OverrideFlags::OverrideFlags(void)
{
   toolbar=true;
   toolbaropts = 0;
   scrollbars=true;
   menu=true;
   frame=true;
   links=true;
   logo=true;
   keyboard=true;
   print=true;
   thumbnails=THUMB_HIDE;
   hor_align=ver_align=DjVuANT::ALIGN_UNSPEC;
   
   cmd_mode=-1;	    // Meaning "not specified". In EMBED mode, if zoom
   cmd_zoom=-1;	    // is not specified, we will use IDC_ZOOM_PAGE
   cmd_rotate=-1;   // with priority SRC_DEFAULT to allow ANTa
		    // override this. Otherwise we will use this value
		    // with priority SRC_TAGS
}

QDBase::QDBase(QWidget * parent, const char * name) : QWidget(parent, name)
{
   DEBUG_MSG("QDBase::QDBase(): Initializing class...\n");
   DEBUG_MAKE_INDENT(3);

   being_destroyed=false;
   
   map_area_tip=0;
   cur_map_area=0;
   display_all_hlinks=false;
   image_size_known=0;
   toolbar=0;
   mode_tbar=0;
   rotate_tbar=0;
   toolbar_shown=0;
   toolbar_enabled=0;
   toolbar_asked=0;
   showing_toolbar=0;
   splitter=0;
   main_widget=0;
   thumb_widget=0;
   cmd_mode=cmd_mode_force=cmd_zoom=-1;
   zoom_src=SRC_DEFAULT;
   mode_src=SRC_DEFAULT;
   rotate_src=SRC_DEFAULT;
   in_hand_scroll=in_paint=in_layout=in_zoom_select=0;
   lastrect=0;
   acc_scroll_dh=acc_scroll_dv=0;
   left_butt_down=0;
   ignore_ant_mode_zoom=0;
   for(int i=0;i<MODE_SOURCE_MAX;i++) 
     mode_prio[i]=i;

   pane_mode=IDC_PANE;
   
   djvu_logo_bmp = GBitmap::create(*CINData::get("ppm_djvu_logo"));
   
   depth=QPaintDeviceMetrics(this).depth();

   new QDTBHider(this);
   
   // Get the display resolution
   QPaintDeviceMetrics m(QApplication::desktop());
   displ_dpi=(int) (m.width()*25.4/m.widthMM());
   displ_dpi=100;  // Override displ_dpi as Leon demands
   DEBUG_MSG("display dpi=" << displ_dpi << "\n");

      // Adjusting margin caches
   pm_cache.setMaxSize(prefs.mcacheSize*1024*1024);
   bm_cache.setMaxSize(prefs.mcacheSize*1024*1024);
   connect(&cache_timer, SIGNAL(timeout(void)), this, SLOT(slotCheckCache(void)));

   main_widget=new QWidget(this, "main_widget");
   main_widget->installEventFilter(this);
   
   pane=new QDPane(main_widget, "djvu_pane");
   pane->installEventFilter(this);
   pane->setFocusPolicy(QWidget::StrongFocus);
   pane->setMouseTracking(TRUE);

      // Creating main area and scroll bars
   hscroll=new QScrollBar(QScrollBar::Horizontal, main_widget, "hscroll");
   vscroll=new QScrollBar(QScrollBar::Vertical, main_widget, "vscroll");
   connect(hscroll, SIGNAL(valueChanged(int)),
           this, SLOT(slotSliderMoved(int)));
   connect(vscroll, SIGNAL(valueChanged(int)), 
           this, SLOT(slotSliderMoved(int)));
   connect(hscroll, SIGNAL(sliderPressed(void)), 
           this, SLOT(slotSliderPressed(void)));
   connect(vscroll, SIGNAL(sliderPressed(void)), 
           this, SLOT(slotSliderPressed(void)));
   connect(hscroll, SIGNAL(sliderReleased(void)), 
           this, SLOT(slotSliderReleased(void)));
   connect(vscroll, SIGNAL(sliderReleased(void)), 
           this, SLOT(slotSliderReleased(void)));
   
   hscroll->resize(hscroll->sizeHint());
   vscroll->resize(vscroll->sizeHint());

   createCursors();
   setBackgroundColor(0xffffff, false);	// Don't redraw
   
      // Setting desired video modes
   setMode(IDC_DISPLAY_COLOR, 0, SRC_DEFAULT);	// Don't redraw
   setZoom(prefs.nDefaultZoom, 0, SRC_DEFAULT);	// Don't relayout
   setRotate(IDC_ROTATE_0, 0, SRC_DEFAULT);	// Don't relayout

   layout(0);			// Disallow redraw
}

void
QDBase::setOverrideFlags(const OverrideFlags & flags)
{
  bool do_layout=false;
  OverrideFlags old_flags=override_flags;
  override_flags=flags;
  
  if (old_flags.toolbar != flags.toolbar)
    enableToolBar(flags.toolbar && prefs.toolBarOn);
  if (flags.toolbaropts & OverrideFlags::TOOLBAR_ALWAYS)
    stickToolBar();
  else if (flags.toolbaropts & OverrideFlags::TOOLBAR_AUTO)
    unStickToolBar();
  else if (prefs.toolBarAlwaysVisible)
    stickToolBar();
  else 
    unStickToolBar();
  
  if (flags.thumbnails)
    showThumbnails();
  else
    hideThumbnails();

  if (old_flags.hilite_rects.size()!=flags.hilite_rects.size() ||
      old_flags.url!=flags.url) 
    {
      createMapAreas(true);
    }
  if (old_flags.hor_align!=flags.hor_align ||
      old_flags.ver_align!=flags.ver_align)
    do_layout = true;
  if (toolbar && old_flags.toolbaropts != flags.toolbaropts)
    {
      toolbar->setOptions(override_flags.toolbaropts);
      do_layout = true;
    }
  if (do_layout)
    layout();
  setCursor();
}

void
QDBase::setDjVuImage(const GP<DjVuImage> & _dimg, int do_redraw)
{
   DEBUG_MSG("QDBase::setDjVuImage(): setting new DjVu image\n");

   eraseMapAreas(true, false);
   anno=0;
   cmd_mode_force=-1;
   
   if (dimg) 
     rectDocument.clear();
   else
   {
     // Just in case if any derived class has already preset position of
     // the rectDocument (in accordance with the saved_data)
     rectDocument.xmax=rectDocument.xmin;
     rectDocument.ymax=rectDocument.ymin;
   }
   
   dimg=_dimg;
   
   image_size_known=0;

   QDBase::imageUpdated();

      // If the current resolution is due to ANTa chunk only, reset it
   if (zoom_src==SRC_ANT) 
     setZoom(prefs.nDefaultZoom, 0, SRC_ANT);
   if (mode_src==SRC_ANT) 
     setMode(IDC_DISPLAY_COLOR, 0, SRC_ANT);
   if (rotate_src==SRC_ANT) 
     setRotate(IDC_ROTATE_0, 0, SRC_ANT);

   decodeAnno(false);	// No redraw
   layout(0);		// Disallow redraw
   setRotate(cmd_rotate, 0, rotate_src); // perform rotation
   if (do_redraw) 
     redraw();
}

void
QDBase::imageUpdated(void)
{
   DEBUG_MSG("QDBase::imageUpdated(): update image ...\n");
   if (dimg)
   {
      int saved_cmd_mode_force=cmd_mode_force;
      GP<DjVuFile> file=dimg->get_djvu_file();
      if (file && (file->is_decode_ok() || file->is_decode_failed() ||
		   file->is_decode_stopped()))
      {
	 if (dimg->is_legal_bilevel()) cmd_mode_force=IDC_DISPLAY_BLACKWHITE;
	 if (dimg->is_legal_photo()) cmd_mode_force=IDC_DISPLAY_COLOR;
      } else cmd_mode_force=-1;

      if (saved_cmd_mode_force!=cmd_mode_force) redraw();
   }
   updateToolBar();
}

void
QDBase::setMappers(void)
{
   if (!rectDocument.isempty())
   {
      mapper.clear();
      mapper.set_input(rectDocument);
      GRect rectImage;
      rectImage.xmax=rectDocument.width();
      rectImage.ymax=rectDocument.height();
      mapper.set_output(rectImage);
      mapper.mirrory();

      ma_mapper.clear();
      rectImage.xmax=dimg->get_width();
      rectImage.ymax=dimg->get_height();
      ma_mapper.set_input(rectImage);
      ma_mapper.set_output(rectDocument);
      ma_mapper.mirrory();
   }
}

void
QDBase::layout(bool allow_redraw)
{
  DEBUG_MSG("QDBase::layout() called\n");
  DEBUG_MAKE_INDENT(3);
  
  if (in_layout)
    {
      DEBUG_MSG("but we seem to be already here => must be called because of resize\n");
      return;
    }

  IncFlag inc(&in_layout);
  
  // Obtain document size
  int doc_w=0;
  int doc_h=0;
  if (dimg)
    {
      doc_w=dimg->get_width();
      doc_h=dimg->get_height();
      if (doc_w>0 && doc_h>0) 
        image_size_known = 1;
    }
  
  if (!doc_w || !doc_h)
    {
      DEBUG_MSG("document rectangle is empty...just hide scroll bars\n");
      hscroll->hide();
      vscroll->hide();
      pane->resize(main_widget->width(), main_widget->height());
      return;
    }

  bool do_redraw = false;
  bool update_tbar = false;
  bool hscroll_visible = false;
  bool vscroll_visible = false;

  int toolbar_height = toolbar->computeHeight(main_widget->width());
  toolbar_enabled = (override_flags.toolbar && prefs.toolBarOn);
  if (toolbar_height + 2 * hscroll->height() > main_widget->height() )
    toolbar_enabled = 0;
  if (!isToolBarEnabled())
    toolbar_height = 0;
  else if (!isToolBarStuck())
    toolbar_height = toolbar_edge;
      
  bool fullscreen = false;
  emit sigQueryFullScreen(fullscreen);

  rectVisible = GRect(0, 0, main_widget->width(), main_widget->height()-toolbar_height);
  if (cmd_zoom==IDC_ZOOM_STRETCH)
    {
      do_redraw |= allow_redraw;
      rectDocument = rectVisible;
    } 
  else
    {
      // The scaling code can reduce the image *fast* 1, 2, 3, 4, 6 or 12 times
      // When "Favor fast magnifications for fast resolutions" is set,
      // we're forcing plugin to use these "fast" reductions.
      // Note, that the actual zoom factor (in %%) will depend on the
      // screen and image dpi.
      const int fast_red[]={ 1, 2, 3, 4, 6, 12 };
      const int fast_reds=sizeof(fast_red)/sizeof(fast_red[0]);
      int red_ind=0;
      float red=1/100000.0;
      while(true)	// Loop until we're sure about scrollbars and toolbar height
        {
          int image_dpi=300;
          if (dimg) image_dpi=dimg->get_rounded_dpi();
          if (image_dpi<=0 || image_dpi>=2400) image_dpi=300;
              
          DEBUG_MSG("displ_dpi=" << displ_dpi << ", image_dpi=" << image_dpi << "\n");
              
          // Compute reduction from cmd_zoom
          if (cmd_zoom>=IDC_ZOOM_MIN && cmd_zoom<=IDC_ZOOM_MAX)
            {
              // Fixed resolution
              int zoom=cmd_zoom-IDC_ZOOM_MIN;
              DEBUG_MSG("zoom=" << zoom << "%\n");
                  
              red=(float) image_dpi/zoom;
            } 
          else if (cmd_zoom==IDC_ZOOM_ONE2ONE)
            {
              red=1;
            } 
          else if (cmd_zoom==IDC_ZOOM_PAGE)
            {
              // IDC_ZOOM_PAGE case
              if (prefs.fastZoom)
                {
                  // We may continue to loop from the previous scroll bar iteration
                  for(;red_ind<fast_reds;red_ind++)
                    {
                      red=fast_red[red_ind];
                      if (doc_w<=rectVisible.width()*red &&
                          doc_h<=rectVisible.height()*red)
                        break;
                    }
                }
                  
              if (doc_w>rectVisible.width()*red ||
                  doc_h>rectVisible.height()*red)
                {
                  // Either we're not in the "fast" mode, or the
                  // capabilities of the "fast" mode are not sufficient
                  // to reduce the document so much
                  float red_w=(float) doc_w/rectVisible.width();
                  float red_h=(float) doc_h/rectVisible.height();
                  float red_new=red_w<red_h ? red_h : red_w;
                      
                  // The following is necessary to avoid infinite loop with
                  // appearing and disappearing scroll bars.
                  if (red_new>red) red=red_new;
                }
            } 
          else
            {
              // Make it IDC_ZOOM_WIDTH even if it's not
              if (prefs.fastZoom)
                {
                  // We may continue to loop from the previous scroll bar iteration
                  for(;red_ind<fast_reds;red_ind++)
                    {
                      red=fast_red[red_ind];
                      if (doc_w<=rectVisible.width()*red)
                        break;
                    }
                }
                  
              if (doc_w>rectVisible.width()*red)
                {
                  // Either we're not in the "fast" mode, or the
                  // capabilities of the "fast" mode are not sufficient
                  // to reduce the document so much
                  float red_new=(float) doc_w/rectVisible.width();
                      
                  // The following is necessary to avoid infinite loop with
                  // appearing and disappearing scroll bars.
                  if (red_new>red) red=red_new;
                }
            }
          DEBUG_MSG("reduction=" << red << ", scale factor=" << (1/red) << "\n");
              
          // determine scrollbar visibility and modify rectVisible
          rectVisible=GRect(0, 0, main_widget->width(),
                            main_widget->height()-toolbar_height);
          DEBUG_MSG("before engaging scrolls rectVisible=(" << rectVisible.xmin <<
                    ", " << rectVisible.ymin << ", " << rectVisible.width() <<
                    ", " << rectVisible.height() << ")\n");
          int sh = hscroll->height();
          int sw = vscroll->width();
          bool hs_vis=false, vs_vis=false;
          if (override_flags.scrollbars && !fullscreen && 
              rectVisible.height()>2*sh && rectVisible.width()>2*sw)
            {
              while(1)
                {
                  if (!hs_vis && doc_w>round(rectVisible.width()*red))
                    {
                      rectVisible.ymax=main_widget->height()-toolbar_height-sh;
                      hs_vis=true;
                    }
                  else if (!vs_vis && doc_h>round(rectVisible.height()*red))
                    {
                      rectVisible.xmax=main_widget->width()-sw;
                      vs_vis=true;
                    }
                  else
                    {
                      break;
                    }
                }
            }
          DEBUG_MSG("after engaging scrolls rectVisible=(" << rectVisible.xmin <<
                    ", " << rectVisible.ymin << ", " << rectVisible.width() <<
                    ", " << rectVisible.height() << ")\n");
              
          if (hs_vis==hscroll_visible && vs_vis==vscroll_visible)
            break;
              
          DEBUG_MSG("Looks like we need another pass thru resolutions:\n");
          DEBUG_MSG("hscroll_visible: was=" << hscroll_visible 
                    << ", now=" << hs_vis << "\n");
          DEBUG_MSG("vscroll_visible: was=" << vscroll_visible 
                    << ", now=" << vs_vis << "\n");
          hscroll_visible=hs_vis;
          vscroll_visible=vs_vis;
        }
          
      DEBUG_MSG("hscroll_visible=" << hscroll_visible <<
                ", vscroll_visible=" << vscroll_visible << "\n");
          
      GRect prevRectDocument=rectDocument;
          
          // Modify rectDocument maintaining document position
      if (round(rectDocument.width()*red)!=doc_w ||
          round(rectDocument.height()*red)!=doc_h)
        {
          if (!rectDocument.isempty())
            {
              GRect rect;
              rect.intersect(rectVisible, rectDocument);
              int cx=(rect.xmin+rect.xmax)/2-rectDocument.xmin;
              int cy=(rect.ymin+rect.ymax)/2-rectDocument.ymin;
              rectDocument.xmin=rectDocument.xmax
                =(int) (rectVisible.width()/2-cx*doc_w/
                        (red*rectDocument.width()));
              rectDocument.ymin=rectDocument.ymax
                =(int) (rectVisible.height()/2-cy*doc_h/
                        (red*rectDocument.height()));
            }
        }
          
      // Determine scaled document rectangle (adjusting xmin and ymin)
      rectDocument.xmax=rectDocument.xmin+round(doc_w/red);
      rectDocument.ymax=rectDocument.ymin+round(doc_h/red);
          
      if (isToolBarEnabled() && 
          (prevRectDocument.width()!=rectDocument.width() ||
           prevRectDocument.height()!=rectDocument.height()))
        update_tbar=true;	// Since getZoom() may now return different value
      if (prevRectDocument.width()!=rectDocument.width() ||
          prevRectDocument.height()!=rectDocument.height() ||
          prevRectDocument.xmin!=rectDocument.xmin ||
          prevRectDocument.ymin!=rectDocument.ymin)
        do_redraw|=allow_redraw;
          
      DEBUG_MSG("before aligning rectDocument=(" << 
                rectDocument.xmin << ", " << rectDocument.ymin << ", " <<
                rectDocument.width() << ", " << rectDocument.height() << ")\n");
          
      // adjust document location. Please note: there is a copy of this code in
      // Scroll(). If you make any changes, make sure to do it twice :)
      int xoff=0;
      int yoff=0;
          
      int hor_align=DjVuANT::ALIGN_CENTER;
      int ver_align=DjVuANT::ALIGN_CENTER;
      if (anno && anno->ant)
        {
          hor_align=anno->ant->hor_align;
          ver_align=anno->ant->ver_align;
        }
      if (override_flags.hor_align!=DjVuANT::ALIGN_UNSPEC)
        hor_align=override_flags.hor_align;
      if (override_flags.ver_align!=DjVuANT::ALIGN_UNSPEC)
        ver_align=override_flags.ver_align;
          
      if (rectDocument.width()<=rectVisible.width())
        {
          switch(hor_align)
            {
            case DjVuANT::ALIGN_LEFT:
              xoff=rectVisible.xmin-rectDocument.xmin;
              break;
            case DjVuANT::ALIGN_CENTER:
            case DjVuANT::ALIGN_UNSPEC:
              xoff=rectVisible.xmin-rectDocument.xmin+
                (rectVisible.width()-rectDocument.width())/2;
              break;
            case DjVuANT::ALIGN_RIGHT:
              xoff=rectVisible.xmax-rectDocument.xmax;
              break;
            }
        } 
      else if (rectDocument.xmin>rectVisible.xmin)
        xoff=rectVisible.xmin-rectDocument.xmin;
      else if (rectDocument.xmax<rectVisible.xmax)
        xoff=rectVisible.xmax-rectDocument.xmax;
          
      if (rectDocument.height()<=rectVisible.height())
        {
          switch(ver_align)
            {
            case DjVuANT::ALIGN_TOP:
              yoff=rectVisible.ymin-rectDocument.ymin;
              break;
            case DjVuANT::ALIGN_CENTER:
            case DjVuANT::ALIGN_UNSPEC:
              yoff=rectVisible.ymin-rectDocument.ymin+
                (rectVisible.height()-rectDocument.height())/2;
              break;
            case DjVuANT::ALIGN_BOTTOM:
              yoff=rectVisible.ymax-rectDocument.ymax;
              break;
            }
        } 
      else if (rectDocument.ymin>rectVisible.ymin)
        yoff=rectVisible.ymin-rectDocument.ymin;
      else if (rectDocument.ymax<rectVisible.ymax)
        yoff=rectVisible.ymax-rectDocument.ymax;
          
      if (rectDocument.width()>0 && rectDocument.height()>0)
        rectDocument.translate(xoff, yoff);
          
      DEBUG_MSG("translated rectDocument=(" << rectDocument.xmin << 
                ", " << rectDocument.ymin << ", " <<
                rectDocument.width() << ", " << rectDocument.height() << ")\n");
    }
      
  // Make the pane be on top so that any transient movements (of the
  // toolbar and scrollbars) will not cause any extra Expose events
  // TODO: RAISE: pane->raise();
      
      // Resize the pane and scroll bars
  if (hscroll_visible)
    {
      hscroll->move(0, rectVisible.height()+toolbar_height);
      hscroll->show();
    } 
  else 
    hscroll->hide();
  if (vscroll_visible)
    {
      vscroll->move(rectVisible.width(), 0);
      vscroll->show();
    } 
  else
    vscroll->hide();

      // Since QDPane is created with WResizeNoErase flag, we need to
      // redraw ourselves if the pane's size changed.
  if (pane->width()!=rectVisible.width() ||
      pane->height()!=rectVisible.height()) 
    do_redraw |= allow_redraw;
      
  pane->resize(rectVisible.width(), rectVisible.height());
  hscroll->resize(rectVisible.width(), hscroll->height());
  vscroll->resize(vscroll->width(), rectVisible.height()+toolbar_height);
      
  if (isToolBarEnabled())
    {
      toolbar->resize(rectVisible.width(), toolbar->height());
      toolbar->move(0, rectVisible.height());
      if (!isToolBarStuck() && isToolBarShown())
        {
          toolbar_shown=false;
          showToolBar(false);
        }
    } else if (toolbar)
      toolbar->move(0, main_widget->height());
      
      // Now, when everything is in place set the stacking order properly
  if (toolbar) toolbar->raise();
  hscroll->raise();
  vscroll->raise();
      
  // Adjust scroll bars parameters
  if (vscroll->isVisible())
    {
      int value, sliderSize;
      value=rectVisible.ymin-rectDocument.ymin;
      if (value<0) value=0;
      sliderSize=rectVisible.height()<rectDocument.height() ?
        rectVisible.height() : rectDocument.height();
      vscroll->setRange(0, rectDocument.height()-sliderSize);
      vscroll->setValue(value);
      vscroll->setSteps(rectDocument.height()/25, sliderSize);
    }
      
  if (hscroll->isVisible())
    {
      int value, sliderSize;
      value=rectVisible.xmin-rectDocument.xmin;
      if (value<0) value=0;
      sliderSize=rectVisible.width()<rectDocument.width() ?
        rectVisible.width() : rectDocument.width();
      hscroll->setRange(0, rectDocument.width()-sliderSize);
      hscroll->setValue(value);
      hscroll->setSteps(rectDocument.width()/25, sliderSize);
    }
      
  // Set rectangle mapper
  setMappers();
      
  for(GPosition pos=map_areas;pos;++pos)
    map_areas[pos]->layout(GRect(0, 0, dimg->get_width(), dimg->get_height()));
      
      // Resizing margin caches:
  bm_cache.resize(rectDocument.width(), rectDocument.height(),
                  rectVisible.width(), rectVisible.height());
  pm_cache.resize(rectDocument.width(), rectDocument.height(),
                  rectVisible.width(), rectVisible.height());
      
  if (do_redraw) redraw();
  if (update_tbar) updateToolBar();
      
  DEBUG_MSG("QDBase::layout(): DONE\n");
}

void
QDBase::setMode(int cmd, bool do_redraw, int src)
{
   DEBUG_MSG("QDBase::setMode(): setting mode=" << cmd << "\n");

   if (mode_prio[mode_src]>mode_prio[src]) return;
   mode_src=src;
   
   if (cmd==IDC_DISPLAY_BLACKWHITE)
   {
      pm_cache.disable(); bm_cache.enable();
      cache_timer.start(0, TRUE);
   } else
   {
      bm_cache.disable(); pm_cache.enable();
      cache_timer.start(0, TRUE);
   }
   
      // Install new mode (remember, that there is "cmd_mode_override" too)
   if (cmd_mode!=cmd)
   {
      cmd_mode=cmd;
      updateToolBar();
      if (do_redraw) redraw();
   }
}

int
QDBase::getZoom(void) const
{
   if (cmd_zoom>=IDC_ZOOM_MIN && cmd_zoom<=IDC_ZOOM_MAX)
      return cmd_zoom-IDC_ZOOM_MIN;
   else
      if (dimg && dimg->get_width() && rectDocument.width())
	 return dimg->get_rounded_dpi()*rectDocument.width()/dimg->get_width();
      else return 100;
}


// Please NOTE! cmd is NOT the zoom in the range from 0% to whatever%. It's
// a COMMAND: one of those listed in djvu_res_base.h (IDC_ZOOM_* stuff)
void
QDBase::setZoom(int cmd, bool do_layout, int src)
{
  DEBUG_MSG("QDBase::setZoom(): setting zoom " << cmd << "\n");
  DEBUG_MAKE_INDENT(3);

  if (mode_prio[zoom_src]>mode_prio[src]) return;
  zoom_src=src;

  if (!dimg)
    {
      if (cmd!=IDC_ZOOM_ZOOMOUT && cmd!=IDC_ZOOM_ZOOMIN)
	{
	  cmd_zoom=cmd;
	  updateToolBar();
	  if (do_layout) layout();
	}
      return;
    }

  int image_dpi=dimg->get_rounded_dpi();
  if (image_dpi<=0 || image_dpi>=2400) image_dpi=300;

  const int std_zoom[]={ 300, 150, 100, 75, 50, 25 };
  const int std_zooms=sizeof(std_zoom)/sizeof(std_zoom[0]);
  int cur_zoom=getZoom();

  if (cmd==IDC_ZOOM_ZOOMIN)
    {
      int new_zoom=cur_zoom*3/2;
      if (new_zoom>IDC_ZOOM_MAX-IDC_ZOOM_MIN)
	new_zoom=IDC_ZOOM_MAX-IDC_ZOOM_MIN;
      if (new_zoom>=20 && new_zoom<=330)
	{
	  int zoom_ind=0;
	  for(int i=0;i<std_zooms;i++)
	    if (abs(std_zoom[i]-new_zoom)<abs(std_zoom[zoom_ind]-new_zoom) &&
		std_zoom[i]>cur_zoom)
	      zoom_ind=i;
	  cmd=IDC_ZOOM_MIN+std_zoom[zoom_ind];
	  if (cmd==cmd_zoom)
	    {
	      if (zoom_ind>0)
		cmd=IDC_ZOOM_MIN+std_zoom[zoom_ind-1];
	      else
		cmd=IDC_ZOOM_MIN+new_zoom;
	    }
	}
      else cmd=IDC_ZOOM_MIN+new_zoom;
    }

  if (cmd==IDC_ZOOM_ZOOMOUT)
    {
      int new_zoom=cur_zoom*2/3;
      if (new_zoom<=5) new_zoom=5;
      if (new_zoom>=20 && new_zoom<=330)
	{
	  int zoom_ind=0;
	  for(int i=0;i<std_zooms;i++)
	    if (abs(std_zoom[i]-new_zoom)<abs(std_zoom[zoom_ind]-new_zoom) &&
		std_zoom[i]<cur_zoom)
	      zoom_ind=i;
	  cmd=IDC_ZOOM_MIN+std_zoom[zoom_ind];
	  if (cmd==cmd_zoom)
	    {
	      if (zoom_ind<std_zooms-1)
		cmd=IDC_ZOOM_MIN+std_zoom[zoom_ind+1];
	      else
		cmd=IDC_ZOOM_MIN+new_zoom;
	    }
	}
      else
	cmd=IDC_ZOOM_MIN+new_zoom;
    }

  if (cmd!=cmd_zoom)
    {
      cmd_zoom=cmd;
      updateToolBar();
      if (do_layout)
	layout();
    }
}

int
QDBase::getRotate(void) const
{
  return cmd_rotate;
}

void
QDBase::setRotate(int cmd, bool do_redraw, int src)
{
  if (mode_prio[rotate_src]>mode_prio[src]) return;
  rotate_src=src;
  
  static int rots[] = { 
    IDC_ROTATE_0, IDC_ROTATE_90, IDC_ROTATE_180, 
    IDC_ROTATE_270, IDC_ROTATE_0 };
  int i;
  if (cmd == IDC_ROTATE_RIGHT)
    for (i=0; i<4; i++)
      if (cmd_rotate == rots[i])
        cmd = rots[i+1];
  if (cmd == IDC_ROTATE_LEFT)
    for (i=1; i<5; i++)
      if (cmd_rotate == rots[i])
        cmd = rots[i-1];
  if (cmd >= IDC_ROTATE_0 && cmd <= IDC_ROTATE_270)
    cmd_rotate = cmd;

  if (dimg)
    {
      int drot = dimg->get_rotate();
      int crot = 0;
      for (crot=0; crot<4; crot++)
        if (cmd_rotate == rots[crot])
          break;
      if (crot>=4 || crot==drot)
        return;
      
      // Update annots
      for (GPosition pos=map_areas; pos; ++pos)
        {
          MapArea *ma=map_areas[pos];
          GRect rect=ma->get_bound_rect();
          dimg->map(rect);
          if ( ma->getComment()==search_results_name )
            ma->gmap_area->transform(rect);
        }
      dimg->set_rotate(crot);
      decodeAnno(false);
      for (GPosition pos=map_areas; pos; ++pos)
        {
          MapArea *ma=map_areas[pos];
          GRect rect=ma->get_bound_rect();
          dimg->unmap(rect);
          if ( ma->getComment()==search_results_name )
            ma->gmap_area->transform(rect);
        }
      layout(0);
      if (do_redraw)
        redraw();
    }
}

void
QDBase::slotEnableDisplayAllHLinks(void)
{
   DEBUG_MSG("QDBase::slotEnableDisplayAllHLinks() called\n");
   DEBUG_MAKE_INDENT(3);

   try
   {
      if (display_all_hlinks)
      {
	 DEBUG_MSG("already enabled => returning\n");
	 return;
      }
   
      GRect grect;
      grect.intersect(rectVisible, rectDocument);

      for(GPosition pos=map_areas;pos;++pos)
      {
	 GP<MapArea> ma=map_areas[pos];
	 ma->setForceAlwaysActive(true);
      }
      display_all_hlinks=true;
   } catch(const GException & exc)
   {
      showError(this, tr("DjVu Error"), exc);
   }
}

void
QDBase::slotDisableDisplayAllHLinks(void)
{
   DEBUG_MSG("QDBase::slotDisableDisplayAllHLinks() called\n");
   DEBUG_MAKE_INDENT(3);

   try
   {
      if (!display_all_hlinks)
      {
	 DEBUG_MSG("already disabled => returning\n");
	 return;
      }

      GRect grect;
      grect.intersect(rectVisible, rectDocument);

	 // We want to hide here
      for(GPosition pos=map_areas;pos;++pos)
      {
	 GP<MapArea> ma=map_areas[pos];
	 ma->setForceAlwaysActive(false);
	 ma->setActive(false, true);
      }

      display_all_hlinks=0;
   } catch(const GException & exc)
   {
      showError(this, tr("DjVu Error"), exc);
   }
}

void
QDBase::setCursor(void)
{
   DEBUG_MSG("QDBase::setCursor(): Installing cursor " <<
	     (in_hand_scroll ? "HAND2" :
	      in_zoom_select ? "ZOOM SELECT" :
	      in_paint ? "WAIT" :
	      cur_map_area ? "HLINK" :
	      "HAND1") << "\n");
   GPQCursor cur=in_hand_scroll ? cur_hand2 :
      in_zoom_select ? cur_zoom_select :
      isLensVisible() ? cur_blank :
#ifdef DEBUG
      in_paint ? cur_wait :
#endif
      pane_mode!=IDC_PANE ? cur_hand_hl :
      (cur_map_area && cur_map_area->isHyperlink()) ? cur_hand_hl : cur_hand1;
   
   if (((const QCursor *)cur)->handle()!=((const QCursor *)cur_last)->handle())
   {
      pane->setCursor(*(const QCursor *)cur);
      cur_last=cur;
   }
}

void
QDBase::createCursors(void)
{
   DEBUG_MSG("QDBase::createCursors(): loading cursors...\n");
   DEBUG_MAKE_INDENT(3);
#ifdef UNIX
   QSize wait_size(wait_width, wait_height);
   cur_wait=new QCursor(QBitmap(wait_size, (u_char *) wait_bits, TRUE),
		    QBitmap(wait_size, (u_char *) wait_mask_bits, TRUE),
		    wait_x, wait_y);

   QSize hand1_size(hand1_width, hand1_height);
   cur_hand1=new QCursor(QBitmap(hand1_size, (u_char *) hand1_bits, TRUE),
		     QBitmap(hand1_size, (u_char *) hand1_mask_bits, TRUE),
		     hand1_x, hand1_y);

   QSize hand2_size(hand2_width, hand2_height);
   cur_hand2=new QCursor(QBitmap(hand2_size, (u_char *) hand2_bits, TRUE),
		     QBitmap(hand2_size, (u_char *) hand2_mask_bits, TRUE),
		     hand2_x, hand2_y);

   QSize zoom_select_size(zoom_select_width, zoom_select_height);
   cur_zoom_select=new QCursor(QBitmap(zoom_select_size, 
                                       (u_char *) zoom_select_bits, TRUE),
                               QBitmap(zoom_select_size, 
                                       (u_char *) zoom_select_mask_bits, TRUE),
		     zoom_select_x, zoom_select_y);

   cur_hand_hl=new QCursor(ArrowCursor);	// TODO: get X11 cursor instead
   cur_ptr=new QCursor(ArrowCursor);
   cur_blank=new QCursor(BlankCursor);
#endif
}

//****************************************************************************
//******************************** Export ************************************
//****************************************************************************

void
QDBase::exportToPNM(void)
{
   DEBUG_MSG("QDViewer::exportToPNM(): exporting the image into a PPM file\n");
   DEBUG_MAKE_INDENT(3);

      // Guess on the format
   bool bw=getMode()==IDC_DISPLAY_BLACKWHITE;

   QString format_str=bw ? "PBM" : "PPM";
   GUTF8String ext=bw ? ".pbm" : ".ppm";
   
   bool raw=0;
   {
     QString mesg = 
       tr("This will export the DjVu image in either 'Raw %1'\n"
          "or 'ASCII %2' format. Which one do you prefer?\n")
       .arg(format_str)
       .arg(format_str);
       
       switch(QMessageBox::information(this, 
                                       tr("Exporting DjVu image..."), mesg,
                                       tr("&Raw"), tr("&Ascii"), tr("&Cancel"), 
                                       0, 2))
       {
         case 0: raw=1; break;
         case 1: raw=0; break;
         default: return;
       }
   }
   
   static const char *filters[] = { 0, 0, 0 };
   QString filter1 = tr("All files (*)");
   filters[0]= ((bw) ? "*.pbm" : "*.ppm");
   filters[1] = filter1;
 
   GURL file_url=dimg->get_djvu_file()->get_url();
   QString save_dir=QeFileDialog::lastSaveDir;
   if (!QFileInfo(save_dir).isDir())
      if (file_url.is_local_file_url())
	 while(true)
	 {
	    QString dir = QFileInfo(QStringFromGString(
                            file_url.UTF8Filename())).dirPath();
	    if (dir)
              {
                save_dir=dir;
	       break;
	    }
	    file_url=GURL::UTF8(file_url.name(),file_url.base().base());
	 }
   if (!QFileInfo(save_dir).isDir()) save_dir=QDir::currentDirPath();

   GUTF8String fname=GURL::expand_name(file_url.fname(), 
                                       GStringFromQString(save_dir));
   QFileInfo fi=QFileInfo(QStringFromGString(fname));
   fname=GStringFromQString(fi.dirPath())+"/"+GOS::basename(fname, ".djvu")+ext;
   
   QeFileDialog fd(save_dir, filters[0], this, "djvu_fd", TRUE);
   fd.setFilters((const char **) filters);
   fd.setCaption(tr("Select export file name..."));
   fd.setSelection(QStringFromGString(fname));
      
   if (fd.exec()==QDialog::Accepted)
   {
      int width=dimg->get_width();
      int height=dimg->get_height();
	 
      QProgressDialog progress(tr("Please stand by..."),
			       tr("&Abort"), height, this, "progress", TRUE);
      progress.setMinimumWidth(progress.sizeHint().width()*3/2);
      progress.setCaption(tr("DjVu: Exporting image..."));
      progress.setMinimumDuration(0);
      progress.setProgress(0);

      static char char_to_str[256][5];
      for(int i=0;i<256;i++) sprintf(char_to_str[i], "%d ", i);
      static short char_to_str_len[256];
      for(int i=0;i<256;i++) char_to_str_len[i]=strlen(char_to_str[i]);
      
      GRect doc_grect(0, 0, width, height);
      GUTF8String selected=GStringFromQString(fd.selectedFile());
      GP<ByteStream> gstr=ByteStream::create(GURL::Filename::UTF8(selected), "wb");
      ByteStream &str=*gstr;
      static const char *generated="#Image generated by DjVu";
      if (bw)
      {
	 GUTF8String head;
	 if (raw) head.format("P4\n%d %d\n", width, height);
	 else head.format("P1\n%s (xdpi=%u,ypdi=%u)\n%d %d\n",
			  generated, dimg->get_dpi(), dimg->get_dpi(),
			  width, height);
	 str.writall((void*)(const char *)head, head.length());
      } else
      {
	 GUTF8String head;
	 if (raw) head.format("P6\n%d %d\n255\n", width, height);
	 else head.format("P3\n%s (xdpi=%u,ydpi=%u)\n%d %d\n255\n",
			  generated, dimg->get_dpi(), dimg->get_dpi(),
			  width, height);
	 str.writall((void*)(const char *)head, head.length());
      }

      const int MAXPIXELS=100000;
      int band_height=MAXPIXELS/width;
      if (band_height>height*7/8) band_height=height;

      for(int band_y=height-band_height;band_height;band_y-=band_height)
      {
	 GRect grect(0, band_y, width, band_height);

	 if (bw)
	 {
	    GP<GBitmap> bmp=dimg->get_bitmap(grect, doc_grect);
	    if (!bmp) 
              G_THROW(ERR_MSG("QDBase.bitmap_not_found"));
	    for(int n=bmp->rows()-1;n>=0;n--)
	    {
	       const unsigned char * row=(*bmp)[n];
	       if (raw)
	       {
		  TArray<char> buffer(bmp->columns()/8+1);
		  char * ptr=buffer;
		  unsigned char acc=0;
		  unsigned char mask=0;
		  for(u_int c=0;c<bmp->columns();c++)
		  {
		     if (mask==0) mask=0x80;
		     if (row[c]) acc|=mask;
		     mask>>=1;
		     if (mask==0)
		     {
			*ptr++=acc;
			acc=mask=0;
		     }
		  }
		  if (mask!=0) *ptr++=acc;
		  str.writall((const char*)buffer, ptr-buffer);
	       }
	       else
	       {
		  TArray<char> buffer(256);
		  char * ptr=buffer;
		  for(u_int c=0;c<bmp->columns();)
		  {
		     unsigned char bit=(row[c] ? '1' : '0');
		     *ptr++=bit;
		     c++;
		     if (c==bmp->columns() || (c & 0xff)==0)
		     {
			*ptr++='\n';
			str.writall((const char*)buffer, ptr-buffer);
			ptr=buffer;
		     }
		  }
	       } // raw or not raw
	    } // for(row=...)
	 } else	// not bw
	 {
	    GP<GPixmap> pix;
	    switch(getMode())
	    {
	       case IDC_DISPLAY_COLOR:
		  pix=dimg->get_pixmap(grect, doc_grect, prefs.dScreenGamma);
		  break;
	       case IDC_DISPLAY_BACKGROUND:
		  pix=dimg->get_bg_pixmap(grect, doc_grect, prefs.dScreenGamma);
		  break;
	       case IDC_DISPLAY_FOREGROUND:
		  pix=dimg->get_fg_pixmap(grect, doc_grect, prefs.dScreenGamma);
		  break;
	    }
	    if (!pix) 
              G_THROW(ERR_MSG("QDBase.pixmap_not_found"));
	    if (raw)
	    {
	       TArray<char> buffer(3*pix->columns());
	       for(int y=pix->rows()-1;y>=0;y--) 
	       {
		  char * ptr=buffer;
		  const GPixel * p=(*pix)[y];
		  for (u_int x=0;x<pix->columns();x++)
		  {
		     *ptr++=p[x].r;
		     *ptr++=p[x].g;
		     *ptr++=p[x].b;
		  }
		  str.writall((void*)buffer, ptr-buffer);
	       } // for(row=...)
	    }
	    else
	    {
	       char buffer[32*13+1];
	       for(int y=pix->rows()-1;y>=0;y--)
	       {
		  const GPixel * p=(*pix)[y];
		  char * ptr=buffer;
		  for (u_int x=0;x<pix->columns();)
		  {
		     unsigned char ch;
		     strcpy(ptr, char_to_str[ch=p[x].r]); ptr+=char_to_str_len[ch];
		     strcpy(ptr, char_to_str[ch=p[x].g]); ptr+=char_to_str_len[ch];
		     strcpy(ptr, char_to_str[ch=p[x].b]); ptr+=char_to_str_len[ch];
		     *ptr++=' ';
		     x++;
		     if (x==pix->columns() || (x & 31)==0)
		     {
			*ptr++='\n';
			str.writall(buffer, ptr-buffer);
			ptr=buffer;
		     }
		  }
	       } // raw or not raw
	    }
	 } // bw or not bw
	 progress.setProgress(height-band_y);
	 if (progress.wasCancelled()) break;
	    
	 if (band_y-band_height<0) band_height=band_y;
      } // for(band_y=...)
   } // if (Accepted)
}

//****************************************************************************
//******************************* Toolbar ************************************
//****************************************************************************

bool
QDTBHider::eventFilter(QObject *, QEvent *)
{
      // Make sure, that toolbar will hide no matter how and no matter
      // where we leave the active zone
   if (!base->being_destroyed && base->toolbar && !base->toolbar->being_destroyed)
      if (base->isToolBarShown() && !base->isToolBarStuck())
	 if (base->needToHideToolBar())
	       // Note: this is the only place where we SLOWLY hide the toolbar
	       // We trust, that we intercept QApplication's events often
	       // enough, that we do not need to watch for anything else.
	    base->hideToolBar(true);
   return FALSE;
}

QDTBHider::QDTBHider(class QDBase * b) : QObject(b)
{
   base=b;
   qApp->setGlobalMouseTracking(TRUE);
   qApp->installEventFilter(this);
}

QDTBHider::~QDTBHider(void)
{
   qApp->setGlobalMouseTracking(FALSE);
}

void
QDBase::createToolBar(void)
{
   if (toolbar) delete toolbar; toolbar=0;
   toolbar_enabled=false;
   
   toolbar=new QDToolBar(main_widget, "qd_toolbar");
   toolbar->lower();
   toolbar->move(0, main_widget->height());
   toolbar->resize(main_widget->width(), toolbar->height());
   
   fillToolBar(toolbar);
   
   toolbar->installEventFilter(this);

   enableToolBar(override_flags.toolbar && prefs.toolBarOn);

   if (override_flags.toolbaropts & OverrideFlags::TOOLBAR_ALWAYS)
     stickToolBar();
   else if (override_flags.toolbaropts & OverrideFlags::TOOLBAR_AUTO)
     unStickToolBar();
   else if (prefs.toolBarAlwaysVisible)
     stickToolBar();
   else 
     unStickToolBar();
   
   toolbar->setOptions(override_flags.toolbaropts);
   updateToolBar();
}

void
QDBase::fillToolBar(QDToolBar * toolbar)
{
   try
     {
       mode_tbar=new QDTBarModePiece(toolbar);
       connect(&toolbar_timer, SIGNAL(timeout(void)), 
               this, SLOT(slotToolBarTimeout(void)));
       connect(mode_tbar, SIGNAL(sigSetZoom(int)),
               this, SLOT(slotSetZoom(int)));
       connect(mode_tbar, SIGNAL(sigSetMode(int)), 
               this, SLOT(slotSetMode(int)));
       connect(mode_tbar, SIGNAL(sigStick(bool)),
               this, SLOT(slotStickToolBar(bool)));
       connect(mode_tbar, SIGNAL(sigSetPaneMode(int)), 
               this, SLOT(slotSetPaneMode(int)));
       rotate_tbar = new QDTBarRotatePiece(toolbar);
       connect(rotate_tbar, SIGNAL(sigRotate(int)), 
               this, SLOT(slotSetRotate(int)));
     } 
   catch(const GException & exc)
     {
       showError(this, tr("DjVu Error"), exc);
     }
}

void
QDBase::updateToolBar(void)
{
   if (mode_tbar)
     {
       bool hastxt = false;
       G_TRY {
         if (dimg && dimg->get_djvu_file()->get_text())
           hastxt = true;
       } G_CATCH_ALL { } G_ENDCATCH;
       mode_tbar->update(getMode(true), true,
	 		 cmd_zoom, getZoom(), pane_mode, hastxt);
     }
   if (toolbar)
   {
      if (!dimg) 
        toolbar->setEnabled(FALSE);
      else if (!toolbar->isEnabled())
        {
          toolbar->setEnabled(TRUE);
          updateToolBar();	// And do the update again
        }
   }
}

bool
QDBase::isToolBarEnabled(void) const
{
   if ( !toolbar ) return FALSE;

   return toolbar_enabled;
}

bool
QDBase::isToolBarShown(void) const
{
   return isToolBarEnabled() && toolbar_shown;
}

bool
QDBase::isToolBarStuck(void) const
{
   return isToolBarEnabled() && mode_tbar->isStuck();
}

void
QDBase::stickToolBar(void)
{
   if (mode_tbar) 
      mode_tbar->stick(true);
}

void
QDBase::unStickToolBar(void)
{
   if (mode_tbar) 
      mode_tbar->stick(false);
}

void
QDBase::enableToolBar(bool on)
{
  if ( !toolbar ) return;

  if (on!=isToolBarEnabled())
    {
      if (on)
	{
	  prefs.toolBarOn=1;
	  layout();
	  updateToolBar();
	}
      else
	{
	  prefs.toolBarOn=0;
	  layout();
	}
    }
}

bool
QDBase::needToShowToolBar(void) const
{
   if (toolbar)
   {
      QPoint pnt=pane->mapFromGlobal(QCursor::pos());
      if (pnt.x()>=0 && pnt.x()<pane->width() &&
	  pnt.y()>=pane->height()-toolbar->height() &&
	  pnt.y()<pane->height()+toolbar_edge)
	 return true;
   }
   return false;
}

bool
QDBase::needToHideToolBar(void) const
{
   if (toolbar)
   {
	 // If any popup menu is active we do not want to hide (it may
	 // happen that this popup menu is a QListBox of one of QComboBoxes
	 // in the toolbar...)
      if (qApp->activePopupWidget()) return false;

	 // Then see if the mouse is not within the area of the toolbar's
	 // activation and return false if it is.
      QPoint pnt=pane->mapFromGlobal(QCursor::pos());
      if (pnt.x()>=0 && pnt.x()<pane->width() &&
	  pnt.y()>=pane->height()-toolbar->height() &&
	  pnt.y()<pane->height()+toolbar_edge)
	 return false;
   }
   return true;
}

void
QDBase::showToolBar(bool slow)
{
   DEBUG_MSG("QDBase::showToolBar(): showing it\n");
   DEBUG_MAKE_INDENT(3);

   if (!dimg || !dimg->get_width() || !dimg->get_height() ||
       !isToolBarEnabled()) return;
   
   if (!isToolBarStuck() && !isToolBarShown())
   {
      if (showing_toolbar) return;

      if (!slow) toolbar->move(0, pane->height()+toolbar_edge+1-toolbar->height());
      else
      {
	    // I need this flag because I run qApp->processEvents() from this
	    // function and I don't want to reenter here.
	 showing_toolbar=1;

	 try
	 {
	    updateToolBar();
      
	    const int steps=10;
	    for(int i=1;i<steps;i++)
	    {
		  // See, maybe the toolbar has been stuck while we're trying
		  // to show it here slowly and smoothly.
	       if (isToolBarStuck()) break;
		  // We recalculate end_y and start_y at each iteration here
		  // because the user might resize the main window while we're
		  // moving the toolbar. We don't want to use outdated numbers
	       int start_y=toolbar->y();
	       int end_y=pane->height()+toolbar_edge+1-toolbar->height();
	       toolbar->move(0, start_y+(end_y-start_y)*i/(steps-1));
	       qApp->processEvents(10);
	       GOS::sleep(10);
	    }
	    toolbar_timer.stop();
	 } catch(...)
	 {
	    showing_toolbar=0;
	    throw;
	 }

	 showing_toolbar=0;
      }
   }
   toolbar_shown=1;
}

void
QDBase::hideToolBar(bool slow)
{
  DEBUG_MSG("QDBase::hideToolBar(): slow=" << slow << "\n");
  DEBUG_MAKE_INDENT(3);

  if (!dimg || !dimg->get_width() || !dimg->get_height() ||
      !isToolBarEnabled())
    return;

  if (!isToolBarStuck() && isToolBarShown())
    {
      if (!slow)
	{
	  toolbar->move(0, pane->height());
	  toolbar_shown=0;
	}
      else if (!toolbar_timer.isActive())
	toolbar_timer.start(prefs.toolBarDelay, TRUE);
    }
}

void
QDBase::slotToolBarTimeout(void)
{
   if (isToolBarShown() && !isToolBarStuck())
   {
	 // Hide the toolbar SLOWLY
      QPixmap cache(pane->width(), toolbar->height());
      paint(&cache, 0, pane->height()-toolbar->height(),
	    GRect(0, pane->height()-toolbar->height(),
		  pane->width(), toolbar->height()));

      QWidget w(pane);
      w.setBackgroundMode(NoBackground);
      w.setGeometry(0, pane->height()-toolbar->height(),
		    pane->width(), toolbar->width());
      w.show();

      const int steps=10;
      int start_y=toolbar->y();
      int end_y=pane->height();
      int old_y=start_y;
      int i;
      for(i=1;i<steps;i++)
      {
	 if (needToShowToolBar())
	       // Mouse moved back inside the toolbar =>
	       // Stop this loop and show the toolbar again.
	    break;
      
	 int new_y=start_y+(end_y-start_y)*i/(steps-1);
	 toolbar->move(0, new_y);
	 qApp->syncX();
	 QPainter painter(&w);
	 int y=old_y-(pane->height()-toolbar->height());
	 painter.drawPixmap(0, y, cache, 0, y,
			    pane->width(), new_y-old_y);
	 painter.end();
	 old_y=new_y;

	 GOS::sleep(10);
      }
      toolbar_shown=0;
      if (i<steps) showToolBar();
      w.hide();
   }
}

void
QDBase::slotSetRotate(int cmd_rotate)
{
  if (getRotate() != cmd_rotate)
    setRotate(cmd_rotate,1,SRC_MANUAL);
}

void
QDBase::slotSetZoom(int cmd_zoom)
{
  if (getCMDZoom()!=cmd_zoom)
    setZoom(cmd_zoom, true, SRC_MANUAL);
}

void
QDBase::slotSetPaneMode(int cmd_pane)
{
   pane_mode=cmd_pane;
   setCursor();
}

void
QDBase::slotSetMode(int cmd_mode)
{
   if (getMode(true)!=cmd_mode)
      setMode(cmd_mode, true, SRC_MANUAL);
}

void
QDBase::slotStickToolBar(bool on)
{
   if (on)
   {
      prefs.toolBarAlwaysVisible=1;
      toolbar_shown=1;
      layout();
   } else
   {
      prefs.toolBarAlwaysVisible=0;
      layout();
   }
}

QWidget *
QDBase::createThumbnails(bool) { return 0; }

void
QDBase::showThumbnails(void)
{
   if (dimg && !splitter &&
       override_flags.cmd_zoom!=IDC_ZOOM_STRETCH)
   {
      
      bool rowMajor = (override_flags.thumbnails == OverrideFlags::THUMB_TOP ||
		       override_flags.thumbnails == OverrideFlags::THUMB_BOTTOM);
      
	 // Weird things happen if I try to recreate a widget with
	 // preset min/max dimensions (splitter takes the maximum size
	 // of the thumb_widget and even resize() doesn't help)
      if (!thumb_widget)
      {
	 thumb_widget=createThumbnails(rowMajor);
      }
      if (thumb_widget)
      {

//  	 static const int qcoord_max=(QCOORD_MAX<32767)?QCOORD_MAX:32767;
//  	 thumb_widget->setMaximumWidth(qcoord_max);
//  	 thumb_widget->resize(thumb_widget->sizeHint());

	 if ( rowMajor ) 
	    splitter=new QSplitter(QSplitter::Vertical, this, "splitter");
	 else 
	    splitter=new QSplitter(this, "splitter");

	 thumb_widget->recreate(splitter, 0, QPoint(0, 0));
	 //splitter->setResizeMode(thumb_widget, QSplitter::KeepSize);
	 splitter->setResizeMode(thumb_widget, QSplitter::FollowSizeHint);
	 main_widget->recreate(splitter, 0, QPoint(0, 0));
	 splitter->setResizeMode(main_widget, QSplitter::Stretch);
	 
	 if ( override_flags.thumbnails == OverrideFlags::THUMB_RIGHT ||
	      override_flags.thumbnails == OverrideFlags::THUMB_BOTTOM ) 
	    splitter->moveToLast(thumb_widget);

	 splitter->resize(size());
	 splitter->show();
      }
   }
}

void
QDBase::hideThumbnails(void)
{
   if (splitter)
   {
      splitter->hide();
      thumb_widget->hide();
      main_widget->recreate(this, 0, QPoint(0, 0));
      thumb_widget->recreate(this, 0, QPoint(0, 0));
      delete splitter;
      splitter=0;
      main_widget->resize(size());
      main_widget->show();
   }
}

// END OF FILE
