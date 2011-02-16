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

#undef DEBUG

#include "qx_imager.h"
#include "qt_painter.h"
#include "debug.h"
#include "col_db.h"

#undef DEBUG

#include <qapplication.h>
#include <qwidget.h>
#include "qlib.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#ifdef USE_XSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

QXImager	* qxImager;

#ifdef USE_XSHM
static bool	xshm_supported;
#endif

//******************************** DXImage ************************************
class DXImage : public GPEnabled
{
   friend class	QXImager;
private:
   Display	* displ;
   XImage	* ximage;
public:
#ifdef USE_XSHM
   static GMap<u_long, GP<DXImage> >	map;

   XShmSegmentInfo* shminfo;
   
   int		created_as_shared;	// If the pool is a shared memory
public:
   int		use_as_shared;		// If can still be used with XShm...()

   DXImage(void) : displ(0), ximage(0), shminfo(0),
      created_as_shared(0), use_as_shared(0) {}
   DXImage(Display * _displ, XImage * _ximage, XShmSegmentInfo * _shminfo) :
	 displ(_displ), ximage(_ximage), shminfo(_shminfo),
	 created_as_shared(0), use_as_shared(_shminfo!=0) {}
#else
   DXImage(void) : displ(0), ximage(0) {}
   DXImage(Display * _displ, XImage * _ximage) :
	 displ(_displ), ximage(_ximage) {}
#endif

   char *	createPool(void);
   operator XImage*(void) { return ximage; }
   
   virtual ~DXImage(void);
};

#ifdef USE_XSHM
GMap<u_long, GP<DXImage> >	DXImage::map;

char *
DXImage::createPool(void)
{
   if (!ximage->data)
   {
      int size=ximage->bytes_per_line*ximage->height;
      if (shminfo && use_as_shared && xshm_supported)
      {
	 if ((shminfo->shmid=shmget(IPC_PRIVATE, size, IPC_CREAT | 0777))>=0)
	 {
	    void * addr;
	    if ((addr=shmat(shminfo->shmid, 0, 0))!=(void *) -1)
	    {
	       shminfo->shmaddr=(char *) addr;
	       shminfo->readOnly=True;
	       if (XShmAttach(displ, shminfo))
	       {
		  created_as_shared=1;
		  ximage->data=(char *) addr;
		  DEBUG_MSG("DXImage::createPool(): successfully created shared memory\n");
		  return ximage->data;
	       }
	       shmdt((char *) addr);
	    }
	    shmctl(shminfo->shmid, IPC_RMID, 0);
	 }
      }
      DEBUG_MSG("DXImage::createPool(): have to use conventional memory\n");
      created_as_shared=0;
      use_as_shared=0;
      if (!(ximage->data=new char[size]))
	 G_THROW("Not enough memory to create XImage.");
   }
   return ximage->data;
}
#else
char *
DXImage::createPool(void)
{
   if (!ximage->data)
   {
      if (!(ximage->data=new char[ximage->bytes_per_line*ximage->height]))
        G_THROW("Not enough memory to create XImage.");
   }
   return ximage->data;
}
#endif
   
DXImage::~DXImage(void)
{
#ifdef USE_XSHM
   if (shminfo)
   {
      if (created_as_shared)
      {
	 XShmDetach(displ, shminfo);
	 shmdt(shminfo->shmaddr);
	 shmctl(shminfo->shmid, IPC_RMID, 0);
	 if (ximage) ximage->data=0;
      }
      delete shminfo;
   }
#endif
   if (ximage)
   {
      delete [] ximage->data; ximage->data=0;
      XDestroyImage(ximage);
      ximage=0;
   }
}

#ifdef USE_XSHM
//******************************* XShmInitializer ******************************

// This class is actually an initializer of dispatching routines and XShm
// extension. Unfortunately it quite difficult to make Intrinsic pay attention
// to XEvents coming from Xserver extensions. This class does it.
//
// It also tests, that XShm extension is really available by creating a dummy
// image and by getting a piece of root window. To guess if you're working thru
// the net, it's not enough to call XShmQueryExtention(), or to create and
// attach image. It's important to *use* it and watch for errors. Stupid.

class XShmInitializer : public QObject
{
  Q_OBJECT
private:
  Display	* displ;
  XExtCodes	* ext_info;
  XErrorHandler old_eh;
  static int	XShmErrorHandler_cb(Display *, XErrorEvent *);
 private slots:
 void		gotX11Event(XEvent * ev);
public:
  int		IsInitialized(void) const { return displ!=0; }
  int		GetFirstEvent(void) const { return ext_info->first_event; }
  void		Initialize(Display * displ);
  
  XShmInitializer(void) : displ(0), ext_info(0), old_eh(0) {}
  ~XShmInitializer(void) {}
};

static XShmInitializer shm_initializer;

void
XShmInitializer::gotX11Event(XEvent * ev)
      // We assume here, that QApplication is QeApplication
      // We don't check this fact here not to waste time
{
   try
   {
      if (ev->type>=ext_info->first_event &&
	  ev->type<ext_info->first_event+ShmNumberEvents)
      {
	 DEBUG_MSG("XShmInitialized::gotX11Event(): type=" << ev->type << "\n");
	 DEBUG_MAKE_INDENT(3);
	 DEBUG_MSG("XShm events range=" << ext_info->first_event << "..." <<
		   (ext_info->first_event+ShmNumberEvents) << "\n");
      
	 XShmCompletionEvent * shmev=(XShmCompletionEvent *) ev;
	 if (DXImage::map.contains(shmev->shmseg))
	 {
	    DEBUG_MSG("destroying image\n");
	    DXImage::map.del(shmev->shmseg);
	 }
      }
      ((QeApplication *) qApp)->x11EventResult=0;
   } catch(const GException & exc)
   {
      const QString qmesg(exc.get_cause());
      showError(0, "DjVu error", qmesg);
   }
}

int
XShmInitializer::XShmErrorHandler_cb(Display * displ, XErrorEvent * error)
{
  DEBUG_MSG("XShmErrorHandler_cb(): Got error in XShmAttach()/XShmGetImage()\n");
  if (error->request_code == shm_initializer.ext_info->major_opcode)
    {
      xshm_supported=false;
      return 0;
    } 
  if (shm_initializer.old_eh)
    return shm_initializer.old_eh(displ, error);
  // We should never get there.
  return 0;
}

