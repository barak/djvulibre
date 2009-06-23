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
// $Id: prefs.cpp,v 1.13 2008/03/16 14:07:06 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "prefs.h"
#include "debug.h"
#include "djvu_base_res.h"

#include <stdio.h>
#include <sys/param.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>

#define DATABASE_FILE_NAME ".djvurc"

const int DjVuPrefs::legal_mag_size[]
   = { 50, 75, 100, 125, 150, 175, 200, 225, 250, 275, 300 };

const int DjVuPrefs::legal_mag_scale[] 
   = { 15, 20, 25, 30, 35, 40, 45, 50 };

const int DjVuPrefs::legal_mag_size_num 
   = sizeof(DjVuPrefs::legal_mag_size)/sizeof(DjVuPrefs::legal_mag_size[0]);

const int DjVuPrefs::legal_mag_scale_num
   = sizeof(DjVuPrefs::legal_mag_scale)/sizeof(DjVuPrefs::legal_mag_scale[0]);

const char *DjVuPrefs::hlb_names[]=
   { "Shift", "Shift + Shift", "Alt", "Alt + Alt", "Ctrl", "Ctrl + Ctrl" };

const char *DjVuPrefs::mag_names[]=
   { "Shift", "Alt", "Ctrl", "Mid" };

GUTF8String
DjVuPrefs::getString(const char * name)
{
  DEBUG_MSG("getString(): Trying to get value for '" << name << "'\n");
  DEBUG_MAKE_INDENT(3);
  
  char * type;
  XrmValue val;
  if (!XrmGetResource((XrmDatabase) database, name, name, &type, &val))
  {
     DEBUG_MSG("ERROR: Failed to find this entry in the database\n");
     return "nothing_found";
  }
  if (strcmp(type, "String"))
  {
     DEBUG_MSG("ERROR: Unknown type '" << type << "' read\n");
     return "nothing_found";
  }
  DEBUG_MSG("read type " << type << ", value='" << (char *) val.addr << "'\n");
  return GUTF8String((char *) val.addr);
}

void
DjVuPrefs::setString(const char * name, const char * value)
{
   DEBUG_MSG("setString(): Updating database: " << name << '=' << value << "\n");
   DEBUG_MAKE_INDENT(3);
   XrmDatabase db = (XrmDatabase)database;
   XrmPutStringResource(&db, name, value);
   database = (void*)db;
}

int
DjVuPrefs::getInt(const char * name)
{
  return atoi(getString(name));
}

void
DjVuPrefs::setInt(const char * name, int value)
{
   char buffer[128];
   sprintf(buffer, "%d", value);
   setString(name, buffer);
}

