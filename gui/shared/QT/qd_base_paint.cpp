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
// $Id: qd_base_paint.cpp,v 1.13 2008/08/05 20:52:26 bpearlmutter Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qd_base_paint.h"

#include "GOS.h"
#include "debug.h"
#include "qt_imager.h"
#include "qd_painter.h"
#include "djvu_base_res.h"
#include "qd_thr_yielder.h"
#include "qlib.h"
#include "qx_imager.h"
#include "qxlib.h"

#include <qapplication.h>

void
QDBase::redraw(const GRect * grect)
{
   DEBUG_MSG("QDBase::redraw(): Redrawing the whole rectVisible\n");

   bm_cache.clear();
   pm_cache.clear();

   QRect qrect;
   if (grect) qrect=G2Q(*grect);
   else qrect=pane->geometry();
   
      // I can't use repaint() here since this will repaint immediately w/o
      // smart event compression
      //
      // update() is also useless because it clears the window with default
      // background, which is awful. If I set the NoBackground mode just
      // temporarily, this doesn't help compressing events... May be X11
      // processes combination XChangeWindowAttributes(), XClearArea()
      // XChangeWindowAttributes() too long, may be QT does smth extra
      //
      // Sending Event_Paint using QApplication::postEvent() doesn't help
      // because QT doesn't optimize events sent in this way
      //
      // Therefore we send the X11 expose event manually :(

#ifdef UNIX
   x11Redraw(pane, &qrect);
#else
   QPaintEvent * ev=new QPaintEvent(qrect);
   QApplication::postEvent(pane, ev);
#endif
}

void
QDBase::setBackgroundColor(u_int32 color, int do_redraw)
{
   if (qeImager)
     {
       back_color=color;
       back_pixmap=qeImager->getColorPixmap(64, 64, color);
     }
   if (do_redraw) 
     redraw();
}   

void
QDBase::paint(const GRect & in_rect)
{
   in_paint=1;
   setCursor();
   try
   {
      paint(pane, 0, 0, in_rect);
      if (isLensVisible()) paintLens(&in_rect);
   } catch(...)
   {
      in_paint=0;
      setCursor();
      throw;
   }
   in_paint=0;
   setCursor();
}

static GRect
invRect(const GRect & rect)
{
   return GRect(rect.xmin, -rect.ymax, rect.width(), rect.height());
}

class HilitedRect : public GPEnabled
{
public:
   GRect	rect;
   GP<GPixmap>	pixmap;

   HilitedRect(const GRect & _rect, const GP<GPixmap> & _pixmap) :
	 rect(_rect), pixmap(_pixmap) {}
};

