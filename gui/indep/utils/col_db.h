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

#ifndef HDR_COL_DB
#define HDR_COL_DB
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "GContainer.h"
#include "GString.h"

#include "int_types.h"


class ColorDB
{
private:
   static GMap<u_int32, int>	 *c32_to_num;
   static GMap<GUTF8String, int> *string_to_num;
   
   static void	InitializeMaps(void);
public:
   class ColorItem
   {
   public:
      const char	* name;
      unsigned char	red, green, blue;
   };
   static ColorItem	color[];
   static int		colors;
   static const char	** name;
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
