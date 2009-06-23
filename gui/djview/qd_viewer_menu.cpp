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
// $Id: qd_viewer_menu.cpp,v 1.16 2007/03/25 20:48:23 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "qd_viewer_menu.h"
#include "debug.h"
#include "djvu_viewer_res.h"
#include "DjVuDocument.h"
#include "qd_doc_info.h"
#include "qd_nav_goto_page.h"
#include "qd_set_zoom.h"
#include "qd_thumb.h"
#include "qlib.h"
#include "DjVuMessage.h"

void
QDViewer::createPopupMenu(void)
{
   DEBUG_MSG("QDViewer::createPopupMenu(): doing the stuff\n");
   DEBUG_MAKE_INDENT(3);
   
   popup_menu=new QPopupMenu(this, "djvu_popup_menu");
   connect(popup_menu, SIGNAL(activated(int)), this, SLOT(slotPopupCB(int)));

   QPopupMenu * displ_pane=new QPopupMenu(this, "displ_pane");
   connect(displ_pane, SIGNAL(activated(int)), popup_menu, SIGNAL(activated(int)));
   displ_pane->setCheckable(TRUE);
   displ_pane->insertItem(tr("&Color"), IDC_DISPLAY_COLOR);
   displ_pane->insertItem(tr("Black and &White"), IDC_DISPLAY_BLACKWHITE);
   displ_pane->insertItem(tr("&Background"), IDC_DISPLAY_BACKGROUND);
   displ_pane->insertItem(tr("&Foreground"), IDC_DISPLAY_FOREGROUND);
#ifndef QT1
   if (! in_netscape) {
     displ_pane->insertSeparator();
     displ_pane->insertItem(tr("Full &Screen"), IDC_FULL_SCREEN);
   }
#endif
   popup_menu->insertItem(tr("&Display"), displ_pane, IDC_DISPLAY);

   QPopupMenu * zoom_pane=new QPopupMenu(this, "zoom_pane");
   connect(zoom_pane, SIGNAL(activated(int)), 
           popup_menu, SIGNAL(activated(int)));
   zoom_pane->setCheckable(TRUE);
   zoom_pane->insertItem("&300 %", IDC_ZOOM_300);
   zoom_pane->insertItem("15&0 %", IDC_ZOOM_150);
   zoom_pane->insertItem("&100 %", IDC_ZOOM_100);
   zoom_pane->insertItem("&75 %", IDC_ZOOM_75);
   zoom_pane->insertItem("&50 %", IDC_ZOOM_50);
   zoom_pane->insertItem("&25 %", IDC_ZOOM_25);
   zoom_pane->insertItem(tr("&Custom..."), IDC_ZOOM_CUSTOM);
   zoom_pane->insertSeparator();
   zoom_pane->insertItem(tr("One &to One"), IDC_ZOOM_ONE2ONE);
   zoom_pane->insertItem(tr("&Stretch"), IDC_ZOOM_STRETCH);
   zoom_pane->insertItem(tr("Fit &Width"), IDC_ZOOM_WIDTH);
   zoom_pane->insertItem(tr("Fit &Page"), IDC_ZOOM_PAGE);
   zoom_pane->insertSeparator();
   zoom_pane->insertItem(tr("Zoom &In"), IDC_ZOOM_ZOOMIN);
   zoom_pane->insertItem(tr("Zoom &Out"), IDC_ZOOM_ZOOMOUT);
   popup_menu->insertItem(tr("&Zoom"), zoom_pane, IDC_ZOOM);

   QPopupMenu * rot_pane=new QPopupMenu(this, "rot_pane");
   connect(rot_pane, SIGNAL(activated(int)), 
           popup_menu, SIGNAL(activated(int)));
   rot_pane->insertItem(tr("Normal"), IDC_ROTATE_0);
   rot_pane->insertItem(tr("Left side down"), IDC_ROTATE_90);
   rot_pane->insertItem(tr("Upside down"), IDC_ROTATE_180);
   rot_pane->insertItem(tr("Right side down"), IDC_ROTATE_270);
   popup_menu->insertItem(tr("&Rotation"), rot_pane, IDC_ROTATE);

   QPopupMenu * nav_pane=new QPopupMenu(this, "nav_pane");
   connect(nav_pane, SIGNAL(activated(int)), 
           popup_menu, SIGNAL(activated(int)));
   nav_pane->insertItem(tr("&Next Page"), IDC_NAV_NEXT_PAGE);
   nav_pane->insertItem(tr("&Previous Page"), IDC_NAV_PREV_PAGE);
   nav_pane->insertItem(tr("&First Page"), IDC_NAV_FIRST_PAGE);
   nav_pane->insertItem(tr("&Last Page"), IDC_NAV_LAST_PAGE);
   nav_pane->insertItem(tr("&Goto Page..."), IDC_NAV_GOTO_PAGE);
   nav_pane->insertSeparator();
   nav_pane->insertItem(tr("&Back"), IDC_HISTORY_BACK);
   nav_pane->insertItem(tr("&Forward"), IDC_HISTORY_FORW);
   popup_menu->insertItem(tr("&Navigate"), nav_pane, IDC_NAVIGATE);
   
   popup_menu->insertSeparator();

   popup_menu->setCheckable(TRUE);
   popup_menu->insertItem(tr("&Find"), IDC_SEARCH);
   popup_menu->insertItem(tr("Page &Information"), IDC_ABOUT_PAGE);
   popup_menu->insertItem(tr("Document In&formation"), IDC_ABOUT_DOC);
   popup_menu->insertItem(tr("Show &Thumbnails"), IDC_THUMB_SHOW);
   popup_menu->insertItem(tr("Show &Bookmarks"), IDC_BOOKMARKS_SHOW);

   popup_menu->insertSeparator();
   
   popup_menu->insertItem(tr("Save D&ocument As ..."), IDC_SAVE_DOC_AS);
   popup_menu->insertItem(tr("Save P&age As ..."), IDC_SAVE_PAGE_AS);
   popup_menu->insertItem(tr("&Export page"), IDC_EXPORT_PAGE);

   popup_menu->insertSeparator();
   popup_menu->insertItem(tr("Print Pa&ge"), IDC_PRINT_PAGE);
   popup_menu->insertItem(tr("&Print Document"), IDC_PRINT_DOC);
   popup_menu->insertItem(tr("Print &Window"), IDC_PRINT_WIN);
   popup_menu->insertSeparator();
   popup_menu->insertItem(tr("P&references ..."), IDC_PREFERENCES);
   if (in_netscape)
   {
      popup_menu->insertSeparator();
      popup_menu->insertItem(tr("A&bout DjVu ..."), IDC_ABOUT_DEJAVU);
      popup_menu->insertItem(tr("&Help ..."), IDC_HELP_DEJAVU);
   }
}

