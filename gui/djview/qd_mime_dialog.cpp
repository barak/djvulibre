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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qd_mime_dialog.h"

#include "prefs.h"
#include "debug.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

bool
QDMimeDialog::dontAsk(void) const
{
   return dontask_butt->isChecked();
}

bool
QDMimeDialog::dontCheck(void) const
{
   return dontcheck_butt->isChecked();
}
      
QDMimeDialog::QDMimeDialog(QWidget * parent, const char * name, bool modal) 
  : QeDialog(parent, name, modal)
{
   setCaption(tr("DjVu"));
  
   QWidget * start=startWidget();
   
   QVBoxLayout * vlay=new QVBoxLayout(start, 15, 15);
   QFrame * frame=new QFrame(start, "frame");
   frame->setFrameStyle(QFrame::Box | QFrame::Sunken);
   vlay->addWidget(frame);

   QVBoxLayout * frame_vlay=new QVBoxLayout(frame, 20, 10);
   
   QString msg = tr("We have just found that your configuration files"
                    " \".mime.types\" and \".mailcap\" should be updated in"
                    " order for the DjVu plugin to work properly. The invalid"
                    " information can prevent Netscape from correctly displaying"
                    " DjVu documents and can make Netscape use incorrect MIME"
                    " type when sending DjVu documents via e-mail.\n\nWould "
                    " you like to update these files now?");

   QLabel * label=new QLabel(msg, frame);
   label->setAlignment(AlignLeft | WordBreak);
   frame_vlay->addWidget(label);

   QFrame * sep=new QFrame(frame, "sep");
   sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   sep->setMinimumHeight(sep->sizeHint().height());
   frame_vlay->addWidget(sep);
   
   again_butt=new QRadioButton(tr("Next time &check this again"), frame);
   frame_vlay->addWidget(again_butt);
   dontask_butt=new QRadioButton(tr("Next time do the update &silently"), frame);
   frame_vlay->addWidget(dontask_butt);
   dontcheck_butt=new QRadioButton(tr("&Never do this check again"), frame);
   frame_vlay->addWidget(dontcheck_butt);
   
   label->setMinimumWidth(again_butt->sizeHint().width()*2);
   
   QButtonGroup * bg=new QButtonGroup(frame);
   bg->hide();
   bg->insert(again_butt);
   bg->insert(dontask_butt);
   bg->insert(dontcheck_butt);
   
   DjVuPrefs prefs;
   dontask_butt->setChecked(prefs.mimeDontAsk);
   dontcheck_butt->setChecked(prefs.mimeDontCheck && !prefs.mimeDontAsk);
   again_butt->setChecked(!prefs.mimeDontCheck && !prefs.mimeDontAsk);
   
   QHBoxLayout * butt_lay=new QHBoxLayout(vlay,5);
   butt_lay->addStretch(1);
   QPushButton * yes_butt=new QPushButton(tr("&Yes"), start, "yes_butt");
   yes_butt->setDefault(TRUE);
   butt_lay->addWidget(yes_butt);
   QPushButton * no_butt=new QPushButton(tr("&No"), start, "no_butt");
   butt_lay->addWidget(no_butt);
   
   connect(yes_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(no_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
}

