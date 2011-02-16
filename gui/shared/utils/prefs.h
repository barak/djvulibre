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

#ifndef HDR_PREFS
#define HDR_PREFS
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "GString.h"

class DjVuPrefs
{
private:
   void 	*database;

   GUTF8String	getString(const char * name);
   void		setString(const char * name, const char * value);
   int		getInt(const char * name);
   void		setInt(const char * name, int value);
public:
   enum HLButtType	{ HLB_SHIFT=0, HLB_SHIFT_SHIFT=1, HLB_ALT=2,
			  HLB_ALT_ALT=3, HLB_CTRL=4, HLB_CTRL_CTRL=5,
			  HLB_ITEMS=6 };
   enum MagButtType	{ MAG_SHIFT=0, MAG_ALT=1, MAG_CTRL=2,
			  MAG_MID=3, MAG_ITEMS=4 };

   static const int legal_mag_size[];
   static const int legal_mag_scale[];
   static const int legal_mag_size_num;
   static const int legal_mag_scale_num;
   static const char *hlb_names[];
   static const char *mag_names[];

   char		bBeginner;

   int		nDefaultZoom;
   double	dScreenGamma;
   double	dPrinterGamma;
   bool		printColor;
   bool 	printAutoOrient;
   bool		printPortrait;
   bool		printToFile;
   bool		printPS;
   int		printLevel;
   int          printZoom;
   bool		printFitPage;
   bool		printAllPages;
   GUTF8String	printCommand;
   GUTF8String	printFile;
   bool         printFrame;
   bool         printCropMarks;
   int          bookletMode;
   int          bookletMax;
   int          bookletAlign;
   int          bookletFold;
   int          bookletXFold;
   bool		hlinksPopup;
   bool		hlinksBorder;
   HLButtType	hlb_num;
   int		pcacheSize;
   int		mcacheSize;
   bool		toolBarOn;
   int		toolBarDelay;
   bool		toolBarAlwaysVisible;
   bool		fastZoom;
   bool		optimizeLCD;
   int		magnifierSize;
   int		magnifierScale;
   MagButtType	magnifierHotKey;
   bool		fastThumb;
   bool		mimeDontAsk;
   bool		mimeDontCheck;
  
   void		save(void);
   void		load(void);

   DjVuPrefs(void);
   virtual ~DjVuPrefs(void);
};

#endif
