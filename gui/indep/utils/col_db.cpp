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
// $Id: col_db.cpp,v 1.8 2008/03/17 12:18:34 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "col_db.h"
#include "GException.h"

ColorDB::ColorItem	ColorDB::color[]=
{
   { "black",		0,   0,   0 },
   { "blue",		0,   0,   255 },
   { "turquoise",	64,  224, 208 },
   { "green", 		0,   255, 0 },
   { "pink",		255, 192, 203 },
   { "magenta",		255, 0,   255 },
   { "red",		255, 0,   0 },
   { "brown",		165, 42,  42 },
   { "yellow",		255, 255, 0 },
   { "white",		255, 255, 255 },
   { "dark blue",	0,   0,   139 },
   { "violet",		238, 130, 238 },
   { "dark red",	139, 0,   0 },
   { "dark yellow",	139, 139, 0 },
   { "dark gray",	128, 128, 128 },
   { "gray",		211, 211, 211 }
};

const char ** ColorDB::name;
unsigned char * ColorDB::red;
unsigned char * ColorDB::green;
unsigned char * ColorDB::blue;
GMap<u_int32, int>	* ColorDB::c32_to_num;
GMap<GUTF8String, int>	* ColorDB::string_to_num;
   
int ColorDB::colors=sizeof(ColorDB::color)/sizeof(ColorDB::color[0]);

static ColorDB color_db;

void ColorDB::InitializeMaps(void)
{
   if (!c32_to_num) c32_to_num=new GMap<u_int32, int>();
   if (!string_to_num) string_to_num=new GMap<GUTF8String, int>();
}

ColorDB::ColorDB(void)
{
   red=new unsigned char[colors];
   green=new unsigned char[colors];
   blue=new unsigned char[colors];
   name=new const char*[colors];
   
   if (!red || !green || !blue || !name)
      G_THROW("ColorDB::ColorDB(): Not enough memory to initialize color database.");
   
   InitializeMaps();
   
   for(int i=0;i<colors;i++)
   {
      red[i]=color[i].red;
      green[i]=color[i].green;
      blue[i]=color[i].blue;
      name[i]=color[i].name;
      
      u_int32 color32=
	 ((u_int32) red[i] << 16) |
	 ((u_int32) green[i] << 8) |
	 (u_int32) blue[i];
      (*c32_to_num)[color32]=i;
      (*string_to_num)[name[i]]=i;
   };
}
