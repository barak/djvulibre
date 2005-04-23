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

#define BUILD_LIBDJVU 1
#include "GMapAreas.h"
#include "GException.h"
#include "debug.h"

#include <math.h>
#include <stdio.h>

#define HILITE_DEFUALT_OPACITY  50
#define HILITE_MIN_OPACITY       0
#define HILITE_MAX_OPACITY     100

static void RotatePoint(int rot, int dx, int dy, int& x, int& y)
{
   int temp=0;
   switch ( rot )
   {
   case 1:
      temp = x;
      x = -y;
      y = temp;
      x += dy;
      break;
   case 2:
      x = -x;
      y = -y;
      x += dx;
      y += dy;
      break;
   case 3:
      temp = x;
      x = y;
      y = -temp;
      y += dx;
      break;
   }
}


/****************************************************************************
***************************** GMapArea definition ***************************
****************************************************************************/

const char GMapArea::MAPAREA_TAG[] =      "maparea";
const char GMapArea::RECT_TAG[] =         "rect";
const char GMapArea::POLY_TAG[] =         "poly";
const char GMapArea::OVAL_TAG[] = 		   "oval";
const char GMapArea::TEXT_TAG[] =         "text";
const char GMapArea::LINE_TAG[] =         "line";

const char GMapArea::TEXT_PUSHPIN_TAG[] = "pushpin";
const char GMapArea::BACK_CLR_TAG[] =     "backclr";
const char GMapArea::TEXT_CLR_TAG[] =     "textclr";
const char GMapArea::LINE_ARROW_TAG [] =  "arrow";
const char GMapArea::LINE_WIDTH_TAG[] =   "width";
const char GMapArea::LINE_CLR_TAG[] =     "lineclr";
const char GMapArea::OPACITY_TAG[] =      "opacity";
const char GMapArea::NO_BORDER_TAG[] =    "none";
const char GMapArea::XOR_BORDER_TAG[] =   "xor";
const char GMapArea::SOLID_BORDER_TAG[] = "border";
const char GMapArea::SHADOW_IN_BORDER_TAG[] = 	"shadow_in";
const char GMapArea::SHADOW_OUT_BORDER_TAG[] = 	"shadow_out";
const char GMapArea::SHADOW_EIN_BORDER_TAG[] = 	"shadow_ein";
const char GMapArea::SHADOW_EOUT_BORDER_TAG[] = "shadow_eout";
const char GMapArea::BORDER_AVIS_TAG[] = 	      "border_avis";
const char GMapArea::HILITE_TAG[] = 		      "hilite";
const char GMapArea::URL_TAG[] =                "url";
const char GMapArea::TARGET_SELF[] = 		      "_self";

static const char zero_width[] = ERR_MSG("GMapAreas.zero_width");
static const char zero_height[] = ERR_MSG("GMapAreas.zero_height");
static const char width_1[] = ERR_MSG("GMapAreas.width_1");
static const char width_3_32 [] = ERR_MSG("GMapAreas.width_3-32");
static const char error_poly_border [] = ERR_MSG("GMapAreas.poly_border");
static const char error_poly_hilite [] = ERR_MSG("GMapAreas.poly_hilite");
static const char error_oval_border [] = ERR_MSG("GMapAreas.oval_border");
static const char error_oval_hilite [] = ERR_MSG("GMapAreas.oval_hilite");
static const char error_too_few_points [] = ERR_MSG("GMapAreas.too_few_points");
static const char error_intersect [] = ERR_MSG("GMapAreas.intersect");
static const char error_line_width [] = ERR_MSG("GMapAreas.line_width");
static const char error_line_border [] = ERR_MSG("GMapAreas.line_border");

#define MIN_LINE_WIDTH   1
#define MAX_LINE_WIDTH   9
#define DEFAULT_LINE_WIDTH   1

   /// Default creator.
GP<GMapRect> 
GMapRect::create(void)
{
  return new GMapRect();
}

   /// Create with the specified GRect.
GP<GMapRect> 
GMapRect::create(const GRect &rect)
{
  return new GMapRect(rect);
}

GP<GMapPoly> 
GMapPoly::create(void)
{
  return new GMapPoly();
}

   /// Create from specified coordinates.
GP<GMapPoly>
GMapPoly::create(
     const int xx[], const int yy[], const int points, const bool open)
{
  return new GMapPoly(xx,yy,points,open);
}

GP<GMapOval>
GMapOval::create(void)
{
  return new GMapOval();
}

   /// Create from the specified GRect.
GP<GMapOval> 
GMapOval::create(const GRect &rect)
{
  return new GMapOval(rect);
}

GRect
GMapArea::get_bound_rect(void) const
{
   return GRect(get_xmin(), get_ymin(), get_width(),
		get_height());
}

char const * const GMapArea::check_object(void)
{
   char const *retval;
   MapAreaType type = get_shape_type();
   if (type != LINE && get_width()==0)
   {
     retval=zero_width;
   }
   else if (type != LINE && get_height()==0)
   {
     retval=zero_height;
   }
   else if ((border_type==XOR_BORDER || border_type==SOLID_BORDER) && border_width!=1)
   {
     retval=width_1;
   }
   else if ((border_type==SHADOW_IN_BORDER ||
             border_type==SHADOW_OUT_BORDER ||
             border_type==SHADOW_EIN_BORDER ||
             border_type==SHADOW_EOUT_BORDER)&&
            (border_width<3 || border_width>32))
   {
     retval=width_3_32;
   }else
   {
     retval=gma_check_object();
   }
   return retval;
}

