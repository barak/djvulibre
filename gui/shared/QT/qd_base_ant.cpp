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

#include "ByteStream.h"
#include "qd_base_ant.h"
#include "djvu_base_res.h"
#include "debug.h"
#include "qlib.h"

#include <qapplication.h>



#include <math.h>
#define Pi 3.14159265359
#define Pi2 (3.14159265359*2)

void
QDBase::eraseSearchResults(void)
{
   for(GPosition pos=map_areas;pos;)
   {
      GP<MapArea> ma=map_areas[pos];
      if (ma->getComment()==search_results_name)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 map_areas.del(this_pos);
	 ma->repaint();
      } else ++pos;
   }
}

void
QDBase::displaySearchResults(const GList<DjVuTXT::Zone *> & zones_list)
{
   eraseSearchResults();

   search_results_name="Search results";
   for(GPosition pos=zones_list;pos;++pos)
   {
      GRect irect=zones_list[pos]->rect;
      dimg->unmap(irect);
      GP<GMapRect> gma=GMapRect::create(irect);
      gma->comment=search_results_name;
      gma->hilite_color=0xff000000;
      gma->border_type=GMapArea::NO_BORDER;
      gma->url="";
      GP<MapArea> ma=new MapRect(gma);
      map_areas.append(ma);
      ma->attachWindow(pane, &ma_mapper);
      ma->layout(GRect(0, 0, dimg->get_width(), dimg->get_height()));
      ma->repaint();
   }

      // Position the page to see the first map area
   DjVuTXT::Zone * first_zone=zones_list[zones_list];
   int xc=(first_zone->rect.xmin+first_zone->rect.xmax)/2;
   int yc=(first_zone->rect.ymin+first_zone->rect.ymax)/2;
   ma_mapper.map(xc, yc);

   scroll(xc-pane->width()/2, yc-pane->height()/2, true);
}

void
QDBase::eraseMapAreas(bool search_results_too, bool allow_draw)
{
   DEBUG_MSG("QDBase::eraseMapAreas(): Erasing the map areas\n");
   DEBUG_MAKE_INDENT(3);

   GRect grect;
   grect.intersect(rectDocument, rectVisible);
   
   for(GPosition pos=map_areas;pos;)
   {
      GP<MapArea> ma=map_areas[pos];
      if (search_results_too || ma->getComment()!=search_results_name)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 map_areas.del(this_pos);
	 
	 if (!grect.isempty() && allow_draw) ma->repaint();
      } else ++pos;
   }
   cur_map_area=0;
   delete map_area_tip; map_area_tip=0;
}

void
QDBase::createMapAreas(bool allow_draw)
{
   DEBUG_MSG("QDBase::createMapAreas(): Creating the map areas\n");
   DEBUG_MAKE_INDENT(3);

   eraseMapAreas(false, allow_draw);

   if (!dimg) return;

   if (override_flags.hilite_rects.size())
   {
      GP<DjVuInfo> info;
      if (dimg) info=dimg->get_info();
      if (info)
	 for(GPosition pos=override_flags.hilite_rects;pos;++pos)
	 {
	    GP<GMapRect> gmap_rect=override_flags.hilite_rects[pos];
	    GRect grect=*gmap_rect;
	    if (grect.xmin<0) grect.xmin=0;
	    if (grect.ymin<0) grect.ymin=0;
	    if (grect.xmax>info->width) grect.xmax=info->width;
	    if (grect.ymax>info->height) grect.ymax=info->height;
	    if (grect.width()>0 && grect.height()>0)
	    {
	       *gmap_rect=grect;
	       map_areas.append(new MapRect(gmap_rect));
	    }
	 }
   }
   
   if (!override_flags.url.is_empty())
   {
      GRect rect(0, 0, dimg->get_width(), dimg->get_height());
      if (!rect.isempty())
      {
	 GP<GMapRect> gma=GMapRect::create(rect);
	 gma->url=override_flags.url.get_string();
	 gma->border_type=GMapArea::SHADOW_OUT_BORDER;
	 gma->border_always_visible=false;
	 gma->border_width=4;
	 GP<MapArea> ma=new MapRect(gma);
	 map_areas.append(ma);
      }
   } else if (anno && anno->ant)
   {
      DjVuANT * ant=anno->ant;
   
      GPosition pos;
      for(pos=ant->map_areas;pos;++pos)
      {
	 GP<MapArea> ma;
	 GMapArea * gmap_area=ant->map_areas[pos];
	 GUTF8String shape_name = gmap_area->get_shape_name();
	 if (shape_name == GMapArea::RECT_TAG)
	    ma=new MapRect((GMapRect *) gmap_area);
	 else if (shape_name == GMapArea::OVAL_TAG)
	    ma=new MapOval((GMapOval *) gmap_area);
	 else if (shape_name == GMapArea::POLY_TAG)
	    ma=new MapPoly((GMapPoly *) gmap_area);
	 if ( ma )
	 {
	    ma->setActiveOutlineMode(!ma->isAlwaysActive() && prefs.hlinksBorder, true);
	    map_areas.append(ma);
	 }
      }
   }
   for(GPosition pos=map_areas;pos;++pos)
   {
       map_areas[pos]->attachWindow(pane, &ma_mapper);
       map_areas[pos]->layout(GRect(0, 0, dimg->get_width(), dimg->get_height()));
   }
}

