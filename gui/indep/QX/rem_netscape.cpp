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

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "throw_error.h"
#include "exc_msg.h"
#include "debug.h"
#include "rem_netscape.h"
#include <sys/param.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 128
#ifdef sun
extern "C" int gethostname(char *, int);
#endif
#endif

#define MOZILLA_VERSION_PROP	"_MOZILLA_VERSION"
#define MOZILLA_LOCK_PROP	"_MOZILLA_LOCK"
#define MOZILLA_COMMAND_PROP	"_MOZILLA_COMMAND"
#define MOZILLA_RESPONSE_PROP	"_MOZILLA_RESPONSE"

#define MAX_TIMEOUT	15

u_long RemoteNetscape::GetTopWindow(void)
{
   if (top_window) return top_window;
   
   DEBUG_MSG("RemoteNetscape::GetTopWindow(): traversing the tree up to reach the shell...\n");
   DEBUG_MAKE_INDENT(3);

   Display * displ=(Display *) RemoteNetscape::displ;
   Window ref_window=(Window) RemoteNetscape::ref_window;
   
   if (!displ || !ref_window)
      throw ERROR_MESSAGE("RemoteNetscape::GetTopWindow", "Display has not been intialized yet.");
   
   Atom atom_wm=XInternAtom(displ, "WM_STATE", True);
   if (!atom_wm) return 0;

   Window shell_win=0;
   Window cur_win=ref_window;
   while(1)
   {
      DEBUG_MSG("looking at window=" << cur_win << "\n");
      
      int props;
      Atom * prop=XListProperties(displ, cur_win, &props);
      if (prop && props)
      {
	 int i;
	 for(i=0;i<props;i++)
	    if (prop[i]==atom_wm) break;
	 XFree(prop);
	 if (i<props)
	 {
	    shell_win=cur_win;
	    break;
	 }
      }
      
      Window root, parent, * child;
      u_int childs;
      if (XQueryTree(displ, cur_win, &root, &parent, &child, &childs))
      {
	 if (child) XFree(child);
	 if (parent==root)
	 {
	    shell_win=cur_win;
	    break;
	 }
	 
	 cur_win=parent;
      }
   }
   DEBUG_MSG("got window=" << shell_win << "\n");
   top_window=(u_long) shell_win;
   return shell_win;
}

