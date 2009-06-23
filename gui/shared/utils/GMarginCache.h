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
// $Id: GMarginCache.h,v 1.10 2008/08/05 20:51:37 bpearlmutter Exp $
// $Name: release_3_5_22 $

#ifndef HDR_GMARGIN_CACHE
#define HDR_GMARGIN_CACHE
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif

#include "GRect.h"
#include "GContainer.h"
#include "GSmartPointer.h"
#include "GPixmap.h"
#include "GException.h"
#include "debug.h"

#include <math.h>
#include <int_types.h>

// If a cache is disabled, it must be empty, but if a cached is enabled it
// may be empty or not.
template<class TPixmap>
class GMarginCache
{
private:
   bool		enabled;
   GP<TPixmap>	left_pix, right_pix;
   GP<TPixmap>	top_pix, bottom_pix;
   GRect	left_rect, right_rect;
   GRect	top_rect, bottom_rect;
   GRect	inner_rect;		// rectangles surrounding the cached
   GRect	outer_rect;		// area (ring) from inside and outside
   int		doc_width, doc_height;
   int		win_width, win_height;
   u_long	max_size;
   
   bool		isSizeOK(void) const;
public:
      /// Sets total maximum size of the pixmap to cache
   void		setMaxSize(u_long _max_size);
      /** Sets dimensions of the window and document.
	  This gives the cache enough information to compute the border
	  for caching. */
   void		resize(int doc_width, int doc_height,
		       int win_width, int win_height);
      /// Allocates internal pixmaps
   void		allocate(void);
      /// Deallocates internal pixmaps. Frees memory.
   void		deallocate(void);
      /// Erases contents of the internal pixmaps. Doesn't free them though
   void		clear(void);
      /// Returns #TRUE# if the cache is allocates
   bool		isAllocated(void) const;
      /// Allocates the internal pixmaps and sets the #enabled# flag
   void		enable(void) { enabled=true; allocate(); }
      /// Deallocates the internal pixmaps and clears the #enabled# flag
   void		disable(void) { deallocate(); enabled=false; }
      /** Returns #TRUE# if the cache is enabled. The cached may be
	  enabled, but not allocated (if it would take too much space
	  to allocate it), but if a cache is disabled it's definitely
	  deallocated. */
   bool		isEnabled(void) const { return enabled; }

      /** Suggests next rectangle from the cache to be filled. Returns
	  #FALSE# if everything has been cached.

	  The coordinate system has #Y# axis pointing upward. */
   bool		nextRectToCache(int maxpixels, GRect & grect);
      /** Adds contents of rectangle #grect# to the cache.
	  The coordinate system has #Y# axis pointing upward */
   void		addRect(const GRect & grect, const GP<TPixmap> & pix);
      /** Accepts rectangle #grect# as a rectangle to fill with cached
	  data. Modifies it according to the available cache contents
	  and sets #grect_cached# to contain:
	  \begin{description}
	     \item[grect_cached] is a rectangle describing the data
	          found in the cached and returned in the output pixmap.
	     \item[grect] is the modified source rectangle, which may
	          be passed to this function again to retrieve the
		  remaining data if any.
	  \end{description}
	  If there is no cached data corresponding to #grect#, #ZERO#
	  is returned and #grect# is not modified.

	  The coordinate system has #Y# axis pointing upward. */
   GP<TPixmap>	getPixmap(GRect & grect, GRect & grect_cached);
   
   GMarginCache(void) : enabled(false), doc_width(0), doc_height(0),
      win_width(0), win_height(0) {}
   virtual ~GMarginCache(void) {}
};

//***************************************************************************
//**************************** Implementation *******************************
//***************************************************************************

template <class TPixmap> bool
GMarginCache<TPixmap>::isSizeOK(void) const
{
   // is there any other way to calculate this without
   // instantiate an object ?
   // BUGGY
   GP<TPixmap> gpix=TPixmap::create();
   TPixmap &pix=*gpix;
   
   return
      (left_rect.width()*left_rect.height()+
       right_rect.width()*right_rect.height()+
       bottom_rect.width()*bottom_rect.height()+
       top_rect.width()*top_rect.height())*sizeof(*pix[0])<max_size;
}