void
XShmInitializer::Initialize(Display * _displ)
{
   if (!displ)
     {
       DEBUG_MSG("XShmInitializer::Initialize(): Initializing...\n");
       displ=_displ;
     } 
   else
     {
       DEBUG_MSG("XShmInitializer::Initialize(): Already initialized...\n");
       return;
     }
   DEBUG_MAKE_INDENT(3);
   // Test if we have a QeApplication
   if (!qApp->inherits("QeApplication"))
     {
       DEBUG_MSG("QeApplication must be used => disabling shm\n");
       xshm_supported = false;
       return;
     }
   // Test presence of extension code
   ext_info = XInitExtension(displ, "MIT-SHM");
   if (!ext_info)
     {
       DEBUG_MSG("XInitExtension(\"MIT-SHM\") failed ==> disabling shm\n");
       xshm_supported = false;
       return;
     }
   // Test even more
   if (! XShmQueryExtension(displ))
     {
       DEBUG_MSG("XShmQueryExtension failed ==> disabling shm\n");
       xshm_supported = false;
       return;
     }
   DEBUG_MSG("Looks like server supports XShm extension...\n");
   DEBUG_MSG("Let's check if we can use it...\n");
   // Unfortunately test above is not sufficient. I need to try to create
   // a dummy shared image and attach it. Only in this way I can guess the
   // case when I work across network.
   XShmSegmentInfo * shminfo = new XShmSegmentInfo;
   XImage * ximage;
   int width=32;
   int height=32;
   int screen_num=DefaultScreen(displ);
   int depth=DefaultDepth(displ, screen_num);
   old_eh=XSetErrorHandler(XShmErrorHandler_cb);
   xshm_supported=true;
   if ((ximage=XShmCreateImage(displ, DefaultVisual(displ, screen_num),
                               depth, ZPixmap, 0, shminfo, width, height)))
     {
       DEBUG_MAKE_INDENT(3);
       DEBUG_MSG("*** image created\n");
       if ((shminfo->shmid=shmget(IPC_PRIVATE, ximage->width*
                                  ximage->bytes_per_line,
                                  IPC_CREAT | 0777))>=0)
	 {
           DEBUG_MSG("*** shmem created\n");
           void * addr;
           if ((addr=shmat(shminfo->shmid, 0, 0))!=(void *) -1)
             {
               DEBUG_MSG("*** shmem attached\n");
	       shminfo->shmaddr=ximage->data=(char *) addr;
	       shminfo->readOnly=False;
	       int rc=XShmAttach(displ, shminfo);
	       XSync(displ, False);
	       if (!rc) 
                 xshm_supported=false;	// also reset by XShmErrorHandler_cb
	       if (xshm_supported)
                 {
                   DEBUG_MSG("*** shmem attached by server\n");
                   Pixmap pixmap=XCreatePixmap(displ, RootWindow(displ, screen_num),
                                               width, height, depth);
                   if (XShmGetImage(displ, pixmap, ximage, 0, 0, 0xffffffff))
                     {
                       XSync(displ, False);
                       DEBUG_MSG("*** got image\n");
                       GC gc=XCreateGC(displ, pixmap, 0, 0);
                       if (XShmPutImage(displ, pixmap, gc, ximage,
                                        0, 0, 0, 0, width, height, False))
                         {
                           XSync(displ, False);
                           DEBUG_MSG("*** put image\n");
                         } else xshm_supported=0;
                       XFreeGC(displ, gc);
                     } else xshm_supported=0;
                   XFreePixmap(displ, pixmap);
                   XShmDetach(displ, shminfo);
                   XSync(displ, False);
                 } // if (XShmAttach())
	       shmdt((char *) addr);
             } // if (shmat())
           shmctl(shminfo->shmid, IPC_RMID, 0);
	 }
       ximage->data=0;
       XDestroyImage(ximage);
     }
   delete shminfo;
   // Time to wrap up
   if (xshm_supported)
     {
       DEBUG_MSG("cool. dummy image initialized and successfully attached\n");
       DEBUG_MSG("it means (?), that XShm is supported...\n");
       DEBUG_MSG("major opcode=" << ext_info->major_opcode << "\n");
       DEBUG_MSG("first event=" << ext_info->first_event << "\n");
       connect(qApp, SIGNAL(gotX11Event(XEvent *)),
               this, SLOT(gotX11Event(XEvent *)));
     } 
   else
     {
       DEBUG_MSG("XShm does not work (could be a network connection) ==> disabling shm\n");
     }
}

#endif   // USE_XSHM


//***************************** QXImager *************************************

u_int32
QXImager::allocateCell(int r, int g, int b)
{
   if (!colormap_cells)
      colormap_cells=new XColor[256];
   XColor * cmap_cells=(XColor *) colormap_cells;
   
   XColor cell;
   cell.flags=DoRed | DoGreen | DoBlue;
   cell.red=r;
   cell.green=g;
   cell.blue=b;
   
   if (colormap_full || !XAllocColor(displ, colormap, &cell))
   {
	 // Failed to allocate exactly this color.
	 // Let's try to find smth close to what we
	 // need and then allocate it
      DEBUG_MSG("Failed to alloc color (" << cell.red <<
		", " << cell.green << ", " << cell.blue 
		<< ")=>trying to find the closest\n");
      if (!colormap_full)
      {
	    // Query colors in the current colormap
	 int pix;
	 memset(cmap_cells, 0, sizeof(XColor)*256);
	 for(pix=0;pix<256;pix++)
	    cmap_cells[pix].pixel=pix;
	 XQueryColors(displ, colormap, cmap_cells, 256);
	 colormap_full=true;
      }
		  
      int closest_distance=0xffff;
      XColor closest_cell = cmap_cells[0];
      for(int pix=0;pix<256;pix++)
      {
	 XColor & c=cmap_cells[pix];
	 if (c.flags)
	 {
	    int distance=abs(c.red-cell.red)+
			 abs(c.green-cell.green)+
			 abs(c.blue-cell.blue);
	    if (closest_distance>distance)
	    {
	       closest_distance=distance;
	       closest_cell=c;
	    }
	 }
      }
		  
      DEBUG_MSG("Instead of (" << cell.red <<
		", " << cell.green << ", " <<
		cell.blue << ") got allocated (" <<
		closest_cell.red << ", " <<
		closest_cell.green << ", " <<
		closest_cell.blue << ")\n");
      int distance=abs(closest_cell.red-cell.red)+
		   abs(closest_cell.green-cell.green)+
		   abs(closest_cell.blue-cell.blue);
      if (distance>max_distance) max_distance=distance;
		  
      cell=closest_cell;
      if (XAllocColor(displ, colormap, &cell))
	 allocated_color[allocated_colors++]=cell.pixel;
      else cell=closest_cell;
   } // if (!XAllocColor()...
   
   return cell.pixel;
}

QXImager::~QXImager(void)
{
#ifdef USE_XSHM
  DXImage::map.empty();
#endif
  if (allocated_colors) 
    XFreeColors(displ, colormap, allocated_color,
                allocated_colors, 0);
  qxImager = 0;
}

