//C-  -*- C++ -*-
//C-
//C- DjVu® Unix Viewer (v. 3.5)
//C- 
//C- Copyright © 2000-2001 LizardTech, Inc. All Rights Reserved.
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

#include "qd_tbar_rotate_piece.h"
#include "debug.h"
#include "qlib.h"
#include "qd_toolbutt.h"
#include "djvu_base_res.h"
#include "cin_data.h"

//****************************************************************************
//***************************** QDTBarRotatePiece *****************************
//****************************************************************************

void
QDTBarRotatePiece::setEnabled(bool en)
{
   rotate90_butt->setEnabled(en);
   rotate270_butt->setEnabled(en);
}

QDTBarRotatePiece::QDTBarRotatePiece(QDToolBar * toolbar) : QDTBarPiece(toolbar)
{
   QFrame * frame;
   
   frame=new QFrame(toolbar, "separator");
   frame->setFrameStyle(QFrame::VLine | QFrame::Sunken);
   frame->setMinimumWidth(10);
   toolbar->addLeftWidget(frame);
   
   rotate90_butt=new QDToolButton(*CINData::get("ppm_rotate90"), true,
				  IDC_ROTATE_90, toolbar, tr("Rotate +90"));
   connect(rotate90_butt, SIGNAL(clicked(void)), this, SLOT(slotRotate()));
   toolbar->addLeftWidget(rotate90_butt);

   rotate270_butt=new QDToolButton(*CINData::get("ppm_rotate270"), true,
				   IDC_ROTATE_270, toolbar, tr("Rotate -90"));
   connect(rotate270_butt, SIGNAL(clicked(void)), this, SLOT(slotRotate()));
   toolbar->addLeftWidget(rotate270_butt);
   
   toolbar->addPiece(this);
   toolbar->adjustPositions();
}


void
QDTBarRotatePiece::slotRotate(void)
{
   const QObject * obj=sender();
   if (obj && obj->isWidgetType() && obj->inherits("QDToolButton"))
   {
      const QDToolButton * butt=(QDToolButton *) obj;
      emit sigRotate(butt->cmd);
   }
}