void
QDViewer::setupMenu(QMenuData * menu)
{
   DEBUG_MSG("QDViewer::setupMenu(): enabling/disabling/setting menu items\n");
   DEBUG_MAKE_INDENT(3);

   if (!dimg || !dimg->get_width() || !dimg->get_height())
   {
	 // Do everything insensitive except for some stuff
      setItemsEnabled(menu, FALSE);
      if (menu->findItem(IDC_ABOUT_DEJAVU))
	 menu->setItemEnabled(IDC_ABOUT_DEJAVU, TRUE);
      if (menu->findItem(IDC_PREFERENCES))
	 menu->setItemEnabled(IDC_PREFERENCES, TRUE);
      if (menu->findItem(IDC_HELP_DEJAVU))
	 menu->setItemEnabled(IDC_HELP_DEJAVU, TRUE);
   } 
   else
   {
	 // Enable everything
      setItemsEnabled(menu, TRUE);

      GP<DjVuFile> djvu_file=dimg->get_djvu_file();
      int doc_page=djvu_doc->url_to_page(djvu_file->get_url());
      int doc_pages=djvu_doc->get_pages_num();

      menu->setItemChecked(IDC_DISPLAY_COLOR, 
                           getMode(true)==IDC_DISPLAY_COLOR);
      menu->setItemChecked(IDC_DISPLAY_BLACKWHITE, 
                           getMode(true)==IDC_DISPLAY_BLACKWHITE);
      menu->setItemChecked(IDC_DISPLAY_FOREGROUND, 
                           getMode(true)==IDC_DISPLAY_FOREGROUND);
      menu->setItemChecked(IDC_DISPLAY_BACKGROUND, 
                           getMode(true)==IDC_DISPLAY_BACKGROUND);

      menu->setItemChecked(IDC_ZOOM_25, 
                           getCMDZoom()==IDC_ZOOM_25);
      menu->setItemChecked(IDC_ZOOM_50, 
                           getCMDZoom()==IDC_ZOOM_50);
      menu->setItemChecked(IDC_ZOOM_75, 
                           getCMDZoom()==IDC_ZOOM_75);
      menu->setItemChecked(IDC_ZOOM_100, 
                           getCMDZoom()==IDC_ZOOM_100);
      menu->setItemChecked(IDC_ZOOM_150, 
                           getCMDZoom()==IDC_ZOOM_150);
      menu->setItemChecked(IDC_ZOOM_300, 
                           getCMDZoom()==IDC_ZOOM_300);
      menu->setItemChecked(IDC_ZOOM_ONE2ONE, 
                           getCMDZoom()==IDC_ZOOM_ONE2ONE);
      menu->setItemChecked(IDC_ZOOM_STRETCH, 
                           getCMDZoom()==IDC_ZOOM_STRETCH);
      menu->setItemChecked(IDC_ZOOM_WIDTH, 
                           getCMDZoom()==IDC_ZOOM_WIDTH);
      menu->setItemChecked(IDC_ZOOM_PAGE, 
                           getCMDZoom()==IDC_ZOOM_PAGE);
      
      menu->setItemChecked(IDC_ZOOM_CUSTOM,
			   !menu->isItemChecked(IDC_ZOOM_25) &&
			   !menu->isItemChecked(IDC_ZOOM_50) &&
			   !menu->isItemChecked(IDC_ZOOM_75) &&
			   !menu->isItemChecked(IDC_ZOOM_100) &&
			   !menu->isItemChecked(IDC_ZOOM_150) &&
			   !menu->isItemChecked(IDC_ZOOM_300) &&
			   !menu->isItemChecked(IDC_ZOOM_ONE2ONE) &&
			   !menu->isItemChecked(IDC_ZOOM_STRETCH) &&
			   !menu->isItemChecked(IDC_ZOOM_WIDTH) &&
			   !menu->isItemChecked(IDC_ZOOM_PAGE));

	 // ZoomIn and ZoomOut cases
      menu->setItemEnabled(IDC_ZOOM_ZOOMIN, 
                           getZoom()<IDC_ZOOM_MAX-IDC_ZOOM_MIN);
      menu->setItemEnabled(IDC_ZOOM_ZOOMOUT, 
                           getZoom()>5);

      if (menu->findItem(IDC_ROTATE))
        {
          menu->setItemChecked(IDC_ROTATE_0,
                               getRotate()==IDC_ROTATE_0);
          menu->setItemChecked(IDC_ROTATE_90,
                               getRotate()==IDC_ROTATE_90);
          menu->setItemChecked(IDC_ROTATE_180,
                               getRotate()==IDC_ROTATE_180);
          menu->setItemChecked(IDC_ROTATE_270,
                               getRotate()==IDC_ROTATE_270);
        }
      
      if (menu->findItem(IDC_NAVIGATE))
        {
          menu->setItemEnabled(IDC_NAVIGATE, doc_pages>1);
          menu->setItemEnabled(IDC_NAV_PREV_PAGE, doc_page>0);
          menu->setItemEnabled(IDC_NAV_NEXT_PAGE, doc_page<=doc_pages-2);
          menu->setItemEnabled(IDC_NAV_FIRST_PAGE, doc_page>0);
          menu->setItemEnabled(IDC_NAV_LAST_PAGE, doc_page!=doc_pages-1);
          menu->setItemEnabled(IDC_NAV_GOTO_PAGE, doc_pages>1);

          menu->setItemEnabled(IDC_HISTORY_BACK, hundo.size());
          menu->setItemEnabled(IDC_HISTORY_FORW, hredo.size());
        }
      
      menu->setItemEnabled(IDC_SAVE_PAGE_AS, 
                           djvu_file->is_all_data_present());
      menu->setItemEnabled(IDC_SAVE_DOC_AS, 
                           djvu_doc->is_init_complete() &&
			   djvu_doc->get_doc_type()
                           != DjVuDocument::SINGLE_PAGE);
      menu->setItemEnabled(IDC_EXPORT_PAGE, 
                           djvu_file->is_all_data_present());
      menu->setItemEnabled(IDC_THUMB_SHOW, 
                           doc_pages>1);
      menu->setItemChecked(IDC_THUMB_SHOW, 
                           thumbnailsShown());
      menu->setItemEnabled(IDC_BOOKMARKS_SHOW, 
                           false);
      menu->setItemEnabled(IDC_PRINT_PAGE, 
                           print_win==0 && getOverrideFlags().print);
      menu->setItemEnabled(IDC_PRINT_DOC, 
                           print_win==0 && getOverrideFlags().print);
      menu->setItemEnabled(IDC_PRINT_WIN, 
                           print_win==0 && getOverrideFlags().print);
   }
}