GUTF8String GMapArea::print(void)
{
      // Make this hard check to make sure, that *no* illegal GMapArea
      // can be stored into a file.
   const char * const errors=check_object();
   if (errors[0])
   {
     G_THROW(errors);
   }
   
   int i;
   GUTF8String tmp;
   GUTF8String url1, target1, comment1;
   const GUTF8String url_str=url;
   for(i=0;i<(int) url_str.length();i++)
   {
      char ch=url_str[i];
      if (ch=='"')
        url1+='\\';
      url1+=ch;
   }
   for(i=0;i<(int) target.length();i++)
   {
      char ch=target[i];
      if (ch=='"')
        target1+='\\';
      target1+=ch;
   }
   for(i=0;i<(int) comment.length();i++)
   {
      char ch=comment[i];
      if (ch=='"')
        comment1+='\\';
      comment1+=ch;
   }
   
   GUTF8String border_color_str;
   border_color_str.format("#%02X%02X%02X",
	   (border_color & 0xff0000) >> 16,
	   (border_color & 0xff00) >> 8,
	   (border_color & 0xff));

   static const GUTF8String left('(');
   static const GUTF8String right(')');
   static const GUTF8String space(' ');
   static const GUTF8String quote('"');
   GUTF8String border_type_str;
   switch(border_type)
   {
      case NO_BORDER:
        border_type_str=left+NO_BORDER_TAG+right;
        break;
      case XOR_BORDER:
        border_type_str=left+XOR_BORDER_TAG+right;
        break;
      case SOLID_BORDER:
        border_type_str=left+SOLID_BORDER_TAG+space+border_color_str+right;
        break;
      case SHADOW_IN_BORDER:
        border_type_str=left+SHADOW_IN_BORDER_TAG+space+GUTF8String(border_width)+right;
        break;
      case SHADOW_OUT_BORDER:
        border_type_str=left+SHADOW_OUT_BORDER_TAG+space+GUTF8String(border_width)+right;
        break;
      case SHADOW_EIN_BORDER:
        border_type_str=left+SHADOW_EIN_BORDER_TAG+space+GUTF8String(border_width)+right;
        break;
      case SHADOW_EOUT_BORDER:
        border_type_str=left+SHADOW_EOUT_BORDER_TAG+space+GUTF8String(border_width)+right;
        break;
      default:
        border_type_str=left+XOR_BORDER_TAG+right;
        break;
   }

   GUTF8String hilite_str;
   if (hilite_color!=0xffffffff)
   {
      hilite_str.format("(%s #%02X%02X%02X)",
	      HILITE_TAG, (hilite_color & 0xff0000) >> 16,
	      (hilite_color & 0xff00) >> 8,
	      (hilite_color & 0xff));
   }
   
   GUTF8String URL;
   if (target1==TARGET_SELF)
   {
      URL=quote+url1+quote;
   }else
   {
      URL=left+URL_TAG+space+quote+url1+quote+space+quote+target1+quote+right;
   }

   GUTF8String total=left+MAPAREA_TAG+space+URL+space+quote+comment1+quote+space;
   total += gma_print() + border_type_str;
   if (border_always_visible)
     total += space+left+BORDER_AVIS_TAG+right;  // (border_avis )
   if ( hilite_str.length() > 0 )
     total += space+hilite_str;    // (hilite #008080 )
   total+=right;
   return total;
}

/// Virtual function generating a list of defining coordinates
/// (default are the opposite corners of the enclosing rectangle)
void GMapArea::get_coords( GList<int> & CoordList ) const
{
   CoordList.append( get_xmin() );
   CoordList.append( get_ymin() );
   CoordList.append( get_xmax() );
   CoordList.append( get_ymax() );
}


/****************************************************************************
**************************** GMapRect definition ****************************
****************************************************************************/

void GMapRect::resize(int new_width, int new_height)
{
   if (get_width()==new_width && get_height()==new_height)
       return;
   xmax=xmin+new_width;
   ymax=ymin+new_height;
}

void GMapRect::transform(const GRect & grect)
{
   xmin=grect.xmin; ymin=grect.ymin;
   xmax=grect.xmax; ymax=grect.ymax;
}

GUTF8String GMapRect::gma_print(void)
{
   GUTF8String buffer;
   buffer.format("(%s %d %d %d %d) ", RECT_TAG, xmin, ymin, xmax-xmin, ymax-ymin);
   GUTF8String extra = "";
   if ( m_n0pacity != HILITE_DEFUALT_OPACITY )
      buffer += extra.format("(%s %d) ", OPACITY_TAG, m_n0pacity );
   return buffer;

}

void 
GMapRect::map(GRectMapper &mapper)
{
    GRect rect;
    rect.xmin = xmin;
    rect.xmax = xmax;
    rect.ymin = ymin;
    rect.ymax = ymax;
    mapper.map(rect);
    xmin = rect.xmin;
    ymin = rect.ymin;
    xmax = rect.xmax;
    ymax = rect.ymax;
}
void 
GMapRect::unmap(GRectMapper &mapper)
{
    GRect rect;
    rect.xmin = xmin;
    rect.xmax = xmax;
    rect.ymin = ymin;
    rect.ymax = ymax;
    mapper.unmap(rect);
    xmin = rect.xmin;
    ymin = rect.ymin;
    xmax = rect.xmax;
    ymax = rect.ymax;
}

void GMapRect::rotateArea(int rot,int cx,int cy)
{
	rot=rot%4;
	if(rot==0)
		return;

   RotatePoint( rot, cx, cy, xmin, ymin);
   RotatePoint( rot, cx, cy, xmax, ymax);

	int temp=0;
	if(xmin>xmax)
	{
		temp=xmin;
		xmin=xmax;
		xmax=temp;
	}
	if(ymin>ymax)
	{
		temp=ymin;
		ymin=ymax;
		ymax=temp;
	}
}
/****************************************************************************
**************************** GMapPoly definition ****************************
****************************************************************************/

inline int
GMapPoly::sign(int x) { return x<0 ? -1 : x>0 ? 1 : 0; }

