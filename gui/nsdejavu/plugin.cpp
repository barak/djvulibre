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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "plugin.h"
#include "path.h"
#include "names.h"
#include "version.h"
#include "saved_data.h"
#include "io.h"
#include "debug.h"
#include "GContainer.h"
#include "GException.h"

#ifndef DJVIEW_VERSION_STR
#include "DjVuVersion.h"
#endif

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <ctype.h>

#ifdef hpux
#include <sys/wait.h>
#else
#include <wait.h>
#endif

#include "npapi.h"

#ifdef hpux
#include <dl.h>
#else
#include <dlfcn.h>
#endif

#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#ifdef sgi
#define FORK fork
#else
#define FORK vfork
#endif

#ifdef hpux
extern "C" {
// Looks like gcc-2.7.2.3 fails on HP with global destructors and constructors.
// It assumes that netscape will call _GLOBAL__DI() when loading the plugin
// and _GLOBAL__DD() when unloading it. In fact, it will call nothing unless
// clearly said so by -Wl,+I,_GLOBAL__DI() function. But even in this
// case it will call _GLOBAL__DI() at loading and unloading though passing
// it different arguments. Unfortunately gcc doesn't know about them.
// The solution is this function, which decides what to call basing
// on what it gets from the loader.
extern void _GLOBAL__DI(void);
extern void _GLOBAL__DD(void);
void plugin_init(shl_t handle, int loading)
{
   if (loading) _GLOBAL__DI();
   else _GLOBAL__DD();
}
}
#endif


// ------------------------------------------------------------


class Instance
{
public:
  Widget		widget;
  Window		window;
  Widget		parent;
  NPP			np_instance;
  int			full_mode;
  
  Instance(void);
  Instance(NPP _np_instance, int _full_mode);
  void Detach(void);
  void Attach(Widget _widget, Window _window, Widget _parent);
  ~Instance(void);
};


Instance::Instance(void) 
  : widget(0), window(0), parent(0), np_instance(0), full_mode(1) 
{
}

Instance::Instance(NPP _np_instance, int _full_mode) 
  : widget(0), window(0),
    parent(0), np_instance(_np_instance), full_mode(_full_mode) 
{
}

void 
Instance::Detach(void)
{
  widget=0; window=0; parent=0;
}

void 
Instance::Attach(Widget _widget, Window _window, Widget _parent)
{
  widget=_widget; window=_window; parent=_parent;
}

Instance::~Instance(void)
{
  widget=0; window=0; parent=0; np_instance=0;
}


// ------------------------------------------------------------


class DelayedRequest
{
public:
  int req_num;
  void *id;
  GUTF8String status, url, target;
  DelayedRequest(void);
  DelayedRequest(int _req_num);
};


DelayedRequest::DelayedRequest(void) 
  : req_num(-1) 
{
}

DelayedRequest::DelayedRequest(int _req_num) 
  : req_num(_req_num) 
{
}



/*******************************************************************************
******************************* Preserved Data  ********************************
*******************************************************************************/



#define ENV_DJVU_STORAGE_PTR	"_DJVU_STORAGE_PTR"

// The plugin can be freely loaded/unloaded by Netscape, so any static
// variable is at risk of being destroyed at any time (after NPP_Shutdown()
// is called)
// We divide all static variables into three cathegory:
//    1. Variables, which are just shortcuts to some global structures
//	 e.g. app_context, which is application-specific. To avoid calling
//       the XtWidgetToApplicationContext(widget) every time, we call
//       it once and cache the result
//    2. Variables, which correspond to resources, which were *created* by
//       this particular load of the nsdejavu.so, and which need to be
//	 destroyed when the nsdejavu.so is being unloaded. Examples:
//	 'instance' map, 'input_id' handler, etc.
//    3. Variables corresponding to resources, which need to be allocated
//       only once in the Netscape lifetime. Examples are 'font10', 'white',
//       etc. These things are application-specific and should not be
//       changed between the plugin's loading/unloading. We store pointers
//       to them in the Netscape's environment.

// ********************** Group 1 ************************
// *********** Shortcuts to global structures ************

static char * reload_msg="An internal error occurred in the DjVu plugin.\n"
			 "Some of the embeded DjVu images may not be visible.\n"
			 "Please reload the page.\n\n";
static char * error_msg="The following internal error occurred in the DjVu plugin:\n\n\"%s\"\n\n"
			"Some of the embeded DjVu images may not be visible.\n"
			"Please reload the page.\n\n";

// ********************** Group 2 ************************
// ********* Things, that need to be destroyed ***********

static int			delay_pipe[2];
static XtInputId		input_id, delay_id;
static GMap<void *, Instance>	instance;
static GMap<void *, int>	strinstance;
static GList<DelayedRequest>	delayed_requests;

// ********************** Group 3 ************************
// ************ Things, created only once ****************

static int		pipe_read = -1, pipe_write=-1, rev_pipe=-1;
static u_long		white, black;
static Colormap         colormap;
static GC		text_gc;
static XFontStruct	*font10, *font12, *font14, *font18, *fixed_font;

struct SavedStatic 
{
  int pipe_read, pipe_write, rev_pipe;
  u_long white, black;
  Colormap colormap;
  GC text_gc;
  XFontStruct *font10, *font12, *font14, *font18, *fixed_font;
};

static void
SaveStatic(void)
  // Saves static variables from Group #3 into the Netscape's
  // environment. Next time nsdejavu.so is loaded, we will
  // read their values again.
{
  DEBUG_MSG("SaveStatic(): saving some static variables into netscape's env\n");
  DEBUG_MAKE_INDENT(3);
  
  char *value;
  SavedStatic *storage = 0;
  if ((value=getenv(ENV_DJVU_STORAGE_PTR))) 
    {
      sscanf(value, "%ld", (long *) &storage);
    } 
  else 
    {
      DEBUG_MSG("allocating new storage\n");
      storage = new SavedStatic;
      char *buffer=new char[128];
      sprintf(buffer, ENV_DJVU_STORAGE_PTR "=%ld", (long)storage);
      putenv(buffer);
   }
  
  storage->pipe_read = pipe_read;
  storage->pipe_write = pipe_write;
  storage->rev_pipe = rev_pipe;
  storage->white = white;
  storage->black = black;
  storage->colormap = colormap;
  storage->text_gc = text_gc;
  storage->font10 = font10;
  storage->font12 = font12;
  storage->font14 = font14;
  storage->font18 = font18;
  storage->fixed_font = fixed_font;
  
  DEBUG_MSG("pipe numbers: " <<
            pipe_read << ", " << pipe_write << ", " << rev_pipe << "\n");
  DEBUG_MSG("colors: " << black << ", " << white << "\n");
  DEBUG_MSG("colormap: " << colormap << "\n");
  DEBUG_MSG("GC: " << text_gc << "\n");
  DEBUG_MSG("fonts: " << font10 << ", " << font12 << ", " <<
            font14 << ", " << font18 << ", " << fixed_font << "\n");
  DEBUG_MSG("saved data='" << storage << "'\n");
}