void
QDBase::paint(QPaintDevice * drawable,	// Where to paint
	      int dr_x, int dr_y,	// Drawable's coords in the pane
	      const GRect & in_rect,	// GRect in the pane
	      bool for_lens)
{
   DEBUG_MSG("QDBase::paint(): drawing rectangle (" <<
	     in_rect.xmin << ", " << in_rect.ymin << ", " <<
	     in_rect.width() << ", " << in_rect.height() << ")\n");
   DEBUG_MAKE_INDENT(3);

   if (!in_rect.width() || !in_rect.height())
   {
      DEBUG_MSG("returning, since rectangle is void\n");
      return;
   }

      // Just in case: add timer to check the margin caches...
   cache_timer.start(0, TRUE);
   
      // Get target rectangle in window and in image
   GRect grectWin=in_rect;
   grectWin.intersect(grectWin, rectVisible);
   grectWin.intersect(grectWin, rectDocument);

   QePainter p(drawable);
   p.translate(-dr_x, -dr_y);
      
      // Display image
   if (dimg && dimg->get_width() && dimg->get_height() && !grectWin.isempty())
   {
      GP<GPixmap> pm;
      GP<GBitmap> bm;
      
      p.setClipRect(grectWin.xmin-dr_x, grectWin.ymin-dr_y,
		    grectWin.width(), grectWin.height());
	 
	 // Mapper required for caching code
      GRectMapper doc_mapper;
      doc_mapper.set_input(rectDocument);
      doc_mapper.set_output(GRect(0, 0, rectDocument.width(), rectDocument.height()));
      doc_mapper.mirrory();
	 
	 // Display by chunks of at most MAXPIXELS pixels
#if THREADMODEL==COTHREADS
      const int MAXPIXELS=200000;
#else
      const int MAXPIXELS=500000;
#endif
      GRect grectBand=grectWin;
      grectBand.ymax=grectBand.ymin;
      while (grectBand.ymax < grectWin.ymax)
      {
	    // Compute new band
	 grectBand.ymin=grectBand.ymax;
	 grectBand.ymax=grectBand.ymin + MAXPIXELS / grectBand.width();
	 if (grectBand.ymax > grectWin.ymax)
	    grectBand.ymax=grectWin.ymax;
	    
	 GRect grectCheck=grectBand;
	 while(!grectCheck.isempty())
	 {
#if THREADMODEL==COTHREADS
	    if (QDThrYielder::getTasksNum()>1) GThread::yield();
#endif
	    
	    GP<GPixmap> pm_cached;
	    GP<GBitmap> bm_cached;
	    GRect left_grect=grectCheck;
	    doc_mapper.map(left_grect);
	    GRect cached_grect;
	    if (bm_cache.isAllocated())
	       bm_cached=bm_cache.getPixmap(left_grect, cached_grect);
	    if (pm_cache.isAllocated())
	       pm_cached=pm_cache.getPixmap(left_grect, cached_grect);
	       
	    GRect grectDisp;
	    if (!for_lens && (bm_cached || pm_cached) &&
		!cached_grect.isempty())
	    {
	       DEBUG_MSG("rect (" << cached_grect.xmin << ", " <<
			 cached_grect.ymin << ", " << cached_grect.width() <<
			 ", " << cached_grect.height() << ") has been cached\n");
		  
	       doc_mapper.unmap(left_grect);
	       doc_mapper.unmap(cached_grect);
	       grectCheck=left_grect;
	       grectDisp=cached_grect;
	       bm=bm_cached;
	       pm=pm_cached;
	    }
            else
	    {
	       DEBUG_MSG("rect (" << grectCheck.xmin << ", " <<
			 grectCheck.ymin << ", " << grectCheck.width() <<
			 ", " << grectCheck.height() << ") has NOT been cached\n");
		  
	       grectDisp=grectCheck;
	       grectCheck.clear();
		  // Compute information for chosen display mode
	       switch (getMode())
	       {
		  case IDC_DISPLAY_COLOR:
		     pm=dimg->get_pixmap(invRect(grectDisp),
					 invRect(rectDocument),
					 prefs.dScreenGamma);
		     if (!pm) bm=dimg->get_bitmap(invRect(grectDisp),
						  invRect(rectDocument),
						  sizeof(int));
		     break;
		  case IDC_DISPLAY_BLACKWHITE:
		     bm=dimg->get_bitmap(invRect(grectDisp),
					 invRect(rectDocument), sizeof(int));
		     break;
		  case IDC_DISPLAY_BACKGROUND:
		     pm=dimg->get_bg_pixmap(invRect(grectDisp),
					    invRect(rectDocument),
					    prefs.dScreenGamma);
		     break;
		  case IDC_DISPLAY_FOREGROUND:
		     pm=dimg->get_fg_pixmap(invRect(grectDisp),
					    invRect(rectDocument),
					    prefs.dScreenGamma);
		     break;
	       }
	    }

	       // Will be used in BW mode with hyperlinks only
	    GRect patch_rect;
	    GP<GPixmap> patch_pm;
	    
	       // First draw everything forcing inACTIVE mode into the GBitmap/GPixmap
	       // If a hyperling is "always active", draw it in full
	    if (pm)
              {
                for(GPosition pos=map_areas;pos;++pos)
                  {
                    GP<MapArea> ma=map_areas[pos];
                    if (ma->isAlwaysActive())
                      ma->draw(grectDisp, pm, MapArea::DRAW_ACTIVE);
                    else ma->draw(grectDisp, pm, MapArea::DRAW_INACTIVE);
                  }
              }
	    else if (bm)
              {
                // Since we have a GBitmap here and hyperlinks are going to
                // display themselves in color, we need to convert it to
                // GPixmap... At least part of it...
                for(GPosition pos=map_areas;pos;++pos)
                  {
                    GRect brect=map_areas[pos]->get_bound_rect();
                    ma_mapper.map(brect);
                    brect.inflate(3, 3);
                    GRect hrect;
                    hrect.recthull(patch_rect, brect);
                    patch_rect=hrect;
                  }
                
                GRect irect;
                irect.intersect(patch_rect, grectDisp);
                patch_rect=irect;
                
                if (!patch_rect.isempty())
                  {
                    GRect rect=patch_rect;
                    rect.translate(-grectDisp.xmin, -grectDisp.ymin);
                    rect=GRect(rect.xmin, bm->rows()-rect.ymax,
                               rect.width(), rect.height());
                    patch_pm=GPixmap::create(*bm, rect);
                    for(GPosition pos=map_areas;pos;++pos)
                      {
                        GP<MapArea> ma=map_areas[pos];
                        if (ma->isAlwaysActive())
                          ma->draw(patch_rect, patch_pm, MapArea::DRAW_ACTIVE);
                        else ma->draw(patch_rect, patch_pm, MapArea::DRAW_INACTIVE);
                      }
                  }
              }
            
            // Now update cache of every hyperlink, that maintains it
            // It's up to the hyperlink to decide if it needs to update it
            // or not. We just give this opportunity to everybody
	    if (!for_lens)
              if (pm || patch_pm)
                for(GPosition pos=map_areas;pos;++pos)
		  {
                    MapArea & ma=*map_areas[pos];
                    if (pm) ma.updateCache(grectDisp, pm, &mapper);
                    else ma.updateCache(patch_rect, patch_pm, &mapper);
		  }
            
            // Now draw hyperlinks, which are not "always active", but
            // are active right now
	    if (pm || patch_pm)
	      for (GPosition pos=map_areas;pos;++pos)
		{
		  GP<MapArea> ma=map_areas[pos];
		  if (!ma->isAlwaysActive() && ma->isActive())
		    {
		      if (pm)
			ma->draw(grectDisp, pm, MapArea::APPLY_ACTIVE);
		      else
			ma->draw(patch_rect, patch_pm, MapArea::APPLY_ACTIVE);
		    }
		}
            
            // Dither the generated GPixmap[s]
	    if (pm)
              {
                // Map rectangle into image rectangle
                GRect grectImg=grectDisp;
                mapper.map(grectImg);
                if (qxImager)
                  qxImager->dither(*pm, grectImg.xmin, grectImg.ymin);
              } 
            else if (patch_pm)
              {
                GRect rect=patch_rect;
                mapper.map(rect);
                if (qxImager)
                  qxImager->dither(*patch_pm, rect.xmin, rect.ymin);
              }
            
            // Finally - draw the GBitmap/GPixmap into the window
	    if (pm) 
              {
                p.drawPixmap(grectDisp, pm, true);
              }
	    else if (bm)
              {
                if (!patch_pm) 
                  p.drawBitmap(grectDisp, bm, true);
                else 
                  p.drawPatchedBitmap(grectDisp, bm,
                                      patch_rect, patch_pm, true);
              }
	    else if (djvu_logo_bmp->rows() && override_flags.logo)
              {
                GRect irect;
                if (irect.intersect(rectDocument, rectVisible))
		  p.drawTiledPixmap(G2Q(irect), back_pixmap,
				    QPoint((irect.xmin-rectDocument.xmin) 
                                           % back_pixmap.width(),
					   (irect.ymin-rectDocument.ymin) 
                                           % back_pixmap.height()));
                GRect bmp_rect(irect.xmax-djvu_logo_bmp->columns()-10,
                               irect.ymax-djvu_logo_bmp->rows()-10,
                               djvu_logo_bmp->columns(), djvu_logo_bmp->rows());
                if (irect.intersect(bmp_rect, grectDisp))
		  p.drawBitmap(irect, irect.xmin-bmp_rect.xmin,
			       irect.ymin-bmp_rect.ymin, djvu_logo_bmp, true);
              } 
            else
              {
                GRect irect;
                if (irect.intersect(rectDocument, rectVisible))
		  p.drawTiledPixmap(G2Q(irect), back_pixmap,
				    QPoint((irect.xmin-rectDocument.xmin) 
                                           % back_pixmap.width(),
					   (irect.ymin-rectDocument.ymin) 
                                           % back_pixmap.height()));
              }
	    
	    if (bm || pm)	// Do not ever draw the logo
              djvu_logo_bmp = GBitmap::create();

	       // And draw those hyperlinks, which are in the "OUTLINE" mode
	    for(GPosition pos=map_areas;pos;++pos)
	    {
	       GP<MapArea> ma=map_areas[pos];
		  // With the outline mode we need to see if we really
		  // want to draw the map area. It's not like the regular
		  // mode when there is one draw() for both ACTIVE and
		  // INACTIVE modes. Here we need to draw smth only if
		  // there is a need to do it.
	       if ((ma->isActive() && ma->isActiveOutlineMode()) ||
		   (!ma->isActive() && ma->isInactiveOutlineMode()))
		 ma->drawOutline(grectDisp, &p);
	    }

	       // And draw hyperlinks with enabled edit controls
	    for(GPosition pos=map_areas;pos;++pos)
	    {
	       GP<MapArea> ma=map_areas[pos];
		  // In outline mode the edit controls are drawn automatically
		  // by drawOutline(). No need to do it here
	       if (ma->editControlsEnabled() &&
		   !ma->isActiveOutlineMode() &&
		   !ma->isInactiveOutlineMode())
		  ma->drawEditControls(grectDisp, &p);
	    }
	 }
      } // while(...)

      p.setClipping(FALSE);
      grectWin = rectVisible;
   }
   else
   {
     grectWin = in_rect;
   }

       // Set new clipping rectangle
   grectWin.intersect(grectWin, in_rect);
   p.setClipRect(grectWin.xmin-dr_x, grectWin.ymin-dr_y,
		 grectWin.width(), grectWin.height());
   
      // Draw margins and frame around the document
   if (rectDocument.xmin>rectVisible.xmin)
   {
      GRect crect=rectVisible;
      crect.xmax=rectDocument.xmin;
      GRect irect;
      if (irect.intersect(crect, in_rect))
	 p.drawTiledPixmap(G2Q(irect), back_pixmap,
			   QPoint((irect.xmin-rectDocument.xmin) 
                                  % back_pixmap.width(),
				  (irect.ymin-rectDocument.ymin) 
                                  % back_pixmap.height()));
   }
   if (rectDocument.xmax<rectVisible.xmax)
   {
      GRect crect=rectVisible;
      crect.xmin=rectDocument.xmax;
      GRect irect;
      if (irect.intersect(crect, in_rect))
	 p.drawTiledPixmap(G2Q(irect), back_pixmap,
			   QPoint((irect.xmin-rectDocument.xmin) 
                                  % back_pixmap.width(),
				  (irect.ymin-rectDocument.ymin) 
                                  % back_pixmap.height()));
   }
   if (rectDocument.ymin>rectVisible.ymin)
   {
      GRect crect=rectVisible;
      crect.ymax=rectDocument.ymin;
      GRect irect;
      if (irect.intersect(crect, in_rect))
	 p.drawTiledPixmap(G2Q(irect), back_pixmap,
			   QPoint((irect.xmin-rectDocument.xmin) 
                                  % back_pixmap.width(),
				  (irect.ymin-rectDocument.ymin) 
                                  % back_pixmap.height()));
   }
   if (rectDocument.ymax<rectVisible.ymax)
   {
      GRect crect=rectVisible;
      crect.ymin=rectDocument.ymax;
      GRect irect;
      if (irect.intersect(crect, in_rect))
	 p.drawTiledPixmap(G2Q(irect), back_pixmap,
			   QPoint((irect.xmin-rectDocument.xmin) 
                                  % back_pixmap.width(),
				  (irect.ymin-rectDocument.ymin) 
                                  % back_pixmap.height()));
   }
   if (override_flags.frame)
     {
       if (qeImager)
         p.setPen(qeImager->getGrayColor(0.7));
       p.drawRect(rectDocument.xmin, rectDocument.ymin,
                  rectDocument.width(), rectDocument.height());
     }
   p.setClipping(FALSE);
   p.end();
   DEBUG_MSG("QDBase::paint(): DONE\n");
}

