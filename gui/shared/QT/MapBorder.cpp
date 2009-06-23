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
// $Id: MapBorder.cpp,v 1.9 2008/08/05 20:52:26 bpearlmutter Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "GContainer.h"
#include "Arrays.h"
#include "GException.h"
#include "MapBorder.h"
#include "debug.h"
#include <math.h>

#define CELL_SIZE		20
#define MAX_RECTANGLE_AREA	(128*128)

inline static int sign(int x) { return x<0 ? -1 : x>0 ? 1 : 0; }

#define GET_SIGN(x, y) ::sign(((x)-start_x)*(end_y-start_y)-((y)-start_y)*(end_x-start_x))

static GList<GRect>
genRect(const int * xx, const int * yy, int points, GRectMapper * mapper)
{
   DEBUG_MSG("genRect(): Generating rectangles covering given polygon...\n");
   DEBUG_MAKE_INDENT(3);
   
   int xmin, ymin, xmax, ymax;
   xmin=xmax=xx[0]; ymin=ymax=yy[0];
   
   for(int i=0;i<points;i++)
   {
      if (xmin>xx[i]) xmin=xx[i];
      if (xmax<xx[i]) xmax=xx[i];
      if (ymin>yy[i]) ymin=yy[i];
      if (ymax<yy[i]) ymax=yy[i];
   }
   
   GRect brect(xmin, ymin, xmax-xmin, ymax-ymin);
   mapper->map(brect);
   brect.inflate(3, 3);
   
   int numx=brect.width()/CELL_SIZE+1;
      //int numy=brect.height()/CELL_SIZE+1;
   
   DArray<GList<int> > passed_cells(numx-1);
   GList<GRect> found_rect;
   int start_point=0, end_point=1;
   int start_x=xx[0], start_y=yy[0];
   mapper->map(start_x, start_y);
   int end_x=xx[1], end_y=yy[1];
   mapper->map(end_x, end_y);
   int cell_i=(start_x-brect.xmin)/CELL_SIZE;
   int cell_j=(start_y-brect.ymin)/CELL_SIZE;
   int cell_xmin=brect.xmin+cell_i*CELL_SIZE;
   int cell_ymin=brect.ymin+cell_j*CELL_SIZE;
   int cell_cnt=0;
   int last_di=0, last_dj=0;
   while(1)
   {
      int cur_cell_reentered=0;
      // Add current cell to the list of passed ones
      GPosition pos;
      if (!passed_cells[cell_i].search(cell_j, pos))
	 passed_cells[cell_i].append(cell_j);
      else cur_cell_reentered=1;

      // As long as the edge remains within the current cell - loop
      while(end_x<=cell_xmin+CELL_SIZE && end_x>=cell_xmin &&
	    end_y<=cell_ymin+CELL_SIZE && end_y>=cell_ymin && end_point)
      {
	    /*
	 DEBUG_MSG("********" << (cur_cell_reentered ? 'R' : '*') <<
		   "*cell=(" << cell_xmin << ", " << cell_ymin <<
		   ", " << (cell_xmin+CELL_SIZE) <<
		   ", " << (cell_ymin+CELL_SIZE) << "): (" << end_x << ", " << end_y << ") skipped\n");*/
	 start_x=end_x; start_y=end_y; start_point=end_point;
	 end_point=(end_point+1) % points;
	 end_x=xx[end_point], end_y=yy[end_point];
	 mapper->map(end_x, end_y);
      }
      if (end_x<=cell_xmin+CELL_SIZE && end_x>=cell_xmin &&
	  end_y<=cell_ymin+CELL_SIZE && end_y>=cell_ymin && !end_point)
      {
	 if (!cur_cell_reentered)
	 {
	    // Here we can be only if there is one cell covering the whole poly
	    int xmin=last_di>0 ? cell_xmin-last_di*cell_cnt*CELL_SIZE : cell_xmin;
	    int ymin=last_dj>0 ? cell_ymin-last_dj*cell_cnt*CELL_SIZE : cell_ymin;
	    GRect grect(xmin, ymin,
			CELL_SIZE*(1+abs(last_di)*cell_cnt),
			CELL_SIZE*(1+abs(last_dj)*cell_cnt));
	    found_rect.append(grect);
	 }
	 break;
      }
      
      int di=0, dj=0;
      // Find the side through which the edge is leaving the cell
      if (end_x>start_x)
      {
	 if (end_y>start_y)
	 {
	    int s=GET_SIGN(cell_xmin+CELL_SIZE, cell_ymin+CELL_SIZE);
	    if (s>0) dj=1;
	    else if (s<0) di=1;
	    else di=dj=1;
	 } else
	 if (end_y<start_y)
	 {
	    int s=GET_SIGN(cell_xmin+CELL_SIZE, cell_ymin);
	    if (s>0) di=1;
	    else if (s<0) dj=-1;
	    else { di=1; dj=-1; }
	 } else di=1;
      } else
      if (end_x<start_x)
      {
	 if (end_y>start_y)
	 {
	    int s=GET_SIGN(cell_xmin, cell_ymin+CELL_SIZE);
	    if (s>0) di=-1;
	    else if (s<0) dj=1;
	    else { di=-1; dj=1; }
	 } else
	 if (end_y<start_y)
	 {
	    int s=GET_SIGN(cell_xmin, cell_ymin);
	    if (s>0) dj=-1;
	    else if (s<0) di=-1;
	    else di=dj=-1;
	 } else di=-1;
      } else dj=::sign(end_y-start_y);

	 /*
      DEBUG_MSG("********" << (cur_cell_reentered ? 'R' : '*') <<
		"*cell=(" << cell_xmin << ", " << cell_ymin <<
		", " << (cell_xmin+CELL_SIZE) << ", " <<
		(cell_ymin+CELL_SIZE) << "): doing (" << start_x << ", " << start_y <<
		")-(" << end_x << ", " << end_y << "), di=" << di << ", dj=" << dj << "\n");*/
      
      GPosition pos1;
      if ((di && dj) || (cell_cnt && (di!=last_di || dj!=last_dj)) ||
	  passed_cells[cell_i+di].search(cell_j+dj, pos1))
      {
	 // We're either leaving the current cell through a corner or a way,
	 // that prevents the current rectangle from growing.
	 // We can also be here because the next cell has already been considered
	 if (!cur_cell_reentered)
	 {
	    int xmin=last_di>0 ? cell_xmin-last_di*cell_cnt*CELL_SIZE : cell_xmin;
	    int ymin=last_dj>0 ? cell_ymin-last_dj*cell_cnt*CELL_SIZE : cell_ymin;
	    GRect grect(xmin, ymin,
			CELL_SIZE*(1+abs(last_di)*cell_cnt),
			CELL_SIZE*(1+abs(last_dj)*cell_cnt));
	    found_rect.append(grect);
	 }
	 cell_cnt=-1;
      }
	 /*
      if (cell_i+di<0 || cell_i+di>=numx || cell_j+dj<0 || cell_j+dj>=numy)
      {
	 DEBUG_MSG("************************************************\n");
	 DEBUG_MSG("cell_i=" << cell_i << ", cell_j=" << cell_j <<
		   ", numx=" << numx << ", numy=" << numy << "\n");
	 DEBUG_MSG("di=" << di << ", dj=" << dj << "\n");
	 DEBUG_MSG("start_point=" << start_point << ", end_point=" << end_point << "\n");
	 DEBUG_MSG("start=(" << start_x << ", " << start_y << "), end=(" <<
		   end_x << ", " << end_y << ")\n");
	 DEBUG_MSG("brect=(" << brect.xmin << ", " << brect.ymin <<
		   ", " << brect.xmax << ", " << brect.ymax << ")\n");
	 DEBUG_MSG("cell_xmin=" << cell_xmin << ", cell_ymin=" << cell_ymin << "\n");
	 
	 if (cell_cnt>=0)
	 {
	    int xmin=last_di>0 ? cell_xmin-last_di*cell_cnt*CELL_SIZE : cell_xmin;
	    int ymin=last_dj>0 ? cell_ymin-last_dj*cell_cnt*CELL_SIZE : cell_ymin;
	    GRect grect(xmin, ymin,
			CELL_SIZE*(1+abs(last_di)*cell_cnt),
			CELL_SIZE*(1+abs(last_dj)*cell_cnt));
	    found_rect.append(grect);
	 }
	 break;
      }*/
      last_di=di; last_dj=dj;
      cell_i+=di;
      cell_j+=dj;
      cell_xmin+=di*CELL_SIZE;
      cell_ymin+=dj*CELL_SIZE;
      if (!cur_cell_reentered) cell_cnt++; else cell_cnt=0;
   }
   
   // Now look if you can substitute all the found rectangles by only one
   // increasing the covered area by no more than 1.5 times:
   int area=0;
   for(GPosition pos=found_rect;pos;++pos)
      area+=found_rect[pos].width()*found_rect[pos].height();
   if (3*area>2*brect.width()*brect.height())
   {
      found_rect.empty();
      found_rect.append(brect);
   } else
   {
	 // Inflate them by one pixel in the display coordinate system.
	 // This is to avoid discrepancies between the point and pixel
	 // coordinate systems
      for(GPosition pos=found_rect;pos;++pos)
      {
	 GRect & rect=found_rect[pos];
	 mapper->map(rect);
	 rect.inflate(1, 1);
	 mapper->unmap(rect);
      }
   }
   
   DEBUG_MSG("genRect(): DONE\n");
   return found_rect;
}