static void
LoadStatic(void)
      // Loads static variables, which have been saved into the environment
      // via SaveStatic() by the previous instance of nsdejavu.so
{
  DEBUG_MSG("LoadStatic(): loading data from environment\n");
  DEBUG_MAKE_INDENT(3);
  
  SavedStatic *storage = 0;
  char * value;
  if ((value=getenv(ENV_DJVU_STORAGE_PTR)))
    sscanf(value, "%ld", (long *) &storage);
  
  if (storage)
    {
      DEBUG_MSG("Reading data from the environment:\n");
      DEBUG_MAKE_INDENT(3);
      pipe_read = storage->pipe_read;
      pipe_write = storage->pipe_write;
      rev_pipe = storage->rev_pipe;
      white = storage->white;
      black = storage->black;
      colormap = storage->colormap;
      text_gc = storage->text_gc;
      font10 = storage->font10;
      font12 = storage->font12;
      font14 = storage->font14;
      font18 = storage->font18;
      fixed_font = storage->fixed_font;
      DEBUG_MSG("pipe numbers: " <<
		pipe_read << ", " << pipe_write << ", " << rev_pipe << "\n");
      DEBUG_MSG("colors: " << black << ", " << white << "\n");
      DEBUG_MSG("colormap: " << colormap << "\n");
      DEBUG_MSG("GC: " << text_gc << "\n");
      DEBUG_MSG("fonts: " << font10 << ", " << font12 << ", " <<
		font14 << ", " << font18 << ", " << fixed_font << "\n");
      DEBUG_MSG("loaded data='" << storage << "'\n");
    } else DEBUG_MSG("No static data in the environment\n");
}






/*******************************************************************************
******************************* Intrinsic Callbacks ****************************
*******************************************************************************/

static void Detach(void * id);
static void Resize(void * id);
static void CloseConnection(void);
static bool IsConnectionOK(bool handshake=false);
static void ProgramDied(void);
static void StartProgram(void);


static void
Destroy_cb(Widget, XtPointer cl_data, XtPointer)
      // Called when the instance's drawing area is about to be destroyed
{
   void * id=cl_data;
   DEBUG_MSG("Destroy_cb(): Netscape's drawing area is about to be destroyed\n");
   DEBUG_MAKE_INDENT(3);
   
   G_TRY {
      Detach(id);
   } G_CATCH(exc) {
      fprintf(stderr, error_msg, exc.get_cause());;
   } G_ENDCATCH;
   DEBUG_MSG("leaving Destroy_cb()\n");
}

static void
Delay_cb(XtPointer, int * fd, XtInputId *)
      // Called when there are delayed requests to be processed
{
   DEBUG_MSG("Delay_cb() called\n");
   DEBUG_MAKE_INDENT(3);

      // Principally, this code can be moved inside the while() loop
      // below, since for every new request there is one byte written
      // into the pipe. But this way the code will be more robust.
   char ch;
   read(delay_pipe[0], &ch, 1);
   
   G_TRY {
      while(delayed_requests.size())
      {
	 GPosition pos=delayed_requests;
	 DelayedRequest req=delayed_requests[pos];
	 delayed_requests.del(pos);
	 
	 switch(req.req_num)
	 {
	    case CMD_SHOW_STATUS:
	    {
	       DEBUG_MSG("processing CMD_SHOW_STATUS request\n");
	       if (instance.contains(req.id))
	       {
		  DEBUG_MSG("id=" << req.id << " is legal\n");
		  Instance & inst=instance[req.id];
		  if (inst.widget) NPN_Status(inst.np_instance, req.status);
	       } else DEBUG_MSG("id=" << req.id << " is illegal\n");
	    }
	    break;
	 
	    case CMD_GET_URL:
	    {
	       DEBUG_MSG("processing CMD_GET_URL request\n");
	       if (instance.contains(req.id))
	       {
		  DEBUG_MSG("id=" << req.id << " is legal\n");
		  Instance & inst=instance[req.id];
		  const char * _target=0;
		  if (req.target.length()) _target=req.target;
		  NPN_GetURL(inst.np_instance, req.url, _target);
		  DEBUG_MSG("request URL using NPN_GetURL(): " << req.url <<"\n");
	       } else DEBUG_MSG("id=" << req.id << " is illegal\n");
	    }
	    break;

	    case CMD_GET_URL_NOTIFY:
	    {
	       DEBUG_MSG("processing CMD_GET_URL_NOTIFY request\n");
	       if (instance.contains(req.id))
	       {
		  DEBUG_MSG("id=" << req.id << " is legal\n");
		  Instance & inst=instance[req.id];
		  const char * _target=0;
		  if (req.target.length()) _target=req.target;
		  if (NPN_GetURLNotify(inst.np_instance, req.url, _target, 0)!=NPERR_NO_ERROR)
		  {
		     NPN_GetURL(inst.np_instance, req.url, _target);
		     DEBUG_MSG("request URL using NPN_GetURLNotify(): " << req.url <<" OK\n");
		  }
		  else
		  {
		     DEBUG_MSG("request URL using NPN_GetURLNotify(): " << req.url <<" FAILED\n");
		  }
	       } else DEBUG_MSG("id=" << req.id << " is illegal\n");
	    }
	    break;

	    default:
	       G_THROW("Unknown request read from the pipe.");
	 }
      }
   } G_CATCH(exc) {
      fprintf(stderr, error_msg, exc.get_cause());
   } G_ENDCATCH;
   DEBUG_MSG("leaving Delay_cb()\n");
}

static void
Input_cb(XtPointer, int * fd, XtInputId *)
      // Called when we receive commands from the 'djview'
      // This function can be called either by Xt toolkit (when Netscape
      // is idle), or by Refresh_cb() when we're waiting for a confirmation
      // from the viewer.
      //
      // Due to the thing above we do not process request here (Netscape
      // doesn't like when you call NPP_GetURL() from NPN_Write()). We
      // queue the request and process it later (in Delay_cb())
{
   DEBUG_MSG("Input_cb() called\n");
   DEBUG_MAKE_INDENT(3);

      // For some weird reason Input_cb() can also be called when
      // there is a problem with the pipes. Ideally, instead of calling
      // this callback (which is XtInputReadMask), Intrinsic should
      // call callback associated with XtInputExceptMask. But it doesn't
      // do it so. Thus, we need to check for the status of pipes manually
   if (!IsConnectionOK())
   {
      ProgramDied();
      return;
   }
   
   G_TRY {
      while(true)
      {
	 int req_num=ReadInteger(rev_pipe);
	 switch(req_num)
	 {
	    case CMD_SHOW_STATUS:
	    {
	       DEBUG_MSG("queueing CMD_SHOW_STATUS request\n");
	       DelayedRequest req(req_num);
	       req.id=ReadPointer(rev_pipe);
	       req.status=ReadString(rev_pipe);
	       delayed_requests.append(req);
	       write(delay_pipe[1], "1", 1);
	    }
	    break;
	 
	    case CMD_GET_URL:
	    {
	       DEBUG_MSG("queueing CMD_GET_URL request\n");
	       DelayedRequest req(req_num);
	       req.id=ReadPointer(rev_pipe);
	       req.url=ReadString(rev_pipe);
	       req.target=ReadString(rev_pipe);
	       delayed_requests.append(req);
	       write(delay_pipe[1], "1", 1);
	    }
	    break;

	    case CMD_GET_URL_NOTIFY:
	    {
	       DEBUG_MSG("queueing CMD_GET_URL_NOTIFY request\n");
	       DelayedRequest req(req_num);
	       req.id=ReadPointer(rev_pipe);
	       req.url=ReadString(rev_pipe);
	       req.target=ReadString(rev_pipe);
	       delayed_requests.append(req);
	       write(delay_pipe[1], "1", 1);
	    }
	    break;

	    default:
	       G_THROW("Unknown request read from the pipe.");
	 }

	    // Process as many requests as possible before returning control
	    // back to Netscape. This pipe tends to overflow, so we don't
	    // want data to stay there too long.
	 fd_set read_fds;
	 FD_ZERO(&read_fds);
	 FD_SET(rev_pipe, &read_fds);
	 timeval tv; tv.tv_sec=0; tv.tv_usec=0;
	 if (select(rev_pipe+1, &read_fds, 0, 0, &tv)!=1 ||
	     !FD_ISSET(rev_pipe, &read_fds)) break;
      }
   } G_CATCH(exc) {
      if (!IsConnectionOK() || strstr(exc.get_cause(), "PipeError"))
	 ProgramDied();
      
      fprintf(stderr, error_msg, exc.get_cause());
   } G_ENDCATCH;
   DEBUG_MSG("leaving Input_cb()\n");
}

