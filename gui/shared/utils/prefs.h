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

#ifndef HDR_PREFS
#define HDR_PREFS
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "GString.h"

class DjVuPrefs
{
private:
   void		* database;

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
   static char	* hlb_names[];
   static char	* mag_names[];

   char		bBeginner;

   int		nDefaultZoom;
   double	dScreenGamma;
   double	dPrinterGamma;
   bool		printColor;
   bool		printPortrait;
   bool		printToFile;
   bool		printPS;
   int		printLevel;
   bool		printFitPage;
   bool		printAllPages;
   GUTF8String	printCommand;
   GUTF8String	printFile;
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
