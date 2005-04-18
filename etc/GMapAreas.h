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

#ifndef _GMAPAREAS_H
#define _GMAPAREAS_H

#include "DjVuGlobal.h"
#include "GSmartPointer.h"
#include "GContainer.h"
#include "GString.h"
#include "GRect.h"
#include "GURL.h"

/** @name GMapAreas.h

    Files #"GMapAreas.h"# and #"GMapAreas.cpp"# implement base objects
    used by the plugin to display and manage hyperlinks and highlighted
    areas inside a \Ref{DjVuImage} page.

    The currently supported areas can be rectangular (\Ref{GMapRect}),
    elliptical (\Ref{GMapOval}) and polygonal (\Ref{GMapPoly}). Every
    map area besides the definition of its shape contains information
    about display style and optional {\bf URL}, which it may refer to.
    If this {\bf URL} is not empty then the map area will work like a
    hyperlink.

    The classes also implement some useful functions to ease geometry
    manipulations

    @memo Definition of base map area classes
    @author Andrei Erofeev <eaf@geocities.com>
    @version
    #$Id$# */
//@{


// ---------- GMAPAREA ---------

/** This is the base object for all map areas. It defines some standard
    interface to access the geometrical properties of the areas and
    describes the area itsef:
    \begin{itemize}
       \item #url# If the optional #URL# is specified, the map area will
             also work as a hyperlink meaning that if you click it with
	     your mouse pointer, the browser will be advised to load
	     the page referenced by the #URL#.
       \item #target# Defines where the specified #URL# should be loaded
       \item #comment# This is a string displayed in a status line or in
             a popup window when the mouse pointer moves over the hyperlink
	     area
       \item #border_type#, #border_color# and #border_width# describes
             how the area border should be drawn
       \item #area_color# describes how the area should be highlighted.
    \end{itemize}

    The map areas can be displayed using two different techniques, which
    can be combined together:
    \begin{itemize}
       \item Visible border. The border of a map area can be drawn in several
             different ways (like #XOR_BORDER# or #SHADOW_IN_BORDER#).
	     It can be made always visible, or appearing only when the
	     mouse pointer moves over the map area.
       \item Highlighted contents. Contents of rectangular map areas can
             also be highlighted with some given color.
    \end{itemize}
*/

class GMapArea : public GPEnabled
{
protected:
   DJVUREFAPI GMapArea(void);
public:
//      // Default creator.
//   static GP<GMapArea> create(void) {return new GMapArea();}

      /// Virtual destructor.
   virtual ~GMapArea() {}

   DJVUREFAPI static const char MAPAREA_TAG [];
   DJVUREFAPI static const char RECT_TAG [];
   DJVUREFAPI static const char POLY_TAG [];
   DJVUREFAPI static const char OVAL_TAG [];
   DJVUREFAPI static const char TEXT_TAG [];
   DJVUREFAPI static const char LINE_TAG [];
   DJVUREFAPI static const char TEXT_PUSHPIN_TAG [];
   DJVUREFAPI static const char BACK_CLR_TAG [];
   DJVUREFAPI static const char TEXT_CLR_TAG [];
   DJVUREFAPI static const char LINE_ARROW_TAG [];
   DJVUREFAPI static const char LINE_WIDTH_TAG [];
   DJVUREFAPI static const char LINE_CLR_TAG [];
   DJVUREFAPI static const char OPACITY_TAG[];
   DJVUREFAPI static const char NO_BORDER_TAG [];
   DJVUREFAPI static const char XOR_BORDER_TAG [];
   DJVUREFAPI static const char SOLID_BORDER_TAG [];
   DJVUREFAPI static const char SHADOW_IN_BORDER_TAG [];
   DJVUREFAPI static const char SHADOW_OUT_BORDER_TAG [];
   DJVUREFAPI static const char SHADOW_EIN_BORDER_TAG [];
   DJVUREFAPI static const char SHADOW_EOUT_BORDER_TAG [];
   DJVUREFAPI static const char BORDER_AVIS_TAG [];
   DJVUREFAPI static const char HILITE_TAG [];
   DJVUREFAPI static const char URL_TAG [];
   DJVUREFAPI static const char TARGET_SELF [];

