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

#include "djvu_base_res.h"

#include "qd_set_zoom.h"

#include <qlabel.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qkeycode.h>



class QDSetZoomVal : public QValidator
{
public:
   virtual void		fixup(QString &);
#ifdef QT1
   virtual State	validate(QString &, int &);
#else
   virtual State	validate(QString &, int &) const;
#endif
   
   QDSetZoomVal(QSpinBox * parent, const char * name=0) :
	 QValidator(parent, name) {};
};

void
QDSetZoomVal::fixup(QString & str)
{
#ifdef QT1
   str.detach();
#else
   str.truncate(0);
#endif

   QSpinBox * spin=(QSpinBox *) parent();
   spin->setValue(spin->value());
   str=spin->text();
}

QValidator::State
QDSetZoomVal::validate(QString & input, int & pos)
#ifndef QT1
const
#endif
{
   if (!input.length()) return Valid;
   
   bool status;
   int zoom=input.toInt(&status);
   if (!status) return Invalid;

   if (zoom>IDC_ZOOM_MAX-IDC_ZOOM_MIN) return Invalid;
   if (zoom<=0) return Invalid;
   if (zoom<5)
     {
       if (input.length()==1)
	 return Valid;
       else
	 return Invalid;
     }

   return Acceptable;
}

QDSetZoom::QDSetZoom(int zoom, QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   setCaption(tr("DjVu: Set Zoom"));

   QWidget * start=startWidget();

      // Create the menu
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);
   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   QLabel * label=new QLabel(tr("Custom zoom"), start, "zoom_label");
   hlay->addWidget(label);
   spin=new QSpinBox(5, IDC_ZOOM_MAX-IDC_ZOOM_MIN, 10, start, "goto_menu");
   spin->setValidator(new QDSetZoomVal(spin));
   spin->setValue(zoom);
   hlay->addWidget(spin);
   label=new QLabel(" %", start, "% label");
   hlay->addWidget(label);

      // Create the buttons
   hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   hlay->addStretch(1);
   QPushButton * ok_butt=new QPushButton(tr("&OK"), start, "ok_butt");
   ok_butt->setDefault(TRUE);
   hlay->addWidget(ok_butt);
   QPushButton * cancel_butt=new QPushButton(tr("&Cancel"), start, "cancel_butt");
   hlay->addWidget(cancel_butt);

      // Connect signals
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));

   spin->setFocus();
}
