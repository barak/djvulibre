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
// $Id: qd_print_dialog.h,v 1.16 2007/03/25 20:48:26 leonb Exp $
// $Name: release_3_5_22 $

#ifndef HDR_QD_PRINT_DIALOG
#define HDR_QD_PRINT_DIALOG
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "DjVuImage.h"
#include "DjVuDocument.h"
#include "DjVuToPS.h"
#include "GRect.h"

#include "qlib.h"

#include <qdialog.h>
#include <qstring.h>
#include <qwidgetstack.h>
#include <sys/types.h>

#include "prefs.h"


class QDPrintDialog : public QeDialog
{
  Q_OBJECT
public:
  enum What { PRINT_PAGE, PRINT_DOC, PRINT_CUSTOM, PRINT_WIN };
private:
  class QRadioButton	*ps_butt, *eps_butt;
  class QRadioButton	*autoorient_butt,*portrait_butt,*landscape_butt;
  class QRadioButton	*color_butt, *grey_butt;
  class QRadioButton	*level1_butt, *level2_butt, *level3_butt;
  class QComboBox	*what_menu;
  class QSpinBox	*copies_spin;
  class QLabel          *custompages_label;
  class QLineEdit	*custompages_text;
  class QCheckBox	*save_butt;
  class QButtonGroup	*format_bg, *orient_bg, *color_bg;
  class QButtonGroup	*scale_bg, *what_bg;
  class QComboBox	*zoom_menu;
  class QSpinBox	*zoom_spin;
  class QWidgetStack	*dst_widget;
  class QWidget		*printer_widget, *file_widget;
  class QRadioButton	*printer_butt, *file_butt;
  class QLineEdit	*printer_text, *file_text;
  class QWidgetStack	*prog_widget;
  class QProgressBar	*progress;
  class QPushButton	*cancel_butt;
  class QCheckBox    	*rectmark_chk, *cropmark_chk;
  class QCheckBox	*bk_mode_butt;
  class QComboBox       *bk_mode_menu;
  class QSpinBox 	*bk_max_spin;
  class QWidget         *bk_normal, *bk_advanced;
  class QSpinBox        *bk_align_spin,*bk_fold_spin,*bk_xfold_spin;

  GP<DjVuDocument>	doc;
  GP<DjVuImage>	        dimg;
  
  int			cur_page_num;
  bool			printing, interrupt_printing;
  double      		progress_low, progress_high;
  DjVuPrefs		*prefs;
  int			displ_mode;
  int			cur_zoom;
  
  GRect			print_rect;
  
  static const QString	id2str(int id);
  static int		str2id(const QString &);
  
  static void		refresh_cb(void *cl_data);
  static void		progress_cb(double done, void *cl_data);
  static void		info_cb(int page_num, int page_cnt, int tot_pages,
				DjVuToPS::Stage stage,void *cl_data);
  
  void			adjustWhat(void);
  void			setSensitivity(void);
  void               	setAlmostDisabled(bool disabled);
 private slots:
  void			slotBrowse(void);
  void			slotFormatChanged(void);
  void			slotDstChanged(void);
  void			slotWhatChanged(const QString & text);
  void			slotZoomChanged(const QString & text);
  void			slotBookChanged(void);
  void                  slotHelp(void);
 protected slots:
  virtual void		done(int);
 signals:
  void			sigDone(void);
public:
  void			setPSFormat(bool ps);
  void			setPortrait(bool portrait, bool autoorient);
  void			setColorMode(bool color);
  void			setPSLevel(int level);
  void			setZoom(int zoom);
  void			setCurZoom(int zoom);
  void			setPrint(What what);
  void			setFileName(const QString &name);
  void			setCommand(const QString &cmd);
  void			setBookletMode(int mode, int sign=0);
  void                  setBookletParm(int align=0, int fold=18, int xfold=200);
  void			printToFile(int file);
  
  QDPrintDialog(const GP<DjVuDocument> & doc, const GP<DjVuImage> & cur_dimg,
                DjVuPrefs *prefs, int displ_mode, int cur_zoom,
                const GRect & prn_rect, QWidget *parent=0,
                const char *name=0, bool modal=FALSE);
  ~QDPrintDialog(void) {}
};

#endif