   enum BorderType { INVALID=-1, NO_BORDER=0, XOR_BORDER=1, SOLID_BORDER=2,
		     SHADOW_IN_BORDER=3, SHADOW_OUT_BORDER=4,
		     SHADOW_EIN_BORDER=5, SHADOW_EOUT_BORDER=6 };

   enum Special_Hilite_Color{ NO_HILITE=0xFFFFFFFF, XOR_HILITE=0xFF000000, DEFAULT_FG_COLOR=0x000000};

   // Enumeration for reporting the type of map area. "MapUnknown" is reported
   // for objects of type GMapArea (there shouldn't be any).
   enum MapAreaType { UNKNOWN, RECT, OVAL, POLY, TEXT, LINE };

      /** Optional URL which this map area can be associated with.
	  If it's not empty then clicking this map area with the mouse
	  will make the browser load the HTML page referenced by
	  this #url#.  Note: This may also be a relative URL, so the
          GURL class is not used. */
   GUTF8String	url;
      /** The target for the #URL#. Standard targets are:
	  \begin{itemize}
	     \item #_blank# - Load the link in a new blank window
	     \item #_self# - Load the link into the plugin window
	     \item #_top# - Load the link into the top-level frame
	  \end{itemize} */
   GUTF8String	target;
      /** Comment (displayed in a status line or as a popup hint when
	  the mouse pointer moves over the map area */
   GUTF8String	comment;
      /** Border type. Defines how the map area border should be drawn
	  \begin{itemize}
	     \item #NO_BORDER# - No border drawn
	     \item #XOR_BORDER# - The border is drawn using XOR method.
	     \item #SOLID_BORDER# - The border is drawn as a solid line
	           of a given color.
	     \item #SHADOW_IN_BORDER# - Supported for \Ref{GMapRect} only.
	     	   The map area area looks as if it was "pushed-in".
	     \item #SHADOW_OUT_BORDER# - The opposite of #SHADOW_OUT_BORDER#
	     \item #SHADOW_EIN_BORDER# - Also for \Ref{GMapRect} only.
	     	   Is translated as "shadow etched in"
	     \item #SHADOW_EOUT_BORDER# - The opposite of #SHADOW_EIN_BORDER#.
	  \end{itemize} */
   BorderType	border_type;
      /** If #TRUE#, the border will be made always visible. Otherwise
	  it will be drawn when the mouse moves over the map area. */
   bool		border_always_visible;
      /// Border color (when relevant) in #0x00RRGGBB# format
   unsigned long int	border_color;
      /// Border width in pixels
   int		border_width;
      /** Specified a color for highlighting the internal area of the map
	  area. Will work with rectangular map areas only. The color is
	  specified in \#00RRGGBB format. A special value of \#FFFFFFFF disables
          highlighting and \#FF000000 is for XOR highlighting. */
   unsigned long int	hilite_color;

      /// Returns xmin of the bounding rectangle
   DJVUREFAPI int		get_xmin(void) const { return xmin; }
      /// Returns ymin of the bounding rectangle
   DJVUREFAPI int		get_ymin(void) const { return ymin; }
      /** Returns xmax of the bounding rectangle. In other words, if #X# is
	  a coordinate of the last point in the right direction, the
	  function will return #X+1# */
   DJVUREFAPI int		get_xmax(void) const { return xmax; }
      /** Returns xmax of the bounding rectangle. In other words, if #Y# is
	  a coordinate of the last point in the top direction, the
	  function will return #Y+1# */
   DJVUREFAPI int		get_ymax(void) const { return ymax; }
      /// Returns the hyperlink bounding rectangle
   DJVUREFAPI GRect	get_bound_rect(void) const;
      /** Checks if the object is OK. Especially useful with \Ref{GMapPoly}
	  where edges may intersect. If there is a problem it returns a
	  string describing it. */
   DJVUREFAPI char const *	const check_object(void);
      /** Stores the contents of the hyperlink object in a lisp-like format
	  for saving into #ANTa# chunk (see \Ref{DjVuAnno}) */
   DJVUREFAPI GUTF8String	print(void);