static void
Refresh_cb(void)
{
   if (rev_pipe)
   {
      fd_set read_fds;
      FD_ZERO(&read_fds);
      FD_SET(rev_pipe, &read_fds);
      struct timeval tv;
      tv.tv_sec=tv.tv_usec=0;
      int rc=select(rev_pipe+1, &read_fds, 0, 0, &tv);
      if (rc>0)
      {
	 DEBUG_MSG("**** Processing reversed request while waiting for confirmation\n");
	 Input_cb(0, 0,0);
      }
   }
}

static void
Resize_hnd(Widget w, XtPointer cl_data, XEvent * event, Boolean * cont)
{
      // This function is necessary because Netscape sometimes doesn't resize the
      // drawing area along with the shell. So I have to do it manually.
   *cont=True;		// Continue to dispatch
   
   if (event->type==ConfigureNotify)	// There can be a GravityNotify too
   {
      void * id=(void *) cl_data;
      DEBUG_MSG("Resize_hnd(): Got resize event\n");
      DEBUG_MAKE_INDENT(3);
   
      G_TRY {
	 if (instance.contains(id))
	 {
	    DEBUG_MSG("id=" << id << " is legal\n");
	    Instance & inst=instance[id];
	    if (inst.widget)
	    {
	       DEBUG_MSG("plugin is attached\n");
	       if (inst.full_mode)
	       {
		  DEBUG_MSG("making plugin window fit its parent window\n");
		  XtConfigureWidget(inst.widget, 0, 0,
				    event->xconfigure.width,
				    event->xconfigure.height, 0);
	       }
	       DEBUG_MSG("sending resize info to the standalone\n");
	       Resize(id);
	    } else DEBUG_MSG("plugin is not attached\n");
	 } else DEBUG_MSG("id=" << id << " is illegal\n");
      } G_CATCH(exc) {
	 if (!IsConnectionOK() || strstr(exc.get_cause(), "PipeError"))
	    ProgramDied();
      
	 fprintf(stderr, error_msg, exc.get_cause());
      } G_ENDCATCH;
   }
}


/*******************************************************************************
***************************** Copying the colormap *****************************
*******************************************************************************/


static void
CopyColormap(Display *displ, Visual *visual, Screen *screen, Colormap cmap)
{
  DEBUG_MSG("Copying the netscape colormap\n");
  DEBUG_MAKE_INDENT(3);
  if (colormap)
    {
      DEBUG_MSG("Colormap is already initialized => ok.\n");
      return;
    }
  
  if (cmap == DefaultColormapOfScreen(screen))
    {
      DEBUG_MSG("Using default colormap => no need to copy.\n");
      colormap = cmap;
      return;
    }

  // Depends on the visual class
  int n;
  switch(visual->c_class)
    {
    case StaticGray:
    case StaticColor:
    case TrueColor:
      // Colormap for which there is nothing to copy
      colormap = XCreateColormap(displ, RootWindowOfScreen(screen), visual, AllocNone);
      break;

    case DirectColor:
      // Use a simple ramp. QT does not like these anyway...
      colormap = XCreateColormap(displ, RootWindowOfScreen(screen), visual, AllocNone);
      for (n=0; n<visual->map_entries; n++)
        {
          XColor cell;
          cell.pixel = n;
          cell.red = cell.green = cell.blue = (n * 0x10000)/visual->map_entries;
          cell.flags = DoRed | DoGreen | DoBlue;
          XAllocColor(displ, colormap, &cell);
        }
      break;

    case PseudoColor:
    case GrayScale:
      // The complicated case
      {
        // Make sure the colormap is rich enough.
	// This happens to be pretty useless code, btw, as:
	//    1. Netscape allocates the colorcube
	//    2. It allocates it as RW, basically leaving no space in the
	//       8bpp colormap that we could use.
	//
	// But we will give it a try anyway...
        int i,j,k;
        bool again = true;
        static unsigned short r1[] = { 0x0000, 0x9999, 0xffff };
        static unsigned short r2[] = { 0x0000, 0x3333, 0x6666, 0x9999, 0xcccc, 0xffff };
	// Allocate 3x3x3 cube first. It's a good idea to allocate the
	// most distinct colors first (from 3x3x3) and then proceed
	// with other colors from 6x6x6. Should the colormap be nearly
	// full, we will still get a reasonable approximation.
        for(i=0; again && i<3; i++)
          for(j=0; again && j<3; j++)
            for(k=0; again && k<3; k++)
              {
                XColor cell;
                cell.red = r1[i];
                cell.green = r1[j];
                cell.blue = r1[k];
                cell.flags = DoRed | DoGreen | DoBlue;
                if (!XAllocColor(displ, cmap, &cell))
                  again = false;
              }
	// Procees with additional colors of 6x6x6 cube.
        for(i=0; again && i<6; i++)
          for(j=0; again && j<6; j++)
            for(k=0; again && k<6; k++)
              {
                XColor cell;
                cell.red = r2[i];
                cell.green = r2[j];
                cell.blue = r2[k];
                cell.flags = DoRed | DoGreen | DoBlue;
                if (!XAllocColor(displ, cmap, &cell))
                  again = false;
              }
        // Now copy the colormap
        DEBUG_MSG("found the needed color in the ns colormap\n");
        DEBUG_MSG("copying them into new colormap\n");
        colormap = XCreateColormap(displ, RootWindowOfScreen(screen), visual, AllocNone);
        unsigned long *pixels = new unsigned long[visual->map_entries];
        XAllocColorCells(displ, colormap, False, pixels, 0, pixels, visual->map_entries);
        XColor *colors = new XColor[visual->map_entries];
        for (n=0; n<visual->map_entries; n++)
          {
            colors[n].pixel = n;
            colors[n].flags = DoRed | DoGreen | DoBlue;	// This can be dropped
          }
        XQueryColors(displ, cmap, colors, visual->map_entries);
        XStoreColors(displ, colormap, colors, visual->map_entries);
        // But make the desired ones read-only (so that QT can use them)
        for (n=0; n<visual->map_entries; n++)
          {
            // Free one cell exactly (all others are read/write)
	    XColor cell = colors[n];
            XFreeColors(displ, colormap, &cell.pixel, 1, 0);
            if (XAllocColor(displ, colormap, &cell))
              {
                DEBUG_MSG("Successful allocation for cell " << n << ".\n");
                if (cell.pixel != colors[n].pixel )
                  {
                    DEBUG_MSG("Got wrong pixel: " << cell.pixel << "! Skipping.\n");
                    cell.pixel = colors[n].pixel;
                    XAllocColorCells(displ, colormap, False, 0, 0, &cell.pixel, 1);
                    XStoreColor(displ, colormap, &cell);
                  }
              }
            else
              {
                DEBUG_MSG("AllocColor failed. What do I do now...?\n");
              }
          }
        // Calling XInstallColormap is needed on SGIs to ``finish-up'' the new colormap.
        // This should be ok since it contains the same colors as Netscape's and 
        // we can expect that the currently installed colormap is Netscape's.
        // Otherwise the screen may flash.
        XSync(displ, False);
        XInstallColormap(displ, colormap);
        // Cleanup
        delete [] colors;
        delete [] pixels;
      }
      break;
      
    default:
      DEBUG_MSG("Unknown visual class\n");
      colormap = cmap;
      break;
    }
}