QXImager::QXImager(Display * _displ, void * _visual,
		   unsigned long _colormap, int _depth,
		   bool _in_netscape, bool _optimizeLCD) :
      in_netscape(_in_netscape), displ(_displ), visual(_visual),
      colormap(_colormap), depth(_depth), is_color(true),
      optimizeLCD(_optimizeLCD)
{
   DEBUG_MSG("QXImager::QXImager(): initializing color/rendering stuff\n");
   DEBUG_MAKE_INDENT(3);

   colormap_full=false;
   max_distance=0;
   colormap_cells=0;
   
   if (qxImager)
      G_THROW("Internal error: Attempt to initialize QXImager twice.");

   visual=_visual;	// Set the class variable (of type void *)
			// And use local variable of correct type
   Visual * visual=(Visual *) _visual;
   
   allocated_colors=0;
   colormap_warned=0;
   
   u_int32 order=1 << 24;
   machine_byte_order=*((u_char *) &order) ? MSBFirst : LSBFirst;
   DEBUG_MSG("machine_byte_order=" <<
	     (machine_byte_order==MSBFirst ? "MSBFirst" : "LSBFirst") << "\n");
#ifdef DEBUG
   DEBUG_MSG("depth=" << depth << "\n");
   switch(visual->c_class)
   {
      case GrayScale: DEBUG_MSG("visual=GrayScale (rw)\n"); break;
      case StaticGray: DEBUG_MSG("visual=StaticGray (ro)\n"); break;
      case PseudoColor: DEBUG_MSG("visual=PseudoColor (rw)\n"); break;
      case StaticColor: DEBUG_MSG("visual=StaticColor (ro)\n"); break;
      case DirectColor: DEBUG_MSG("visual=DirectColor (rw)\n"); break;
      case TrueColor: DEBUG_MSG("visual=TrueColor (ro)\n"); break;
   }
#endif
   
   // Catch some funny cases
   if (depth<8)
   {
      char buffer[1024];
      sprintf(buffer, "Depth %d is not supported.", depth);
      if (in_netscape) 
	strcat(buffer, 
	       "\n\nPlease rerun Netscape with '-visual' flag to change the\n"
	       "visual type or switch to a different hardware.");
      G_THROW(buffer);
   }
   if (visual->c_class==PseudoColor && depth>=15)
     {
       char buffer[1024];
       sprintf(buffer, "Visual PseudoColor is not supported for depth %d.", depth);
       if (in_netscape) 
	 strcat(buffer, 
		"\n\nPlease rerun Netscape with '-visual' flag to change the\n"
		"visual type or switch to a different hardware.");
       G_THROW(buffer);
     }
   if (visual->c_class==StaticColor && depth>=15)
     {
       char buffer[1024];
       sprintf(buffer, "Visual StaticColor is not supported for depth %d.", depth);
       if (in_netscape) 
	 strcat(buffer, 
		"\n\nPlease rerun Netscape with '-visual' flag to change the\n"
		"visual type or switch to a different hardware.");
       G_THROW(buffer);
     }
   if (visual->c_class==DirectColor && depth>=15)
     {
       char buffer[1024];
       sprintf(buffer, "Visual DirectColor is not supported for depth %d.", depth);
       if (in_netscape) 
	 strcat(buffer, 
		"\n\nPlease rerun Netscape with '-visual' flag to change the\n"
		"visual type or switch to a different hardware.");
       G_THROW(buffer);
     }
   if (visual->c_class==StaticGray && depth!=8)
     {
       char buffer[1024];
       sprintf(buffer, "Visual StaticGray is not supported for depth %d.", depth);
       if (in_netscape) 
	 strcat(buffer, 
		"\n\nPlease rerun Netscape with '-visual' flag to change the\n"
		"visual type or switch to a different hardware.");
       G_THROW(buffer);
     }
   if (visual->c_class==GrayScale && depth!=8)
   {
     char buffer[1024];
      sprintf(buffer, "Visual GrayScale is not supported for depth %d.", depth);
      if (in_netscape) 
	strcat(buffer, 
	       "\n\nPlease rerun Netscape with '-visual' flag to change the\n"
	       "visual type or switch to a different hardware.");
      G_THROW(buffer);
   }

   is_color=!(visual->c_class==GrayScale || visual->c_class==StaticGray);
   
   if (depth>=8 && depth<15)
   {
	 // For these depths we want to ALLOCATE every color.
      if (!is_color)
      {
	 int i;
	 
	 bool done[256];
	 for(i=0;i<256;i++)
	    done[i]=false;

	    // Allocate as many gray colors as possible
	 for(int step=128;step>0;step--)
	    for(int i=0;i<256;i+=step)
	       if (!done[i])
	       {
		  done[i]=true;

		  int comp=i<<8;
		  table8[i]=allocateCell(comp, comp, comp);
	       }

	    // Now set up the table8_idx_*[] stuff so that it can be used
	    // in this grayscale mode as well
	 for(i=0;i<256;i++)
	    table8_idx_red[i]=(i*20)>>6;
	 for(i=0;i<256;i++)
	    table8_idx_green[i]=i>>1;
	 for(i=0;i<256;i++)
	    table8_idx_blue[i]=i-(table8_idx_red[i]+table8_idx_green[i]);
      } else
      {
	 DEBUG_MSG("processing depth " << depth << " using 8-bit model\n"
		   "(allocation of each color and 8-bit dithering\n");
	 int i;
	 int j=0;
	 for(i=0;i<6;i++)
	    for(;j<(i+1)*0x33 && j<256;j++)
	    {
	       table8_idx_red[j]=i*6*6;
	       table8_idx_green[j]=i*6;
	       table8_idx_blue[j]=i;
	    }
      
	 for(i=0;i<6;i++)
	    for(j=0;j<6;j++)
	       for(int k=0;k<6;k++)
		  table8[k+6*(j+6*i)]=allocateCell(i*0x3333, j*0x3333, k*0x3333);
      }
      
      if (max_distance>256 && !colormap_warned)
      {
	 colormap_warned=1;
	 if (in_netscape)
	    showInfo(0, "DjVu: Colormap problem",
		     "Due to the lack of colors in the currently installed colormap\n"
		     "we have to use color approximation, which significantly\n"
		     "spoils the picture quality.\n\n"
		     "To fix the problem please either use 16-bit display or\n"
		     "run Netscape with '-install' switch.");
	 else
	    showInfo(0, "DjVu: Colormap problem",
		     "Due to the lack of colors in the currently installed colormap\n"
		     "we have to use color approximation, which significantly\n"
		     "spoils the picture quality.\n\n"
		     "To fix the problem please either use 16-bit or better display or\n"
		     "run the program with '-install' switch.");
      }
   } else // if (depth>=8 && depth<15)...
   if (depth>=15 && depth<24)
   {
      // For these depths we use 15-bit dithering. We also assume,
      // that the visual is either TrueColor or DirectColor, which
      // allows us to build the mask tables here. No color allocation
      // is used. The masks are prepared in the machine-native
      // format. This (and the fact, that image is also prepared in
      // machine-native format) will allow us to copy 2 or 3 bytes at
      // a time.
      DEBUG_MSG("processing depth=" << depth << " using 16-bit model\n");
      
      {
	 DEBUG_MSG("creating table16_red\n");
	 u_int32 mask=visual->red_mask;
	 DEBUG_MSG("red mask=" << mask << "\n");
	 int shift;
	 for(shift=0;(mask & 1)==0;shift++) mask=mask>>1;
	 for(u_int32 _mask=mask;_mask & 1;shift++) _mask=_mask>>1;
	 shift-=5;
	 int j=0;
	 for(int i=0;i<32;i++)
	    for(;j<3+(i+1)*8 && j<256;j++)
	       table16_red[j]=(i & mask) << shift;
      }
      
      {
	 DEBUG_MSG("creating table16_green\n");
	 u_int32 mask=visual->green_mask;
	 DEBUG_MSG("green mask=" << mask << "\n");
	 int shift;
	 for(shift=0;(mask & 1)==0;shift++) mask=mask>>1;
	 for(u_int32 _mask=mask;_mask & 1;shift++) _mask=_mask>>1;
	 shift-=5;
	 int j=0;
	 for(int i=0;i<32;i++)
	    for(;j<3+(i+1)*8 && j<256;j++)
	       table16_green[j]=(i & mask) << shift;
      }
      
      {
	 DEBUG_MSG("creating table16_blue\n");
	 u_int32 mask=visual->blue_mask;
	 DEBUG_MSG("blue mask=" << mask << "\n");
	 int shift;
	 for(shift=0;(mask & 1)==0;shift++) mask=mask>>1;
	 for(u_int32 _mask=mask;_mask & 1;shift++) _mask=_mask>>1;
	 shift-=5;
	 int j=0;
	 for(int i=0;i<32;i++)
	    for(;j<3+(i+1)*8 && i<256;j++)
	       table16_blue[j]=(i & mask) << shift;
      }
   } else // if (depth>=15 || depth<24)
   if (depth>=24 && depth<=32)
   {
      // For these depths we don't use dithering. The colors in the
      // pixmap are assumed to be 24-bit deep. We also assume,
      // that the visual is either TrueColor or DirectColor, which
      // allows us to build the mask tables here. No color allocation
      // is used. The masks are prepared in the machine-native
      // format. This (and the fact, that image is also prepared in
      // machine-native format) will allow us to copy 3 or 4 bytes at
      // a time.
      DEBUG_MSG("processing depth=" << depth << " using 24-bit model\n");
      
      {
	 DEBUG_MSG("creating table24_red\n");
	 u_int32 mask=visual->red_mask;
	 DEBUG_MSG("red mask=" << mask << "\n");
	 int shift;
	 for(shift=0;(mask & 1)==0;shift++) mask=mask>>1;
	 for(u_int32 _mask=mask;_mask & 1;shift++) _mask=_mask>>1;
	 shift-=8;
	 for(int i=0;i<256;i++)
             table24l_red[i] = table24_red[i]=(i & mask) << shift;
         table24l_red[255]=(254 & mask) << shift;
      }
      
      {
	 DEBUG_MSG("creating table24_green\n");
	 u_int32 mask=visual->green_mask;
	 DEBUG_MSG("green mask=" << mask << "\n");
	 int shift;
	 for(shift=0;(mask & 1)==0;shift++) mask=mask>>1;
	 for(u_int32 _mask=mask;_mask & 1;shift++) _mask=_mask>>1;
	 shift-=8;
	 for(int i=0;i<256;i++)
	     table24l_green[i] = table24_green[i] = (i & mask) << shift;
         table24l_green[255]=(254 & mask) << shift;
      }
      
      {
	 DEBUG_MSG("creating table24_blue\n");
	 u_int32 mask=visual->blue_mask;
	 DEBUG_MSG("blue mask=" << mask << "\n");
	 int shift;
	 for(shift=0;(mask & 1)==0;shift++) mask=mask>>1;
	 for(u_int32 _mask=mask;_mask & 1;shift++) _mask=_mask>>1;
	 shift-=8;
	 for(int i=0;i<256;i++)
	     table24l_blue[i] = table24_blue[i] = (i & mask) << shift;
         table24l_blue[255]=(254 & mask) << shift;
      }
   } else // if (depth>=24 && depth<=32)
   if (depth>32)
   {
      char buffer[1024];
      sprintf(buffer,
	      "Sorry, but your hardware has depth of %d, which is\n"
	      "currently unsupported.", depth);
      G_THROW(buffer);
   }
   // Make it current
   qxImager=this;
}