template <class TPixmap> void
GMarginCache<TPixmap>::setMaxSize(u_long _max_size)
{
   DEBUG_MSG("GMarginCache::setMaxSize(): max_size=" << _max_size << "\n");
   max_size=_max_size;
   
   if (!isSizeOK()) deallocate();
   else allocate();
}

template <class TPixmap> void
GMarginCache<TPixmap>::allocate(void)
{
   DEBUG_MSG("GMarginCache::allocate(): Allocating for doc=(" << doc_width << "x" <<
	     doc_height << "), win=(" << win_width << "x" << win_height << ")\n");
   DEBUG_MAKE_INDENT(3);
   if (!doc_width && !doc_height)
   {
      DEBUG_MSG("doc_rect is empty => returning\n");
      return;
   }
   if (isAllocated())
   {
      DEBUG_MSG("but we're already allocated => return\n");
      return;
   }
   
   int xmin, xmax, ymin, ymax;
   xmin=doc_width-win_width;	if (xmin<0) xmin=0;
   xmax=win_width;		if (xmax>doc_width) xmax=doc_width;
   ymin=doc_height-win_height;	if (ymin<0) ymin=0;
   ymax=win_height;		if (ymax>doc_height) ymax=doc_height;
   if (xmax<xmin) xmax=xmin;
   if (ymax<ymin) ymax=ymin;
   inner_rect=GRect(xmin, ymin, xmax-xmin, ymax-ymin);
   if (inner_rect.isempty())
      inner_rect=GRect(doc_width/2, doc_height/2, 0, 0);
   
   outer_rect=inner_rect;
   
   left_rect=GRect(0, 0, inner_rect.xmin, doc_height);
   right_rect=GRect(inner_rect.xmax, 0, doc_width-inner_rect.xmax, doc_height);
   if (inner_rect.isempty())
   {
      top_rect.clear(); bottom_rect.clear();
   } else
   {
      top_rect=GRect(inner_rect.xmin, inner_rect.ymax,
		     inner_rect.width(), doc_height-inner_rect.ymax);
      bottom_rect=GRect(inner_rect.xmin, 0, inner_rect.width(),
			inner_rect.ymin);
   }
   
   if (!isSizeOK())
   {
      DEBUG_MSG("everything seems to be too huge => give up caching\n");
      deallocate();
   } else
   {
      DEBUG_MSG("estimated size is OK => allocating pixmaps\n");
      if (left_rect.isempty()) left_pix=0;
      else left_pix=TPixmap::create(left_rect.height(), left_rect.width());
      if (right_rect.isempty()) right_pix=0;
      else right_pix=TPixmap::create(right_rect.height(), right_rect.width());
      if (top_rect.isempty()) top_pix=0;
      else top_pix=TPixmap::create(top_rect.height(), top_rect.width());
      if (bottom_rect.isempty()) bottom_pix=0;
      else bottom_pix=TPixmap::create(bottom_rect.height(), bottom_rect.width());
   }
}

template <class TPixmap> void
GMarginCache<TPixmap>::resize(int doc_width, int doc_height,
			      int win_width, int win_height)
{
   DEBUG_MSG("GMarginCache::resize(): resizing to doc=(" << doc_width << "x" <<
	     doc_height << "), win=(" << win_width << "x" << win_height << ")\n");
   DEBUG_MAKE_INDENT(3);
   
   if (((!doc_width)+(!doc_height) ||
        (!win_width)+(!win_height)) % 4) G_THROW("Invalid dimensions passed to PixmapCache.");
   
   if (doc_width!=GMarginCache<TPixmap>::doc_width ||
       doc_height!=GMarginCache<TPixmap>::doc_height ||
       win_width!=GMarginCache<TPixmap>::win_width ||
       win_height!=GMarginCache<TPixmap>::win_height)
   {
      DEBUG_MSG("document or window size changed\n");
      deallocate();
      GMarginCache<TPixmap>::doc_width=doc_width;
      GMarginCache<TPixmap>::doc_height=doc_height;
      GMarginCache<TPixmap>::win_width=win_width;
      GMarginCache<TPixmap>::win_height=win_height;
      if (enabled) allocate();
   } else
   {
      DEBUG_MSG("nothing changed => just return\n");
   }
}

template <class TPixmap> void
GMarginCache<TPixmap>::clear(void)
{
   DEBUG_MSG("GMarginCache::clear(): forgetting all the cache contents\n");
   outer_rect=inner_rect;
}