void
QDViewer::runPopupMenu(QMouseEvent * ev)
      // Don't forget that there is also slotAboutToShowMenu() function
      // doing the same job with the menu bar etc.
{
   DEBUG_MSG("QDViewer::runPopupMenu(): adjusting and showing the menu\n");
   DEBUG_MAKE_INDENT(3);

   try
     {
       setupMenu(popup_menu);
       bool fullscreen = false;
       emit sigQueryFullScreen(fullscreen);
       popup_menu->setItemChecked(IDC_FULL_SCREEN, fullscreen);
       // Strange as it may seem, but I can't process popup menu commands
       // directly from a slot connected to the proper activated() signal.
       // The reason is the fact, that QT dislikes when a modal dialog
       // is created while the popup menu is still running.
       popup_menu_id=-1;
       popup_menu->exec(QCursor::pos());
       if (popup_menu_id >= 0) 
         processCommand(popup_menu_id);
     } 
   catch(const GException & exc)
     {
       showError(this, exc);
     }
}

void
QDViewer::processCommand(int cmd)
{
   DEBUG_MSG("QDViewer::processCommand(): cmd=" << cmd << "\n");
   DEBUG_MAKE_INDENT(3);

   try
   {
     int doc_page=0, doc_pages=1;
      if (dimg)
        {
          doc_page=djvu_doc->url_to_page(dimg->get_djvu_file()->get_url());
          doc_pages=djvu_doc->get_pages_num();
        }
      
      switch(cmd)
        {
        case IDC_DISPLAY_COLOR:
        case IDC_DISPLAY_BLACKWHITE:
        case IDC_DISPLAY_FOREGROUND:
        case IDC_DISPLAY_BACKGROUND:
          setMode(cmd, true, SRC_MANUAL);
          break;
          
        case IDC_ZOOM_25:
        case IDC_ZOOM_50:
        case IDC_ZOOM_75:
        case IDC_ZOOM_100:
        case IDC_ZOOM_150:
        case IDC_ZOOM_300:
        case IDC_ZOOM_ONE2ONE:
        case IDC_ZOOM_STRETCH:
        case IDC_ZOOM_WIDTH:
        case IDC_ZOOM_PAGE:
        case IDC_ZOOM_ZOOMIN:
        case IDC_ZOOM_ZOOMOUT:
          setZoom(cmd, true, SRC_MANUAL);
          break;
          
        case IDC_ROTATE_0:
        case IDC_ROTATE_90:
        case IDC_ROTATE_180:
        case IDC_ROTATE_270:
          setRotate(cmd, true, SRC_MANUAL);
          break;
          
        case IDC_ZOOM_CUSTOM:
          {
	    QDSetZoom zoom_d(getZoom(), this, "qd_set_zoom");
	    if (zoom_d.exec()==QDialog::Accepted)
              setZoom(zoom_d.getZoom()+IDC_ZOOM_MIN, 1, SRC_MANUAL);
	    break;
	 }
          
        case IDC_NAV_PREV_PAGE:
          if (doc_page>0) 
            gotoPage(doc_page-1, false);
          break;
        case IDC_NAV_NEXT_PAGE:
          if (doc_page<doc_pages-1) 
            gotoPage(doc_page+1, false);
          break;
        case IDC_NAV_PREV_PAGE10:
          if (doc_page>=10) 
            gotoPage(doc_page-10);
          break;
        case IDC_NAV_NEXT_PAGE10:
          if (doc_page<=doc_pages-11) 
            gotoPage(doc_page+10);
          break;
          
        case IDC_NAV_FIRST_PAGE:
          if (doc_page>0) 
            gotoPage(0);
          break;
          
        case IDC_NAV_LAST_PAGE:
          if (doc_page!=doc_pages-1) 
            gotoPage(doc_pages-1);
          break;
          
        case IDC_NAV_GOTO_PAGE:
          {
            QDNavGotoPage d(djvu_doc, dimg, this, "goto_dialog");
            if (d.exec()==QDialog::Accepted)
              gotoPage(d.getPageNum());
            break;
          }

        case IDC_HISTORY_BACK:
          {
            int pageno = history_undo();
            if (pageno>=0 && pageno<doc_pages)
              gotoPage(pageno, false);
          }
          break;
          
        case IDC_HISTORY_FORW:
          {
            int pageno = history_redo();
            if (pageno>=0 && pageno<doc_pages)
              gotoPage(pageno, false);
          }
          break;
          
        case IDC_PREFERENCES:
          slotShowPrefs();
          break;
          
        case IDC_ABOUT_DEJAVU:
          slotAbout();
          break;
          
        case IDC_HELP_DEJAVU:
          slotHelp();
          break;
          
        case IDC_SEARCH:
          search();
          break;
          
        case IDC_ABOUT_PAGE:
          if (dimg && dimg->get_width() && dimg->get_height())
            {
              GUTF8String desc = dimg->get_long_description();
              GUTF8String ldesc = DjVuMessageLite::LookUpUTF8(desc);
              showMessage(this, tr("DjVu: Page Information"), 
                          QStringFromGString(ldesc), 1, 1);
            }
          break;
          
        case IDC_ABOUT_DOC:
          if (djvu_doc)
            {
              QDDocInfo info(djvu_doc, this, "doc_info", TRUE);
              connect(&info, SIGNAL(sigGotoPage(int)),
                      this, SLOT(slotGotoPage(int)));
              info.exec();
            }
          break;
          
        case IDC_THUMB_SHOW:
          {
	    if (thumbnailsShown()) 
              hideThumbnails();
	    else 
              showThumbnails();
	    break;
          }
          
        case IDC_PRINT_DOC:
        case IDC_PRINT_PAGE:
        case IDC_PRINT_WIN:
          print(cmd);
          break;
          
        case IDC_SAVE_PAGE_AS:
          savePageAs();
          break;
          
        case IDC_SAVE_DOC_AS:
          saveDocumentAs();
          break;

        case IDC_EXPORT_PAGE:
          exportToPNM();
          break;
          
        case IDC_FULL_SCREEN:
          emit sigToggleFullScreen();
          break;
          
        case IDC_GOTO_DJVU:
          getURL("http://www.lizardtech.com", "_blank");
          break;
        }
   } 
   catch(const GException & exc)
   {
     showError(this, exc);
   }
}