bool
GMapPoly::does_side_cross_rect(const GRect & grect, int side)
{
   int x1=xx[side], x2=xx[(side+1)%points];
   int y1=yy[side], y2=yy[(side+1)%points];
   int xmin=x1<x2 ? x1 : x2;
   int ymin=y1<y2 ? y1 : y2;
   int xmax=x1+x2-xmin;
   int ymax=y1+y2-ymin;

   if (xmax<grect.xmin || xmin>grect.xmax ||
       ymax<grect.ymin || ymin>grect.ymax) return false;

   return
      x1>=grect.xmin && x1<=grect.xmax && y1>=grect.ymin && y1<=grect.ymax ||
      x2>=grect.xmin && x2<=grect.xmax && y2>=grect.ymin && y2<=grect.ymax ||
      do_segments_intersect(grect.xmin, grect.ymin, grect.xmax, grect.ymax,
			    x1, y1, x2, y2) ||
      do_segments_intersect(grect.xmax, grect.ymin, grect.xmin, grect.ymax,
			    x1, y1, x2, y2);
}

bool
GMapPoly::is_projection_on_segment(int x, int y, int x1, int y1, int x2, int y2)
{
   int res1=(x-x1)*(x2-x1)+(y-y1)*(y2-y1);
   int res2=(x-x2)*(x2-x1)+(y-y2)*(y2-y1);
   return sign(res1)*sign(res2)<=0;
}

bool
GMapPoly::do_segments_intersect(int x11, int y11, int x12, int y12,
				int x21, int y21, int x22, int y22)
{
   int res11=(x11-x21)*(y22-y21)-(y11-y21)*(x22-x21);
   int res12=(x12-x21)*(y22-y21)-(y12-y21)*(x22-x21);
   int res21=(x21-x11)*(y12-y11)-(y21-y11)*(x12-x11);
   int res22=(x22-x11)*(y12-y11)-(y22-y11)*(x12-x11);
   if (!res11 && !res12)
   {
      // Segments are on the same line
      return
	 is_projection_on_segment(x11, y11, x21, y21, x22, y22) ||
	 is_projection_on_segment(x12, y12, x21, y21, x22, y22) ||
	 is_projection_on_segment(x21, y21, x11, y11, x12, y12) ||
	 is_projection_on_segment(x22, y22, x11, y11, x12, y12);
   }
   int sign1=sign(res11)*sign(res12);
   int sign2=sign(res21)*sign(res22);
   return sign1<=0 && sign2<=0;
}

bool
GMapPoly::are_segments_parallel(int x11, int y11, int x12, int y12,
				int x21, int y21, int x22, int y22)
{
   return (x12-x11)*(y22-y21)-(y12-y11)*(x22-x21)==0;
}

char const * const
GMapPoly::check_data(void)
{
  if (open && points<2 || !open && points<3) 
    return error_too_few_points;
  for(int i=0;i<sides;i++)
  {
    for(int j=i+2;j<sides;j++)
    {
      if (i != (j+1)%points )
      {
        if (do_segments_intersect(xx[i], yy[i], xx[i+1], yy[i+1],
				      xx[j], yy[j], xx[(j+1)%points], yy[(j+1)%points]))
        {
          return error_intersect;
        }
      }
    }
  }
  return "";
}

void GMapPoly::optimize_data(void)
{
   // Removing segments of length zero
   int i;
   for(i=0;i<sides;i++)
   {
      while(xx[i]==xx[(i+1)%points] && yy[i]==yy[(i+1)%points])
      {
	      for(int k=(i+1)%points;k<points-1;k++)
	      {
	         xx[k]=xx[k+1]; 
            yy[k]=yy[k+1];
	      }
	      points--; 
         sides--;
	      if (!points) 
            return;
      }
   }
   // Concatenating consequitive parallel segments
   for(i=0;i<sides;i++)
   {
      while((open && i+1<sides || !open) &&
	         are_segments_parallel(xx[i], yy[i],
				                     xx[(i+1)%points], yy[(i+1)%points],
				                     xx[(i+1)%points], yy[(i+1)%points],
				                     xx[(i+2)%points], yy[(i+2)%points]))
      {
	      for(int k=(i+1)%points;k<points-1;k++)
	      {
	         xx[k]=xx[k+1]; 
            yy[k]=yy[k+1];
	      }
	      points--; 
         sides--;
	      if (!points) 
            return;
      }
   }
}

bool GMapPoly::is_point_inside(const int xin, const int yin) const
{
   if (open)
     return false;
   if ( !(xin>=xmin && xin<xmax && yin>=ymin && yin<ymax) ) 
      return false;
   
   int xfar=get_xmax()+(get_width());
   
   int intersections=0;
   for(int i=0;i<points;i++)
   {
      int res1=yy[i]-yin;
      if (!res1) continue;
      int res2, isaved=i;
      while(!(res2=yy[(i+1)%points]-yin)) i++;
      if (isaved!=i)
      {
	      // Some points fell exactly on the line
	      if ((xx[(isaved+1)%points]-xin)*(xx[i%points]-xin)<=0)
	      {
	         // Test point is exactly on the boundary
	         return true;
	      }
      }
      if (res1<0 && res2>0 || res1>0 && res2<0)
      {
	      int x1=xx[i%points], y1=yy[i%points];
	      int x2=xx[(i+1)%points], y2=yy[(i+1)%points];
	      int _res1=(xin-x1)*(y2-y1)-(yin-y1)*(x2-x1);
	      int _res2=(xfar-x1)*(y2-y1)-(yin-y1)*(x2-x1);
	      if (!_res1 || !_res2)
	      {
	         // The point is on this boundary
	         return true;
	      }
	      if (sign(_res1)*sign(_res2)<0) 
            intersections++;
      }
   }
   return (intersections % 2)!=0;
}

void GMapPoly::move(int dx, int dy)
{
   if (dx || dy)
   {
      xmin+=dx;
      ymin+=dy;
      xmax+=dx;
      ymax+=dy;
      for(int i=0;i<points;i++)
      {
         xx[i]+=dx; yy[i]+=dy;
      }
   }
}