      /// Returns the width of the rectangle
   DJVUREFAPI int get_width(void) const { return xmax-xmin; }
      /// Returns the height of the rectangle
   DJVUREFAPI int get_height(void) const { return ymax-ymin; }

   virtual GUTF8String get_xmltag(const int height) const=0;

      /// Virtual function returning the shape type.
   virtual MapAreaType const get_shape_type( void ) const { return UNKNOWN; };
      /// Virtual function returning the shape name.
   virtual char const * const	get_shape_name(void) const=0;
      /// Virtual function generating a copy of this object
   virtual GP<GMapArea>	get_copy(void) const=0;
      /// Virtual function generating a list of defining coordinates
      /// (default are the opposite corners of the enclosing rectangle)
   virtual void get_coords( GList<int> & CoordList ) const;
   /// Virtual function maps maparea from one area to another using mapper
   virtual void map(GRectMapper &mapper)=0;
   /// Virtual function unmaps maparea from one area to another using mapper
   virtual void unmap(GRectMapper &mapper)=0;

   //rotate Area by 90,180,270 degree hen rot =1,2,3.
   virtual void		rotateArea(int rot,int cx,int cy) = 0;
   virtual void		move(int dx, int dy) = 0;
   virtual void		resize(int new_width, int new_height) = 0;
   virtual void		transform(const GRect & grect) = 0;
   virtual bool		is_point_inside(const int x, const int y) const=0;

   static BorderType BorderName2BorderType(const GUTF8String & name)
   {
      BorderType type =
                     name==GMapArea::NO_BORDER_TAG ? GMapArea::NO_BORDER :
                     name==GMapArea::XOR_BORDER_TAG ? GMapArea::XOR_BORDER :
                     name==GMapArea::SOLID_BORDER_TAG ? GMapArea::SOLID_BORDER :
                     name==GMapArea::SHADOW_IN_BORDER_TAG ? GMapArea::SHADOW_IN_BORDER :
                     name==GMapArea::SHADOW_OUT_BORDER_TAG ? GMapArea::SHADOW_OUT_BORDER :
                     name==GMapArea::SHADOW_EIN_BORDER_TAG ? GMapArea::SHADOW_EIN_BORDER :
                     name==GMapArea::SHADOW_EOUT_BORDER_TAG ? GMapArea::SHADOW_EOUT_BORDER : 
                     INVALID;
      return type;


   }
protected:
   int		xmin, xmax, ymin, ymax;

   virtual char const * const	gma_check_object(void) const=0;
   virtual GUTF8String	gma_print(void)=0;
   GUTF8String  XmlTagCommon() const;
};

// ---------- GMAPRECT ---------

/** Implements rectangular map areas. This is the only kind of map areas
    supporting #SHADOW_IN_BORDER#, #SHADOW_OUT_BORDER#, #SHADOW_EIN_BORDER#
    and #SHADOW_EOUT_BORDER# types of border and area highlighting. */

class GMapRect: public GMapArea
{
protected:
   DJVUREFAPI GMapRect(void);
   DJVUREFAPI GMapRect(const GRect & rect);
public:
   /// Default creator.
   DJVUREFAPI static GP<GMapRect> create(void);
   /// Create with the specified GRect.
   DJVUREFAPI static GP<GMapRect> create(const GRect &rect);

   virtual ~GMapRect() {}

      /// Changes the #GMapRect#'s geometry
   DJVUREFAPI GMapRect & operator=(const GRect & rect);