void
QXImager::dither(GPixmap & gpix, int x0, int y0)
{
  if (is_color)
    {
      if (depth<15)
	gpix.ordered_666_dither(x0, y0);
      else if (depth<24)
	gpix.ordered_32k_dither(x0, y0);
    }
}

void
QXImager::setOptimizeLCD(bool _optimizeLCD)
{
   DEBUG_MSG("QXImager::setOptimizeLCD(): optimizeLCD=" << _optimizeLCD << "\n");
   DEBUG_MAKE_INDENT(3);
   if (optimizeLCD!=_optimizeLCD)
      optimizeLCD=_optimizeLCD;
}

QPixmap
QXImager::getColorPixmap(int width, int height,
			 u_char r, u_char g, u_char b)
{
   DEBUG_MSG("QXImager::getColorPixmap(): creating pixmap for (" << (int) r <<
	     ", " << (int) g << ", " << (int) b << ")\n");
   DEBUG_MAKE_INDENT(3);

   GP<GPixmap> gpix=GPixmap::create(width, height);
   GPixmap &pix=*gpix;
   for(u_int i=0;i<pix.rows();i++)
      for(u_int j=0;j<pix.columns();j++)
      {
	 GPixel & p=pix[i][j];
	 p.r=r; p.g=g; p.b=b;
      }
   if (depth<15) pix.ordered_666_dither();
   else if (depth<24) pix.ordered_32k_dither();
      
   QPixmap pixmap(width, height, depth);
   QePainter p(&pixmap);
   bool savOptimizeLCD = optimizeLCD;  // Better avoid LCD mode for
   optimizeLCD = false;                // uniform pixmaps.
   p.drawPixmap(GRect(0, 0, width, height), &pix);
   optimizeLCD = savOptimizeLCD;
   p.end();

   return pixmap;
}

QPixmap
QXImager::getColorPixmap(int width, int height, u_int32 color)
{
   return getColorPixmap(width, height,
			 ColorDB::C32_GetRed(color),
			 ColorDB::C32_GetGreen(color),
			 ColorDB::C32_GetBlue(color));
}

u_int32
QXImager::getGrayXColor(float level)
{
   DEBUG_MSG("QXImager::getGrayXColor(): Looking color for gray level=" << level << "\n");
   if (level<0) level=0;
   if (level>1) level=1;
   int gray=(int) (level*255);
   
   u_int32 pixel=0;
   if (depth<15)
      pixel=table8[table8_idx_red[gray]+
		   table8_idx_green[gray]+
		   table8_idx_blue[gray]];
   if (depth>=15 && depth<24)
      pixel=table16_red[gray] | table16_green[gray] | table16_blue[gray];
   if (depth>=24 && depth<=32)
      pixel=table24_red[gray] | table24_green[gray] | table24_blue[gray];
   return pixel;
}

u_int32
QXImager::getXColor(u_char r, u_char g, u_char b)
{
   DEBUG_MSG("QXImager::getXColor(): Looking color for (" << (int) r << ", " <<
	     (int) g << ", " << (int) b << ")\n");
   
   u_int32 pixel=0;
   if (depth<15)
      pixel=table8[table8_idx_red[r]+
		   table8_idx_green[g]+
		   table8_idx_blue[b]];
   if (depth>=15 && depth<24)
      pixel=table16_red[r] | table16_green[g] | table16_blue[b];
   if (depth>=24 && depth<=32)
      pixel=table24_red[r] | table24_green[g] | table24_blue[b];
   return pixel;
}

u_int32
QXImager::getXColor(u_int32 color)
{
   return getXColor(ColorDB::C32_GetRed(color),
		    ColorDB::C32_GetGreen(color),
		    ColorDB::C32_GetBlue(color));
}

