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

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "DjVuInfo.h"
#include "GException.h"
#include "ByteStream.h"
#include "GString.h"

// ----------------------------------------
// CLASS DJVUINFO


#define STRINGIFY(x) STRINGIFY_(x)
#define STRINGIFY_(x) #x


DjVuInfo::DjVuInfo()
  : width(0), height(0), 
    version(DJVUVERSION),
    dpi(300), gamma(2.2), compressable(false), orientation(GRect::BULRNR)
{
}

void 
DjVuInfo::decode(ByteStream &bs)
{
  // Set to default values
  width = 0;
  height = 0;
  version = DJVUVERSION;
  dpi = 300;
  gamma = 2.2;
  compressable=false;
  orientation=GRect::BULRNR;
  // Read data
  unsigned char buffer[10];
  int  size = bs.readall((void*)buffer, sizeof(buffer));
  if (size == 0)
    G_THROW( ByteStream::EndOfFile );
  if (size < 5)
    G_THROW( ERR_MSG("DjVuInfo.corrupt_file") );
  // Analyze data with backward compatibility in mind!
  if (size>=2)
    width = (buffer[0]<<8) + buffer[1];
  if (size>=4)
    height = (buffer[2]<<8) + buffer[3];
  if (size>=5)
    version = buffer[4];
  if (size>=6 && buffer[5]!=0xff)
    version = (buffer[5]<<8) + buffer[4];
  if (size>=8 && buffer[7]!=0xff)
    dpi = (buffer[7]<<8) + buffer[6];
  if (size>=9)
    gamma = 0.1 * buffer[8];
  int flags=0;
  if (size>=10)
    flags = buffer[9];
  // Fixup
  if (gamma<0.3)
     gamma=0.3;
  if (gamma>5.0)
     gamma=5.0;
  if (dpi < 25 || dpi > 6000)
    dpi = 300;
  if(flags&COMPRESSABLE_FLAG)
    compressable=true;
  if(version>=DJVUVERSION_ORIENTATION)
  {
    orientation=(GRect::Orientations)(flags&((int)GRect::BOTTOM_UP|(int)GRect::MIRROR|(int)GRect::ROTATE90_CW));
  }
}

void 
DjVuInfo::encode(ByteStream &bs)
{
  bs.write16(width);
  bs.write16(height);
  bs.write8(version & 0xff);
  bs.write8(version >> 8);
  bs.write8(dpi & 0xff);
  bs.write8(dpi >> 8);
  bs.write8((int)(10.0*gamma+0.5) );
  unsigned char flags=orientation;
  if(compressable) 
  {
    flags|=COMPRESSABLE_FLAG;
  }
  bs.write8(flags);
}

unsigned int 
DjVuInfo::get_memory_usage() const
{
  return sizeof(DjVuInfo);
}

GUTF8String
DjVuInfo::get_paramtags(void) const
{
  const int angle=GRect::findangle(orientation);
  GUTF8String retval;
  if(angle)
  {
    retval+="<PARAM name=\"ROTATE\" value=\""+GUTF8String(angle)+"\" />\n";
  }
  if(orientation == GRect::rotate(angle,GRect::TDLRNR))
  {
    retval+="<PARAM name=\"VFLIP\" value=\"true\" />\n";
  }
  if(dpi)
  {
    retval+="<PARAM name=\"DPI\" value=\""+GUTF8String(dpi)+"\" />\n";
  }
  if(gamma)
  {
    retval+="<PARAM name=\"GAMMA\" value=\""+GUTF8String(gamma)+"\" />\n";
  }
  return retval;
}

void
DjVuInfo::writeParam(ByteStream &str_out) const
{
  str_out.writestring(get_paramtags());
}

