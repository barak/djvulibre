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

#ifndef HDR_QD_TOOLBAR
#define HDR_QD_TOOLBAR
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "GContainer.h"

#include <qframe.h>

class QDToolBar : public QFrame
{
   Q_OBJECT
private:
   GList<QWidget *>		left_list, right_list;
   GList<class QDTBarPiece *>	pieces;

   bool		positionWidgets(int width, int rows, bool move=false, int * height_ptr=0);
   int		positionWidgets(void);
   void		addLeftWidgets(const GList<QWidget *> & list);
private slots:
   void		slotWidgetDestroyed(void);
protected:
   virtual bool	event(QEvent * ev);
public:
   bool		being_destroyed;
   
   int		computeHeight(int width);
   void		addLeftWidget(QWidget * widget);
   void		addLeftWidgets(QWidget * w1, QWidget * w2, QWidget * w3=0,
			       QWidget * w4=0, QWidget * w5=0, QWidget * w6=0);
   void		addRightWidget(QWidget * widget);
   void		deleteWidget(QWidget * widget);
   void		adjustPositions(void);
   void		addPiece(QDTBarPiece * piece);
   
   virtual void	setEnabled(bool en);
   
   QDToolBar(QWidget * parent=0, const char * name=0);
   ~QDToolBar(void) { being_destroyed=true; }
};

class QDTBarPiece : public QObject
{
   Q_OBJECT
protected:
   QWidget	* toolbar;
public:
   virtual void	setEnabled(bool on)=0;

   QDTBarPiece(QWidget * _toolbar) :
	 QObject(_toolbar), toolbar(_toolbar) {}
};

#endif