void
QDBase::paintLens(const GRect * clip_rect)
{
   DEBUG_MSG("QDBase::paintLens(): repainting lens contents\n");
   DEBUG_MAKE_INDENT(3);

   GRect irect=lens_rect;
   if (clip_rect)
      irect.intersect(lens_rect, *clip_rect);
   if (!irect.isempty())
   {
      GRect savedRectDocument=rectDocument;
      try
      {
	 int xc=lens_rect.xmin+lens_rect.width()/2;
	 int yc=lens_rect.ymin+lens_rect.height()/2;

	 rectDocument.xmin=xc-(xc-rectDocument.xmin)*prefs.magnifierScale/10;
	 rectDocument.ymin=yc-(yc-rectDocument.ymin)*prefs.magnifierScale/10;
	 rectDocument.xmax=xc+(rectDocument.xmax-xc)*prefs.magnifierScale/10;
	 rectDocument.ymax=yc+(rectDocument.ymax-yc)*prefs.magnifierScale/10;

	 setMappers();

	    // Do not call paint(irect) here, or you'll get into
	    // infinite recursion. Besides we need special processing:
	    //   - No margin caches
	    //   - No hyperlink caches updates
	 paint(pane, 0, 0, irect, true);

	 rectDocument=savedRectDocument;
	 setMappers();

	 QPainter p(pane);
	 if (clip_rect)
	    p.setClipRect(G2Q(*clip_rect));
	 p.setPen(black);
	 p.drawRect(G2Q(lens_rect));
	 
	 int dash=lens_rect.width()/10;
	 p.drawLine(lens_rect.xmin, yc, lens_rect.xmin+dash, yc);
	 p.drawLine(lens_rect.xmax-1, yc, lens_rect.xmax-1-dash, yc);
	 p.drawLine(xc, lens_rect.ymin, xc, lens_rect.ymin+dash);
	 p.drawLine(xc, lens_rect.ymax-1, xc, lens_rect.ymax-1-dash);
         p.setClipping(FALSE);
         p.end();
      } 
      catch(...)
      {
	 rectDocument=savedRectDocument;
	 setMappers();
	 throw;
      }
   }
}