/*******************************************************************************
************************************* Utilities ********************************
*******************************************************************************/
static inline int
max(int x, int y)
{
   return x<y ? y : x;
}

static bool
IsConnectionOK(bool handshake)
{
   if (pipe_read<=0 || pipe_write<=0 || rev_pipe<=0)
   {
      DEBUG_MSG("IsConnectionOK(): No - (pipe_read<=0 || pipe_write<=0 || rev_pipe<=0)\n");
      return false;
   }
   if (handshake)
   {
      G_TRY {
	 WriteInteger(pipe_write, CMD_HANDSHAKE);
	 ReadResult(pipe_read, "HANDSHAKE", rev_pipe, Refresh_cb);
      } G_CATCH(exc) {
	 DEBUG_MSG("IsConnectionOK(): No - handshake failed\n");
	 exc.get_line();	// For compiler not to bark.
	 return false;
      } G_ENDCATCH;
   }
   return true;
}

static void
ProgramDied(void)
   // This function may not throw any exception
{
   DEBUG_MSG("ProgramDied(): closing pipes, cleaning instances and restarting program\n");
   DEBUG_MAKE_INDENT(3);
   
   G_TRY
   {
      CloseConnection();
	 /** Do not clear instances here as you used to!
	     Netscape will probably call NPP_Destroy() later, and
	     you will need to unregister handlers (such as Resize_hnd)
	     instance.empty();
	     strinstance.empty();*/
      StartProgram();
   } G_CATCH(exc)
   {
      fprintf(stderr, "Failed to restart program:\n%s\n", exc.get_cause());
   } G_ENDCATCH;
}

static void
CloseConnection(void)
{
   DEBUG_MSG("CloseConnection(): Closing all the pipes and killing input handlers\n");
   DEBUG_MAKE_INDENT(3);

      // Close all connections to the viewer
   if (input_id) XtRemoveInput(input_id); input_id=0;
   if (pipe_read>0) close(pipe_read); pipe_read=-1;
   if (pipe_write>0) close(pipe_write); pipe_write=-1;
   if (rev_pipe>0) close(rev_pipe); rev_pipe=-1;

      // Update the environment
   SaveStatic();
}