void RemoteNetscape::ObtainLock(void)
{
   DEBUG_MSG("RemoteNetscape::ObtainLock(): trying to obtain lock on Netscape's window.\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!displ || !ref_window)
      throw ERROR_MESSAGE("RemoteNetscape::ObtainLock", 
                          "Display has not been intialized yet.");

   Display * displ=(Display *) RemoteNetscape::displ;
   
   Atom atom_lock=XInternAtom(displ, MOZILLA_LOCK_PROP, False);
   if (!atom_lock)
      throw ERROR_MESSAGE("RemoteNetscape::ObtainLock", "Failed to get '"
			  MOZILLA_LOCK_PROP "' atom.");
   
   char lock_data[MAXHOSTNAMELEN+128];
   sprintf(lock_data, "Locked by pid%d@", (int) getpid());
   char *hostname = lock_data+strlen(lock_data); 
#if defined(HAVE_GETHOSTNAME) || !defined(AUTOCONF)
   if (gethostname(hostname, MAXHOSTNAMELEN) < 0)
#endif
     strcpy(hostname,"unknownhost");
   
   XSelectInput(displ, GetTopWindow(), PropertyChangeMask);
   
   int locked=0;
   do
   {
      XGrabServer(displ);	// Oh, mine... What do I do?!
      DEBUG_MSG("grabbed server...\n");
      
      Atom type=0;
      int format;
      u_long items, bytes_to_go;
      u_char * prop=0;
      if (XGetWindowProperty(displ, GetTopWindow(), atom_lock,
			     0, 0xffff/sizeof(long), False,
			     XA_STRING, &type, &format, &items,
			     &bytes_to_go, &prop)!=Success || !prop || !type)
      {
	 XChangeProperty(displ, GetTopWindow(), atom_lock, XA_STRING,
			 8, PropModeReplace, (u_char *) lock_data,
			 strlen(lock_data)+1);
	 locked=1;
	 DEBUG_MSG("locked!\n");
      }
      if (prop) XFree(prop);
      
      XUngrabServer(displ);	// Back to life :)
      XSync(displ, False);
      DEBUG_MSG("released server...\n");
      
      if (!locked)
      {
	 DEBUG_MSG("waiting to acquire the lock...\n");
	 // Wait here for a while waiting for the lock to go away
	 time_t start_time=time(0);
	 while(1)
	 {
	    XEvent event;
	    while(!XCheckMaskEvent(displ, PropertyChangeMask, &event))
	    {
	       if (time(0)-start_time>MAX_TIMEOUT)
		  throw ERROR_MESSAGE("RemoteNetscape::ObtainLock",
				      "Failed to obtain lock: timed out.");
	       sleep(1);
	    }
	    if (event.type==PropertyNotify &&
		event.xproperty.state==PropertyDelete &&
		event.xproperty.window==GetTopWindow() &&
		event.xproperty.atom==atom_lock) break;
	 }
      }
   } while(!locked);
   have_lock=1;
}

void RemoteNetscape::ReleaseLock(void)
{
   DEBUG_MSG("RemoteNetscape::ReleaseLock(): Releasing the lock.\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!displ || !ref_window)
      throw ERROR_MESSAGE("RemoteNetscape::ReleaseLock", 
                          "Display has not been intialized yet.");

   Display * displ=(Display *) RemoteNetscape::displ;
   
   Atom atom_lock=XInternAtom(displ, MOZILLA_LOCK_PROP, False);
   if (!atom_lock)
      throw ERROR_MESSAGE("RemoteNetscape::ReleaseLock", "Failed to get '"
			  MOZILLA_LOCK_PROP "' atom.");
   
   char lock_data[MAXHOSTNAMELEN+128];
   sprintf(lock_data, "Locked by pid%d@", (int) getpid());
   if (gethostname(lock_data+strlen(lock_data), MAXHOSTNAMELEN)<0)
      ThrowError("RemoteNetscape::ObtainLock", "Failed to query local host name");
   
   Atom type=0;
   int format;
   u_long items, bytes_to_go;
   u_char * prop=0;
   if (XGetWindowProperty(displ, GetTopWindow(), atom_lock,
			  0, 0xffff/sizeof(long), True, // Will be deleted
			  XA_STRING, &type, &format, &items,
			  &bytes_to_go, &prop)!=Success || !prop || !type)
      throw ERROR_MESSAGE("RemoteNetscape::ReleaseLock", "Internal error: no lock to release.");
   if (prop && type)
   {
      have_lock=0;
      
      if (strcmp((char *) prop, lock_data))
	 throw ERROR_MESSAGE("RemoteNetscape::ReleaseLock",
			     "Internal error: released lock did not belong to me.");
      XFree(prop);
   }
}

void RemoteNetscape::SendCommand(const char * cmd)
{
   DEBUG_MSG("RemoteNetscape::SendCommand(): sending command '" << cmd << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!displ || !ref_window)
      throw ERROR_MESSAGE("RemoteNetscape::SendCommand", "Display has not been intialized yet.");

   Display * displ=(Display *) RemoteNetscape::displ;
   
   Atom atom_cmd=XInternAtom(displ, MOZILLA_COMMAND_PROP, False);
   if (!atom_cmd)
      throw ERROR_MESSAGE("RemoteNetscape::SendCommand", "Failed to get '"
			  MOZILLA_COMMAND_PROP "' atom.");
   Atom atom_res=XInternAtom(displ, MOZILLA_RESPONSE_PROP, False);
   if (!atom_res)
      throw ERROR_MESSAGE("RemoteNetscape::SendCommand", "Failed to get '"
			  MOZILLA_RESPONSE_PROP "' atom.");
   
   XChangeProperty(displ, GetTopWindow(), atom_cmd, XA_STRING, 8,
		   PropModeReplace, (u_char *) cmd, strlen(cmd)+1);
   DEBUG_MSG("command sent...\n");
   
   time_t start_time=time(0);
   while(1)
   {
      XEvent event;
      while(!XCheckMaskEvent(displ, PropertyChangeMask, &event))
      {
	 if (time(0)-start_time>MAX_TIMEOUT)
	    throw ERROR_MESSAGE("RemoteNetscape::SendCommand",
				"Failed to receive reply: timed out.");
	 sleep(1);
      }
      if (event.type==PropertyNotify &&
	  event.xproperty.state==PropertyNewValue &&
	  event.xproperty.window==GetTopWindow() &&
	  event.xproperty.atom==atom_res)
      {
	 Atom type=0;
	 int format;
	 u_long items, bytes_to_go;
	 u_char * prop=0;
	 try
	 {
	    if (XGetWindowProperty(displ, GetTopWindow(), atom_res,
				   0, 0xffff/sizeof(long), True, // Will be deleted
				   XA_STRING, &type, &format, &items,
				   &bytes_to_go, &prop)!=Success || !prop || !type)
	       throw ERROR_MESSAGE("RemoteNetscape::SendCommand", "Failed to read reply.");
	    
	    DEBUG_MSG("got reply '" << prop << "'\n");
	    if (strlen((char *) prop)<5)
	       throw ERROR_MESSAGE("RemoteNetscape::SendCommand", "Got corrupted reply.");
	    if (*prop!='2')
	       throw ERROR_MESSAGE("RemoteNetscape::SendCommand", "Command failed: "+
				   GUTF8String((char *) prop+4));
	    XFree(prop); prop=0;
	 } catch(...)
	 {
	    if (prop) XFree(prop);
	    throw;
	 }
	 break;
      }
   }
}

void RemoteNetscape::SendPage(void)
{
   DEBUG_MSG("RemoteNetscape::SendPage(): Telling netscape to send the page...\n");
   ObtainLock();
   try
   {
      SendCommand("sendPage()");
   } catch(...)
   {
      ReleaseLock();
      throw;
   }
   ReleaseLock();
}

RemoteNetscape::~RemoteNetscape(void)
{
   DEBUG_MSG("RemoteNetscape::~RemoteNetscape(): shutting down\n");
   DEBUG_MAKE_INDENT(3);
   
   try
   {
      if (have_lock)
      {
	 DEBUG_MSG("releasing lock...\n");
	 ReleaseLock();
      }
   } catch(...)
   {
      // Can't let exceptions go beyond the destructor
   }
   displ=0; ref_window=0; top_window=0;
}
