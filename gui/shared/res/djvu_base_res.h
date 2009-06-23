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
// $Id: djvu_base_res.h,v 1.9 2007/03/25 20:48:28 leonb Exp $
// $Name: release_3_5_22 $

#ifndef HDR_DJVU_BASE_RES
#define HDR_DJVU_BASE_RES
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#define IDC_SAVE			40000
#define IDC_SAVE_DOC			40001
#define IDC_SAVE_DOC_AS			40002
#define IDC_SAVE_PAGE			40003
#define IDC_SAVE_PAGE_AS		40004
#define IDC_SAVE_PAGES_AS		40005
#define IDC_COMPRESS_DOC		40006
#define IDC_EXPORT_PAGE			40007
#define IDC_SEND_PAGE			40008
#define IDC_PRINT			40009
#define IDC_PRINT_DOC			40010
#define IDC_PRINT_PAGE			40011
#define IDC_PRINT_WIN			40012
#define IDC_PRINT_CUSTOM		40013
#define IDC_PREFERENCES			40014

#define IDC_MODE			40100
#define IDC_DISPLAY			40100
#define IDC_DISPLAY_COLOR		40101
#define IDC_DISPLAY_BLACKWHITE		40102
#define IDC_DISPLAY_BACKGROUND		40103
#define IDC_DISPLAY_FOREGROUND		40104
#define IDC_ZOOM			40200

// PAGE==MIN-1, WIDTH==MIN-2 and STRETCH=MIN-3.
// Don't change it!
#define IDC_ZOOM_STRETCH		40201
#define IDC_ZOOM_CUSTOM			(IDC_ZOOM_STRETCH+1)
#define IDC_ZOOM_ONE2ONE		(IDC_ZOOM_CUSTOM+1)
#define IDC_ZOOM_WIDTH			(IDC_ZOOM_ONE2ONE+1)
#define IDC_ZOOM_PAGE			(IDC_ZOOM_WIDTH+1)
#define IDC_ZOOM_MIN			(IDC_ZOOM_PAGE+1)
#define IDC_ZOOM_25			(IDC_ZOOM_MIN+25)
#define IDC_ZOOM_50			(IDC_ZOOM_MIN+50)
#define IDC_ZOOM_75			(IDC_ZOOM_MIN+75)
#define IDC_ZOOM_100			(IDC_ZOOM_MIN+100)
#define IDC_ZOOM_150			(IDC_ZOOM_MIN+150)
#define IDC_ZOOM_300			(IDC_ZOOM_MIN+300)
#define IDC_ZOOM_MAX			(IDC_ZOOM_MIN+999)
#define IDC_ZOOM_ZOOMIN			(IDC_ZOOM_MAX+1)
#define IDC_ZOOM_ZOOMOUT		(IDC_ZOOM_MAX+2)

#define IDC_PANE                        42000
#define IDC_ZOOM_SELECT                 42010
#define IDC_TEXT_SELECT                 42020

#define IDC_ROTATE                      42100
#define IDC_ROTATE_0                    42101
#define IDC_ROTATE_90                   42102
#define IDC_ROTATE_180                  42103
#define IDC_ROTATE_270                  42104
#define IDC_ROTATE_RIGHT                42105
#define IDC_ROTATE_LEFT                 42106

#define IDC_OPEN			41300
#define IDC_CLOSE			41301
#define IDC_EXIT			41302
#define IDC_FILE			41303
#define IDC_IMPORT			41304
#define IDC_FULL_SCREEN			41305

#define IDC_NAVIGATE			41400
#define IDC_NAV_FIRST_PAGE		41401
#define IDC_NAV_LAST_PAGE		41402
#define IDC_NAV_NEXT_PAGE		41403
#define IDC_NAV_PREV_PAGE		41404
#define IDC_NAV_NEXT_PAGE10		41405
#define IDC_NAV_PREV_PAGE10		41406
#define IDC_NAV_GOTO_PAGE		41407

#define IDC_HISTORY                     41450
#define IDC_HISTORY_BACK	        41451
#define IDC_HISTORY_FORW	        41452

#define IDC_THUMB_SHOW			41500
#define IDC_THUMB_GENERATE		41502
#define IDC_THUMB_REMOVE		41503

#define IDC_BOOKMARKS_SHOW		41550

#define IDC_SEARCH			41600
#define IDC_VIEW			41601
#define IDC_INFO			41602
#define IDC_NEW_WINDOW			41603

#endif