void
MapPoly::ma_generatePieces(void)
{
   DEBUG_MSG("MapPoly::ma_generatePieces(): Generating MapPiece's...\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!mapper) return;
   
   int points=gmap_poly->get_points_num();
   TArray<int> xx(points-1), yy(points-1);
   for(int i=0;i<points;i++)
   {
      xx[i]=gmap_poly->get_x(i); yy[i]=gmap_poly->get_y(i);
   }
   GList<GRect> rect=genRect(xx, yy, points, mapper);
   
   // Converting found rectangles to pieces
   pieces.empty();
   
   for(GPosition pos=rect;pos;++pos)
   {
      GRect grect=rect[pos];
      grect.inflate(1, 1);
      mapper->unmap(grect);
      pieces.append(new MapPiece(grect, mapper));
   }
}

#ifndef Pi
#define Pi 3.14159265359
#endif

void
MapOval::ma_generatePieces(void)
{
   DEBUG_MSG("MapOval::ma_generatePieces(): Generating MapPiece's...\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!mapper) return;
   
   // First - try to generate a polygon with sufficient accurary.
   // Accuracy is "sufficient", if the polygon stays no more than 1 pixel
   // away from the ellipse.
   
   // Create a new ellipse (in screen coords)
   GRect brect(get_xmin(), get_ymin(), get_xmax()-get_xmin(), get_ymax()-get_ymin());
   mapper->map(brect);
   if (!brect.width() || !brect.height()) return;
   GP<GMapOval> oval=GMapOval::create(brect);
   
   int a=oval->get_a(), b=oval->get_b();
   int xc=(oval->get_xmax()+oval->get_xmin())/2;
   int yc=(oval->get_ymax()+oval->get_ymin())/2;
   if (!oval->get_rmin()) return;
   
   int last_x=xc, last_y=yc;
   int points=(int) (2*Pi*sqrt((double)(oval->get_rmax())))+1;
   TArray<int> xx(points-1), yy(points-1);
   int i, cnt=0;
   for(i=0;i<points;i++)
   {
      float angle=2*Pi*i/points;
      int x=(int) (xc+a*cos(angle));
      int y=(int) (yc+b*sin(angle));
      if (x!=last_x || y!=last_y)
      {
	 xx[cnt]=x; yy[cnt++]=y;
	 last_x=x; last_y=y;
      }
   }
   points=cnt;
   if (xx[points-1]==xx[0] && yy[points-1]==yy[0]) points--;
   
   for(i=0;i<points;i++) mapper->unmap(xx[i], yy[i]);
   GList<GRect> rect=genRect(xx, yy, points, mapper);
   
   // Converting found rectangles to pieces
   pieces.empty();
   
   for(GPosition pos=rect;pos;++pos)
   {
      GRect grect=rect[pos];
      grect.inflate(1, 1);
      mapper->unmap(grect);
      pieces.append(new MapPiece(grect, mapper));
   }
}

void
MapRect::ma_generatePieces(void)
{
   DEBUG_MSG("MapRect::ma_generatePieces(): regenerating pieces\n");
   DEBUG_MAKE_INDENT(3);

      // +/-3 in this code is used to include edit controls into pieces
   
   if (!mapper)
   {
      DEBUG_MSG("there is no mapper here => return...\n");
      return;
   }

   GRect brect=gmap_area->get_bound_rect();
   mapper->map(brect);
   
   pieces.empty();
   if (brect.width()*brect.height()<=MAX_RECTANGLE_AREA)
   {
      brect.inflate(3, 3);
      mapper->unmap(brect);
      pieces.append(new MapPiece(brect, mapper));
   } else
   {
      GRect prect;

	 // Top side
      prect=GRect(brect.xmin, brect.ymin-3, brect.width(), gmap_area->border_width+3);
      mapper->unmap(prect);
      pieces.append(new MapPiece(prect, mapper));

	 // Right side
      prect=GRect(brect.xmax-gmap_area->border_width, brect.ymin+gmap_area->border_width,
		  gmap_area->border_width+3, brect.ymax-brect.ymin-2*gmap_area->border_width);
      mapper->unmap(prect);
      pieces.append(new MapPiece(prect, mapper));

	 // Bottom side
      prect=GRect(brect.xmin, brect.ymax-gmap_area->border_width,
		  brect.width(), gmap_area->border_width+3);
      mapper->unmap(prect);
      pieces.append(new MapPiece(prect, mapper));

	 // Left side
      prect=GRect(brect.xmin-3, brect.ymin+gmap_area->border_width,
		  gmap_area->border_width+3, brect.ymax-brect.ymin-2*gmap_area->border_width);
      mapper->unmap(prect);
      pieces.append(new MapPiece(prect, mapper));
   }
}