void GMapPoly::resize(int new_width, int new_height)
{
   if (get_width()==new_width && get_height()==new_height)
       return;

   int width=get_width();
   int height=get_height();
   for(int i=0;i<points;i++)
   {
      xx[i]=xmin+(xx[i]-xmin)*new_width/width;
      yy[i]=ymin+(yy[i]-ymin)*new_height/height;
   }
   ComputeBoundingBox();
}

void GMapPoly::transform(const GRect & grect)
{
   if (grect.xmin==get_xmin() && grect.ymin==get_ymin() &&
       grect.xmax==get_xmax() && grect.ymax==get_ymax())
       return;

   int width=get_width();
   int height=get_height();
   for(int i=0;i<points;i++)
   {
      xx[i]=grect.xmin+(xx[i]-xmin)*grect.width()/width;
      yy[i]=grect.ymin+(yy[i]-ymin)*grect.height()/height;
   }
   ComputeBoundingBox();
}

char const * const GMapPoly::gma_check_object(void) const
{
   const char * str;
   str=(border_type!=NO_BORDER &&
        border_type!=SOLID_BORDER &&
        border_type!=XOR_BORDER) ? error_poly_border:
       ((hilite_color!=0xffffffff) ? error_poly_hilite:"");
   return str;
}

GMapPoly::GMapPoly(const int * _xx, const int * _yy, int _points, bool _open) :
   open(_open), points(_points)
{
   sides=points-(open!=0);
   
   xx.resize(points-1); yy.resize(points-1);
   for(int i=0;i<points;i++)
   {
      xx[i]=_xx[i]; yy[i]=_yy[i];
   }
   optimize_data();
   ComputeBoundingBox();
   char const * const res=check_data();
   if (res[0])
     G_THROW(res);
}

int GMapPoly::add_vertex(int x, int y)
{
    points++;
    sides=points-(open!=0);

    xx.resize(points-1); yy.resize(points-1);
    xx[points-1] = x;
    yy[points-1] = y;

    ComputeBoundingBox();
    return points;
}

void GMapPoly::close_poly()
{
    open = false;
    sides=points;
}

GUTF8String GMapPoly::gma_print(void)
{
   static const GUTF8String space(' ');
   GUTF8String res=GUTF8String('(')+POLY_TAG+space;
   for(int i=0;i<points;i++)
   {
      GUTF8String buffer;
      res+=buffer.format("%d %d ", xx[i], yy[i]);
   }
   res.setat(res.length()-1, ')');
   res+=space;
   return res;
}

/// Virtual function generating a list of defining coordinates
void GMapPoly::get_coords( GList<int> & CoordList ) const
{
  for(int i = 0 ; i < points ; i++)
  {
    CoordList.append( xx[i] );
    CoordList.append( yy[i] );
  }
}

void GMapPoly::map(GRectMapper &mapper)
{
   for(int i=0; i<points; i++)
   {
      mapper.map(xx[i], yy[i]);
   }
   ComputeBoundingBox();
}

void GMapPoly::unmap(GRectMapper &mapper)
{
   for(int i=0; i<points; i++)
   {
      mapper.unmap(xx[i], yy[i]);
   }
   ComputeBoundingBox();
}


void GMapPoly::rotateArea(int rot,int cx,int cy)
{
	rot=rot%4;
	if(rot==0)
		return;

   for(int i=0; i<points; i++)
      RotatePoint( rot, cx, cy, xx[i], yy[i]);
	
   ComputeBoundingBox();
}
/****************************************************************************
**************************** GMapOval definition ****************************
****************************************************************************/

void GMapOval::resize(int new_width, int new_height)
{
   if (get_width()==new_width && get_height()==new_height)
       return;
   xmax=xmin+new_width;
   ymax=ymin+new_height;
   initialize();
}

void GMapOval::transform(const GRect & grect)
{
   if (grect.xmin==get_xmin() && grect.ymin==get_ymin() &&
       grect.xmax==get_xmax() && grect.ymax==get_ymax())
       return;
   xmin=grect.xmin; ymin=grect.ymin;
   xmax=grect.xmax; ymax=grect.ymax;
   initialize();
}

bool GMapOval::is_point_inside(const int x, const int y) const
{   
   if ( !(x>=xmin && x<xmax && y>=ymin && y<ymax) ) 
      return false;

   return
      sqrt((double)((x-xf1)*(x-xf1)+(y-yf1)*(y-yf1)))+
      sqrt((double)((x-xf2)*(x-xf2)+(y-yf2)*(y-yf2)))<=2*m_nRmax;
}

char const * const GMapOval::gma_check_object(void) const
{
   return (border_type!=NO_BORDER &&
       border_type!=SOLID_BORDER &&
       border_type!=XOR_BORDER)?error_oval_border:
      ((hilite_color!=0xffffffff) ? error_oval_hilite:"");
}

void GMapOval::initialize(void)
{
   int xc=(xmax+xmin)/2;
   int yc=(ymax+ymin)/2;
   int f;
   
   m_nA=(xmax-xmin)/2;
   m_nB=(ymax-ymin)/2;
   if (m_nA > m_nB)
   {
      m_nRmin=m_nB; m_nRmax=m_nA;
      f=(int)sqrt((double)(m_nRmax*m_nRmax-m_nRmin*m_nRmin));
      xf1=xc+f; xf2=xc-f; yf1=yf2=yc;
   } else
   {
      m_nRmin=m_nA; m_nRmax=m_nB;
      f=(int)sqrt((double)(m_nRmax*m_nRmax-m_nRmin*m_nRmin));
      yf1=yc+f; yf2=yc-f; xf1=xf2=xc;
   }
}

GMapOval::GMapOval(const GRect & rect)
{
   xmin = rect.xmin;
   xmax = rect.xmax;
   ymin = rect.ymin;
   ymax = rect.ymax;
   initialize();
}

GUTF8String GMapOval::gma_print(void)
{
   GUTF8String buffer;
   return buffer.format("(%s %d %d %d %d) ", OVAL_TAG, xmin, ymin, xmax-xmin, ymax-ymin);
}