      /// Returns \Ref{GRect} describing the map area's rectangle
   operator GRect(void)
   {
      return GRect(xmin, ymin, xmax-xmin, ymax-ymin);
   }
   
   virtual GUTF8String get_xmltag(const int height) const;
      /// Returns MapRect
   virtual MapAreaType const get_shape_type( void ) const { return RECT; };
      /// Returns #"rect"#
   virtual char const * const	get_shape_name(void) const { return RECT_TAG; };
      /// Returns a copy of the rectangle
   virtual GP<GMapArea>	get_copy(void) const;
      /// Virtual function maps rectangle from one area to another using mapper
   virtual void map(GRectMapper &mapper);
      /// Virtual function unmaps rectangle from one area to another using mapper
   virtual void unmap(GRectMapper &mapper);
	  
   virtual void rotateArea(int rot,int cx,int cy);
   virtual void		move(int dx, int dy);
   virtual void		resize(int new_width, int new_height);
   virtual void		transform(const GRect & grect);
   virtual bool		is_point_inside(const int x, const int y) const;

   void              SetOpacity(int nO);
   int               GetOpacity()  const;

protected:
   virtual char const * const gma_check_object(void) const { return ""; };
   virtual GUTF8String	gma_print(void);

private:
   int m_n0pacity;  // 0 -- 100, Default value 50
};

// ---------- GMAPPOLY ---------

/** Implements polygonal map areas. The only supported types of border
    are #NO_BORDER#, #XOR_BORDER# and #SOLID_BORDER#. Its contents can not
    be highlighted either. It's worth mentioning here that despite its
    name the polygon may be open, which basically makes it a broken line.
    This very specific mode is used by the hyperlink editor when creating
    the polygonal hyperlink. */

class GMapPoly : public GMapArea
{
protected:
   DJVUREFAPI GMapPoly(void);
   DJVUREFAPI GMapPoly(const int * xx, const int * yy, int points, bool open=false);
public:
   /// Default creator
   DJVUREFAPI static GP<GMapPoly> create(void);

   /// Create from specified coordinates.
   DJVUREFAPI static GP<GMapPoly> create(
     const int xx[], const int yy[], const int points, const bool open=false);

   /// Virtual destructor.
   virtual ~GMapPoly() {}

      /// Returns 1 if side #side# crosses the specified rectangle #rect#.
   DJVUREFAPI bool		does_side_cross_rect(const GRect & grect, int side);

      /// Returns the number of vertices in the polygon
   DJVUREFAPI int		get_points_num(void) const { return points; };

      /// Returns the number sides in the polygon
   DJVUREFAPI int		get_sides_num(void) const { return sides; };

      /// Returns x coordinate of vertex number #i#
   DJVUREFAPI int		get_x(int i) const { return xx[i]; };
   
      /// Returns y coordinate of vertex number #i#
   DJVUREFAPI int		get_y(int i) const { return yy[i]; };

      /// Moves vertex #i# to position (#x#, #y#)
   DJVUREFAPI void		move_vertex(int i, int x, int y);

      /// Adds a new vertex and returns number of vertices in the polygon
   DJVUREFAPI int      add_vertex(int x, int y);

      /// Closes the polygon if it is not closed
   DJVUREFAPI void     close_poly();
      /// Optimizes the polygon 
   DJVUREFAPI void		optimize_data(void);
      /// Checks validity of the polygon 
   DJVUREFAPI char const * const	check_data(void);

   virtual GUTF8String get_xmltag(const int height) const;
      /// Returns MapPoly
   virtual MapAreaType const get_shape_type( void ) const { return POLY; };
      /// Returns #"poly"# all the time
   virtual char const * const 	get_shape_name(void) const { return POLY_TAG; };
      /// Returns a copy of the polygon
   virtual GP<GMapArea>	get_copy(void) const;
      /// Virtual function generating a list of defining coordinates
   virtual void get_coords( GList<int> & CoordList ) const;
      /// Virtual function maps polygon from one area to another using mapper
   virtual void map(GRectMapper &mapper);
   /// Virtual function unmaps polygon from one area to another using mapper
   virtual void unmap(GRectMapper &mapper);

