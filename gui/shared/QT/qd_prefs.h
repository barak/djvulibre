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
// $Id: qd_prefs.h,v 1.10 2007/03/25 20:48:26 leonb Exp $
// $Name: release_3_5_22 $

#ifndef HDR_QD_PREFS
#define HDR_QD_PREFS
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include <qframe.h>
#include <qgroupbox.h>

#include "prefs.h"


// These are components, which will be used by QDViewerPrefs or
// QDEditorPrefs in their "Preferences" dialogs. It's up to these two
// classes to position the following boxes inside the dialog. Each of
// the boxes is self contained

// Gamma correction preview
class QDGammaDispl : public QFrame
{
   Q_OBJECT
private:
   double gamma;
protected:
  virtual void paintEvent(QPaintEvent *);
public:
  virtual void resize(int, int);
  virtual void setGeometry(int, int, int, int);
public slots:
  void setGamma(int _gamma);
public:
  QDGammaDispl(QWidget * parent=0, const char * name=0);
  ~QDGammaDispl(void) {}
};

// "Gamma Correction: box
class QDGammaPrefs : public QWidget
{
   Q_OBJECT
private:
   class QCheckBox	* match_butt;
   class QSlider	* displ_slider, * print_slider;
   class QDGammaDispl	* preview;
private slots:
   void		matchToggled(bool state);
public:
   double	displGamma(void) const;
   double	printGamma(void) const;
   bool		match(void) const;

   QDGammaPrefs(DjVuPrefs * prefs, QWidget * parent=0, const char * name=0);
   ~QDGammaPrefs(void) {}
};

// "Magnifying Glass" box
class QDLensPrefs : public QWidget
{
   Q_OBJECT
private:
   class QComboBox	* size_menu, * scale_menu, * hkey_menu;
private slots:
   void		slotHotKeyChanged(const QString & text);
signals:
   void		sigHotKeyChanged(DjVuPrefs::MagButtType key);
public slots:
   void		slotHlHotKeyChanged(DjVuPrefs::HLButtType key);
public:
   DjVuPrefs::MagButtType hotKey(void) const;
   int		size(void) const;
   int		scale(void) const;

   QDLensPrefs(DjVuPrefs * prefs, QWidget * parent=0, const char * name=0);
   ~QDLensPrefs(void) {}
};

// "Optimization" box
class QDOptimPrefs : public QWidget
{
   Q_OBJECT
private:
   class QCheckBox	* fastzoom_butt, * lcd_butt, * fastthumb_butt;
public:
   bool		fastZoom(void) const;
   bool		fastThumb(void) const;
   bool		optimizeLCD(void) const;

   QDOptimPrefs(DjVuPrefs * prefs, QWidget * parent=0, const char * name=0);
   ~QDOptimPrefs(void) {}
};

// "Cache Preferences" box
class QDCachePrefs : public QWidget
{
   Q_OBJECT
private:
   class QComboBox		* mcache_menu;
   class QLabel		* pcache_label;
   class QComboBox		* pcache_menu;
   class QPushButton		* pcache_butt;
private slots:
   void		slotClearCache(void);
public:
   int		mcacheSize(void) const;
   int		pcacheSize(void) const;
   void		enablePCache(bool en);
   void		resetPCache(void);
   
   QDCachePrefs(DjVuPrefs * prefs, bool pcache_on,
		QWidget * parent=0, const char * name=0);
   ~QDCachePrefs(void) {}
};

// "Toolbar Preferences Box"
class QDTbarPrefs : public QWidget
{
   Q_OBJECT
private:
   class QCheckBox		* on_butt;
   class QCheckBox		* vis_butt;
   class QLabel		* delay_label;
   class QSpinBox		* delay_spin;
private slots:
   void		slotTBToggled(bool);
public:
   bool		enabled(void) const;
   bool		visible(void) const;
   int		delay(void) const;
   
   QDTbarPrefs(DjVuPrefs * prefs, QWidget * parent=0, const char * name=0);
   ~QDTbarPrefs(void) {}
};

#endif