u_int32
QXImager::getXColor(const char * name)
{
   XColor cell;
   if (XParseColor(displ, colormap, name, &cell))
      return getXColor(cell.red>>8, cell.green>>8, cell.blue>>8);
   else return getXColor(255, 255, 255);
}

QColor
QXImager::getGrayColor(float level)
{
   u_long col=getGrayXColor(level);
   u_char comp=(u_char) (255*level);
   return QColor((comp<<16) | (comp<<8) | comp, col);
}

QColor
QXImager::getColor(u_char r, u_char g, u_char b)
{
   u_long col=getXColor(r, g, b);
   return QColor((r<<16) | (g<<8) | b, col);
}

QColor
QXImager::getColor(u_int32 color)
{
   return getColor(ColorDB::C32_GetRed(color),
		   ColorDB::C32_GetGreen(color),
		   ColorDB::C32_GetBlue(color));
}

QColor
QXImager::getColor(const char * name)
{
   XColor cell;
   if (XParseColor(displ, colormap, name, &cell))
      return getColor(cell.red>>8, cell.green>>8, cell.blue>>8);
   else return getColor(255, 255, 255);
}

QColor
QXImager::getColor(const QColor & col)
{
   return getColor(col.red(), col.green(), col.blue());
}

#ifdef USE_XSHM
#define PREPARE_FOR(bits) \
     if (image->use_as_shared)\
     {\
	if (ximage->bits_per_pixel!=bits)\
	{\
	   DEBUG_MSG("blin, bits_per_pixel is bad => disabling shared mode\n");\
	   image->use_as_shared=0;\
	}\
     }\
     if (!image->use_as_shared)\
     {\
	DEBUG_MSG("setting bits_per_pixel=" << bits << " and bytes_per_line=...\n");\
	ximage->bits_per_pixel=bits;\
	ximage->bytes_per_line=ximage->width*(bits/8);\
     }\
     \
     u_char *p, *pbyte;\
     pbyte=p=(u_char *) image->createPool();\
     int bytes_per_line=ximage->bytes_per_line;\
     int bytes_per_pixel=ximage->bits_per_pixel/8;
#else
#define PREPARE_FOR(bits) \
     DEBUG_MSG("setting bits_per_pixel=" << bits << " and bytes_per_line=...\n");\
     ximage->bits_per_pixel=bits;\
     ximage->bytes_per_line=ximage->width*(bits/8);\
     \
     u_char *p, *pbyte;\
     pbyte=p=(u_char *) image->createPool();\
     int bytes_per_line=ximage->bytes_per_line;\
     int bytes_per_pixel=ximage->bits_per_pixel/8;
#endif

void
QXImager::copyPixmap(DXImage * image, const GRect & im_rect,
		     GPixmap *pm, int pm_x0, int pm_y0,
		     int use_shm_extension)
      // Copies part of pixmap 'pm' into the image.
      // It overwrites im_rect in the image and retrieves data from the
      // rectangle of the same size with top-left corner at (pm_x0, pm_y0)
      // of the pixmap. The function trims the rectangle if necessary
      // not to go beyond any image structure.
{
   DEBUG_MSG("QXImager::copyPixmap(): Copying pixmap into DXImage\n");
   DEBUG_MAKE_INDENT(3);

   XImage * ximage=*image;
   
      // Adjust rectangle to fit all image structures
   GRect rect=im_rect;
   if (rect.xmin<0) rect.xmin=0;
   if (rect.ymin<0) rect.ymin=0;
   if (rect.xmax>ximage->width) rect.xmax=ximage->width;
   if (rect.ymax>ximage->height) rect.ymax=ximage->height;
   if (pm_x0+rect.width()>(int) pm->columns()) rect.xmax=(int) pm->columns()-pm_x0+rect.xmin;
   if (pm_y0+rect.height()>(int) pm->rows()) rect.ymax=(int) pm->rows()-pm_y0+rect.ymin;
   if (rect.width()<=0 || rect.height()<=0)
   {
      DEBUG_MSG("there is nothing to display (intersected rect is empty)\n");
      return;
   }
  
      // WARNING: Instead of use_shm_extension use image->use_as_shared everywhere
      // below this point
  
   DEBUG_MSG("width=" << ximage->width << ", height=" << ximage->height << "\n");
   DEBUG_MSG("default byte_order=" << (ximage->byte_order==LSBFirst ? "LSBFirst" : "MSBFirst")<< "\n");
   DEBUG_MSG("default bitmap_unit=" << ximage->bitmap_unit << "\n");
   DEBUG_MSG("default bitmap_bit_order=" << ximage->bitmap_bit_order << "\n");
   DEBUG_MSG("default bytes_per_line=" << ximage->bytes_per_line << "\n");
   DEBUG_MSG("default bits_per_pixel=" << ximage->bits_per_pixel<< "\n");

   if (depth==8)
   {
      DEBUG_MSG("processing depth=" << depth << "\n");
      DEBUG_MSG("creating 8-bit image and using 8-bit color tables\n");
      PREPARE_FOR(8);
      p+=bytes_per_line*rect.ymin;
      
      int yend=pm->rows()-1-pm_y0-rect.height();
      for (int y=pm->rows()-1-pm_y0;y>yend;y--)
      {
	 GPixel *pix = (*pm)[y]+pm_x0;
	 u_char * pstart=p;
	 p+=rect.xmin*bytes_per_pixel;
	 for (int x=rect.width(); x>0; x--, pix++)
	    *p++ = (u_char) table8[table8_idx_red[pix->r]+
				  table8_idx_green[pix->g]+
				  table8_idx_blue[pix->b]];
	 p=pstart+bytes_per_line;
      }
   }
  
   if (depth>8 && depth<=16)
   {
	 // We will use machine native byte order here to take
	 // advantage of 2-bytes-at-a-time copying.
      DEBUG_MSG("processing depth=" << depth << "\n");
      DEBUG_MSG("setting byte_order to machine_byte_order=" <<
		(machine_byte_order==MSBFirst ? "MSBFirst" : "LSBFirst") << "\n");
      ximage->byte_order=machine_byte_order;
      PREPARE_FOR(16);
      p+=bytes_per_line*rect.ymin;
     
      if (depth<15)
      {
	 DEBUG_MSG("creating 16-bit image and using 8-bit color tables\n");
	 int yend=pm->rows()-1-pm_y0-rect.height();
	 for (int y=pm->rows()-1-pm_y0; y>yend; y--)
	 {
	    GPixel *pix = (*pm)[y]+pm_x0;
	    u_char * pstart=p;
	    p+=rect.xmin*bytes_per_pixel;
	    for (int x=rect.width(); x>0; x--, pix++)
	    {
	       *((u_int16 *) p)=table8[table8_idx_red[pix->r]+
				      table8_idx_green[pix->g]+
				      table8_idx_blue[pix->b]];
	       p+=sizeof(u_int16);
	    }
	    p=pstart+bytes_per_line;
	 }
      } else
      {
	 DEBUG_MSG("creating 16-bit image and using 16-bit color tables\n");
	 int yend=pm->rows()-1-pm_y0-rect.height();
	 for (int y=pm->rows()-1-pm_y0; y>yend; y--)
	 {
	    GPixel *pix = (*pm)[y]+pm_x0;
	    u_char * pstart=p;
	    p+=rect.xmin*bytes_per_pixel;
	    for (int x=rect.width(); x>0; x--, pix++)
	    {
	       u_int16 pixel=table16_red[pix->r] |
			     table16_green[pix->g] |
			     table16_blue[pix->b];
	       *((u_int16 *) p)=pixel;
	       p+=sizeof(u_int16);
	    }
	    p=pstart+bytes_per_line;
	 }
      }
   }
  
   if (depth>16 && depth<=32)
   {
	 // We will use machine native byte order here to take
	 // advantage of 4-bytes-at-a-time copying.
      DEBUG_MSG("processing depth=" << depth << "\n");
      DEBUG_MSG("setting byte_order to machine_byte_order=" <<
		(machine_byte_order==MSBFirst ? "MSBFirst" : "LSBFirst") << "\n");
      ximage->byte_order=machine_byte_order;
      PREPARE_FOR(32);
      p+=bytes_per_line*rect.ymin;
     
      if (depth<24)
      {
	 DEBUG_MSG("creating 32-bit image and using 16-bit color tables\n");
	 int yend=pm->rows()-1-pm_y0-rect.height();
	 for (int y=pm->rows()-1-pm_y0; y>yend; y--)
	 {
	    GPixel *pix = (*pm)[y]+pm_x0;
	    u_char * pstart=p;
	    p+=rect.xmin*bytes_per_pixel;
	    for (int x=rect.width(); x>0; x--, pix++)
	    {
	       u_int32 pixel=table16_red[pix->r] |
			     table16_green[pix->g] |
			     table16_blue[pix->b];
	       *((u_int32 *) p)=pixel;
	       p+=sizeof(u_int32);
	    }
	    p=pstart+bytes_per_line;
	 }
      } else
      {
	 u_int32 * table_red=optimizeLCD ? table24l_red : table24_red;
	 u_int32 * table_green=optimizeLCD ? table24l_green : table24_green;
	 u_int32 * table_blue=optimizeLCD ? table24l_blue : table24_blue;
	 
	 int yend=pm->rows()-1-pm_y0-rect.height();
	 for (int y=pm->rows()-1-pm_y0; y>yend; y--)
	 {
	    GPixel *pix = (*pm)[y]+pm_x0;
	    u_char * pstart=p;
	    p+=rect.xmin*bytes_per_pixel;
	    for (int x=rect.width(); x>0; x--, pix++)
	    {
	       u_int32 pixel=table_red[pix->r] |
			     table_green[pix->g] |
			     table_blue[pix->b];
	       *((u_int32 *) p)=pixel;
	       p+=sizeof(u_int32);
	    }
	    p=pstart+bytes_per_line;
	 }
      }
   }
}