template <class TPixmap> bool
GMarginCache<TPixmap>::isAllocated(void) const
{
   return left_pix!=0 || right_pix!=0 || top_pix!=0 || bottom_pix!=0;
}

template <class TPixmap> void
GMarginCache<TPixmap>::deallocate(void)
{
   DEBUG_MSG("GMarginCache::deallocate(): destroying all the data\n");
   left_pix=right_pix=top_pix=bottom_pix=0;
   left_rect.clear();
   right_rect.clear();
   top_rect.clear();
   bottom_rect.clear();
}

template <class TPixmap> bool
GMarginCache<TPixmap>::nextRectToCache(int maxpixels, GRect & grect)
{
   DEBUG_MSG("GMarginCache::nextRectToCache(): Checking if anything else is to be cached\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!enabled || (!left_pix && !right_pix && !top_pix && !bottom_pix))
   {
      DEBUG_MSG("caching is disabled => returning 0\n");
      return false;
   }
   
   if (outer_rect.width()==doc_width &&
       outer_rect.height()==doc_height) return false;
   
   if (outer_rect.isempty())
   {
      int width=(int) sqrt((double)(maxpixels*doc_width/doc_height));
      if (!width) width++;
      int height=maxpixels/width;
      if (!height) height++;
      grect.xmin=(doc_width-width)/2; if (grect.xmin<0) grect.xmin=0;
      grect.xmax=grect.xmin+width;
      if (grect.xmax>doc_width) grect.xmax=doc_width;
      grect.ymin=(doc_height-height)/2; if (grect.ymin<0) grect.ymin=0;
      grect.ymax=grect.ymin+height;
      if (grect.ymax>doc_height) grect.ymax=doc_height;
      return true;
   }
   
   int left=outer_rect.xmin;
   int right=doc_width-outer_rect.xmax;
   int bottom=outer_rect.ymin;
   int top=doc_height-outer_rect.ymax;
   
   if (left>right && left>bottom && left>top)
   {
      // Caching left margin
      grect.ymin=outer_rect.ymin;
      grect.ymax=outer_rect.ymax;
      grect.xmax=outer_rect.xmin;
      grect.xmin=grect.xmax-maxpixels/grect.height();
      if (grect.xmin<0) grect.xmin=0;
      if (grect.xmin==grect.xmax) grect.xmin--;
      DEBUG_MSG("returning grect=(" << grect.xmin <<
		", " << grect.ymin << ", " << grect.width() << ", " <<
		grect.height() << ")\n");
      return true;
   }
   if (right>bottom && right>top)
   {
      // Caching right margin
      grect.ymin=outer_rect.ymin;
      grect.ymax=outer_rect.ymax;
      grect.xmin=outer_rect.xmax;
      grect.xmax=grect.xmin+maxpixels/grect.height();
      if (grect.xmax>doc_width) grect.xmax=doc_width;
      if (grect.xmax==grect.xmin) grect.xmax++;
      DEBUG_MSG("returning grect=(" << grect.xmin <<
		", " << grect.ymin << ", " << grect.width() << ", " <<
		grect.height() << ")\n");
      return true;
   }
   if (top>bottom)
   {
      // Caching top margin
      grect.xmin=outer_rect.xmin;
      grect.xmax=outer_rect.xmax;
      grect.ymin=outer_rect.ymax;
      grect.ymax=grect.ymin+maxpixels/grect.width();
      if (grect.ymax>doc_height) grect.ymax=doc_height;
      if (grect.ymax==grect.ymin) grect.ymax++;
      DEBUG_MSG("returning grect=(" << grect.xmin <<
		", " << grect.ymin << ", " << grect.width() << ", " <<
		grect.height() << ")\n");
      return true;
   }
   // Caching bottom margin
   grect.xmin=outer_rect.xmin;
   grect.xmax=outer_rect.xmax;
   grect.ymax=outer_rect.ymin;
   grect.ymin=grect.ymax-maxpixels/grect.width();
   if (grect.ymin<0) grect.ymin=0;
   if (grect.ymin==grect.ymax) grect.ymin--;
   DEBUG_MSG("returning grect=(" << grect.xmin <<
	     ", " << grect.ymin << ", " << grect.width() << ", " <<
	     grect.height() << ")\n");
   return true;
}

template <class TPixmap> void
GMarginCache<TPixmap>::addRect(const GRect & grect, const GP<TPixmap> & pix)
{
   DEBUG_MSG("GMarginCache::addRect(): caching rect=(" << grect.xmin <<
	     ", " << grect.ymin << ", " << grect.width() << ", " <<
	     grect.height() << ")\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!enabled || (!left_pix && !right_pix && !top_pix && !bottom_pix))
   {
      DEBUG_MSG("caching is disabled => returning\n");
      return;
   }

   // rotation may cause some round-off errors - exact fit may not be possible
   // BALLOONY. +/- 90 degree rotations should not produce round-off errors.
   if ((int) pix->columns() != grect.width() ||
       (int) pix->rows() != grect.height())
   {
      G_THROW("Pixmap being cached doesn't fit into the rectangle.");
   }
   
   if (outer_rect.isempty())
   {
      outer_rect=grect;
   } else
      if (grect.xmax==outer_rect.xmin)
      {
	 // Left margin
	 outer_rect.xmin=grect.xmin;
      } else
	 if (grect.xmin==outer_rect.xmax)
         {
	    // Right margin
	    outer_rect.xmax=grect.xmax;
	 } else
	    if (grect.ymax==outer_rect.ymin)
	    {
	       // Bottom margin
	       outer_rect.ymin=grect.ymin;
	    } else
	       if (grect.ymin==outer_rect.ymax)
	       {
		  // Top margin
		  outer_rect.ymax=grect.ymax;
	       } else
		  G_THROW("Request to cache invalid rectangle.");

   // Copying the pixmap into the proper margins
   GRect irect;
   if (irect.intersect(left_rect, grect))
   {
      if (left_pix->get_grays()!=pix->get_grays())
	 left_pix->set_grays(pix->get_grays());
      for(int i=irect.ymin-grect.ymin, j=irect.ymin;j<irect.ymax;i++, j++)
	 memcpy((*left_pix)[j]+irect.xmin,
		(*pix)[i]+irect.xmin-grect.xmin, sizeof(*(*pix)[0])*irect.width());
   }
   if (irect.intersect(right_rect, grect))
   {
      if (right_pix->get_grays()!=pix->get_grays())
	 right_pix->set_grays(pix->get_grays());
      for(int i=irect.ymin-grect.ymin, j=irect.ymin;j<irect.ymax;i++, j++)
	 memcpy((*right_pix)[j]+irect.xmin-right_rect.xmin,
		(*pix)[i]+irect.xmin-grect.xmin, sizeof(*(*pix)[0])*irect.width());
   }
   if (irect.intersect(bottom_rect, grect))
   {
      if (bottom_pix->get_grays()!=pix->get_grays())
	 bottom_pix->set_grays(pix->get_grays());
      for(int i=irect.ymin-grect.ymin, j=irect.ymin;j<irect.ymax;i++, j++)
	 memcpy((*bottom_pix)[j]+irect.xmin-bottom_rect.xmin,
		(*pix)[i]+irect.xmin-grect.xmin, sizeof(*(*pix)[0])*irect.width());
   }
   if (irect.intersect(top_rect, grect))
   {
      if (top_pix->get_grays()!=pix->get_grays())
	 top_pix->set_grays(pix->get_grays());
      for(int i=irect.ymin-grect.ymin, j=irect.ymin-top_rect.ymin;
	  j<irect.ymax-top_rect.ymin;i++, j++)
	 memcpy((*top_pix)[j]+irect.xmin-top_rect.xmin,
		(*pix)[i]+irect.xmin-grect.xmin, sizeof(*(*pix)[0])*irect.width());
   }
}

template <class TPixmap> GP<TPixmap>
GMarginCache<TPixmap>::getPixmap(GRect & grect, GRect & grect_cached)
{
   DEBUG_MSG("GMarginCache::getPixmap(): checking rect=(" << grect.xmin <<
	     ", " << grect.ymin << ", " << grect.width() << ", " <<
	     grect.height() << ")\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!enabled || (!left_pix && !right_pix && !top_pix && !bottom_pix))
   {
      DEBUG_MSG("caching is disabled => returning 0\n");
      return 0;
   }
   
   if (grect.xmin<outer_rect.xmin ||
       grect.ymin<outer_rect.ymin ||
       grect.xmax>outer_rect.xmax ||
       grect.ymax>outer_rect.ymax)
   {
      DEBUG_MSG("goes outside outer_grect=(" << outer_rect.xmin <<
	     ", " << outer_rect.ymin << ", " << outer_rect.width() << ", " <<
	     outer_rect.height() << ")\n");
      return 0;
   }
   
   if (grect.xmin<inner_rect.xmin)
   {
      // Using left margin
      grect_cached=grect;
      if (grect_cached.xmax>inner_rect.xmin)
	 grect.xmin=grect_cached.xmax=inner_rect.xmin;
      else grect.xmin=grect.xmax;
   } else
      if (grect.xmax>inner_rect.xmax)
      {
	 // Using right margin
	 grect_cached=grect;
	 if (grect_cached.xmin<inner_rect.xmax)
	    grect.xmax=grect_cached.xmin=inner_rect.xmax;
	 else grect.xmin=grect.xmax;
      } else
	 if (grect.ymin<inner_rect.ymin)
	 {
	    // Using bottom margin
	    grect_cached=grect;
	    if (grect_cached.ymax>inner_rect.ymin)
	       grect.ymin=grect_cached.ymax=inner_rect.ymin;
	    else grect.ymin=grect.ymax;
	 } else
	    if (grect.ymax>inner_rect.ymax)
	    {
	       // Using top margin
	       grect_cached=grect;
	       if (grect_cached.ymin<inner_rect.ymax)
		  grect.ymax=grect_cached.ymin=inner_rect.ymax;
	       else grect.ymin=grect.ymax;
	    } else
	    {
	       DEBUG_MSG("nothing useful has been cached\n");
	       return 0;
	    }
      
   DEBUG_MSG("returning grect=(" << grect_cached.xmin <<
	     ", " << grect_cached.ymin << ", " << grect_cached.width() <<
	     ", " << grect_cached.height() << ")\n");
   
   GP<TPixmap> pix=TPixmap::create(grect_cached.height(), grect_cached.width());
   
   // Copying the pixmap from the proper margins
   GRect irect;
   if (irect.intersect(left_rect, grect_cached))
   {
      if (pix->get_grays()!=left_pix->get_grays())
	 pix->set_grays(left_pix->get_grays());
      for(int i=irect.ymin-grect_cached.ymin, j=irect.ymin;j<irect.ymax;i++, j++)
	 memcpy((*pix)[i]+irect.xmin-grect_cached.xmin, (*left_pix)[j]+irect.xmin,
		sizeof(*(*pix)[0])*irect.width());
   }
   if (irect.intersect(right_rect, grect_cached))
   {
      if (pix->get_grays()!=right_pix->get_grays())
	 pix->set_grays(right_pix->get_grays());
      for(int i=irect.ymin-grect_cached.ymin, j=irect.ymin;j<irect.ymax;i++, j++)
	 memcpy((*pix)[i]+irect.xmin-grect_cached.xmin,
		(*right_pix)[j]+irect.xmin-right_rect.xmin,
		sizeof(*(*pix)[0])*irect.width());
   }
   if (irect.intersect(bottom_rect, grect_cached))
   {
      if (pix->get_grays()!=bottom_pix->get_grays())
	 pix->set_grays(bottom_pix->get_grays());
      for(int i=irect.ymin-grect_cached.ymin, j=irect.ymin;j<irect.ymax;i++, j++)
	 memcpy((*pix)[i]+irect.xmin-grect_cached.xmin,
		(*bottom_pix)[j]+irect.xmin-bottom_rect.xmin,
		sizeof(*(*pix)[0])*irect.width());
   }
   if (irect.intersect(top_rect, grect_cached))
   {
      if (pix->get_grays()!=top_pix->get_grays())
	 pix->set_grays(top_pix->get_grays());
      for(int i=irect.ymin-grect_cached.ymin, j=irect.ymin-top_rect.ymin;
	  j<irect.ymax-top_rect.ymin;i++, j++)
	 memcpy((*pix)[i]+irect.xmin-grect_cached.xmin,
		(*top_pix)[j]+irect.xmin-top_rect.xmin,
		sizeof(*(*pix)[0])*irect.width());
   }
   return pix;
}


#endif
