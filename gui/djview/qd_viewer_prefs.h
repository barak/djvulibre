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
// $Id: qd_viewer_prefs.h,v 1.11 2007/03/25 20:48:23 leonb Exp $
// $Name: release_3_5_22 $

#ifndef HDR_QD_VIEWER_PREFS
#define HDR_QD_VIEWER_PREFS
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "qlib.h"
#include "qd_prefs.h"

#include <qdialog.h>


// "Hyperlink Preferences box"
class QDHlinkPrefs : public QWidget
{
   Q_OBJECT
private:
   QCheckBox		* popup_butt;
   QCheckBox		* border_butt;
   QComboBox		* key_menu;
private slots:
   void		slotHotKeyChanged(int);
signals:
   void		sigHotKeyChanged(DjVuPrefs::HLButtType key);
public slots:
   void		slotMagHotKeyChanged(DjVuPrefs::MagButtType key);
public:
   bool		disablePopup(void) const;
   bool		simpleBorder(void) const;
   DjVuPrefs::HLButtType	hlinkKey(void) const;
   
   QDHlinkPrefs(DjVuPrefs * prefs, QWidget * parent=0, const char * name=0);
   ~QDHlinkPrefs(void) {}
};

// The "Viewer Preferences" dialog
class QDViewerPrefs : public QeDialog
{
   Q_OBJECT
private:
   QDGammaPrefs		* gamma_prefs;
   QDLensPrefs		* lens_prefs;
   QDOptimPrefs		* optim_prefs;
   QDHlinkPrefs		* hlink_prefs;
   QDCachePrefs		* cache_prefs;
   QDTbarPrefs		* tbar_prefs;
   class QCheckBox	* global_butt;
   bool	          	  cache_disabled;
   DjVuPrefs		* prefs;
protected slots:
   void slotGlobalToggled(bool);
   void slotHelp(void);
   virtual void done(int);
public:
   void	disableCache(void);
   QDViewerPrefs(DjVuPrefs * prefs, QWidget * parent=0,
		 const char * name=0, bool modal=FALSE);
   ~QDViewerPrefs(void) {};
};

#endif