void GMapOval::map(GRectMapper &mapper)
{
    GRect rect;
    rect.xmin = xmin;
    rect.xmax = xmax;
    rect.ymin = ymin;
    rect.ymax = ymax;
    mapper.map(rect);
    xmin = rect.xmin;
    ymin = rect.ymin;
    xmax = rect.xmax;
    ymax = rect.ymax;
    initialize();
}

void GMapOval::unmap(GRectMapper &mapper)
{
    GRect rect;
    rect.xmin = xmin;
    rect.xmax = xmax;
    rect.ymin = ymin;
    rect.ymax = ymax;
    mapper.unmap(rect);
    xmin = rect.xmin;
    ymin = rect.ymin;
    xmax = rect.xmax;
    ymax = rect.ymax;
    initialize();
}

void GMapOval::rotateArea(int rot,int cx,int cy)
{
	rot=rot%4;
	if(rot==0)
		return;

   RotatePoint( rot, cx, cy, xmin, ymin);
   RotatePoint( rot, cx, cy, xmax, ymax);

   int temp = 0;
	if(xmin>xmax)
	{
		temp=xmin;
		xmin=xmax;
		xmax=temp;
	}
	if(ymin>ymax)
	{
		temp=ymin;
		ymin=ymax;
		ymax=temp;
	}
	initialize();
}


GMapArea::GMapArea(void) : target("_self"), border_type(NO_BORDER),
   border_always_visible(false), border_color(0xff), border_width(1),
   hilite_color(0xffffffff),
   xmin(0), ymin(0), xmax(0), ymax(0)
{}

GMapRect::GMapRect(void) 
{
   m_n0pacity = HILITE_DEFUALT_OPACITY;
}

GMapRect::GMapRect(const GRect & rect)
{
  m_n0pacity = HILITE_DEFUALT_OPACITY;
  xmin = rect.xmin;
  ymin = rect.ymin;
  xmax = rect.xmax;
  ymax = rect.ymax;
}

GMapRect & GMapRect::operator=(const GRect & rect)
{
   xmin=rect.xmin;
   xmax=rect.xmax;
   ymin=rect.ymin;
   ymax=rect.ymax;
   return *this;
}

void GMapRect::move(int dx, int dy)
{
   if (dx || dy)
   {
      xmin+=dx;
      ymin+=dy;
      xmax+=dx;
      ymax+=dy;
   }
}

bool GMapRect::is_point_inside(const int x, const int y) const
{
   return (x>=xmin)&&(x<xmax)&&(y>=ymin)&&(y<ymax);
}

GP<GMapArea> GMapRect::get_copy(void)const 
{ 
   return new GMapRect(*this); 
}

void  GMapRect::SetOpacity(int nO)
{
   if ( nO >=HILITE_MIN_OPACITY && nO <= HILITE_MAX_OPACITY )
      m_n0pacity = nO;
}

int  GMapRect::GetOpacity()  const
{
   return m_n0pacity;
}

GMapPoly::GMapPoly(void)
: points(0), sides(0) 
{}

void GMapPoly::ComputeBoundingBox()
{
   if ( points == 0 )
   {
      xmin = xmax = ymin = ymax = 0;
      return;
   }
   xmin = xmax = xx[0];
   ymin = ymax = yy[0];
   for ( int i=1; i<points; i++ )
   {
      if (xmin>xx[i]) 
         xmin=xx[i];
      else if (xmax<xx[i]) 
         xmax = xx[i];
      if (ymin>yy[i]) 
         ymin=yy[i];
      else if ( ymax<yy[i] )
         ymax=yy[i];
   }
}

void GMapPoly::move_vertex(int i, int x, int y)
{
   xx[i]=x; yy[i]=y;
   ComputeBoundingBox();
}

GP<GMapArea> GMapPoly::get_copy(void) const 
{ 
   return new GMapPoly(*this); 
}

GMapOval::GMapOval(void) 
{
   initialize();
}

void GMapOval::move(int dx, int dy)
{
   if (dx || dy)
   {
      xmin+=dx;
      ymin+=dy;
      xmax+=dx;
      ymax+=dy;
      xf1+=dx; yf1+=dy; xf2+=dx; yf2+=dy;
   }
}

GP<GMapArea> GMapOval::get_copy(void) const
{
  return new GMapOval(*this);
}

GUTF8String  GMapArea::XmlTagCommon() const
{
   GUTF8String retval;
   retval.format( "shape=\"%s\" alt=\"%s\"", get_shape_name(), (char const*)comment.toEscaped() );

   if(url.length())
   {
      retval = retval + " href=\"" + url + "\"";
   }else
   {
      retval = retval + " nohref=\"nohref\"";
   }
   if(target.length())
   {
      retval += " target=\"" + target.toEscaped() + "\"";
   }
   //  highlight
   if( hilite_color != GMapArea::NO_HILITE &&
      hilite_color != GMapArea::XOR_HILITE )
   {
      retval += GUTF8String().format( " highlight=\"#%06X\"", hilite_color );
   }
   const char *b_type="none";
   switch( border_type )
   {
   case GMapArea::NO_BORDER:
      b_type = "none";
      break;
   case GMapArea::XOR_BORDER:
      b_type = "xor";
      break;
   case GMapArea::SOLID_BORDER:
      b_type = "solid";
      break;
   case GMapArea::SHADOW_IN_BORDER:
      b_type = "shadowin";
      break;
   case GMapArea::SHADOW_OUT_BORDER:
      b_type = "shadowout";
      break;
   case GMapArea::SHADOW_EIN_BORDER:
      b_type = "etchedin";
      break;
   case GMapArea::SHADOW_EOUT_BORDER:
      b_type = "etchedout";
      break;
   }
   retval = retval + " bordertype=\"" + b_type + "\"";
   if( border_type != GMapArea::NO_BORDER)
   {
      retval = retval + " bordercolor=\"" + GUTF8String().format("#%06X",border_color)
                       + "\" border=\"" + GUTF8String(border_width) + "\"";
   }
   if(border_always_visible )
      retval = retval + " visible=\"visible\"";
   return retval;
}