#ifdef USE_XSHM
#define CREATE_XIMAGE \
  if (use_shm_extension && rect.width()*rect.height()<2480)\
  {\
     DEBUG_MSG("image is too small => disabling shared mode\n");\
     use_shm_extension=0;\
  }\
  if (use_shm_extension)\
  {\
     if (!shm_initializer.IsInitialized()) shm_initializer.Initialize(displ);\
     if (xshm_supported)\
     {\
	DEBUG_MSG("found XShm extension => trying to use it...\n");\
	if (!QWidget::find((WId) drawable))\
	{\
	   DEBUG_MSG("drawable is not a widget => disabling shared mode\n");\
	   use_shm_extension=0;\
	}\
     } else\
     {\
	DEBUG_MSG("XShm extension is not supported\n");\
	use_shm_extension=0;\
     }\
  }\
  XShmSegmentInfo * shminfo=0;\
  if (use_shm_extension)\
  {\
     shminfo=new XShmSegmentInfo;\
     ximage=XShmCreateImage(displ, (Visual *) visual, depth, ZPixmap, 0, shminfo,\
			    rect.width(), rect.height());\
     if (!shminfo || !ximage)\
	G_THROW("Not enough memory to create XImage.");\
     image=new DXImage(displ, ximage, shminfo);\
  } else\
  {\
     DEBUG_MSG("not using XShm extension as directed...\n");\
     ximage=XCreateImage(displ, (Visual *) visual, depth, ZPixmap, 0, 0,\
			 rect.width(), rect.height(), 8, 0);\
     if (!ximage) \
        G_THROW("Not enough memory to create XImage.");\
     image=new DXImage(displ, ximage, 0);\
  }\
  \
  if (image->use_as_shared && ximage->xoffset)\
  {\
     DEBUG_MSG("blin, ximage->xoffset!=0 => disabling shared mode\n");\
     image->use_as_shared=0;\
  }\
  \
  if (image->use_as_shared && ximage->byte_order!=machine_byte_order)\
  {\
     DEBUG_MSG("blin, ximage->byte_order is weird => disabling shared mode\n");\
     image->use_as_shared=0;\
  }
#else
#define CREATE_XIMAGE \
  ximage=XCreateImage(displ, (Visual *) visual, depth, ZPixmap, 0, 0,\
		      rect.width(), rect.height(), 8, 0);\
  if (!ximage) \
     G_THROW("Not enough memory to create XImage.");\
  image=new DXImage(displ, ximage);
#endif

void
QXImager::displayPixmap(u_long _drawable, GC gc,
			const GRect &dr_rect, int pm_x0, int pm_y0,
			GPixmap *pm, int use_shm_extension)
      // dr_rect is the rectangle to be filled in the drawable
      // (pm_x0, pm_y0) specifies top-level corner of the source
      // rectangle with the same dimention as dr_rect in the pixmap
      // The rectangle will be adjusted if it doesn't fit the pixmap
{
   DEBUG_MSG("QXImager::displayPixmap(): Trying to display the given pixmap\n");
   DEBUG_MAKE_INDENT(3);

   Drawable drawable=(Drawable) _drawable;

      // Check dr_rect and (pm_x0, pm_y0) first:
   GRect rect=dr_rect;
   if (pm_x0+rect.width()>(int) pm->columns()) rect.xmax=rect.xmin+(int) pm->columns()-pm_x0;
   if (pm_y0+rect.height()>(int) pm->rows()) rect.ymax=rect.ymin+(int) pm->rows()-pm_y0;
   if (rect.width()<=0 || rect.height()<=0)
   {
      DEBUG_MSG("there is nothing to display (intersected rect is empty)\n");
      return;
   }
  
   DEBUG_MSG("creating image\n");
  
   XImage * ximage=0;
   GP<DXImage> image;
  
   CREATE_XIMAGE;
  
      // WARNING: Instead of use_shm_extension use image->use_as_shared everywhere
      // below this point
  
   DEBUG_MSG("setting ximage->xoffset to 0\n");
   ximage->xoffset=0;
  
   DEBUG_MSG("width=" << ximage->width << ", height=" << ximage->height << "\n");
   DEBUG_MSG("default byte_order=" << (ximage->byte_order==LSBFirst ? "LSBFirst" : "MSBFirst")<< "\n");
   DEBUG_MSG("default bitmap_unit=" << ximage->bitmap_unit << "\n");
   DEBUG_MSG("default bitmap_bit_order=" << ximage->bitmap_bit_order << "\n");
   DEBUG_MSG("default bytes_per_line=" << ximage->bytes_per_line << "\n");
   DEBUG_MSG("default bits_per_pixel=" << ximage->bits_per_pixel<< "\n");

   copyPixmap(image, GRect(0, 0, ximage->width, ximage->height),
	      pm, pm_x0, pm_y0, use_shm_extension);

#ifdef USE_XSHM
   if (image->use_as_shared)
   {
      DXImage::map[shminfo->shmseg]=image;
      XShmPutImage(displ, drawable, gc, ximage, 0, 0,
		   rect.xmin, rect.ymin, rect.width(), rect.height(), True);
      DEBUG_MSG("sent image using XShm extension\n");
   } else
#endif
   {
      XPutImage(displ, drawable, gc, ximage, 0, 0,
		rect.xmin, rect.ymin, rect.width(), rect.height());
      DEBUG_MSG("sent image in a regular way (w/o XShm extension)\n");
   }
}

