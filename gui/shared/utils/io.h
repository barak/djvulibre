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

#ifndef HDR_IO
#define HDR_IO
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma interface
#endif


#include "GString.h"
#include "Arrays.h"

#define OK_STRING	"OK"
#define ERR_STRING	"ERR"

#define CMD_SHUTDOWN		0
#define CMD_NEW			1
#define CMD_DETACH_WINDOW	2
#define CMD_ATTACH_WINDOW	3
#define CMD_RESIZE		4
#define CMD_DESTROY		5
#define CMD_PRINT		6
#define CMD_NEW_STREAM		7
#define CMD_WRITE		8
#define CMD_DESTROY_STREAM	9
#define CMD_SHOW_STATUS		10
#define CMD_GET_URL		11
#define CMD_GET_URL_NOTIFY	12
#define CMD_URL_NOTIFY		13
#define CMD_HANDSHAKE		14

struct PipeError { static const char *Tag; };

void	WriteString(int fd, const char * str);
void	WriteInteger(int fd, int var);
void	WriteDouble(int fd, double var);
void	WritePointer(int fd, const void * ptr);
void	WriteArray(int fd, const TArray<char> & array);

GUTF8String 	ReadString(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
int     	ReadInteger(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
double		ReadDouble(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
void *  	ReadPointer(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
TArray<char>	ReadArray(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
void            ReadResult(int fd, const char * cmd_name, int refresh_pipe=-1);

#endif