#if 0
static GUTF8String  GMapArea2xmltag( const GMapArea &area, const GUTF8String &coords)
{
  GUTF8String retval;
  retval.format( "<AREA coords=\"%s\" shape=\"%s\" alt=\"%s\"", 
                 (char const*)coords, area.get_shape_name(), (char const*)area.comment.toEscaped() );
  if(area.url.length())
  {
    retval = retval + " href=\"" + area.url + "\"";
  }else
  {
    retval = retval + " nohref=\"nohref\"";
  }
  if(area.target.length())
  {
    retval += " target=\"" + area.target.toEscaped() + "\"";
  }
  //  highlight
  if( area.hilite_color != GMapArea::NO_HILITE &&
      area.hilite_color != GMapArea::XOR_HILITE )
  {
    retval += GUTF8String().format( " highlight=\"#%06X\"", area.hilite_color );
  }
  const char *b_type="none";
  switch( area.border_type )
  {
  case GMapArea::NO_BORDER:
    b_type = "none";
    break;
  case GMapArea::XOR_BORDER:
    b_type = "xor";
    break;
  case GMapArea::SOLID_BORDER:
    b_type = "solid";
    break;
  case GMapArea::SHADOW_IN_BORDER:
    b_type = "shadowin";
    break;
  case GMapArea::SHADOW_OUT_BORDER:
    b_type = "shadowout";
    break;
  case GMapArea::SHADOW_EIN_BORDER:
    b_type = "etchedin";
    break;
  case GMapArea::SHADOW_EOUT_BORDER:
    b_type = "etchedout";
    break;
  }
  retval = retval + " bordertype=\"" + b_type + "\"";
  if( area.border_type != GMapArea::NO_BORDER)
  {
    retval = retval + " bordercolor=\"" + GUTF8String().format("#%06X",area.border_color)
      + "\" border=\"" + GUTF8String(area.border_width) + "\"";
  }
  if(area.border_always_visible )
    retval = retval + " visible=\"visible\"";
  return retval+" />\n";
}
#endif

GUTF8String GMapRect::get_xmltag(const int height) const
{
   GUTF8String retval;
   retval.format( "<AREA coords=\"%d,%d,%d,%d\" ", xmin, height-1-ymax, xmax, height-1-ymin );
   retval += XmlTagCommon();

   GUTF8String extra = "";
   if ( m_n0pacity != HILITE_DEFUALT_OPACITY )
      retval += extra.format(" %s=\"%d\"", OPACITY_TAG, m_n0pacity );

   retval += " />\n";
   return retval;
}

GUTF8String GMapOval::get_xmltag(const int height) const
{ 
   GUTF8String retval;
   retval.format( "<AREA coords=\"%d,%d,%d,%d\" ", xmin, height-1-ymax, xmax, height-1-ymin );
   retval += XmlTagCommon();
   retval += " />\n";
   return retval;
}

GUTF8String GMapPoly::get_xmltag(const int height) const
{
   GList<int> CoordList;
   get_coords(CoordList);
   GPosition pos=CoordList;
   GUTF8String coords;
   if(pos)
   {
      coords = GUTF8String(CoordList[pos]);
      while(++pos)
      {
         coords+=","+GUTF8String(height-1-CoordList[pos]);
         if(! ++pos)
            break;
         coords+=","+GUTF8String(CoordList[pos]);
      }
   }
   GUTF8String retval;
   retval.format( "<AREA coords=\"%s\" ", (char const *)coords );
   retval += XmlTagCommon();
   retval += " />\n";
   return retval;
}


/////////////////////////////////////////////////////////////
//
// class GMapText
//
/////////////////////////////////////////////////////////////
GMapText::GMapText(bool bPushPin) 
{
   m_ulBkColor = NO_HILITE;   // transparant back ground
   m_ulTextColor = DEFAULT_FG_COLOR;  // black 
   m_bPushPin = bPushPin;
}

GMapText::GMapText(const GRect & rect, bool bPushPin)
{
   m_ulBkColor = NO_HILITE;   // transpatrant back ground
   m_ulTextColor = DEFAULT_FG_COLOR;  // black 

   m_bPushPin = bPushPin;
   xmin = rect.xmin;
   ymin = rect.ymin;
   xmax = rect.xmax;
   ymax = rect.ymax;
}

GP<GMapText> GMapText::create(bool bPushPin /*= false */)
{
   return new GMapText(bPushPin);
}

   /// Create with the specified GRect.
GP<GMapText> GMapText::create(const GRect &rect, bool bPushPin /*= false */)
{
   return new GMapText(rect, bPushPin);
}

GP<GMapArea> GMapText::get_copy(void) const
{
   return new GMapText(*this);     // TODO: STang
}

void GMapText::map(GRectMapper &mapper)
{
    GRect rect;
    rect.xmin = xmin;
    rect.xmax = xmax;
    rect.ymin = ymin;
    rect.ymax = ymax;
    mapper.map(rect);
    xmin = rect.xmin;
    ymin = rect.ymin;
    xmax = rect.xmax;
    ymax = rect.ymax;
}

void GMapText::unmap(GRectMapper &mapper)
{
    GRect rect;
    rect.xmin = xmin;
    rect.xmax = xmax;
    rect.ymin = ymin;
    rect.ymax = ymax;
    mapper.unmap(rect);
    xmin = rect.xmin;
    ymin = rect.ymin;
    xmax = rect.xmax;
    ymax = rect.ymax;
}

void GMapText::move(int dx, int dy)
{
   if (dx || dy)
   {
      xmin+=dx;
      ymin+=dy;
      xmax+=dx;
      ymax+=dy;
   }
}