static void
Resize(void * id)
{
   // Instead of selecting ConfigureEvent in the application I catch resizeCallback
   // here and send the appropriate request to the application
   DEBUG_MSG("Resize(): Getting and sending widget's dimensions\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!instance.contains(id))
   {
      DEBUG_MSG("id=" << id << " is illegal\n");
      return;
   } else DEBUG_MSG("id=" << id << " is legal\n");
   
   Instance & inst=instance[id];
   if (inst.widget)
   {
      DEBUG_MSG("plugin is attached\n");
      Dimension width, height;
      XtVaGetValues(instance[id].widget,
		    XtNwidth, &width,
		    XtNheight, &height, 0);
      
      if (IsConnectionOK(true))
      {
	 WriteInteger(pipe_write, CMD_RESIZE);
	 WritePointer(pipe_write, id);
	 WriteInteger(pipe_write, width);
	 WriteInteger(pipe_write, height);
	 ReadResult(pipe_read, "RESIZE", rev_pipe, Refresh_cb);
      }
   } else DEBUG_MSG("plugin is not attached\n");
}

static void
Detach(void * id)
{
   DEBUG_MSG("Detach(): detaching old window\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!instance.contains(id))
   {
      DEBUG_MSG("id=" << id << " is illegal\n");
      return;
   } else DEBUG_MSG("id=" << id << " is legal\n");
   
   Instance & inst=instance[id];
   if (inst.widget)
   {
      DEBUG_MSG("plugin is attached\n");
      XtRemoveCallback(inst.widget, XtNdestroyCallback, Destroy_cb, id);
      if (inst.full_mode)
	 XtRemoveEventHandler(inst.parent, StructureNotifyMask, False, Resize_hnd, id);
      else
	 XtRemoveEventHandler(inst.widget, StructureNotifyMask, False, Resize_hnd, id);
      inst.Detach();
      
      if (IsConnectionOK(true))
      {
	 WriteInteger(pipe_write, CMD_DETACH_WINDOW);
	 WritePointer(pipe_write, id);
	 ReadResult(pipe_read, "DETACH_WINDOW", rev_pipe, Refresh_cb);
      }
   } else DEBUG_MSG("plugin is not attached\n");
}

static void
Attach(Display * displ, Window window, void * id)
{
   DEBUG_MSG("Attach(): attaching new window\n");
   DEBUG_MAKE_INDENT(3);

   XSync(displ, False);
   
   if (!instance.contains(id))
   {
      DEBUG_MSG("id=" << id << " is illegal\n");
      return;
   } else DEBUG_MSG("id=" << id << " is legal\n");
   
   Instance & inst=instance[id];
   
   Widget parent=0, widget=0;
   G_TRY {
      widget=XtWindowToWidget(displ, window);
      parent=XtParent(widget);
      DEBUG_MSG("window=" << window << ", widget=" << widget << "\n");
      XtAddCallback(widget, XtNdestroyCallback, Destroy_cb, id);
      if (inst.full_mode)
	 XtAddEventHandler(parent, StructureNotifyMask, False, Resize_hnd, id);
      else
	 XtAddEventHandler(widget, StructureNotifyMask, False, Resize_hnd, id);
      XtAppContext app_context=XtWidgetToApplicationContext(widget);
      if (!input_id)
      {
	 DEBUG_MSG("installing Input_cb\n");
	 input_id=XtAppAddInput(app_context, rev_pipe,
				(XtPointer) XtInputReadMask, Input_cb, 0);
      }
      if (!delay_id)
      {
	 DEBUG_MSG("installing Delay_cb\n");
	 delay_id=XtAppAddInput(app_context, delay_pipe[0],
				(XtPointer) XtInputReadMask, Delay_cb, 0);
      }
      
	 // Preparing CMD_ATTACH_WINDOW attributes
      char * tmp=DisplayString(displ);
      if (!tmp) tmp=getenv("DISPLAY");
      if (!tmp) tmp=":0";
      GUTF8String displ_name=tmp;
      int depth;
      Colormap cmap;
      Visual *visual;
      Widget shell=widget;
      while(!XtIsShell(shell)) shell=XtParent(shell);
      XtVaGetValues(shell, XtNvisual, &visual, XtNcolormap, &cmap,
		    XtNdepth, &depth, 0);
      if (!visual) visual=XDefaultVisualOfScreen(XtScreen(shell));
      if (inst.full_mode)
      {
	 Dimension width, height;
	 XtVaGetValues(parent, XtNwidth, &width, XtNheight, &height, 0);
	 XtConfigureWidget(widget, 0, 0, width, height, 0);
      }
      if (visual->c_class==StaticGray)
	 G_THROW("We are sorry, but StaticGray visuals\n"
	       "are not supported by the DjVu plugin.");
      if (visual->c_class==GrayScale)
	 G_THROW("We are sorry, but GrayScale visuals\n"
	       "are not supported by the DjVu plugin.");
      if (visual->c_class==TrueColor && depth<=8)
	 G_THROW("We are sorry, but TrueColor visuals with depth\n"
	       "less than 8 bits are not supported by the DjVu plugin.");
	 
         // Process colormap
      if (!colormap)
      {
             // Allocating black and white colors
        DEBUG_MSG("allocating black & white colors\n");
        XColor white_screen, white_exact;
        XAllocNamedColor(displ, cmap, "white", &white_screen, &white_exact);
        white=white_screen.pixel;
        XColor black_screen, black_exact;
        XAllocNamedColor(displ, cmap, "black", &black_screen, &black_exact);
        black=black_screen.pixel;
        DEBUG_MSG("white=" << white << ", black=" << black << "\n");
             // Creating a copy of cmap
        CopyColormap(displ, visual, XtScreen(shell), cmap);
      }
	 // Get the background color for passing it to the application
      Dimension width, height;
      u_long back_color;
      char back_color_str[128]; back_color_str[0]=0;
      XtVaGetValues(widget, XtNwidth, &width, XtNheight, &height,
		    XtNbackground, &back_color, 0);
      XColor cell;
      cell.flags=DoRed | DoGreen | DoBlue;
      cell.pixel=back_color;
      XQueryColor(displ, cmap, &cell);
      sprintf(back_color_str, "rgb:%X/%X/%X", cell.red, cell.green, cell.blue);
      XtMapWidget(widget);
      XSync(displ, False);
	 // Creating GC for "Stand by..." message
      if (!text_gc)
      {
	 text_gc=XCreateGC(displ, window, 0, 0);
	 XSetForeground(displ, text_gc, black);
      }
      XFontStruct * font=0;
      const char * text="DjVu plugin is being loaded. Please stand by...";
      
	 // Load font for "Stand by..." message
      if (!font)
      {
	 if (!font18)
	 {
	    DEBUG_MSG("loading font18\n");
	    font18=XLoadQueryFont(displ, "-*-helvetica-medium-r-normal--18-*");
	 }
	 if (font18 && XTextWidth(font18, text, strlen(text))*5<=width*4)
	    font=font18;
      }
      
      if (!font)
      {
	 if (!font14)
	 {
	    DEBUG_MSG("loading font14\n");
	    font14=XLoadQueryFont(displ, "-*-helvetica-medium-r-normal--14-*");
	 }
	 if (font14 && XTextWidth(font14, text, strlen(text))*5<=width*4)
	    font=font14;
      }
      if (!font)
      {
	 if (!font12)
	 {
	    DEBUG_MSG("loading font12\n");
	    font12=XLoadQueryFont(displ, "-*-helvetica-medium-r-normal--12-*");
	 }
	 if (font12 && XTextWidth(font12, text, strlen(text))*5<=width*4)
	    font=font12;
      }
      if (!font)
      {
	 if (!font10)
	 {
	    DEBUG_MSG("loading font10\n");
	    font10=XLoadQueryFont(displ, "-*-helvetica-medium-r-normal--10-*");
	 }
	 if (font10 && XTextWidth(font10, text, strlen(text))*5<=width*4)
	    font=font10;
      }
      if (!font)
      {
	 if (!fixed_font)
	 {
	    DEBUG_MSG("loading fixed_font\n");
	    fixed_font=XLoadQueryFont(displ, "fixed");
	 }
	 if (fixed_font && XTextWidth(fixed_font, text, strlen(text))*6<=width*5)
	    font=fixed_font;
      }
      
	 // Paint the drawing area and display "Stand by..." message
      XtVaSetValues(widget, XtNforeground, black, XtNbackground, white, 0);
      if (font && text_gc)
      {
	 DEBUG_MSG("printing 'Stand by...' message\n");
	 XSetFont(displ, text_gc, font->fid);
	 int text_width=XTextWidth(font, text, strlen(text));
	 XDrawString(displ, window, text_gc, (width-text_width)/2,
		     height/2, text, strlen(text));
      }
      XSync(displ,False);
      WriteInteger(pipe_write, CMD_ATTACH_WINDOW);
      WritePointer(pipe_write, id);
      WriteString(pipe_write, (const char *)displ_name);
      WriteString(pipe_write, back_color_str);
      WriteInteger(pipe_write, window);
      WriteInteger(pipe_write, colormap);
      WriteInteger(pipe_write, XVisualIDFromVisual(visual));
      WriteInteger(pipe_write, width);
      WriteInteger(pipe_write, height);
      ReadResult(pipe_read, "ATTACH_WINDOW", rev_pipe, Refresh_cb);
      
      inst.Attach(widget, window, parent);
   } G_CATCH(exc) {
      if (widget) XtRemoveCallback(widget, XtNdestroyCallback, Destroy_cb, id);
      if (inst.full_mode)
      {
	 if (parent)
	    XtRemoveEventHandler(parent, StructureNotifyMask, False, Resize_hnd, id);
      } else if (widget)
	 XtRemoveEventHandler(widget, StructureNotifyMask, False, Resize_hnd, id);
      inst.Detach();
      exc.get_line();	// For compiler not to warn
      G_RETHROW;
   } G_ENDCATCH;
}

static void
StartProgram(void)
{
   DEBUG_MSG("StartProgram(): starting the program\n");
   DEBUG_MAKE_INDENT(3);
   
   if (IsConnectionOK(true)) return;
   
   GUTF8String path=GetViewerPath();
   if(!path.length())
      G_THROW("Failed to find '" DJVIEW_NAME "' executable.");
   
   DEBUG_MSG("got path to DejaVu's stuff=" << (const char *) path << "\n");
   
   DEBUG_MSG("creating pipes for SENDING commands...\n");
   int fd[2];
   int _pipe_read, _pipe_write;
   if (pipe(fd)<0) G_THROW(GUTF8String("Failed to create pipe:\n")+strerror(errno));
   pipe_read=fd[0];
   _pipe_write=fd[1];
   if (pipe(fd)<0) G_THROW(GUTF8String("Failed to create pipe:\n")+strerror(errno));
   pipe_write=fd[1];
   _pipe_read=fd[0];
   
   DEBUG_MSG("creating pipe for RECEIVING commands...\n");
   int _rev_pipe;
   if (pipe(fd)<0) G_THROW(GUTF8String("Failed to create pipe:\n")+strerror(errno));
   rev_pipe=fd[0];
   _rev_pipe=fd[1];

   // We want to wait for this child.
#ifdef sgi
   typedef void (*sighandler_t)(...);
#else
   typedef void (*sighandler_t)(int);
#endif
   sighandler_t sigsave=signal(SIGCHLD,SIG_DFL);
   
   DEBUG_MSG("trying to fork...\n");
   pid_t pid;

   // Avoid crashing Netscape when it can't find this process ID.
   if ((pid=FORK())<0) G_THROW(GUTF8String("Failed to fork:\n")+strerror(errno));
   if (!pid)
   {
	// These three lines look crazy, but the this is the only way I know
	// to orphan a child under all versions of Unix.  Otherwise the
	// SIGCHLD may cause Netscape to crash.

#ifdef NO_DEBUG
	// Don't do it in DEBUG mode => all DEBUG messages got lost.
      setsid();
#endif
      signal(SIGCHLD,SIG_IGN);
      if(FORK()) _exit(0);

        // Real Child
      close(pipe_read);
      close(pipe_write);
      close(rev_pipe);
      
      close(3); dup(_pipe_read); close(_pipe_read);
      close(4); dup(_pipe_write); close(_pipe_write);
      close(5); dup(_rev_pipe); close(_rev_pipe);
      
      // Duplication above will guarantee, that the new file descriptors
      // will not be closed on exec.

      // Now close all file descriptors which we don't use. For some reasons
      // we WILL inherit some from Netscape if we don't close them
      for(int i=8;i<1024;i++) close(i);

      const char mzh[]="MOZILLA_HOME";
      if (!getenv(mzh))
      {
	 char * buffer=new char[MAXPATHLEN+1];
	 sprintf(buffer, "%s=%s", mzh, (const char *) path);
	 putenv(buffer);
      }
      DEBUG_MSG("trying to exec program '" << (const char *) path << "\n");

	// This is needed for RedHat's version of Netscape.
      char * ptr=0;
      if ((ptr=getenv("LD_PRELOAD")) && strlen(ptr))
      {
	  // Whatever you say, putenv() doesn't help for RH 5.1. unsetenv() is needed.
#ifdef __linux__
	  unsetenv("LD_PRELOAD");
#else
	  putenv(strdup("LD_PRELOAD="));
#endif
      }
      if ((ptr=getenv("XNLSPATH")) && strlen(ptr))
      {
	  // Whatever you say, putenv() doesn't help for RH 5.1. unsetenv() is needed.
#ifdef __linux__
	  unsetenv("XNLSPATH");
#else
	  putenv(strdup("XNLSPATH="));
#endif
      }

	 // Autoinstaller fails to set the "executable" flag. Do it now
      struct stat st;
      if (stat(path, &st)>=0)
      {
	 mode_t mode=st.st_mode;
	 if (mode & S_IRUSR) mode|=S_IXUSR;
	 if (mode & S_IRGRP) mode|=S_IXGRP;
	 if (mode & S_IROTH) mode|=S_IXOTH;
	 chmod(path, mode);
      }
      
      execl(path, path, "-netscape", 0);
      DEBUG_MSG("execl() failed, reporting error back\n");
      
      GUTF8String error="Failed to execute program '"+path+"':\n";
      error=error+strerror(errno)+"\n";
      fprintf(stderr, "%s\n", (const char *) error);
      fflush(stderr);
	// Netscape sets a bunch of atexit() functions we don't want to call.
      _exit(1);
   }
   // Parent
   close(_pipe_write);
   close(_pipe_read);
   close(_rev_pipe);

	// Wait for the primary child
   { int s; waitpid(pid, &s, WNOHANG); }

	// Now reset the signal handler back to what Netscape uses.
   signal(SIGCHLD,sigsave);
   
   DEBUG_MSG("waiting for the child to report back\n");
   G_TRY
   {
      ReadString(pipe_read);
   } G_CATCH(exc)
   {
      exc.get_line();	// For compiler not to bark.
      CloseConnection();
      G_RETHROW;
   } G_ENDCATCH;
}

/*******************************************************************************
***************************** Netscape plugin interface ************************
*******************************************************************************/

NPError
NPP_Initialize(void)
{
//#ifndef NO_DEBUG
#if 0
   FILE *fd=fopen("/dev/tty", "a");
   if (fd) DjVuDebug::set_debug_file(fd);
#endif

   DEBUG_MSG("NPP_Initialize() called\n");
   DEBUG_MAKE_INDENT(3);

   G_TRY
   {
      LoadStatic();

      if (pipe(delay_pipe)<0)
	 G_THROW(GUTF8String("Failed to create pipe:\n")+strerror(errno));
      
      if (!IsConnectionOK(true))
      {
	 CloseConnection();
	 StartProgram();
      }
   } G_CATCH(exc)
   {
      if (!IsConnectionOK() || strstr(exc.get_cause(), "PipeError"))
	 CloseConnection();
      
      fprintf(stderr, error_msg, exc.get_cause());
      return NPERR_GENERIC_ERROR;
   } G_ENDCATCH;
   return NPERR_NO_ERROR;
}

void
NPP_Shutdown(void)
{
   DEBUG_MSG("NPP_Shutdown() called\n");
   DEBUG_MAKE_INDENT(3);
   
   G_TRY {
      DEBUG_MSG("Destroying vars with plugin life-length\n");
      
	 // Destroy here all static variables from Group #2: things,
	 // which are allocated only for the life time of the plugin
      if (input_id) XtRemoveInput(input_id); input_id=0;
      if (delay_id) XtRemoveInput(delay_id); delay_id=0;
      close(delay_pipe[0]);
      close(delay_pipe[1]);
      instance.empty();
      strinstance.empty();

      SaveStatic();

      DEBUG_MSG("Sending CMD_SHUTDOWN request to djview\n");
	 // Let the viewer know, that it may exit, if it wants. In
	 // reality, the viewer will stay alive for some time and will
	 // exit in the absence of new connections
      if (IsConnectionOK())
	 WriteInteger(pipe_write, CMD_SHUTDOWN);
   } G_CATCH(exc) {
	 // Just do nothing.
      exc.get_line();	// For compiler not to warn
   } G_ENDCATCH;
}

NPError
NPP_New(NPMIMEType, NPP np_inst, uint16 np_mode, int16 argc,
	char* argn[], char* argv[], NPSavedData * saved)
{
   DEBUG_MSG("NPP_New() called\n");
   DEBUG_MAKE_INDENT(3);

   if (!IsConnectionOK(true))
   {
      fprintf(stderr, "%s", reload_msg);
      CloseConnection();
      StartProgram();
   }
   
   void * id=0;
   G_TRY
   {
      GUTF8String path = GetLibraryPath();

      WriteInteger(pipe_write, CMD_NEW);
      WriteInteger(pipe_write, np_mode==NP_FULL);
      WriteString(pipe_write, (const char *)path);
      WriteInteger(pipe_write, argc);
      for(int i=0;i<argc;i++)
      {
	 WriteString(pipe_write, argn[i]);
	 WriteString(pipe_write, argv[i]);
      }
      if (saved && saved->buf &&
	  saved->len==sizeof(SavedData))
      {
	 SavedData * data=(SavedData *) saved->buf;
	 WriteInteger(pipe_write, 1);
	 WriteInteger(pipe_write, data->cmd_mode);
	 WriteInteger(pipe_write, data->cmd_zoom);
	 WriteInteger(pipe_write, data->imgx);
	 WriteInteger(pipe_write, data->imgy);
      } else WriteInteger(pipe_write, 0);
      ReadResult(pipe_read, "NEW", rev_pipe, Refresh_cb);
	 
      id=ReadPointer(pipe_read);
      DEBUG_MSG("got id=" << id << "\n");
      if (instance.contains(id))
        {
          // This can happen because we do not clear
          // the instance array when restarting djview.
          // We just undo it...
          instance.del(id);
        }
      np_inst->pdata=id;
      instance[id]=Instance(np_inst, np_mode==NP_FULL);
   } G_CATCH(exc)
   {
      if (!IsConnectionOK() || strstr(exc.get_cause(), "PipeError"))
	 ProgramDied();
      
      if (instance.contains(id)) instance.del(id);

      fprintf(stderr, error_msg, exc.get_cause());
      return NPERR_GENERIC_ERROR;
   } G_ENDCATCH;
   return NPERR_NO_ERROR;
}

NPError
NPP_Destroy(NPP np_inst, NPSavedData ** save)
{
   DEBUG_MSG("NPP_Destroy() called\n");
   DEBUG_MAKE_INDENT(3);
   
   G_TRY {
      void * id=np_inst->pdata;
      if (!instance.contains(id))
      {
	 DEBUG_MSG("id=" << id << " is illegal\n");
	 return NPERR_INVALID_INSTANCE_ERROR;
      } else DEBUG_MSG("id=" << id << " is legal\n");
      
	 // Detach the main window, if not already detached
      NPP_SetWindow(np_inst, 0);
      instance.del(id);
      np_inst->pdata=0;
      
      if (IsConnectionOK())
      {
	 WriteInteger(pipe_write, CMD_DESTROY);
	 WritePointer(pipe_write, id);
	 ReadResult(pipe_read, "DESTROY", rev_pipe, Refresh_cb);
	 
	 SavedData saved_data;
	 saved_data.cmd_mode=ReadInteger(pipe_read);
	 saved_data.cmd_zoom=ReadInteger(pipe_read);
	 saved_data.imgx=ReadInteger(pipe_read);
	 saved_data.imgy=ReadInteger(pipe_read);
	 
	 if (!*save && saved_data.cmd_mode>0 &&
	     saved_data.cmd_zoom>0)
	 {
	    DEBUG_MSG("saving data into NPSavedData\n");
	    SavedData *data = (SavedData*) NPN_MemAlloc(sizeof(SavedData));
	    NPSavedData *saved = (NPSavedData*) NPN_MemAlloc(sizeof(NPSavedData));
	    if (saved && data)
	    {
	       *data=saved_data;
	       saved->len = sizeof(SavedData);
	       saved->buf = (void*)data;
	       *save = saved;
	    }
	 }
      }
   } G_CATCH(exc) {
      if (!IsConnectionOK() || strstr(exc.get_cause(), "PipeError"))
	 ProgramDied();
      
      fprintf(stderr, error_msg, exc.get_cause());
      return NPERR_GENERIC_ERROR;
   } G_ENDCATCH;
   return NPERR_NO_ERROR;
}

NPError
NPP_SetWindow(NPP np_inst, NPWindow * win_str)
{
   DEBUG_MSG("NPP_SetWindow() called\n");
   DEBUG_MAKE_INDENT(3);
   
   G_TRY
   {
      void * id=np_inst->pdata;
      if (!instance.contains(id))
      {
	 DEBUG_MSG("id=" << id << " is illegal\n");
	 return NPERR_INVALID_INSTANCE_ERROR;
      } else DEBUG_MSG("id=" << id << " is legal\n");
      
      Window cur_window=instance[id].window;
      Window new_window=win_str ? (Window) win_str->window : 0;
      
      if (cur_window)
      {
	 if (new_window==cur_window)
	 {
	    DEBUG_MSG("new window is the same as current\n");
	    Resize(id);
	    return NPERR_NO_ERROR;
	 }
	 Detach(id);
      }
      if (new_window)
      {
	 if (!IsConnectionOK()) return NPERR_GENERIC_ERROR;
	 DEBUG_MSG("attaching new window\n");
	 NPSetWindowCallbackStruct * cbs=(NPSetWindowCallbackStruct *) win_str->ws_info;
	 Display * displ=cbs->display;
	 Attach(displ, new_window, id);
      }
   } G_CATCH(exc)
   {
      if (!IsConnectionOK() || strstr(exc.get_cause(), "PipeError"))
	 ProgramDied();
      
      fprintf(stderr, error_msg, exc.get_cause());
      return NPERR_GENERIC_ERROR;
   } G_ENDCATCH;
   return NPERR_NO_ERROR;
}

void
NPP_Print(NPP np_inst, NPPrint* printInfo)
{
   DEBUG_MSG("NPP_Print() called\n");
   DEBUG_MAKE_INDENT(3);
   
   G_TRY
   {
      void * id=np_inst->pdata;
      if (!instance.contains(id))
      {
	 DEBUG_MSG("id=" << id << " is illegal\n");
	 return;
      } else DEBUG_MSG("id=" << id << " is legal\n");
      
      Instance & inst=instance[id];
      if (inst.widget)
      {
	 DEBUG_MSG("plugin is attached\n");
	 if (printInfo && printInfo->mode==NP_FULL)
	    printInfo->print.fullPrint.pluginPrinted=TRUE;
	 
	 if (IsConnectionOK())
	 {
	    WriteInteger(pipe_write, CMD_PRINT);
	    WritePointer(pipe_write, id);
	    WriteInteger(pipe_write, printInfo && printInfo->mode==NP_FULL);
	    ReadResult(pipe_read, "PRINT", rev_pipe, Refresh_cb);
	 }
      } else DEBUG_MSG("plugin is not attached\n");
   } G_CATCH(exc)
   {
      if (!IsConnectionOK() || strstr(exc.get_cause(), "PipeError"))
	 ProgramDied();
      
      fprintf(stderr, error_msg, exc.get_cause());
      return;
   } G_ENDCATCH;
}

NPError
NPP_NewStream(NPP np_inst, NPMIMEType type, NPStream *stream,
	      NPBool seekable, uint16 *stype)
{
   if (!IsConnectionOK()) return NPERR_GENERIC_ERROR;
   DEBUG_MSG("NPP_NewStream() called\n");
   DEBUG_MAKE_INDENT(3);
   
   void * sid=0;
   G_TRY
   {
      void * id=np_inst->pdata;
      if (!instance.contains(id))
      {
	 DEBUG_MSG("id=" << id << " is illegal\n");
	 return NPERR_INVALID_INSTANCE_ERROR;
      } else DEBUG_MSG("id=" << id << " is legal\n");
      
      WriteInteger(pipe_write, CMD_NEW_STREAM);
      WritePointer(pipe_write, id);
      WriteString(pipe_write, stream->url);
      ReadResult(pipe_read, "NEW_STREAM", rev_pipe, Refresh_cb);
      
      sid=ReadPointer(pipe_read);
      stream->pdata=sid;
	 // ZERO sid means, that we do not want this stream
      if (sid) strinstance[sid]=1;
   } G_CATCH(exc)
   {
      if (!IsConnectionOK() || strstr(exc.get_cause(), "PipeError"))
	 ProgramDied();
      
      if (strinstance.contains(sid)) strinstance.del(sid);
      
      fprintf(stderr, error_msg, exc.get_cause());
      return NPERR_GENERIC_ERROR;
   } G_ENDCATCH;
   return NPERR_NO_ERROR;
}

int32
NPP_WriteReady(NPP, NPStream *)
{
   return 0x7fffffff;
}

int32
NPP_Write(NPP, NPStream *stream, int32, int32 len, void *buffer)
{
   if (!IsConnectionOK()) return NPERR_GENERIC_ERROR;
   DEBUG_MSG("NPP_Write() called\n");
   DEBUG_MAKE_INDENT(3);
   
   int res=0;
   G_TRY
   {
      void * sid=stream->pdata;
      if (!sid)
      {
	 DEBUG_MSG("ZERO ID => do not pipe the data\n");
	 res=len;	// Don't even pass the data
      } else
      {
	 if (!strinstance.contains(sid))
	 {
	    DEBUG_MSG("sid=" << sid << " is illegal\n");
	    return res;
	 } else DEBUG_MSG("sid=" << sid << " is legal\n");

	 TArray<char> data(len-1);
	 memcpy((char *) data, buffer, len);
      
	 WriteInteger(pipe_write, CMD_WRITE);
	 WritePointer(pipe_write, sid);
	 WriteArray(pipe_write, data);
	 ReadResult(pipe_read, "WRITE", rev_pipe, Refresh_cb);
      
	 res=ReadInteger(pipe_read);

	    // ZERO means, that the stream ID sid is meaningless to
	    // the viewer. This may happen from time to time. Especially
	    // when user hits Reload or when viewer dies and another one
	    // is started back (of course, the new instance of the viewer
	    // knows nothing about streams or plugins, which were active
	    // in the viewer, which just died)
	 if (res==0) strinstance.del(sid);
      }
   } G_CATCH(exc)
   {
      if (!IsConnectionOK() || strstr(exc.get_cause(), "PipeError"))
	 ProgramDied();
      
      fprintf(stderr, error_msg, exc.get_cause());
      return res;
   } G_ENDCATCH;
   return res;
}

NPError
NPP_DestroyStream(NPP, NPStream *stream, NPError reason)
{
   if (!IsConnectionOK()) return NPERR_GENERIC_ERROR;
   DEBUG_MSG("NPP_DestroyStream() called\n");
   DEBUG_MAKE_INDENT(3);
   
   G_TRY
   {
      void * sid=stream->pdata;
      if (!strinstance.contains(sid))
      {
	 DEBUG_MSG("sid=" << sid << " is NOT legal\n");
	 return NPERR_INVALID_INSTANCE_ERROR;
      } else DEBUG_MSG("sid=" << sid << " is legal\n");
      strinstance.del(sid);
      
      WriteInteger(pipe_write, CMD_DESTROY_STREAM);
      WritePointer(pipe_write, sid);
      WriteInteger(pipe_write, reason==NPRES_DONE);
      ReadResult(pipe_read, "DESTROY_STREAM", rev_pipe, Refresh_cb);
   } G_CATCH(exc)
   {
      if (!IsConnectionOK() || strstr(exc.get_cause(), "PipeError"))
	 ProgramDied();
      
      fprintf(stderr, error_msg, exc.get_cause());
      return NPERR_GENERIC_ERROR;
   } G_ENDCATCH;
   return NPERR_NO_ERROR;
}

/*******************************************************************************
***************************** Stuff, really stuff ******************************
*******************************************************************************/

void
NPP_StreamAsFile(NPP np_inst, NPStream *stream, const char* fname)
{
   DEBUG_MSG("NPP_StreamAsFile() called\n");
   // Unused
}

int16
NPP_HandleEvent(NPP np_inst, void* event)
{
   DEBUG_MSG("NPP_HandleEvent() called\n");
   // Unused
   return 0;
}

void
NPP_URLNotify(NPP, const char * url, NPReason reason, void *)
{
   if (!IsConnectionOK()) return;
   DEBUG_MSG("NPP_URLNotify() called, url='" << url << "', reason=" <<
	     (reason==NPRES_DONE ? "NPRES_DONE" :
	      reason==NPRES_USER_BREAK ? "NPRES_USER_BREAK" :
	      reason==NPRES_NETWORK_ERR ? "NPRES_NETWORK_ERRK" :
	      "UNKNOWN") << "\n");
   DEBUG_MAKE_INDENT(3);
   
   G_TRY
   {
      WriteInteger(pipe_write, CMD_URL_NOTIFY);
      WriteString(pipe_write, url);
      WriteInteger(pipe_write,
		   reason==NPRES_DONE ? 0 :
		   reason==NPRES_USER_BREAK ? 1 : 2);
      ReadResult(pipe_read, "URL_NOTIFY", rev_pipe, Refresh_cb);
   } G_CATCH(exc)
   {
      if (!IsConnectionOK() || strstr(exc.get_cause(), "PipeError"))
	 ProgramDied();
      
      fprintf(stderr, error_msg, exc.get_cause());
   } G_ENDCATCH;
}

char *
NPP_GetMIMEDescription(void)
{
   return "image/x-djvu:djvu,djv:DjVu File;"
          "image/x.djvu::DjVu File;"
          "image/x-dejavu::DjVu File (obsolete mime type);"
          "image/x-iw44:iw44,iw4:DjVu File (obsolete mime type);"
      	  "image/djvu::DjVu File;"
      	  "image/x-djvu:djvu,djv:DjVu File";
}

jref
NPP_GetJavaClass(void)
{
   return NULL;
}

NPError
NPP_GetValue(void *future, NPPVariable variable, void *value)
{
   NPError err = NPERR_NO_ERROR;

   switch (variable)
   {
      case NPPVpluginNameString:
         *((char **)value) = "LizardTech, Inc.";
         break;
      case NPPVpluginDescriptionString:
         *((char **)value) =
           "LizardTech, Inc. DjVu Plug-In (Version "
#ifdef DJVIEW_VERSION_STR
           DJVIEW_VERSION_STR 
#else
	   DJVU_VERSION " (EXPERIMENTAL)"
#endif
           ") for displaying compressed DjVu images";
         break;
      default:
         err = NPERR_GENERIC_ERROR;
   }
   return err;
}