void
QXImager::copyBitmap(DXImage * image, const GRect & im_rect,
		     GBitmap * bm, int bm_x0, int bm_y0,
		     int use_shm_extension)
      // Copies part of bitmap 'bm' into the image.
      // It overwrites im_rect in the image and retrieves data from the
      // rectangle of the same size with top-left corner at (bm_x0, bm_y0)
      // of the bitmap. The function trims the rectangle if necessary
      // not to go beyond any image structure.
{
   DEBUG_MSG("QXImager::copyBitmap(): Copying bitmap to DXImage\n");
   DEBUG_MAKE_INDENT(3);

   XImage * ximage=*image;

      // Adjust rectangle to fit all image structures
   GRect rect=im_rect;
   if (rect.xmin<0) rect.xmin=0;
   if (rect.ymin<0) rect.ymin=0;
   if (rect.xmax>ximage->width) rect.xmax=ximage->width;
   if (rect.ymax>ximage->height) rect.ymax=ximage->height;
   if (bm_x0+rect.width()>(int) bm->columns()) rect.xmax=(int) bm->columns()-bm_x0+rect.xmin;
   if (bm_y0+rect.height()>(int) bm->rows()) rect.ymax=(int) bm->rows()-bm_y0+rect.ymin;
   if (rect.width()<=0 || rect.height()<=0)
   {
      DEBUG_MSG("there is nothing to display (intersected rect is empty)\n");
      return;
   }
   
      // WARNING: Instead of use_shm_extension use image->use_as_shared everywhere
      // below this point
  
   DEBUG_MSG("default byte_order=" << (ximage->byte_order==LSBFirst ? "LSBFirst" : "MSBFirst")<< "\n");
   DEBUG_MSG("default bitmap_unit=" << ximage->bitmap_unit << "\n");
   DEBUG_MSG("default bitmap_bit_order=" << ximage->bitmap_bit_order << "\n");
   DEBUG_MSG("default bytes_per_line=" << ximage->bytes_per_line << "\n");
   DEBUG_MSG("default bits_per_pixel=" << ximage->bits_per_pixel<< "\n");

   DEBUG_MSG("Preparing grayscale palette basing on the number of bitmap grays\n");
   u_int32 gray[256];
   int grays=bm->get_grays();
   u_int32 color=0xff0000;
   int decrement=color/(grays-1);
   for(int i=0;i<grays;i++)
   {
      int intensity=color>>16;
      if (depth<15)
	 gray[i]=table8[table8_idx_red[intensity]+
		       table8_idx_green[intensity]+
		       table8_idx_blue[intensity]];
      else if (depth>=15 && depth<24)
	 gray[i]=table16_red[intensity] |
		 table16_green[intensity] |
		 table16_blue[intensity];
      else if (depth>=24 && depth<=32)
      {
	 if (optimizeLCD)
	    gray[i]=table24l_red[intensity] |
		    table24l_green[intensity] |
		    table24l_blue[intensity];
         else
	    gray[i]=table24_red[intensity] |
		    table24_green[intensity] |
		    table24_blue[intensity];
      }
    
      color-=decrement;
   }
   DEBUG_MSG("approximated " << grays << " grays using current color colormap\n");

   if (depth==8)
   {
      DEBUG_MSG("processing depth=" << depth << "\n");
      PREPARE_FOR(8);
      p+=bytes_per_line*rect.ymin;

      int yend=bm->rows()-1-bm_y0-rect.height();
      for (int y=bm->rows()-1-bm_y0; y>yend; y--)
      {
	 u_char *pix = (*bm)[y]+bm_x0;
	 u_char * pstart=p;
	 p+=rect.xmin*bytes_per_pixel;
	 for (int x=rect.width(); x>0; x--, pix++)
	    *p++ = (u_char) gray[*pix];
	 p=pstart+bytes_per_line;
      }
   }
  
   if (depth>8 && depth<=16)
   {
	 // We will use machine native byte order here to take
	 // advantage of 2-bytes-at-a-time copying.
      DEBUG_MSG("processing depth=" << depth << "\n");
      DEBUG_MSG("setting byte_order to machine_byte_order=" <<
		(machine_byte_order==MSBFirst ? "MSBFirst" : "LSBFirst") << "\n");
      ximage->byte_order=machine_byte_order;
      PREPARE_FOR(16);
      p+=bytes_per_line*rect.ymin;

      int yend=bm->rows()-1-bm_y0-rect.height();
      for (int y=bm->rows()-1-bm_y0; y>yend; y--)
      {
	 u_char *pix = (*bm)[y]+bm_x0;
	 u_char * pstart=p;
	 p+=rect.xmin*bytes_per_pixel;
	 for (int x=rect.width(); x>0; x--, pix++)
	 {
	    *((u_int16 *) p)=gray[*pix];
	    p+=sizeof(u_int16);
	 }
	 p=pstart+bytes_per_line;
      }
   }
   
   if (depth>16 && depth<=32)
   {
	 // We will use machine native byte order here to take
	 // advantage of 4-bytes-at-a-time copying.
      DEBUG_MSG("processing depth=" << depth << "\n");
      DEBUG_MSG("setting byte_order to machine_byte_order=" <<
		(machine_byte_order==MSBFirst ? "MSBFirst" : "LSBFirst") << "\n");
      ximage->byte_order=machine_byte_order;
      PREPARE_FOR(32);
      p+=bytes_per_line*rect.ymin;

      int yend=bm->rows()-1-bm_y0-rect.height();
      for (int y=bm->rows()-1-bm_y0; y>yend; y--)
      {
	 u_char *pix = (*bm)[y]+bm_x0;
	 u_char * pstart=p;
	 p+=rect.xmin*bytes_per_pixel;
	 for (int x=rect.width(); x>0; x--, pix++)
	 {
	    *((u_int32 *) p)=gray[*pix];
	    p+=sizeof(u_int32);
	 }
	 p=pstart+bytes_per_line;
      }
   }
}

