//C-  -*- C++ -*-
//C- -----------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- DjVuLibre-3.5 is derived from the DjVu (r) Reference Library.
//C- 
//C- DjVu (r) Reference Library (v. 3.5)
//C- Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
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
//C- -----------------------------------------------------------
// 
// $Id$
// $Name$

#ifndef HDR_COL_DB
#define HDR_COL_DB
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "int_types.h"
#include "GContainer.h"
#include "GString.h"

class ColorDB
{
private:
   static GMap<u_int32, int>	* c32_to_num;
   static GMap<GUTF8String, int>	* string_to_num;
   
   static void	InitializeMaps(void);
public:
   class ColorItem
   {
   public:
      char		* name;
      unsigned char	red, green, blue;
   };
   static ColorItem	color[];
   static int		colors;
   static char 		** name;
   static unsigned char	* red, * green, * blue;
   
   static int	GetColorNum(u_int32 color);
   static int	GetColorNum(const char * color);
   static int	C32_to_Num(u_int32 color);
   static u_int32	Num_to_C32(int color_num);
   static const char *	Num_to_Name(int color_num);
   
   
   static unsigned char	C32_GetRed(u_int32 color);
   static unsigned char	C32_GetGreen(u_int32 color);
   static unsigned char	C32_GetBlue(u_int32 color);

   static u_int32	RGB_to_C32(unsigned char r,
				   unsigned char g,
				   unsigned char b);
   
   ColorDB(void);
};

inline int ColorDB::GetColorNum(u_int32 color)
{
   if (!c32_to_num) InitializeMaps();
   return c32_to_num->contains(color) ? (*c32_to_num)[color] : -1;
}

inline int ColorDB::GetColorNum(const char * color)
{
   if (!string_to_num) InitializeMaps();
   return string_to_num->contains(color) ? (*string_to_num)[color] : -1;
}

inline int ColorDB::C32_to_Num(u_int32 color)
{
   if (!c32_to_num) InitializeMaps();
   return c32_to_num->contains(color) ? (*c32_to_num)[color] : -1;
}

inline u_int32 ColorDB::Num_to_C32(int color_num)
{
   if (color_num<0 || color_num>=colors) color_num=0;
   return
      ((u_int32) red[color_num] << 16) |
      ((u_int32) green[color_num] << 8) |
      (u_int32) blue[color_num];
}

inline const char * ColorDB::Num_to_Name(int color_num)
{
   if (color_num<0 || color_num>=colors) color_num=0;
   return name[color_num];
}

inline unsigned char ColorDB::C32_GetRed(u_int32 color)
{
   return (color >> 16) & 0xff;
}

inline unsigned char ColorDB::C32_GetGreen(u_int32 color)
{
   return (color >> 8) & 0xff;
}

inline unsigned char ColorDB::C32_GetBlue(u_int32 color)
{
   return color & 0xff;
}

inline u_int32 ColorDB::RGB_to_C32(unsigned char r,
				   unsigned char g,
				   unsigned char b)
{
   return (r << 16) | (g << 8) | b;
}

#endif
