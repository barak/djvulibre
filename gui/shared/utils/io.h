//C-  -*- C++ -*-
//C-
//C- DjVu® Unix Viewer (v. 3.5)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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

#ifndef HDR_IO
#define HDR_IO
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif


#include "GString.h"
#include "Arrays.h"

#ifndef PLUGIN
#include "exc_sys.h"
#endif

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

#ifndef PLUGIN
DECLARE_EXCEPTION(PipeError, "PipeError", SystemError);
#define PIPE_ERROR(func, msg) PipeError(func, msg, 0, __FILE__, __LINE__)
#endif

void	WriteString(int fd, const char * str);
void	WriteInteger(int fd, int var);
void	WriteDouble(int fd, double var);
void	WritePointer(int fd, const void * ptr);
void	WriteArray(int fd, const TArray<char> & array);

// Note: refresh_cb() works only for the plugin (-DPLUGIN)
// Otherwise it affects nothing.
GUTF8String 	ReadString(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
int     	ReadInteger(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
double		ReadDouble(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
void *  	ReadPointer(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
TArray<char>	ReadArray(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
void            ReadResult(int fd, const char * cmd_name, 
                           int refresh_pipe=-1, void (* refresh_cb)(void)=0);

#endif