void
DjVuPrefs::load(void)
{
  DEBUG_MSG("DjVuPrefs::load(): loading preferences\n");
  DEBUG_MAKE_INDENT(3);
  int i;
  static bool xrm_initialized=false;
  if (!xrm_initialized) 
    { 
      XrmInitialize(); 
      xrm_initialized=true; 
    }
  if (database) 
    {
      XrmDestroyDatabase((XrmDatabase) database); 
      database=0;
    }
  DEBUG_MSG("creating default database from string\n");
  char buffer[4096];
  sprintf(buffer, "Zoom:		%d\n"
	  	  "MagnifierSize:	100\n"
		  "MagnifierScale:	30\n"
		  "MagnifierHotKey:	%s\n"
		  "ScreenGamma:		2.2\n"
		  "PrinterGamma:	0.0\n"
		  "Beginner:		dummy\n"
		  "PrintColor:		1\n"
		  "PrintAutoOrient:	1\n"
		  "PrintPortrait:	1\n"
	  	  "PrintToFile:		0\n"
		  "PrintPS:		1\n"
	  	  "PrintLevel:		2\n"
	  	  "PrintZoom:		100\n"
	  	  "PrintFitPage:	1\n"
	  	  "PrintAllPages:	0\n"
		  "PrintCommand:	lp\n"
		  "PrintFile:		image.ps\n"
                  "PrintFrame:          0\n"
                  "PrintCropMarks:      0\n"
                  "BookletMode:         0\n"
                  "BookletMax:          0\n"
                  "BookletAlign:        0\n"
                  "BookletFold:         18\n"
                  "BookletXFold:        200\n"
		  "HLinksPopup:		1\n"
		  "HLinksBorder:	0\n"
		  "HLinksButt:		%s\n"
		  "PCacheSize:		10\n"
		  "MCacheSize:		0\n"
		  "ToolBarOn:		1\n"
		  "ToolBarDelay:	500\n"
	  	  "ToolBarAlwaysVisible: 0\n"
	  	  "FastZoom:		0\n"
		  "OptimizeLCD:		0\n"
	  	  "FastThumb:		1\n"
	  	  "MimeDontAsk:		0\n"
		  "MimeDontCheck:	0\n",
	  IDC_ZOOM_WIDTH-IDC_ZOOM_MIN,
	  mag_names[MAG_CTRL], 
          hlb_names[HLB_SHIFT]);

  database = (void*) XrmGetStringDatabase(buffer);
  const char * home=getenv("HOME"); if (!home) home="";
  GUTF8String db_name=GUTF8String(home)+"/"+DATABASE_FILE_NAME;
  DEBUG_MSG("merging database '" << db_name << "'\n");
  XrmDatabase db = (XrmDatabase)database;
  XrmCombineFileDatabase(db_name, &db, True);
  database = (void*)db;
  
  DEBUG_MSG("now initializing variables\n");
  GUTF8String strTemp;
  bBeginner=1;
  nDefaultZoom=getInt("Zoom")+IDC_ZOOM_MIN;
  magnifierSize=getInt("MagnifierSize");
  for(i=0;i<legal_mag_size_num;i++)
     if (magnifierSize<=legal_mag_size[i] ||
	 i==legal_mag_size_num)
	break;
  magnifierSize=legal_mag_size[i];
  magnifierScale=getInt("MagnifierScale");
  for(i=0;i<legal_mag_scale_num;i++)
     if (magnifierScale<=legal_mag_scale[i] ||
	 i==legal_mag_scale_num)
	break;
  magnifierScale=legal_mag_scale[i];
  strTemp=getString("MagnifierHotKey");
  magnifierHotKey=(MagButtType) 0;
  for(int i=0;i<MAG_ITEMS;i++)
    if (strTemp==mag_names[i]) {
      magnifierHotKey=(MagButtType) i;
      break;
    }
  strTemp=getString("ScreenGamma");
  dScreenGamma=atof((const char*)strTemp);
  if (dScreenGamma<0.1 || dScreenGamma>18) dScreenGamma=2.2;
  strTemp=getString("PrinterGamma");
  dPrinterGamma=atof((const char*)strTemp);
  if (dPrinterGamma<0.1 || dPrinterGamma>10.0) dPrinterGamma=0.0;
  printColor=getInt("PrintColor");
  printAutoOrient=getInt("PrintAutoOrient");
  printPortrait=getInt("PrintPortrait");
  printToFile=getInt("PrintToFile");
  printPS=getInt("PrintPS");
  printLevel=getInt("PrintLevel");
  if (printLevel<1 || printLevel>3) printLevel=2;
  printZoom=getInt("PrintZoom");
  if (printZoom<25) printZoom=100;
  printFitPage=getInt("PrintFitPage");
  printAllPages=getInt("PrintAllPages");
  printCommand=getString("PrintCommand");
  printFile=getString("PrintFile");
  printFrame=getInt("PrintFrame");
  printCropMarks=getInt("PrintCropMarks");
  bookletMode=getInt("BookletMode");
  bookletMax=getInt("BookletMax");
  bookletAlign=getInt("BookletAlign");
  bookletFold=getInt("BookletFold");
  bookletXFold=getInt("BookletXFold");
  hlinksPopup=getInt("HLinksPopup");
  hlinksBorder=getInt("HLinksBorder");
  GUTF8String hlb_tmp=getString("HLinksButt");
  hlb_num=(HLButtType) 0;
  for(int i=0;i<HLB_ITEMS;i++)
    if (hlb_tmp==hlb_names[i]) { 
      hlb_num=(HLButtType) i; 
      break; 
    }
  pcacheSize=getInt("PCacheSize");
  mcacheSize=getInt("MCacheSize");
  toolBarOn=getInt("ToolBarOn");
  toolBarDelay=getInt("ToolBarDelay");
  toolBarAlwaysVisible=getInt("ToolBarAlwaysVisible");
  fastZoom=getInt("FastZoom");
  optimizeLCD=getInt("OptimizeLCD");
  fastThumb=getInt("FastThumb");
  mimeDontAsk=getInt("MimeDontAsk");
  mimeDontCheck=getInt("MimeDontCheck");
  strTemp=getString("Beginner");
  if (strTemp==__DATE__ " " __TIME__)
    bBeginner=0;
  if (bBeginner)
    fastZoom=0;
}

