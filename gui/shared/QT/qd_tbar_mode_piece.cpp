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

#include "qd_tbar_mode_piece.h"
#include "debug.h"
#include "qlib.h"
#include "qd_base.h"
#include "qd_toolbutt.h"
#include "djvu_base_res.h"
#include "cin_data.h"

#include <qcombobox.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <qcheckbox.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

//  #define ONE2ONE_STR	"1 : 1"
//  #define STRETCH_STR	"Stretch"
//  #define FIT_WIDTH_STR	"Fit Width"
//  #define FIT_PAGE_STR	"Fit Page"

class QDZoomValidator : public QValidator
{
public:
   virtual void		fixup(QString &);
#ifdef QT1
   virtual State	validate(QString &, int &);
#else
   virtual State	validate(QString &, int &) const;
#endif
   
   QDZoomValidator(QComboBox * parent, const char * name=0) :
	 QValidator(parent, name) {}
};

void
QDZoomValidator::fixup(QString & str)
{
#ifdef QT1
   str.detach();
#else
   str.truncate(0);
#endif

   QComboBox * menu=(QComboBox *) parent();
   menu->setEditText(str=menu->text(menu->currentItem()));
}

QValidator::State
QDZoomValidator::validate(QString & input, int & pos)
#ifndef QT1
const
#endif
{
   if (!input.length()) return Valid;

   char buffer[128];
   strncpy(buffer, input, 127); buffer[127]=0;
   char * ptr=buffer+strlen(buffer)-1;
   while(isspace(*ptr) || *ptr=='%') *ptr--=0;
   QString str(buffer);
   
   bool status;
   int zoom=str.toInt(&status);
   if (!status) return Invalid;

   if (zoom>IDC_ZOOM_MAX-IDC_ZOOM_MIN) return Invalid;
   if (zoom<=0) return Invalid;
   if (zoom<5)
     {
       if (str.length()==1)
	 return Valid;
       else
	 return Invalid;
     }

   if (zoom<5 || zoom>IDC_ZOOM_MAX-IDC_ZOOM_MIN) return Invalid;
   return Acceptable;
}

//****************************************************************************
//***************************** QDTBarModePiece ******************************
//****************************************************************************

void
QDTBarModePiece::setEnabled(bool en)
{
   zoom_menu->setEnabled(en);
   mode_menu->setEnabled(en);
   zoom_in_butt->setEnabled(en);
   zoom_out_butt->setEnabled(en);

   zoom_select_butt->setEnabled(en);
   text_select_butt->setEnabled(en);
   pane_butt->setEnabled(en);

   if ( pin_butt )
      pin_butt->setEnabled(en);
}

static const int menu_items_size=10;
static const struct MenuItems {
  const char *str;
  int zoom;
} menu_items[menu_items_size] = {
  {"300 %",IDC_ZOOM_MIN+300},
  {"150 %",IDC_ZOOM_MIN+150},
  {"100 %",IDC_ZOOM_MIN+100},
  {"75 %",IDC_ZOOM_MIN+75},
  {"50 %",IDC_ZOOM_MIN+50},
  {"25 %",IDC_ZOOM_MIN+25},
  {QT_TRANSLATE_NOOP("QDTBarModePiece","Fit Width"),IDC_ZOOM_WIDTH},
  {QT_TRANSLATE_NOOP("QDTBarModePiece","Fit Page"),IDC_ZOOM_PAGE},
  {"1 : 1",IDC_ZOOM_ONE2ONE},
  {QT_TRANSLATE_NOOP("QDTBarModePiece","Stretch"),IDC_ZOOM_STRETCH},
};

