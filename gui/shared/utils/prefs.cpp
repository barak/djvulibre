//C-  -*- C++ -*-
//C-
//C- DjVu® Unix Viewer (v. 3.5)
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
//C-
// 
// $Id$
// $Name$

#ifdef HAVE_CONFIG_H
#include "config.h"
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

char *DjVuPrefs::hlb_names[]=
   { "Shift", "Shift + Shift", "Alt", "Alt + Alt", "Ctrl", "Ctrl + Ctrl" };

char *DjVuPrefs::mag_names[]=
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
   
   XrmPutStringResource((XrmDatabase *) &database, name, value);
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
  if (!xrm_initialized) { XrmInitialize(); xrm_initialized=true; }
  
  if (database) XrmDestroyDatabase((XrmDatabase) database); database=0;
  
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
		  "PrintPortrait:	1\n"
	  	  "PrintToFile:		0\n"
		  "PrintPS:		1\n"
	  	  "PrintLevel:		2\n"
	  	  "PrintFitPage:	1\n"
	  	  "PrintAllPages:	0\n"
		  "PrintCommand:	lp\n"
		  "PrintFile:		image.ps\n"
		  "HLinksPopup:		1\n"
		  "HLinksBorder:	0\n"
		  "HLinksButt:		%s\n"
		  "PCacheSize:		10\n"
		  "MCacheSize:		0\n"
		  "ToolBarOn:		1\n"
		  "ToolBarDelay:	500\n"
	  	  "ToolBarAlwaysVisible: 0\n"
	  	  "FastZoom:		1\n"
		  "OptimizeLCD:		0\n"
	  	  "FastThumb:		1\n"
	  	  "MimeDontAsk:		0\n"
		  "MimeDontCheck:	0\n",
	  IDC_ZOOM_WIDTH-IDC_ZOOM_MIN,
	  mag_names[MAG_CTRL], hlb_names[HLB_SHIFT]);
  database=XrmGetStringDatabase(buffer);
  
  char * home=getenv("HOME"); if (!home) home="";
  GUTF8String db_name=GUTF8String(home)+"/"+DATABASE_FILE_NAME;
  DEBUG_MSG("merging database '" << db_name << "'\n");
  XrmCombineFileDatabase(db_name, (XrmDatabase *) &database, True);
  
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
     if (strTemp==mag_names[i])
     {
	magnifierHotKey=(MagButtType) i; break;
     }

  strTemp=getString("ScreenGamma");
  dScreenGamma=atof((const char*)strTemp);
  if (dScreenGamma<0.1 || dScreenGamma>18) dScreenGamma=2.2;
  strTemp=getString("PrinterGamma");
  dPrinterGamma=atof((const char*)strTemp);
  if (dPrinterGamma<0.1 || dPrinterGamma>10.0) dPrinterGamma=0.0;
  printColor=getInt("PrintColor");
  printPortrait=getInt("PrintPortrait");
  printToFile=getInt("PrintToFile");
  printPS=getInt("PrintPS");
  printLevel=getInt("PrintLevel");
  if (printLevel<1 || printLevel>3) printLevel=2;
  printFitPage=getInt("PrintFitPage");
  printAllPages=getInt("PrintAllPages");
  printCommand=getString("PrintCommand");
  printFile=getString("PrintFile");
  hlinksPopup=getInt("HLinksPopup");
  hlinksBorder=getInt("HLinksBorder");
  GUTF8String hlb_tmp=getString("HLinksButt");
  hlb_num=(HLButtType) 0;
  for(int i=0;i<HLB_ITEMS;i++)
     if (hlb_tmp==hlb_names[i]) { hlb_num=(HLButtType) i; break; }
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
  setInt("PrintPortrait", printPortrait);
  setInt("PrintToFile", printToFile);
  setInt("PrintPS", printPS);
  setInt("PrintLevel", printLevel);
  setInt("PrintFitPage", printFitPage);
  setInt("PrintAllPages", printAllPages);
  setString("PrintCommand", printCommand);
  setString("PrintFile", printFile);
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
  char * home=getenv("HOME"); if (!home) home="";
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
