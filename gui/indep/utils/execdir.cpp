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

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <qfileinfo.h>
#include <qdir.h>

#include "execdir.h"

QString
getExecDir(QString argv0)
{
   QString dir;

   const char *progname=argv0;

   if (progname && progname[0])
   {
      // find out if the progname exists in the path 
      if (progname[0]!='.' && progname[0]!='/')
      {
	 char * path=getenv("PATH");
	 if (path)
	 {
	    char * start=path;
	    char * end;
	    do
	    {
	       for(end=start;*end && *end!=':';end++);
	       if (end>start)
	       {
		  // +1 for '\0' required by QCString
		  QString pdir=QCString(start, end-start+1);
		  QFileInfo fi=QFileInfo(QDir(pdir), progname);
		  if (fi.isFile())
		     return dir=fi.dirPath();
	       }
	       start=end+1;
	    } while(*end);
	 }
      }

      // it may be given as absolute/relative path 
      QFileInfo fi=QFileInfo(progname);
      if (fi.isFile())
	 dir=fi.dirPath();
      
   }
   return dir;
}

// end execdir.cpp