void GMapText::rotateArea(int rot,int cx,int cy)
{
   rot=rot%4;
	if(rot==0)
		return;

   RotatePoint( rot, cx, cy, xmin, ymin);
   RotatePoint( rot, cx, cy, xmax, ymax);

   int temp = 0;
	if(xmin>xmax)
	{
		temp=xmin;
		xmin=xmax;
		xmax=temp;
	}
	if(ymin>ymax)
	{
		temp=ymin;
		ymin=ymax;
		ymax=temp;
	}
}

void GMapText::resize(int new_width, int new_height)
{
   if (get_width()==new_width && get_height()==new_height)
       return;
   xmax=xmin+new_width;
   ymax=ymin+new_height;
}

void GMapText::transform(const GRect & grect)
{
   xmin=grect.xmin; ymin=grect.ymin;
   xmax=grect.xmax; ymax=grect.ymax;
}

bool GMapText::is_point_inside(const int x, const int y) const
{
   return (x>=xmin)&&(x<xmax)&&(y>=ymin)&&(y<ymax);
}

bool GMapText::GetPushPin() const
{
   return  m_bPushPin;
}
unsigned long int GMapText::GetBackClr() const
{
   return m_ulBkColor;
}
unsigned long int GMapText::GetTextClr() const
{
   return m_ulTextColor;
}

void   GMapText::SetPushPin(bool b)
{
   m_bPushPin = b;
}
void   GMapText::SetBackClr(unsigned long int clr)
{
   m_ulBkColor = clr;
}
void   GMapText::SetTextClr(unsigned long int clr)
{
   m_ulTextColor = clr;
}


char const * const  GMapText::gma_check_object(void) const
{
   return "";  // TODO: STang
}

GUTF8String GMapText::get_xmltag(const int height) const
{
   GUTF8String retval;
   retval.format( "<AREA coords=\"%d,%d,%d,%d\" ", xmin, height-1-ymax, xmax, height-1-ymin );
   retval += XmlTagCommon();

   GUTF8String extra = "";
   if ( m_bPushPin )
      retval += extra.format(" %s=\"true\"", TEXT_PUSHPIN_TAG );
   if ( m_ulBkColor != NO_HILITE )
      retval += extra.format(" %s=\"#%06X\"", BACK_CLR_TAG, m_ulBkColor );
   if ( m_ulTextColor != DEFAULT_FG_COLOR )  // black 
      retval += extra.format(" %s=\"#%06X\"", TEXT_CLR_TAG, m_ulTextColor );
   retval += " />\n";
   return retval;
}

GUTF8String  GMapText::gma_print(void)
{
   GUTF8String buffer;
   buffer.format("(%s %d %d %d %d) ", TEXT_TAG, xmin, ymin, xmax-xmin, ymax-ymin);
   GUTF8String extra = "";
   if ( m_bPushPin )
      buffer += extra.format("(%s) ", TEXT_PUSHPIN_TAG );
   if ( m_ulBkColor != NO_HILITE )
      buffer += extra.format("(%s #%06X) ", BACK_CLR_TAG, m_ulBkColor );
   if ( m_ulTextColor != DEFAULT_FG_COLOR )  // black 
      buffer += extra.format("(%s #%06X) ", TEXT_CLR_TAG, m_ulTextColor );
   return buffer;
}


/////////////////////////////////////////////////////////////
//
// class GMapLine
//
/////////////////////////////////////////////////////////////
#define LINE_BOUNDING_BOX_MIN_SPACE   32
GMapLine::GMapLine(bool bArrow)
{
   m_ulLineColor = DEFAULT_FG_COLOR;
   m_nLineWidth = DEFAULT_LINE_WIDTH;
   m_bArrow = bArrow;
}

GMapLine::GMapLine(const SPoint& ptStart, const SPoint& ptEnd, bool bArrow)
{
   m_ulLineColor = DEFAULT_FG_COLOR;
   m_nLineWidth = DEFAULT_LINE_WIDTH;
   m_ptStart = ptStart;
   m_ptEnd = ptEnd;
   m_bArrow = bArrow;
   ComputeBoundingBox();
}

GP<GMapLine> GMapLine::create(bool bArrow /*= false */) 
{ 
   return new GMapLine(bArrow); 
}

GP<GMapLine> GMapLine::create(const SPoint& ptStart, const SPoint& ptEnd, bool bArrow /* = false*/ ) 
{ 
   return new GMapLine(ptStart, ptEnd, bArrow); 
}

void GMapLine::ComputeBoundingBox()
{
   if ( m_ptStart.m_nX < m_ptEnd.m_nX )
   {
      xmin = m_ptStart.m_nX;
      xmax = m_ptEnd.m_nX;
   }
   else
   {
      xmin = m_ptEnd.m_nX;
      xmax = m_ptStart.m_nX;
   }

   if ( m_ptStart.m_nY < m_ptEnd.m_nY )
   {
      ymin = m_ptStart.m_nY;
      ymax = m_ptEnd.m_nY;
   }
   else
   {
      ymin = m_ptEnd.m_nY;
      ymax = m_ptStart.m_nY;
   }
   xmin -= LINE_BOUNDING_BOX_MIN_SPACE;
   xmax += LINE_BOUNDING_BOX_MIN_SPACE;
   ymin -= LINE_BOUNDING_BOX_MIN_SPACE;
   ymax += LINE_BOUNDING_BOX_MIN_SPACE;
   if ( xmin < 0 )
      xmin = 0;
   if ( ymin < 0 )
      ymin = 0;

}

GP<GMapArea> GMapLine::get_copy(void) const
{
   return new GMapLine(*this);     // TODO: STang
}

void GMapLine::map(GRectMapper &mapper)
{
   mapper.map(m_ptStart.m_nX, m_ptStart.m_nY);
   mapper.map(m_ptEnd.m_nX,   m_ptEnd.m_nY);
   ComputeBoundingBox();
}

void GMapLine::unmap(GRectMapper &mapper)
{
   mapper.unmap(m_ptStart.m_nX, m_ptStart.m_nY);
   mapper.unmap(m_ptEnd.m_nX,   m_ptEnd.m_nY);
   ComputeBoundingBox();
}