   virtual void rotateArea(int rot,int cx,int cy);
   virtual void		move(int dx, int dy);
   virtual void		resize(int new_width, int new_height);
   virtual void		transform(const GRect & grect);
   virtual bool		is_point_inside(const int x, const int y) const;
   
   virtual bool		is_open() const {return open;}
   
protected:
   virtual char const * const gma_check_object(void) const;
   virtual GUTF8String	gma_print(void);
private:
   void  ComputeBoundingBox();

   bool		open;
   int		points, sides;
   GTArray<int>	xx, yy;
   static int	sign(int x);
   static bool	is_projection_on_segment(int x, int y, int x1, int y1, int x2, int y2);
   static bool	do_segments_intersect(int x11, int y11, int x12, int y12,
				      int x21, int y21, int x22, int y22);
   static bool	are_segments_parallel(int x11, int y11, int x12, int y12,
				      int x21, int y21, int x22, int y22);
};

// ---------- GMAPOVAL ---------

/** Implements elliptical map areas. The only supported types of border
    are #NO_BORDER#, #XOR_BORDER# and #SOLID_BORDER#. Its contents can not
    be highlighted either. */

class GMapOval: public GMapArea
{
protected:
   DJVUREFAPI GMapOval(void);
   DJVUREFAPI GMapOval(const GRect & rect);
public:
   /// Default creator.
   DJVUREFAPI static GP<GMapOval> create(void);

   /// Create from the specified GRect.
   DJVUREFAPI static GP<GMapOval> create(const GRect &rect);

   /// Virtual destructor. 
   virtual ~GMapOval() {}

      /// Returns (xmax-xmin)/2
   DJVUREFAPI int		get_a(void) const { return m_nA; }
      /// Returns (ymax-ymin)/2
   DJVUREFAPI int		get_b(void) const { return m_nB; }
      /// Returns the lesser of \Ref{get_a}() and \Ref{get_b}()
   DJVUREFAPI int		get_rmin(void) const { return m_nRmin; }
      /// Returns the greater of \Ref{get_a}() and \Ref{get_b}()
   DJVUREFAPI int		get_rmax(void) const { return m_nRmax; }

   virtual GUTF8String get_xmltag(const int height) const;
      /// Returns MapOval
   virtual MapAreaType const get_shape_type( void ) const { return OVAL; };
      /// Returns #"oval"#
   virtual char const * const get_shape_name(void) const { return OVAL_TAG; };
      /// Returns a copy of the oval
   virtual GP<GMapArea>	get_copy(void) const;
      /// Virtual function maps oval from one area to another using mapper
   virtual void map(GRectMapper &mapper);
      /// Virtual function unmaps oval from one area to another using mapper
   virtual void unmap(GRectMapper &mapper);

   virtual void rotateArea(int rot,int cx,int cy);
   virtual void		move(int dx, int dy);
   virtual void		resize(int new_width, int new_height);
   virtual void		transform(const GRect & grect);
   virtual bool		is_point_inside(const int x, const int y) const;

protected:
   virtual char const * const	gma_check_object(void) const;
   virtual GUTF8String	gma_print(void);
private:
   int		m_nRmax, m_nRmin;
   int		m_nA, m_nB;
   int		xf1, yf1, xf2, yf2;
   
   void		initialize(void);
};

class GMapText: public GMapArea
{
protected:
   DJVUREFAPI GMapText(bool bPushPin);
   DJVUREFAPI GMapText(const GRect & rect, bool bPushPin);
public:
   DJVUREFAPI static GP<GMapText> create(bool bPushPin = false);
   DJVUREFAPI static GP<GMapText> create(const GRect &rect, bool bPushPin = false);
   virtual ~GMapText() {}