void
QDBase::decodeAnno(bool allow_redraw)
      // Will create DjVuAnno and call processAnno()
{
   DEBUG_MSG("QDBase::decodeAnno(): decoding...\n");
   DEBUG_MAKE_INDENT(3);
   
   if ( !dimg ) return;
   G_TRY {
      bool anno_processed=false;
      anno=dimg->get_decoded_anno();
      
      if (anno && anno->ant)
      {
	 processAnno(allow_redraw);
	 anno_processed=true;
      }
      if (!anno_processed)
      {
	 // If we didn't find anything to process, clean annotations
	 // from the previous (if any) page.
	 cleanAnno(allow_redraw);
      }
   } G_CATCH(exc) {
      anno=0;
      showError(this, exc);
   } G_ENDCATCH;
}

void
QDBase::processAnno(bool allow_redraw)
      // Will process DjVuAnno structure, which should have already
      // been created by decodeAnno() or anybody else.
{
   DEBUG_MSG("QDBase::processAnno(): processing...\n");
   DEBUG_MAKE_INDENT(3);

   G_TRY {
      eraseMapAreas(false, allow_redraw);
   
      if (!dimg || !anno || !anno->ant) return;

      DjVuANT * ant=anno->ant;
      
      int do_redraw=0, do_layout=0;

      if (!ignore_ant_mode_zoom)
      {
	 switch(ant->mode)
	 {
	    case DjVuANT::MODE_COLOR:
	       setMode(IDC_DISPLAY_COLOR, 0, SRC_ANT); do_redraw=1; break;
	    case DjVuANT::MODE_FORE:
	       setMode(IDC_DISPLAY_FOREGROUND, 0, SRC_ANT); do_redraw=1; break;
	    case DjVuANT::MODE_BACK:
	       setMode(IDC_DISPLAY_BACKGROUND, 0, SRC_ANT); do_redraw=1; break;
	    case DjVuANT::MODE_BW:
	       setMode(IDC_DISPLAY_BLACKWHITE, 0, SRC_ANT); do_redraw=1; break;
	    default:
	       break;
	 }
	 switch(ant->zoom)
	 {
	    case DjVuANT::ZOOM_STRETCH:
	       setZoom(IDC_ZOOM_STRETCH, 0, SRC_ANT); do_layout=1; break;
	    case DjVuANT::ZOOM_ONE2ONE:
	       setZoom(IDC_ZOOM_ONE2ONE, 0, SRC_ANT); do_layout=1; break;
	    case DjVuANT::ZOOM_WIDTH:
	       setZoom(IDC_ZOOM_WIDTH, 0, SRC_ANT); do_layout=1; break;
	    case DjVuANT::ZOOM_PAGE:
	       setZoom(IDC_ZOOM_PAGE, 0, SRC_ANT); do_layout=1; break;
	    default:
	       if (ant->zoom!=DjVuANT::ZOOM_UNSPEC)
	       {
		  setZoom(IDC_ZOOM_MIN+ant->zoom, 0, SRC_ANT); do_layout=1;
	       }
	 }
         static int orient2cmd[] = { IDC_ROTATE_0, IDC_ROTATE_90,
                                     IDC_ROTATE_180, IDC_ROTATE_270 };
         int rotation = orient2cmd[dimg->get_rotate() & 3];
         setRotate(rotation,0,SRC_ANT);
      }

      u_int32 bg_color=ant->bg_color;
      if (bg_color==0xffffffff) bg_color=0xffffff;
      if (bg_color!=back_color)
      {
	 do_redraw=1;
	 setBackgroundColor(bg_color, false);	// Don't redraw
      }

      if ((ant->hor_align!=DjVuANT::ALIGN_CENTER &&
	   ant->hor_align!=DjVuANT::ALIGN_UNSPEC) ||
	  (ant->ver_align!=DjVuANT::ALIGN_CENTER &&
	   ant->ver_align!=DjVuANT::ALIGN_UNSPEC))
	do_layout=1;
   
      createMapAreas(false); do_redraw=1;
   
      if (do_layout) layout(0);	// Disallow redraw
      if (allow_redraw && (do_layout || do_redraw)) redraw();
   } G_CATCH(exc) {
      showError(this, exc);
   } G_ENDCATCH;
}

void
QDBase::cleanAnno(bool allow_redraw)
{
   anno=0;

      // Is required to get rid of highlighted areas from annotations
      // and keep areas from URL or EMBED flags
   createMapAreas(false);

      // Restore background color
   if (back_color!=0xffffff)
      setBackgroundColor(0xffffff, false);

      // Switch zoom and mode back, if necessary
   if (zoom_src==SRC_ANT) 
     setZoom(prefs.nDefaultZoom, 0, SRC_ANT);
   if (mode_src==SRC_ANT) 
     setMode(IDC_DISPLAY_COLOR, 0, SRC_ANT);

      // Alignment will be picked up by layout() automatically.
   
   if (allow_redraw) redraw();
}