void
QXImager::displayBitmap(u_long _drawable, GC gc,
			const GRect &dr_rect, int bm_x0, int bm_y0,
			GBitmap *bm, int use_shm_extension)
{
   DEBUG_MSG("QXImager::displayBitmap(): Trying to display given bitmap\n");
   DEBUG_MAKE_INDENT(3);

   Drawable drawable=(Drawable) _drawable;

      // Check dr_rect and (bm_x0, bm_y0) first:
   GRect rect=dr_rect;
   if (bm_x0+rect.width()>(int) bm->columns()) rect.xmax=rect.xmin+(int) bm->columns()-bm_x0;
   if (bm_y0+rect.height()>(int) bm->rows()) rect.ymax=rect.ymin+(int) bm->rows()-bm_y0;
   if (rect.width()<=0 || rect.height()<=0)
   {
      DEBUG_MSG("there is nothing to display (intersected rect is empty)\n");
      return;
   }
  
   DEBUG_MSG("creating image\n");
  
   XImage * ximage=0;
   GP<DXImage> image;
  
   CREATE_XIMAGE;

      // WARNING: Instead of use_shm_extension use image->use_as_shared everywhere
      // below this point
  
   DEBUG_MSG("setting ximage->xoffset to 0\n");
   ximage->xoffset=0;
  
   DEBUG_MSG("default byte_order=" << (ximage->byte_order==LSBFirst ? "LSBFirst" : "MSBFirst")<< "\n");
   DEBUG_MSG("default bitmap_unit=" << ximage->bitmap_unit << "\n");
   DEBUG_MSG("default bitmap_bit_order=" << ximage->bitmap_bit_order << "\n");
   DEBUG_MSG("default bytes_per_line=" << ximage->bytes_per_line << "\n");
   DEBUG_MSG("default bits_per_pixel=" << ximage->bits_per_pixel<< "\n");

   copyBitmap(image, GRect(0, 0, ximage->width, ximage->height),
	      bm, bm_x0, bm_y0, use_shm_extension);

#ifdef USE_XSHM
   if (image->use_as_shared)
   {
      DXImage::map[shminfo->shmseg]=image;
      XShmPutImage(displ, drawable, gc, ximage, 0, 0,
		   rect.xmin, rect.ymin, rect.width(), rect.height(), True);
      DEBUG_MSG("sent image using XShm extension\n");
   } else
#endif
   {
      XPutImage(displ, drawable, gc, ximage, 0, 0,
		rect.xmin, rect.ymin, rect.width(), rect.height());
      DEBUG_MSG("sent image in a regular way (w/o XShm extension)\n");
   }
}

void
QXImager::displayPatchedBitmap(u_long drawable, GC gc,
			       const GRect & bm_rect, int bm_x0,
			       int bm_y0, GBitmap * bm,
			       const GRect & pm_rect, int pm_x0,
			       int pm_y0, GPixmap * pm,
			       int use_shm_extension)
      // Copies contents of bitmap 'bm' in the rectangle bm_rect of the
      // drawable. It takes data from rectangle of the same size with
      // top-left corner at (bm_x0, bm_y0) of the bitmap. Then it uses
      // pm_rect, pm_x0, pm_y0 and pm to output the pixmap on top of
      // the bitmap. Rectangles can go beyond the bitmap or pixmap.
{
   DEBUG_MSG("QXImager::displayPatchedBitmap(): displaying given bitmap and pixmap on top of it\n");
   DEBUG_MAKE_INDENT(3);

   GPList<PatchRect> list;
   list.append(new PatchRect(pm_rect, pm, pm_x0, pm_y0));
   displayPatchedBitmaps(drawable, gc, bm_rect, bm_x0, bm_y0, bm,
			list, use_shm_extension);
}

void
QXImager::displayPatchedBitmaps(u_long _drawable, GC gc,
				const GRect & bm_rect, int bm_x0,
				int bm_y0, GBitmap * bm,
				const GPList<PatchRect> & pm_list,
				int use_shm_extension)
      // Copies contents of bitmap 'bm' in the rectangle bm_rect of the
      // drawable. It takes data from rectangle of the same size with
      // top-left corner at (bm_x0, bm_y0) of the bitmap. Then it repeats
      // this procedure for every pixmap from the pm_list. The 'rect'
      // field in every PatchRect structure is in the drawable's coordinates
{
   DEBUG_MSG("QXImager::displayPatchedBitmaps(): displaying bitmap and " <<
	     pm_list.size() << " pixmaps on top of it\n");
   DEBUG_MAKE_INDENT(3);
   
   Drawable drawable=(Drawable) _drawable;

      // Adjust rectangle using *Bitmap*'s dimensions. Pixmap is assumed
      // to be a "color patch" put onto the bitmap picture. If it doesn't
      // fit, nobody cares. It's the BITMAP which must be displayed in full
   GRect rect=bm_rect;
   if (bm_x0+rect.width()>(int) bm->columns()) rect.xmax=rect.xmin+(int) bm->columns()-bm_x0;
   if (bm_y0+rect.height()>(int) bm->rows()) rect.ymax=rect.ymin+(int) bm->rows()-bm_y0;
   if (rect.width()<=0 || rect.height()<=0)
   {
      DEBUG_MSG("there is nothing to display (intersected rect is empty)\n");
      return;
   }
  
   DEBUG_MSG("creating image\n");
  
   XImage * ximage=0;
   GP<DXImage> image;
  
   CREATE_XIMAGE;

      // WARNING: Instead of use_shm_extension use image->use_as_shared everywhere
      // below this point
  
   DEBUG_MSG("setting ximage->xoffset to 0\n");
   ximage->xoffset=0;
  
   DEBUG_MSG("default byte_order=" << (ximage->byte_order==LSBFirst ? "LSBFirst" : "MSBFirst")<< "\n");
   DEBUG_MSG("default bitmap_unit=" << ximage->bitmap_unit << "\n");
   DEBUG_MSG("default bitmap_bit_order=" << ximage->bitmap_bit_order << "\n");
   DEBUG_MSG("default bytes_per_line=" << ximage->bytes_per_line << "\n");
   DEBUG_MSG("default bits_per_pixel=" << ximage->bits_per_pixel<< "\n");

   GRect im_rect(0, 0, ximage->width, ximage->height);
   copyBitmap(image, im_rect, bm, bm_x0, bm_y0, use_shm_extension);
   for(GPosition pos=pm_list;pos;++pos)
   {
      PatchRect & p=*pm_list[pos];
      im_rect=GRect(p.rect.xmin-bm_rect.xmin,	// Maybe negative
		    p.rect.ymin-bm_rect.ymin,	// Maybe negative
		    p.rect.width(), p.rect.height());
      copyPixmap(image, im_rect, p.pixmap, p.pm_x0, p.pm_y0, use_shm_extension);
   }

#ifdef USE_XSHM
   if (image->use_as_shared)
   {
      DXImage::map[shminfo->shmseg]=image;
      XShmPutImage(displ, drawable, gc, ximage, 0, 0,
		   rect.xmin, rect.ymin, rect.width(), rect.height(), True);
      DEBUG_MSG("sent image using XShm extension\n");
   } else
#endif
   {
      XPutImage(displ, drawable, gc, ximage, 0, 0,
		rect.xmin, rect.ymin, rect.width(), rect.height());
      DEBUG_MSG("sent image in a regular way (w/o XShm extension)\n");
   }
}

#ifdef USE_XSHM
#include "qx_imager_moc.inc"
#endif