void
DjVuPrefs::save()
{
  DEBUG_MSG("DjVuPrefs::save(): Saving preferences to disk\n");
  DEBUG_MAKE_INDENT(3);
  
  setInt("Zoom", nDefaultZoom-IDC_ZOOM_MIN);
  setInt("MagnifierSize", magnifierSize);
  setInt("MagnifierScale", magnifierScale);
  setString("MagnifierHotKey", mag_names[magnifierHotKey]);
  GUTF8String strTemp;
  strTemp.format("%1.2f", dScreenGamma);
  setString("ScreenGamma", strTemp);
  strTemp.format("%1.2f", dPrinterGamma);
  setString("PrinterGamma", strTemp);
  setInt("PrintColor", printColor);
  setInt("PrintAutoOrient", printAutoOrient);
  setInt("PrintPortrait", printPortrait);
  setInt("PrintToFile", printToFile);
  setInt("PrintPS", printPS);
  setInt("PrintLevel", printLevel);
  setInt("PrintZoom", printZoom);
  setInt("PrintFitPage", printFitPage);
  setInt("PrintAllPages", printAllPages);
  setString("PrintCommand", printCommand);
  setString("PrintFile", printFile);
  setInt("PrintFrame", printFrame);
  setInt("PrintCropMarks", printCropMarks);
  setInt("BookletMode", bookletMode);
  setInt("BookletMax", bookletMax);
  setInt("BookletAlign", bookletAlign);
  setInt("BookletFold", bookletFold);
  setInt("BookletXFold", bookletXFold);
  setInt("HLinksPopup", hlinksPopup);
  setInt("HLinksBorder", hlinksBorder);
  setString("HLinksButt", hlb_names[hlb_num]);
  setInt("PCacheSize", pcacheSize);
  setInt("MCacheSize", mcacheSize);
  setInt("ToolBarOn", toolBarOn);
  setInt("ToolBarDelay", toolBarDelay);
  setInt("ToolBarAlwaysVisible", toolBarAlwaysVisible);
  setInt("FastZoom", fastZoom);
  setInt("OptimizeLCD", optimizeLCD);
  setInt("FastThumb", fastThumb);
  setInt("MimeDontAsk", mimeDontAsk);
  setInt("MimeDontCheck", mimeDontCheck);
  if (bBeginner == 0)
    setString("Beginner", __DATE__ " " __TIME__ );
  const char * home=getenv("HOME"); if (!home) home="";
  GUTF8String db_name=GUTF8String(home)+"/"+DATABASE_FILE_NAME;
  DEBUG_MSG("updating database '" << db_name << "'\n");
  XrmPutFileDatabase((XrmDatabase) database, db_name);
}

DjVuPrefs::DjVuPrefs(void)
{
   database=0;
   load();
}

DjVuPrefs::~DjVuPrefs(void)
{
   if (database) XrmDestroyDatabase((XrmDatabase) database);
   database=0;
}