void GMapLine::rotateArea(int rot,int cx,int cy)
{
	rot=rot%4;
	if(rot==0)
		return;

   RotatePoint( rot, cx, cy, m_ptStart.m_nX, m_ptStart.m_nY);
   RotatePoint( rot, cx, cy, m_ptEnd.m_nX,   m_ptEnd.m_nY);
   ComputeBoundingBox();
}
void GMapLine::move(int dx, int dy)
{
   if (dx || dy)
   {
      xmin+=dx;
      ymin+=dy;
      xmax+=dx;
      ymax+=dy;
      m_ptStart.m_nX += dx;
      m_ptStart.m_nY += dy;
      m_ptEnd.m_nX += dx;
      m_ptEnd.m_nY += dy;
   }
}

void GMapLine::resize(int new_width, int new_height)
{
   if (get_width()==new_width && get_height()==new_height)
       return;
   int width=get_width();
   int height=get_height();
   m_ptStart.m_nX=xmin+(m_ptStart.m_nX-xmin)*new_width/width;
   m_ptStart.m_nY=ymin+(m_ptStart.m_nY-ymin)*new_height/height;
   m_ptEnd.m_nX=xmin+(m_ptEnd.m_nX-xmin)*new_width/width;
   m_ptEnd.m_nY=ymin+(m_ptEnd.m_nY-ymin)*new_height/height;
   ComputeBoundingBox();
}

void GMapLine::transform(const GRect & grect)
{
   if (grect.xmin==get_xmin() && grect.ymin==get_ymin() &&
       grect.xmax==get_xmax() && grect.ymax==get_ymax())
       return;

   int width=get_width();
   int height=get_height();
   m_ptStart.m_nX=xmin+(m_ptStart.m_nX-xmin)*grect.width()/width;
   m_ptStart.m_nY=ymin+(m_ptStart.m_nY-ymin)*grect.height()/height;
   m_ptEnd.m_nX=xmin+(m_ptEnd.m_nX-xmin)*grect.width()/width;
   m_ptEnd.m_nY=ymin+(m_ptEnd.m_nY-ymin)*grect.height()/height;
   ComputeBoundingBox();
}

bool GMapLine::is_point_inside(const int x, const int y) const
{
   return false;
}

char const * const  GMapLine::gma_check_object(void) const
{
   if ( m_nLineWidth < MIN_LINE_WIDTH || m_nLineWidth > MAX_LINE_WIDTH )
      return error_line_width;
   else if ( border_type != NO_BORDER )
      return error_line_border;

   return "";
}

GUTF8String GMapLine::get_xmltag(const int height) const
{
   GList<int> CoordList;
   get_coords(CoordList);
   GPosition pos=CoordList;
   GUTF8String coords;
   if(pos)
   {
      coords = GUTF8String(CoordList[pos]);
      while(++pos)
      {
         coords+=","+GUTF8String(height-1-CoordList[pos]);
         if(! ++pos)
            break;
         coords+=","+GUTF8String(CoordList[pos]);
      }
   }
   GUTF8String retval;
   retval.format( "<AREA coords=\"%s\" ", (char const *)coords );
   retval += XmlTagCommon();

   GUTF8String extra = "";
   if ( m_bArrow )
      retval += extra.format(" %s=\"true\"", LINE_ARROW_TAG );
   if ( m_nLineWidth != DEFAULT_LINE_WIDTH )
      retval += extra.format(" %s=\"%d\"", LINE_WIDTH_TAG, m_nLineWidth );
   if ( m_ulLineColor != DEFAULT_FG_COLOR )  // black 
      retval += extra.format(" %s=\"#%06X\"", LINE_CLR_TAG, m_ulLineColor );

   retval += " />\n";
   return retval;
}

GUTF8String  GMapLine::gma_print(void)
{
   GUTF8String buffer;
   buffer.format("(%s %d %d %d %d) ", LINE_TAG, m_ptStart.m_nX, m_ptStart.m_nY, m_ptEnd.m_nX, m_ptEnd.m_nY );
   GUTF8String extra = "";
   if ( m_bArrow )
      buffer += extra.format("(%s) ", LINE_ARROW_TAG );
   if ( m_nLineWidth != DEFAULT_LINE_WIDTH )
      buffer += extra.format("(%s %d) ", LINE_WIDTH_TAG, m_nLineWidth );
   if ( m_ulLineColor != DEFAULT_FG_COLOR )  // black 
      buffer += extra.format("(%s #%06X) ", LINE_CLR_TAG, m_ulLineColor );
   return buffer;
}

int   GMapLine::GetLineWidth() const
{
   return m_nLineWidth;
}
unsigned long int   GMapLine::GetLineColor() const
{
   return m_ulLineColor;
}
bool   GMapLine::GetLineArrow() const
{
   return m_bArrow;
}
SPoint   GMapLine::GetLineStartPoint() const
{
   return m_ptStart;
}
SPoint   GMapLine::GetLineEndPoint() const
{
   return m_ptEnd;
}

void   GMapLine::SetLineWidth(int w)
{
   if ( !(w < MIN_LINE_WIDTH && w > MAX_LINE_WIDTH) )
      m_nLineWidth = w;
}
void   GMapLine::SetLineColor(unsigned long int clr)
{
   m_ulLineColor = (clr&0xFFFFFF);
}
void   GMapLine::SetLineArrow(bool bArrow)
{
   m_bArrow = bArrow;
}
void   GMapLine::SetLineStartPoint(const SPoint& pt)
{
   m_ptStart = pt;
   ComputeBoundingBox();
}
void   GMapLine::SetLineEndPoint(const SPoint& pt)
{
   m_ptEnd = pt;
   ComputeBoundingBox();
}

void GMapLine::get_coords( GList<int> & CoordList ) const
{
   CoordList.append( m_ptStart.m_nX );
   CoordList.append( m_ptStart.m_nY );
   CoordList.append( m_ptEnd.m_nX );
   CoordList.append( m_ptEnd.m_nY );
}