   virtual GUTF8String get_xmltag(const int height) const;
      /// Returns MapOval
   virtual MapAreaType const get_shape_type( void ) const { return TEXT; };
      /// Returns #"oval"#
   virtual char const * const get_shape_name(void) const { return TEXT_TAG; };
      /// Returns a copy of the oval
   virtual GP<GMapArea>	get_copy(void) const;
      /// Virtual function maps oval from one area to another using mapper
   virtual void map(GRectMapper &mapper);
      /// Virtual function unmaps oval from one area to another using mapper
   virtual void unmap(GRectMapper &mapper);

   virtual void rotateArea(int rot,int cx,int cy);
   virtual void		move(int dx, int dy);
   virtual void		resize(int new_width, int new_height);
   virtual void		transform(const GRect & grect);
   virtual bool		is_point_inside(const int x, const int y) const;

   bool              GetPushPin() const;
   unsigned long int GetBackClr() const;
   unsigned long int GetTextClr() const;
   void              SetPushPin(bool b);
   void              SetBackClr(unsigned long int clr);
   void              SetTextClr(unsigned long int clr);

protected:
   virtual char const * const	gma_check_object(void) const;
   virtual GUTF8String	gma_print(void);

private:
   bool              m_bPushPin;   // Default  false
   unsigned long int	m_ulBkColor;  // Default 0xFFFFFFFF (transparant back ground)
   unsigned long int	m_ulTextColor;  // Default 0x000000 (black)

};

class GMapLine: public GMapArea
{
protected:
   DJVUREFAPI GMapLine( bool bArrow);
   DJVUREFAPI GMapLine(const SPoint& ptStart, const SPoint& ptEnd, bool bArrow);
public:
   DJVUREFAPI static GP<GMapLine> create(bool bArrow = false); 
   DJVUREFAPI static GP<GMapLine> create(const SPoint& ptStart, const SPoint& ptEnd, bool bArrow = false);
   virtual ~GMapLine() {}

   virtual GUTF8String get_xmltag(const int height) const;
      /// Returns MapOval
   virtual MapAreaType const get_shape_type( void ) const { return LINE; };
      /// Returns #"oval"#
   virtual char const * const get_shape_name(void) const { return LINE_TAG; };
      /// Returns a copy of the oval
   virtual GP<GMapArea>	get_copy(void) const;
      /// Virtual function maps oval from one area to another using mapper
   virtual void map(GRectMapper &mapper);
      /// Virtual function unmaps oval from one area to another using mapper
   virtual void unmap(GRectMapper &mapper);

   virtual void rotateArea(int rot,int cx,int cy);
   virtual void		move(int dx, int dy);
   virtual void		resize(int new_width, int new_height);
   virtual void		transform(const GRect & grect);
   virtual bool		is_point_inside(const int x, const int y) const;

   int               GetLineWidth() const;
   unsigned long int GetLineColor() const;
   bool              GetLineArrow() const;
   SPoint            GetLineStartPoint() const;
   SPoint            GetLineEndPoint() const;
   
   void              SetLineWidth(int w);
   void              SetLineColor(unsigned long int clr);
   void              SetLineArrow(bool bArrow);
   void              SetLineStartPoint(const SPoint& pt);
   void              SetLineEndPoint(const SPoint& pt);
   virtual  void     get_coords( GList<int> & CoordList ) const;

protected:
   virtual char const * const	gma_check_object(void) const;
   virtual GUTF8String	gma_print(void);

private:
   void  ComputeBoundingBox();

   // Data member
private:
   SPoint m_ptStart, m_ptEnd; // line from m_ptStart  to  m_ptEnd
   int    m_nLineWidth;       // 1 .. 9,  default 1
   // Format: 0x00RRGGBB
   unsigned long int	m_ulLineColor;  // Defualt 0x000000  (black)
   bool   m_bArrow;   // if true, arrow from m_ptStart  to  m_ptEnd, Default  false
};


#endif
