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

#include "prefs.h"
#include "mime_check.h"
#include "mime_utils.h"
#include "GString.h"
#include "ByteStream.h"
#include "qd_mime_dialog.h"
#include "GURL.h"

#include <stdlib.h>
#include <unistd.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include "debug.h"

class QMimeChecker : public QObject
{
   Q_OBJECT
public slots:
   void	slotCheckMimeTypes(void);
};

void
QMimeChecker::slotCheckMimeTypes(void)
{
   DEBUG_MSG("QMimeChecker::slotCheckMimeTypes\n");
   DjVuPrefs prefs;
   if (prefs.mimeDontCheck) return;
   
   G_TRY {
      GUTF8String ptr=getenv("HOME");
      if (ptr.length())
      {
	 GUTF8String name_in=ptr+"/.mime.types";
	 GUTF8String name_out=name_in+".new";
	 bool fixed=fixMimeTypes(name_in, name_out);

	 if (fixed)
	 {
	    bool do_it=false;
	    if (prefs.mimeDontAsk) do_it=true;
	    else
	    {
	       QDMimeDialog d(QStringFromGString(name_in), 0, "mime_check_dialog", TRUE);
	       if (d.exec()==QDialog::Accepted)
	       {
		  do_it=true;
		  QMessageBox::information(0, "DjVu",
					   tr("Please restart Netscape for the changes to take effect"));
	       }
	       prefs.mimeDontAsk=d.dontAsk();
	       prefs.mimeDontCheck=d.dontCheck();
	       prefs.save();
	    }
	    if (do_it)
	    {
	       unlink(name_in);
	       GP<ByteStream> str_in=ByteStream::create(GURL::Filename::UTF8(name_out),"r");
	       GP<ByteStream> str_out=ByteStream::create(GURL::Filename::UTF8(name_in),"w");
	       str_out->copy(*str_in);
	       unlink(name_out);
	    } else unlink(name_out);
	 }
      }
   } G_CATCH(exc) {} G_ENDCATCH;
}

void
checkMimeTypes(void)
{
   DEBUG_MSG("checkMimeTypes\n");
   static QTimer * timer;
   static QMimeChecker * checker;

   DjVuPrefs prefs;
   if (prefs.mimeDontCheck) return;
   
   if (!timer) timer=new QTimer();
   if (!checker) checker=new QMimeChecker();
   QObject::connect(timer, SIGNAL(timeout(void)), checker, SLOT(slotCheckMimeTypes(void)));

   timer->start(5000, TRUE);
}

void
fixMimeTypes(void)
{
   DEBUG_MSG("fixMimeTypes\n");
   GUTF8String ptr=getenv("HOME");
   if (ptr.length())
   {
      GUTF8String name_in=ptr+"/.mime.types";
      GUTF8String name_out=name_in+".new";
      bool fixed=fixMimeTypes(name_in, name_out);

      if (fixed)
      {
	 unlink(name_in);
	 GP<ByteStream> str_in=ByteStream::create(GURL::Filename::UTF8(name_out),"r");
	 GP<ByteStream> str_out=ByteStream::create(GURL::Filename::UTF8(name_in),"w");
	 str_out->copy(*str_in);
      }
      unlink(name_out);
   }
}

#include "mime_check_moc.inc"