QDTBarModePiece::QDTBarModePiece(QWidget * toolbar) 
  : QDTBarPiece(toolbar)
{
   if ( dynamic_cast<QDToolBar *>(toolbar) )
      qdtoolbar_child=TRUE;
   else
      qdtoolbar_child=FALSE;
   
   mode_menu=new QComboBox(FALSE, toolbar, "mode_menu");
   mode_menu->insertItem(tr("Color"));
   mode_menu->insertItem(tr("B&W"));
   mode_menu->insertItem(tr("Fore"));
   mode_menu->insertItem(tr("Back"));
   connect(mode_menu, SIGNAL(activated(int)), this, SLOT(slotMode(int)));
   QToolTip::add(mode_menu, tr("Display mode"));

   if ( qdtoolbar_child ) 
      ((QDToolBar *)toolbar)->addLeftWidget(mode_menu);

   zoom_menu=new QComboBox(TRUE, toolbar, "zoom_menu");
   zoom_menu->setInsertionPolicy(QComboBox::NoInsertion);
   zoom_menu->setValidator(new QDZoomValidator(zoom_menu));
   int i;
   for(i=0;i<menu_items_size;i++)
   {
     zoom_menu->insertItem(tr(menu_items[i].str));
   }
   connect(zoom_menu, SIGNAL(activated(const QString &)),
	   this, SLOT(slotZoom(const QString &)));
   QToolTip::add(zoom_menu, tr("Zoom"));
   if ( qdtoolbar_child )
      ((QDToolBar *)toolbar)->addLeftWidget(zoom_menu);

   zoom_in_butt=new QDToolButton(*CINData::get("ppm_vzoom_in"), true,
				 IDC_ZOOM_ZOOMIN, toolbar, tr("Zoom In"));
   connect(zoom_in_butt, SIGNAL(clicked(void)), this, SLOT(slotZoom(void)));

   zoom_out_butt=new QDToolButton(*CINData::get("ppm_vzoom_out"), true,
				  IDC_ZOOM_ZOOMOUT, toolbar, tr("Zoom Out"));
   connect(zoom_out_butt, SIGNAL(clicked(void)), this, SLOT(slotZoom(void)));

   if ( qdtoolbar_child ) 
      ((QDToolBar *)toolbar)->addLeftWidgets(zoom_in_butt, zoom_out_butt);

   pane_butt=new QDToolButton(*CINData::get("ppm_hand1"), true,
				  IDC_PANE, toolbar, tr("Pane Mode"));
   pane_butt->setToggleButton(TRUE);
   connect(pane_butt, SIGNAL(clicked(void)), 
           this, SLOT(slotPaneMode(void)));

   zoom_select_butt=new QDToolButton(*CINData::get("ppm_zoomselect"), true,
                                     IDC_ZOOM_SELECT, toolbar,
                                     tr("Zoom Selected Area"));
   zoom_select_butt->setToggleButton(TRUE);
   connect(zoom_select_butt, SIGNAL(clicked(void)), 
           this, SLOT(slotPaneMode(void)));
   
   text_select_butt=new QDToolButton(*CINData::get("ppm_textselect"), true,
                                     IDC_TEXT_SELECT, toolbar, 
                                     tr("Select Text in Selected Area"));
   text_select_butt->setToggleButton(TRUE);
   connect(text_select_butt, SIGNAL(clicked(void)), 
           this, SLOT(slotPaneMode(void)));
   
   if ( qdtoolbar_child ) 
     ((QDToolBar *)toolbar)->addLeftWidgets(pane_butt, 
                                            zoom_select_butt, 
                                            text_select_butt);
   
   if ( qdtoolbar_child )
     {
       pin_butt=new QDToolButton(*CINData::get("ppm_vpin_out"), false,
                                 -1, toolbar, tr("Stick"));
       pin_butt->setToggleButton(TRUE);
       pin_butt->setOnPixmap(*CINData::get("ppm_vpin_in"));
       connect(pin_butt, SIGNAL(toggled(bool)), this, SIGNAL(sigStick(bool)));
       ((QDToolBar *)toolbar)->addRightWidget(pin_butt);
       
       ((QDToolBar *)toolbar)->addPiece(this);
     }
   else
     {
       pin_butt=NULL;
     }
}


void
QDTBarModePiece::setOptions(int opts)
{
  bool b;
  b = !(opts & QDBase::OverrideFlags::TOOLBAR_NO_RESCOMBO);
  showOrHide(zoom_menu, b);
  b = !(opts & QDBase::OverrideFlags::TOOLBAR_NO_DISPCOMBO);
  showOrHide(mode_menu, b);
  b = !(opts & QDBase::OverrideFlags::TOOLBAR_NO_ZOOM);
  showOrHide(zoom_in_butt, b);
  showOrHide(zoom_out_butt, b);
  b = !(opts & QDBase::OverrideFlags::TOOLBAR_NO_PAN);
  showOrHide(pane_butt, b);
  b = !(opts & QDBase::OverrideFlags::TOOLBAR_NO_ZOOMSEL);
  showOrHide(zoom_select_butt, b);
  b = !(opts & QDBase::OverrideFlags::TOOLBAR_NO_TEXTSEL);
  showOrHide(text_select_butt, b);
}


void
QDTBarModePiece::update(int cmd_mode, bool mode_enabled, int cmd_zoom, int zoom,
			int pane_mode, int has_text)
{
   int i;
   for(i=0;i<menu_items_size;i++)
   {
     if(cmd_zoom == menu_items[i].zoom)
     {
       zoom_menu->setCurrentItem(i);
       break;
     }
   }
   if(i==menu_items_size)
   {
      char buffer[64];
      sprintf(buffer, "%d %%", zoom);
      zoom_menu->setEditText(buffer);
   }

   zoom_in_butt->setEnabled(zoom<IDC_ZOOM_MAX-IDC_ZOOM_MIN);
   zoom_out_butt->setEnabled(zoom>5);

   pane_butt->setOn(pane_mode==IDC_PANE);
   zoom_select_butt->setOn(pane_mode==IDC_ZOOM_SELECT);
   if (has_text)
   {
      if (!text_select_butt->isEnabled())
	 text_select_butt->setEnabled(true);
      text_select_butt->setOn(pane_mode==IDC_TEXT_SELECT);
   }
   else
   {
      if (text_select_butt->isEnabled())
	 text_select_butt->setEnabled(false);
   }
   
   
   switch(cmd_mode)
   {
      case IDC_DISPLAY_COLOR:
        mode_menu->setCurrentItem(0);
        break;
      case IDC_DISPLAY_BLACKWHITE:
        mode_menu->setCurrentItem(1);
        break;
      case IDC_DISPLAY_FOREGROUND:
        mode_menu->setCurrentItem(2);
        break;
      case IDC_DISPLAY_BACKGROUND:
        mode_menu->setCurrentItem(3);
        break;
      default:
        break;
   }

   if ( !qdtoolbar_child )
      zoom_menu->setEnabled(true);
   
   mode_menu->setEnabled(mode_enabled);

   zoom_menu->setFixedSize(zoom_menu->sizeHint());
   mode_menu->setFixedSize(mode_menu->sizeHint());

      // Keep everything disabled if the toolbar is disabled.
   if (!toolbar->isEnabled()) setEnabled(false);
}

bool
QDTBarModePiece::isStuck(void) const
{
   if ( !pin_butt ) return FALSE;
   return pin_butt->isOn();
}

void
QDTBarModePiece::stick(bool en)
{
   if ( pin_butt )
      pin_butt->setOn(en);
}

void
QDTBarModePiece::slotZoom(const QString & qstr)
{
  int i;
  for(i=0;i<menu_items_size;i++)
  {
    if(qstr==tr(menu_items[i].str))
    {
      emit sigSetZoom(menu_items[i].zoom);
      break;
    }
  }
  if(i == menu_items_size)
  {
     const char * const str=qstr;
    i=atoi(str);
    if(i>0)
    {
      emit sigSetZoom(i+IDC_ZOOM_MIN);
    }
  }
}

void
QDTBarModePiece::slotZoom(void)
{
   const QObject * obj=sender();
   if (obj && obj->isWidgetType() && obj->inherits("QDToolButton"))
   {
      const QDToolButton * butt=(QDToolButton *) obj;
      emit sigSetZoom(butt->cmd);
   }
}

void
QDTBarModePiece::slotPaneMode(void)
{
   const QObject * obj=sender();
   if (obj && obj->inherits("QDToolButton") && ((QDToolButton *)obj)->isToggleButton())
   {
      QDToolButton * butt=(QDToolButton *) obj;
      
      if (butt->isOn())
      {
	 // a bit inefficient, but I don't want to use button group either
	 zoom_select_butt->setOn(FALSE);
	 text_select_butt->setOn(FALSE);
	 pane_butt->setOn(FALSE);
      }
      butt->setOn(TRUE);
      emit sigSetPaneMode(butt->cmd);
   }
}

void
QDTBarModePiece::slotMode(int index)
{
   switch(index)
   {
      case 0:
        emit sigSetMode(IDC_DISPLAY_COLOR);
        break;
      case 1:
        emit sigSetMode(IDC_DISPLAY_BLACKWHITE);
        break;
      case 2:
        emit sigSetMode(IDC_DISPLAY_FOREGROUND);
        break;
      case 3:
        emit sigSetMode(IDC_DISPLAY_BACKGROUND);
        break;
      default:
        break;
   }
}

// END OF FILE
