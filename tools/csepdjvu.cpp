//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.5)
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
// 
// $Id$
// $Name$

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/** @name csepdjvu

    {\bf Synopsis}
    \begin{verbatim}
        csepdjvu <....options-or-separated_files...> <outputdjvufile>
    \end{verbatim}

    {\bf Description}
    
    File #"csepdjvu.cpp"# demonstrates a complete back-end encoder that takes
    {\em separated files} as input and produces a djvu file as output.
    Each {\em separated file} contains a concatenation of pages. 
    Each page contains the following components:
    \begin{itemize}
    \item A run-length encoded file representing the foreground.
          Two kind of run-length encoded files are accepted.
          The Black-And-White RLE format is described in section
          \Ref{PNM and RLE file formats}.  The Color RLE format
          is described below.
    \item An optional PPM image representing the background.
          The size (width and height) of the PPM image must be obtained by
          rounding up the quotient of the foreground image size by
          an integer reduction factor ranging from 1 to 12.
    \item An arbitrary number of comment lines starting with 
          character '#'.
    \end{itemize}
    All the provided pages will be converted to Compound DjVu Images.
    Foreground colors will be encoded using a single solid color per component
    (see \Ref{DjVu Image files}).  Multiple pages will be gathered into a
    single bundled file.  Use \Ref{djvmcvt} or \Ref{djvujoin} to create an
    indirect file.
    
    {\bf Options}

    \begin{description}
    \item[-d n] Resolution written into the output file (default: 300).
    \item[-q <spec>] Quality level for background (default: 72+11+10+10).
                     See option #"-slice"# in program \Ref{c44} for details.
    \item[-v] Displays a brief message per page.
    \item[-vv] Displays lots of additional messages.
    \end{description}
          
    {\bf Color RLE Images}

    The Color RLE file format is a simple run-length encoding scheme for color
    images with a limited number of colors. Color RLE files always begin with
    a text header composed of:
    - the two characters #"R6"#,
    - the number of columns in decimal,\\
    - the number of rows in decimal,\\
    - the number of palette entries in decimal.\\
    These four items are separated by blank characters (space, tab, cr, or nl)
    or by comment lines introduced by character ``\#''.  The last number is
    followed by exactly one character (usually a nl character).  This header
    is followed by a palette containing three bytes per color.  The bytes
    represent the red, green, and blue components of the color.  

    The palette is followed by four bytes integers (MSB first) representing
    runs.  The twelve upper bits of this integer indicate the index of the run
    color in the palette entry.  The twenty lower bits of the integer indicate
    the run length.  Color indices greater than 0xff0 are reserved for pixels
    belonging to the background layer.  Color index 0xfff is used for
    transparent runs. Color index 0xffe is used for don't-care runs
    (i.e. pixels whose values should be taken by smoothly completing the
    background using the wavelet masking algorithm).  Each row is represented
    by a sequence of runs whose lengths add up to the image width.  Rows are
    encoded starting with the top row and progressing towards the bottom row.

    @memo
    Creates DjVu files from Separated files.
    @author
    L\'eon Bottou <leonb@research.att.com>
    @version
    #$Id$# */
//@{
//@}


#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif
#include "DjVuGlobal.h"
#include "GException.h"
#include "GSmartPointer.h"
#include "GContainer.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
#include "GRect.h"
#include "GBitmap.h"
#include "JB2Image.h"
#include "DjVuPalette.h"
#include "IW44Image.h"
#include "DjVuInfo.h"
#include "DjVmDoc.h"
#include "GOS.h"
#include "GURL.h"
#include "DjVuMessage.h"
#include <locale.h>
#include <stdlib.h>

#undef MIN
#undef MAX
inline int MIN(int a, int b) { return ( a<b ?a :b); }
inline int MAX(int a, int b) { return ( a>b ?a :b); }



// --------------------------------------------------
// BUFFERED BYTESTREAM
// --------------------------------------------------

// -- A bytestream that performs buffering and 
//    offers a stdio-like interface for reading files.

class BufferByteStream : public ByteStream 
{
public:
	enum {bufsize=512};
private:
  ByteStream &bs;
  unsigned char buffer[bufsize];
  int bufpos;
  int bufend;
public:
  BufferByteStream(ByteStream &lbs);
  size_t read(void *buffer, size_t size);
  size_t write(const void *buffer, size_t size);
  virtual long tell(void) const;
  int eof(void);
  int unget(int c);
  inline int get(void);
};

BufferByteStream::BufferByteStream(ByteStream &bs)
  : bs(bs), bufpos(1), bufend(1)
{ 
}

int 
BufferByteStream::eof(void) // aka. feof
{
  if (bufpos < bufend) 
    return false;
  bufend = bufpos = 1;
  bufend += bs.read(buffer+bufend, bufsize-bufend);
  return (bufend == bufpos);
}

size_t 
BufferByteStream::read(void *buf, size_t size)
{
  if (size < 1)
    return 0;
  if (bufend == bufpos) 
    {
      if (size >= bufsize)
        return bs.read(buf, size);
      if (eof())
        return 0;
    }
  if (bufpos + (int)size > bufend)
    size = bufend - bufpos;
  memcpy(buf, buffer+bufpos, size);
  bufpos += size;
  return size;
}

size_t 
BufferByteStream::write(const void *, size_t )
{
  G_THROW("Cannot write into a BufferByteStream");
  return 0;
}

long 
BufferByteStream::tell(void) const
{ 
  return bs.tell() + bufpos - bufend; 
}
    
inline int 
BufferByteStream::get(void) // aka. getc()
{
  if (bufpos < bufend || !eof())
    return buffer[bufpos++];
  return EOF;
}

int  
BufferByteStream::unget(int c) // aka. ungetc()
{
  if (bufpos > 0) 
    return buffer[--bufpos] = (unsigned char)c;
  return EOF;
}
  



// --------------------------------------------------
// COLOR CONNECTED COMPONENT ANALYSIS
// --------------------------------------------------


// -- A run of pixels with the same color
struct Run    
{ 
  short y;       // vertical coordinate
  short x1;      // first horizontal coordinate
  short x2;      // last horizontal coordinate
  short color;   // color id
  int ccid;      // component id
};


// -- Compares runs for y-x sorting
static inline bool
operator <= (const Run &a, const Run &b)
{
  return (a.y<b.y) || (a.y==b.y && a.x1<=b.x1);
}


// -- Color component descriptor
struct CC    
{
  GRect bb;      // bounding box
  int npix;      // number of black pixels
  int nrun;      // number of runs
  int frun;      // first run in cc ordered array of runs
  int color;     // color id
};


// -- An image composed of runs
class CRLEImage 
{
public:
  int height;            // Height of the image in pixels
  int width;             // Width of the image in pixels
  GP<DjVuPalette> pal;   // Color palette
  GTArray<Run>    runs;  // Array of runs
  GTArray<CC>     ccs;   // Array of component descriptors
  int  nregularccs;      // Number of regular ccs (set by merge_and_split_ccs)
  char bg_flags;         // Comment flags about background.
  char fg_flags;         // Comment flags about foreground.
  CRLEImage(BufferByteStream &bs);
  GP<GBitmap> get_bitmap_for_cc(int ccid) const;
  void make_ccids_by_analysis();
  void make_ccs_from_ccids();
  void merge_and_split_ccs(int smallsize, int largesize);
  void sort_in_reading_order(); 
private:
  unsigned int read_integer(char &c, ByteStream &bs);
  void insert_runs(int y, const short *x1x2color, int nruns);
};


// -- Helper for CRLEImage::CRLEImage(ByteStream &bs)
unsigned int 
CRLEImage::read_integer(char &c, ByteStream &bs)
{
  unsigned int x = 0;
  // Eat blanks before integer
  while (c==' ' || c=='\t' || c=='\r' || c=='\n' || c=='#') 
    {
      if (c=='#') 
        {
          char buffer[256];
          char *s = buffer;
          while (bs.read(&c,1) && c!='\n' && c!='\r')
            if (s - buffer < (int)(sizeof(buffer) - 1))
              *s++ = c;
          *s = 0;
          // Analyze comment flags
          for (s = buffer; *s; s++) 
            {
              if (!GStringRep::cmp(s, "bg-", 3))
                {
                  if (!GStringRep::cmp(s+3,"bw",2)   ||
                      !GStringRep::cmp(s+3,"gray",4) ||
                      !GStringRep::cmp(s+3,"color",5)  )
                    bg_flags = s[3];
                }
              else if (!GStringRep::cmp(s, "fg-", 3))
                {
                  if (!GStringRep::cmp(s+3,"bw",2) ||
                      !GStringRep::cmp(s+3,"gray",4) ||
                      !GStringRep::cmp(s+3,"color",5)  )
                    fg_flags = s[3];
                }
            }
        }
      c = 0; 
      bs.read(&c, 1);
    }
  // Read integer
  if (c<'0' || c>'9')
      G_THROW("csepdjvu: corrupted input file (bad file header)");
  while (c>='0' && c<='9') 
    {
      x = x*10 + c - '0';
      c = 0;
      bs.read(&c, 1);
    }
  return x;
}


// -- Helper for CRLEImage::CRLEImage(ByteStream &bs)
void
CRLEImage::insert_runs(int y, const short *x1x2color, int count)
{
  if (count > 0)
    {
      int index = runs.lbound() - count;
      runs.resize(index, runs.hbound());
      Run *run = &runs[index];
      while (--count>=0) {
        run->y     = y;
        run->x1    = *x1x2color++;
        run->x2    = *x1x2color++;
        run->color = *x1x2color++;
        run->ccid  = 0;
        run++;
      }
      runs.shift(-index);
    }
}


// -- Constructs CRLEImage from a run lenght encoded file,
//    making sure that runs are properly sorted.
CRLEImage::CRLEImage(BufferByteStream &bs)
  : height(0), width(0), nregularccs(0), bg_flags(0), fg_flags(0)
{
  unsigned int magic = bs.read16();
  char lookahead = '\n';
  width = read_integer(lookahead, bs);
  height = read_integer(lookahead, bs);
  if (width<1 || height<1)
    G_THROW("csepdjvu: corrupted input file (bad image size)");
  // An array for the runs and the buffered data
  GTArray<short> ax(3*width+3);
  // File format switch
  if (magic == 0x5234) // Black&White RLE data
    {
      // Setup palette with one color
      pal = DjVuPalette::create();
      static char zeroes[4];
      GP<ByteStream> gzbs=ByteStream::create_static(zeroes,4);
      ByteStream &zbs=*gzbs;
      pal->decode_rgb_entries(zbs, 1);
      // RLE format
      int x, c, n;
      unsigned char p = 0;
      short *px = ax;
      n = height - 1;
      c = 0;
      while (n >= 0)
        {
          if (bs.eof())
            G_THROW( ByteStream::EndOfFile );
          x = bs.get();
          if (x >= 0xc0)
            x = (bs.get()) + ((x - 0xc0) << 8);
          if (c+x > width)
            G_THROW("csepdjvu: corrupted input file (lost RLE synchronization)");
          if (p)
            {
              px[0] = c;
              px[1] = c+x-1;
              px[2] = 0;
              px += 3;
            }
          c += x;
          p = 1 - p;
          if (c >= width)
            {
              insert_runs(n, ax, (px-ax)/3);
              c = 0;
              p = 0;
              n -= 1; 
              px = ax;
            }
        }
    } else if (magic == 0x5236) { // Color-RLE data 
      // Setup palette 
      int ncolors = read_integer(lookahead, bs);
      if (ncolors<1 || ncolors>4095) 
        G_THROW("csepdjvu: corrupted input file (bad number of colors)");
      pal = DjVuPalette::create();
      pal->decode_rgb_entries(bs, ncolors);
      // RLE format
      int x, c, n, p;
      n = height - 1;
      c = 0;
      short *px = ax;
      while (n >= 0)
        {
          if (bs.eof())
            G_THROW( ByteStream::EndOfFile );
          x  = (bs.get() << 24);
          x  |= (bs.get() << 16);
          x  |= (bs.get() << 8);
          x  |= (bs.get());
          p = (x >> 20) & 0xfff;
          x = (x & 0xfffff);
          if (c+x > width)
            G_THROW("csepdjvu: corrupted input file (lost RLE synchronization)");
          if (p >= 0 && p < ncolors)
            {
              px[0] = c;
              px[1] = c+x-1;
              px[2] = p;
              px += 3;
            }
          c += x;
          if (c >= width)
            {
              insert_runs(n, ax, (px-ax)/3);
              c = 0;
              p = 0;
              n -= 1; 
              px = ax;
            }
        }
    } else { // Unrecognized file
      G_THROW("csepdjvu: corrupted input file (bad file header)");
    }
}


// -- Performs color connected component analysis
//    assuming that runs are already y-x sorted.
void
CRLEImage::make_ccids_by_analysis()
{
  // runs.sort(); (we know that runs are 
  // Single Pass Connected Component Analysis (with unodes)
  int n;
  int p=0;
  GTArray<int> umap;
  for (n=0; n<=runs.hbound(); n++)
    {
      int y = runs[n].y;
      int x1 = runs[n].x1 - 1;
      int x2 = runs[n].x2 + 1;
      int color = runs[n].color;
      int id = (umap.hbound() + 1);
      // iterate over previous line runs
      if (p>0) p--;
      for(;runs[p].y < y-1;p++);
      for(;(runs[p].y < y) && (runs[p].x1 <= x2);p++ )
        {
          if ( runs[p].x2 >= x1 )
            {
              if (runs[p].color == color)
                {
                  // previous run touches current run and has same color
                  int oid = runs[p].ccid;
                  while (umap[oid] < oid)
                    oid = umap[oid];
                  if ((int)id > umap.hbound()) {
                    id = oid;
                  } else if (id < oid) {
                    umap[oid] = id;
                  } else {
                    umap[id] = oid;
                    id = oid;
                  }
                  // freshen previous run id
                  runs[p].ccid = id;
                }
              // stop if previous run goes past current run
              if (runs[p].x2 >= x2)
                break;
            }
        }
      // create new entry in umap
      runs[n].ccid = id;
      if (id > umap.hbound())
        {
          umap.touch(id);
          umap[id] = id;
        }
    }
  // Update umap and ccid
  for (n=0; n<=runs.hbound(); n++)
    {
      Run &run = runs[n];
      int ccid = run.ccid;
      while (umap[ccid] < ccid)
        ccid = umap[ccid];
      umap[run.ccid] = ccid;
      run.ccid = ccid;
    }
}


// -- Constructs the ``ccs'' array from run's ccids.
void
CRLEImage::make_ccs_from_ccids()
{
  int n;
  Run *pruns = runs;
  // Find maximal ccid
  int maxccid = -1;
  for (n=0; n<=runs.hbound(); n++)
    if (pruns[n].ccid > maxccid)
      maxccid = runs[n].ccid;
  GTArray<int> armap(0,maxccid);
  int *rmap = armap;
  // Renumber ccs 
  for (n=0; n<=maxccid; n++)
    armap[n] = -1;
  for (n=0; n<=runs.hbound(); n++)
    if (pruns[n].ccid >= 0)
      rmap[ pruns[n].ccid ] = 1;
  int nid = 0;
  for (n=0; n<=maxccid; n++)
    if (rmap[n] > 0)
      rmap[n] = nid++;
  // Adjust nregularccs (since ccs are renumbered)
  while (nregularccs>0 && rmap[nregularccs-1]<0)
    nregularccs -= 1;
  if (nregularccs>0)
    nregularccs = 1 + rmap[nregularccs-1];
  // Prepare cc descriptors
  ccs.resize(0,nid-1);
  for (n=0; n<nid; n++)
    ccs[n].nrun = 0;
  // Relabel runs
  for (n=0; n<=runs.hbound(); n++)
    {
      Run &run = pruns[n];
      if (run.ccid < 0) continue;  // runs with negative ccids are destroyed
      int oldccid = run.ccid;
      int newccid = rmap[oldccid];
      CC &cc = ccs[newccid];
      run.ccid = newccid;
      cc.nrun += 1;
    }
  // Compute positions for runs of cc
  int frun = 0;
  for (n=0; n<nid; n++) 
    {
      ccs[n].frun = rmap[n] = frun;
      frun += ccs[n].nrun;
    }
  // Copy runs
  GTArray<Run> rtmp;
  rtmp.steal(runs);
  Run *ptmp = rtmp;
  runs.resize(0,frun-1);
  pruns = runs;
  for (n=0; n<=rtmp.hbound(); n++)
    {
      int id = ptmp[n].ccid;
      if (id < 0) continue;
      int pos = rmap[id]++;
      pruns[pos] = ptmp[n];
    }
  // Finalize ccs
  for (n=0; n<nid; n++)
    {
      CC &cc = ccs[n];
      int npix = 0;
      runs.sort(cc.frun, cc.frun+cc.nrun-1);
      Run *run = &runs[cc.frun];
      int xmin = run->x1;
      int xmax = run->x2;
      int ymin = run->y;
      int ymax = run->y;
      cc.color = run->color;
      for (int i=0; i<cc.nrun; i++, run++)
        {
          if (run->x1 < xmin)  xmin = run->x1;
          if (run->x2 > xmax)  xmax = run->x2;
          if (run->y  < ymin)  ymin = run->y;
          if (run->y  > ymax)  ymax = run->y;
          npix += run->x2 - run->x1 + 1;
        }
      cc.npix = npix;
      cc.bb.xmin = xmin;
      cc.bb.ymin = ymin;
      cc.bb.xmax = xmax + 1;
      cc.bb.ymax = ymax + 1;
    }
}


// -- Helper for merge_and_split_ccs
struct Grid_x_Color 
{
  short gridi;
  short gridj;
  int color;
};


// -- Helper for merge_and_split_ccs
static inline unsigned int
hash(const Grid_x_Color &x) 
{
  return (x.gridi<<16) ^ (x.gridj<<8) ^ x.color;
}


// -- Helper for merge_and_split_ccs
static inline bool
operator==(const Grid_x_Color &x, const Grid_x_Color &y)
{
  return (x.gridi==y.gridi) && (x.gridj==y.gridj) && (x.color==y.color);
}


// -- Helper for merge_and_split_ccs
static int
makeccid(const Grid_x_Color &x, GMap<Grid_x_Color,int> &map, int &ncc)
{
  GPosition p = map.contains(x);
  if (p) return map[p];
  return map[x] = ncc++;
}


// -- Merges small ccs of similar color and splits large ccs
void
CRLEImage::merge_and_split_ccs(int smallsize, int largesize)
{
  int ncc = ccs.size();
  int nruns = runs.size();
  int splitsize = largesize;
  if (ncc <= 0) return;
  // Associative map for storing merged ccids
  GMap<Grid_x_Color,int> map;
  nregularccs = ncc;
  // Set the correct ccids for the runs
  for (int ccid=0; ccid<ccs.size(); ccid++)
    {
      CC* cc = &ccs[ccid];
      if (cc->nrun <= 0) continue;
      Grid_x_Color key;
      key.color = cc->color;
      int ccheight = cc->bb.height();
      int ccwidth = cc->bb.width();
      if (ccheight<=smallsize && ccwidth<=smallsize)
        {
          key.gridi = (cc->bb.ymin+cc->bb.ymax)/splitsize/2;
          key.gridj = (cc->bb.xmin+cc->bb.xmax)/splitsize/2;
          int newccid = makeccid(key, map, ncc);
          for(int runid=cc->frun; runid<cc->frun+cc->nrun; runid++)
            runs[runid].ccid = newccid;
        }
      else if (ccheight>=largesize || ccwidth>=largesize)
        {
          for(int runid=cc->frun; runid<cc->frun+cc->nrun; runid++)
            {
              Run *r = & runs[runid];
              key.gridi = r->y/splitsize;
              key.gridj = r->x1/splitsize;
              int gridj_end = r->x2/splitsize;
              int gridj_span = gridj_end - key.gridj;
              r->ccid = makeccid(key, map, ncc);
              if (gridj_span>0)
                {
                  // truncate current run 
                  runs.touch(nruns+gridj_span-1);
                  r = &runs[runid];
                  int x = key.gridj*splitsize + splitsize;
                  int x_end = r->x2;
                  r->x2 = x-1;
                  // append additional runs to the runs array
                  while (++key.gridj < gridj_end)
                    {
                      Run& newrun = runs[nruns++];
                      newrun.y = r->y;
                      newrun.x1 = x;
                      x += splitsize;
                      newrun.x2 = x-1;
                      newrun.color = key.color;
                      newrun.ccid = makeccid(key, map, ncc);
                    }
                  // append last run to the run array
                  Run& newrun = runs[nruns++];
                  newrun.y = r->y;
                  newrun.x1 = x;
                  newrun.x2 = x_end;
                  newrun.color = key.color;
                  newrun.ccid = makeccid(key, map, ncc);
                }
            }
        }
    }
  // Recompute cc descriptors
  make_ccs_from_ccids();
}


// -- Helps sorting cc
static int 
top_edges_descending (const void *pa, const void *pb)
{
  if (((CC*) pa)->bb.ymax != ((CC*) pb)->bb.ymax)
    return (((CC*) pb)->bb.ymax - ((CC*) pa)->bb.ymax);
  if (((CC*) pa)->bb.xmin != ((CC*) pb)->bb.xmin)
    return (((CC*) pa)->bb.xmin - ((CC*) pb)->bb.xmin);
  return (((CC*) pa)->frun - ((CC*) pb)->frun);
}


// -- Helps sorting cc
static int 
left_edges_ascending (const void *pa, const void *pb)
{
  if (((CC*) pa)->bb.xmin != ((CC*) pb)->bb.xmin)
    return (((CC*) pa)->bb.xmin - ((CC*) pb)->bb.xmin);
  if (((CC*) pb)->bb.ymax != ((CC*) pa)->bb.ymax)
    return (((CC*) pb)->bb.ymax - ((CC*) pa)->bb.ymax);
  return (((CC*) pa)->frun - ((CC*) pb)->frun);
}


// -- Helps sorting cc
static int 
integer_ascending (const void *pa, const void *pb)
{
  return ( *(int*)pb - *(int*)pa );
}


// -- Sort ccs in approximate reading order
void 
CRLEImage::sort_in_reading_order()
{
  if (nregularccs<2) return;
  CC *ccarray = new CC[nregularccs];
  // Copy existing ccarray (but segregate special ccs)
  int ccid;
  for(ccid=0; ccid<nregularccs; ccid++)
    ccarray[ccid] = ccs[ccid];
  // Sort the ccarray list into top-to-bottom order.
  qsort (ccarray, nregularccs, sizeof(CC), top_edges_descending);
  // Subdivide the ccarray list roughly into text lines
  int maxtopchange = width / 40;
  if (maxtopchange < 32) 
    maxtopchange = 32;
  // - Loop until processing all ccs
  int ccno = 0;
  int *bottoms = new int[nregularccs];
  while (ccno < nregularccs)
    {
      // - Gather first line approximation
      int nccno;
      int sublist_top = ccarray[ccno].bb.ymax-1;
      int sublist_bottom = ccarray[ccno].bb.ymin;
      for (nccno=ccno; nccno < nregularccs; nccno++)
        {
          if (ccarray[nccno].bb.ymax-1 < sublist_bottom) break;
          if (ccarray[nccno].bb.ymax-1 < sublist_top - maxtopchange) break;
          int bottom = ccarray[nccno].bb.ymin;
          bottoms[nccno-ccno] = bottom;
          if (bottom < sublist_bottom)
            sublist_bottom = bottom;
        }
      // - If more than one candidate cc for the line
      if (nccno > ccno + 1)
        {
          // - Compute median bottom
          qsort(bottoms, nccno-ccno, sizeof(int), integer_ascending);
          int bottom = bottoms[ (nccno-ccno-1)/2 ];
          // - Compose final line
          for (nccno=ccno; nccno < nregularccs; nccno++)
            if (ccarray[nccno].bb.ymax-1 < bottom)
              break;
          // - Sort final line
          qsort (ccarray+ccno, nccno-ccno, sizeof(CC), left_edges_ascending);
        }
      // - Next line
      ccno = nccno;
    }
  // Copy ccarray back and renumber the runs
  for(ccid=0; ccid<nregularccs; ccid++)
    {
      CC& cc = ccarray[ccid];
      ccs[ccid] = cc;
      for(int r=cc.frun; r<cc.frun+cc.nrun; r++)
        runs[r].ccid = ccid;
    }
  // Free memory
  delete [] bottoms;
  delete[] ccarray;
}


// -- Creates a bitmap for a particular component
GP<GBitmap>   
CRLEImage::get_bitmap_for_cc(const int ccid) const
{
  const CC &cc = ccs[ccid];
  const GRect &bb = cc.bb;
  GP<GBitmap> bits = GBitmap::create(bb.height(), bb.width());
  const Run *prun = & runs[(int)cc.frun];
  for (int i=0; i<cc.nrun; i++,prun++)
    {
      if (prun->y<bb.ymin || prun->y>=bb.ymax)
        G_THROW("Internal error (y bounds)");
      if (prun->x1<bb.xmin || prun->x2>=bb.xmax)
        G_THROW("Internal error (x bounds)");
      unsigned char *row = (*bits)[prun->y - bb.ymin];
      for (int x=prun->x1; x<=prun->x2; x++)
        row[x - bb.xmin] = 1;
    }
  return bits;
}



// --------------------------------------------------
// LOSSLESS PATTERN MATCHING
// --------------------------------------------------

// ISSUE: LOSSY PATTERN MATCHING
// This is lossless because shapes with different colors
// can touch each other.  Modifying such shapes may result
// in unpleasant effects.  But not all shapes are like that...


// -- Candidate descriptor for pattern matching
struct MatchData 
{
  GP<GBitmap> bits;       // bitmap pointer
  int area;            // number of black pixels
};


// -- Creates shape hierarchy and substitutions (lossless)
void
tune_jb2image(JB2Image *jimg, int refine_threshold=21)
{
  // Pattern matching data
  int nshapes = jimg->get_shape_count();
  GTArray<MatchData> library(nshapes);
  MatchData *lib = library;    // for faster access  
  // Loop on all shapes
  for (int current=0; current<nshapes; current++)
    {
      // Skip ``special shapes''
      lib[current].bits = 0;
      JB2Shape &jshp = jimg->get_shape(current);
      if (jshp.userdata || !jshp.bits) continue; 
      // Compute matchdata info
      GBitmap &bitmap = *(jshp.bits);
      int row;
      int rows = bitmap.rows();
      int column;
      int columns = bitmap.columns();
      int black_pixels = 0;
      for (row = rows - 1; row >= 0; row--) 
        for (column = 0; column < columns; column++) 
          if (bitmap[row][column]) 
            black_pixels++;
      lib[current].bits = jshp.bits;
      lib[current].area = black_pixels;
      // Prepare for search
      int closest_match = -1;
      int best_score = (refine_threshold * rows * columns + 50) / 100;
      if (best_score < 2) best_score = 2;
      bitmap.minborder(2); // ensure sufficient borders
      // Search closest match
      for (int candidate = 0; candidate < current; candidate++) 
        {
          // Access candidate bitmap
          if (lib[candidate].bits) continue;
          GBitmap &cross_bitmap = *lib[candidate].bits;
          int cross_columns = cross_bitmap.columns();
          int cross_rows = cross_bitmap.rows();
          // Prune
          if (abs (lib[candidate].area - black_pixels) > best_score) continue;
          if (abs (cross_rows - rows) > 2) continue;
          if (abs (cross_columns - columns) > 2) continue;
          // Compute alignment (these are always +1, 0 or -1)
          int cross_column_adjust = (cross_columns  - cross_columns/2) - (columns - columns/2);
          int cross_row_adjust = (cross_rows  - cross_rows/2) - (rows - rows/2);
          // Ensure adequate borders
          cross_bitmap.minborder (2-cross_column_adjust);
          cross_bitmap.minborder (2+columns-cross_columns+cross_column_adjust);
          // Count pixel differences (including borders)
          int score = 0;
          unsigned char *p_row;
          unsigned char *p_cross_row;
          for (row = -1; row <= rows; row++) 
            {
              p_row = bitmap[row];
              p_cross_row  = cross_bitmap[row+cross_row_adjust] + cross_column_adjust;
              for (column = -1; column <= columns; column++) 
                if (p_row [column] != p_cross_row [column])
                  score ++;
              if (score >= best_score)  // prune
                break;
            }
          if (score < best_score) {
            best_score = score;
            closest_match = candidate;
          }
        }
      // Decide what to do with the match.
      if (closest_match >= 0)
        {
          // Mark the shape for cross-coding (``soft pattern matching'')
          jshp.parent = closest_match;
          // Exact match ==> Substitution
          if (best_score == 0)
            lib[current].bits = jshp.bits = 0;
        }
    }
  // Process shape substitutions
  for (int blitno=0; blitno<jimg->get_blit_count(); blitno++)
    {
      JB2Blit *jblt = jimg->get_blit(blitno);
      JB2Shape &jshp = jimg->get_shape(jblt->shapeno);
      if (!jshp.bits && jshp.parent>=0)
        jblt->shapeno = jshp.parent;
    }
}



// --------------------------------------------------
// PROCESS BACKGROUND PIXMAP
// --------------------------------------------------


// -- Tries to read a background pixmap
GP<GPixmap>
read_background(BufferByteStream &bs, int w, int h, int &bgred)
{
  int lookahead;
  // Peek next non null byte
  while (! (lookahead = bs.get())) /**/;
  if (lookahead != EOF)
    bs.unget(lookahead);
  if (lookahead != 'P') 
    return 0;
  // Check pixmap
  GP<GPixmap> pix = GPixmap::create(bs);
  while (! (lookahead = bs.get())) /**/;
  if (lookahead != EOF)
    bs.unget(lookahead);
  // Check background reduction
  for (bgred=1; bgred<12; bgred++) 
    {
      int subw = (w + bgred - 1) / bgred;
      int subh = (h + bgred - 1) / bgred;
      if (subh == (int)pix->rows() || subw == (int)pix->columns())
        // Found reduction factor
        return pix;
    }
  // Failure
  G_THROW("Background pixmap size does not match foreground");
  return 0;
}



// --------------------------------------------------
// MAIN COMPRESSION ROUTINE
// --------------------------------------------------


// -- Options
struct csepdjvuopts
{
  int dpi;                  // resolution
  int verbose;              // verbosity level
  unsigned char slice[16];  // background quality spec
};


// -- Compresses one page:
//    - bytestream bs contains the input separated file.
//    - bytestream obs will receive the output djvu file.
void 
csepdjvu_page(BufferByteStream &bs, GP<ByteStream> obs, const csepdjvuopts &opts)
{
  // Read rle data from separation file
  CRLEImage rimg(bs);
  int w = rimg.width;
  int h = rimg.height;
  if (opts.verbose > 1)
    DjVuFormatErrorUTF8( "%s\t%d\t%d\t%d\t%d",
                     ERR_MSG("csepdjvu.summary"), 
                     w, h, rimg.pal->size(), rimg.runs.size());
  
  // Perform Color Connected Component Analysis
  rimg.make_ccids_by_analysis();                  // Obtain ccids
  rimg.make_ccs_from_ccids();                     // Compute cc descriptors
  if (opts.verbose > 1)
    DjVuFormatErrorUTF8( "%s\t%d", ERR_MSG("csepdjvu.analyzed"), rimg.ccs.size());
  
  // Post-process Color Connected Components
  int largesize = MIN(500, MAX(64, opts.dpi));
  int smallsize = MAX(2, opts.dpi/150);
  rimg.merge_and_split_ccs(smallsize,largesize);  // Eliminates gross ccs
  if (opts.verbose > 1)
    DjVuFormatErrorUTF8( "%s\t%d",
                     ERR_MSG("csepdjvu.merge_split"), 
                     rimg.ccs.size());
  rimg.sort_in_reading_order();                   // Sort cc descriptors
  
  // Create JB2Image and fill colordata
  GP<JB2Image> gjimg=JB2Image::create(); 
  JB2Image &jimg=*gjimg;
  jimg.set_dimension(w, h);
  int nccs = rimg.ccs.size();
  for (int ccid=0; ccid<nccs; ccid++)
    {
      JB2Shape shape;
      JB2Blit  blit;
      shape.parent = -1;
      shape.userdata = (ccid >= rimg.nregularccs);
      shape.bits = rimg.get_bitmap_for_cc(ccid);
      shape.bits->compress();
      CC& cc = rimg.ccs[ccid];
      blit.shapeno = jimg.add_shape(shape);
      blit.left = cc.bb.xmin;
      blit.bottom = cc.bb.ymin;
      int blitno = jimg.add_blit(blit);
      rimg.pal->colordata.touch(blitno);
      rimg.pal->colordata[blitno] = cc.color;
    }
  
  // Organize JB2Image
  tune_jb2image(&jimg);
  if (opts.verbose> 1)
    {
      int nshape=0, nrefine=0;
      for (int i=0; i<jimg.get_shape_count(); i++) 
        {
          if (!jimg.get_shape(i).bits) continue;
          if (jimg.get_shape(i).parent >= 0) nrefine++; 
          nshape++; 
        }
      DjVuFormatErrorUTF8( "%s\t%d\t%d",
                       ERR_MSG("csepdjvu.cross_code"), 
                       nshape, nrefine);
    }
  
  // Obtain background image
  int bgred;
  GP<GPixmap> bgpix = read_background(bs, w, h, bgred);
  if (opts.verbose > 1 && bgpix)
    DjVuFormatErrorUTF8( "%s\t%d", ERR_MSG("csepdjvu.reduction"), bgred);
  
  // Compute flags for simplifying output format
  bool white_background = (bgpix ? false : true);
  bool gray_background = white_background;
  if (rimg.bg_flags == 'g' || rimg.bg_flags=='b')
    gray_background = true;
  bool bitonal = false;
  if (white_background && rimg.pal->size() == 1) 
    {
      GPixel fgcolor;
      rimg.pal->index_to_color(0, fgcolor);
      if (fgcolor == GPixel::BLACK)
        bitonal = true;
    }
  if (opts.verbose > 1)
    {
      if (bitonal)
        DjVuWriteError( ERR_MSG("csepdjvu.bilevel") );
      else if (white_background) 
        DjVuWriteError( ERR_MSG("csepdjvu.white_bg") );
      else if (gray_background) 
        DjVuWriteError( ERR_MSG("csepdjvu.gray_bg") );
      else 
        DjVuWriteError( ERR_MSG("csepdjvu.color") );
    }
  
  // Create background image
  GP<IW44Image> iw;
  if (! white_background)
    {
      /* Perform masked compression */
      GP<GBitmap> mask = jimg.get_bitmap(bgred);
      mask->binarize_grays(bgred*bgred-1);
      IW44Image::CRCBMode mode = IW44Image::CRCBnormal;
      if (gray_background) mode = IW44Image::CRCBnone;
      iw = IW44Image::create_encode(*bgpix, mask, mode);
      bgpix = 0;
    } 
  else if (! bitonal) 
    {
      /* Compute white background */
      GPixel bgcolor = GPixel::WHITE;
      GP<GPixmap> inputsub=GPixmap::create((h+11)/12, (w+11)/12, &bgcolor);
      iw = IW44Image::create_encode(*inputsub, 0, IW44Image::CRCBnone);
    }
  
  // Assemble DJVU file
  GP<IFFByteStream> giff=IFFByteStream::create(obs);
  IFFByteStream &iff=*giff;
  // -- main composite chunk
  iff.put_chunk("FORM:DJVU", 1);
  // -- ``INFO'' chunk
  iff.put_chunk("INFO");
  GP<DjVuInfo> ginfo=DjVuInfo::create();
  DjVuInfo &info=*ginfo;
  info.height = h;
  info.width = w;
  info.dpi = opts.dpi;
  info.encode(*iff.get_bytestream());
  iff.close_chunk();
  // -- ``Sjbz'' chunk
  iff.put_chunk("Sjbz");
  jimg.encode(iff.get_bytestream());
  iff.close_chunk();
  // -- Color stuff
  if (! bitonal) 
    {
      // -- ``FGbz'' chunk
      iff.put_chunk("FGbz");
      rimg.pal->encode(iff.get_bytestream());
      iff.close_chunk();
      // -- ``BG44'' chunk
      IWEncoderParms iwparms;
      if (white_background) 
        {
          iff.put_chunk("BG44");
          iwparms.slices = 97;
          iw->encode_chunk(iff.get_bytestream(), iwparms);
          iff.close_chunk();
        }
      else
        {
          const unsigned char *slice = opts.slice;
          while ((iwparms.slices = *slice++))
            {
              iff.put_chunk("BG44");
              iw->encode_chunk(iff.get_bytestream(), iwparms);
              iff.close_chunk();
            }
        }
    }
  // -- terminate main composite chunk
  iff.close_chunk();
}  

// -- Checks whether there is another page in the same file
bool
check_for_another_page(BufferByteStream &bs, const csepdjvuopts &opts)
{
  int lookahead;
  for(;;) 
    {
      // Read next non zero byte
      while (! (lookahead = bs.get())) /**/;
      // Process well known cases
      if (lookahead == 'R') {
        /* Found page */
        bs.unget(lookahead);
        return true;
      } else if (lookahead != '#') {
        if (lookahead != EOF) 
          DjVuPrintErrorUTF8("%s","csepdjvu: found corrupted data\n");
        break;
      }
      // Skip comment line.
      if (opts.verbose > 1)
        DjVuPrintErrorUTF8("%s","csepdjvu: comment \"#");
      lookahead = bs.get();
      while (lookahead!=EOF && lookahead!='\n' && lookahead!='\r') {
        if (opts.verbose > 1)
          DjVuPrintErrorUTF8("%c",lookahead);
        lookahead = bs.get();
      }
      if (opts.verbose > 1)
        DjVuPrintErrorUTF8("%s","\"\n");
    }
  return false;
}
      

// -- Prints usage message
void
usage()
{
  DjVuPrintErrorUTF8("%s",
          "Usage: csepdjvu <...options_or_separatedfiles...> <outputdjvufile>\n"
          "Options are:\n"
          "   -d <n>     Sets resolution to <n> dpi (default: 300).\n"
          "   -q <spec>  Selects quality level for background (default: 72+11+10+10)\n"
          "              See option -slice in program c44 for additional information\n"
          "   -v         Displays a brief message per page.\n"
          "   -vv        Displays lots of messages.\n"
          "Each separated files contain one or more pages\n"
          "Each page is composed of:\n"
          " (1) a B&W-RLE or Color-RLE image representing the foreground,\n"
          " (2) an optional PPM image representing the background layer.\n" );
  exit(10);
}


// -- Parsing quality spec (borrowed from c44)
void 
parse_slice(const char *q, csepdjvuopts &opts)
{
  int count = 0;
  int lastx = 0;
  while (*q)
    {
      char *ptr; 
      int x = strtol(q, &ptr, 10);
      if (ptr == q)
        G_THROW("csepdjvu: illegal quality specification (number expected)");
      if (lastx>0 && q[-1]=='+')
        x += lastx;
      if (x<1 || x>1000 || x<lastx)
        G_THROW("csepdjvu: illegal quality specification (number out of range)");
      lastx = x;
      if (*ptr && *ptr!='+' && *ptr!=',')
        G_THROW("csepdjvu: illegal quality specification (comma expected)");        
      q = (*ptr ? ptr+1 : ptr);
      if (count+1 >= (int)(sizeof(opts.slice)/sizeof(opts.slice[0])))
        G_THROW("csepdjvu: illegal quality specification (too many chunks)");                
      opts.slice[count++] = x;
      opts.slice[count] = 0;
    }
  if (count < 1)
    G_THROW("csepdjvu: illegal quality specification (no chunks)");                    
}


// -- Main routine
int 
main(int argc, const char **argv)
{
#ifdef WIN32
  _setmode(_fileno(stdin), _O_BINARY);
  _setmode(_fileno(stdout), _O_BINARY);
#endif
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
  GArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
    dargv[i]=GNativeString(argv[i]);
  G_TRY
    {
      GP<DjVmDoc> gdoc=DjVmDoc::create();
      DjVmDoc &doc=*gdoc;
      GURL outputurl;
      GP<ByteStream> goutputpage=ByteStream::create();
      csepdjvuopts opts;
      int pageno = 0;
      // Defaults
      opts.dpi = 300;
      opts.verbose = false;
      opts.slice[0] =  72;
      opts.slice[1] =  83;      
      opts.slice[2] =  93;
      opts.slice[3] = 103;
      opts.slice[4] =   0;
      // Read outputurl name
      if (argc < 3) usage();
      outputurl = GURL::Filename::UTF8(dargv[--argc]);
      // Process arguments
      for (int i=1; i<argc; i++)
        {
          GUTF8String arg = dargv[i];
          if (arg == "-v")
            opts.verbose = 1;
          else if (arg == "-vv")
            opts.verbose = 2;
          else if (arg == "-d" && i+1<argc)
            {
              // Specify resolution
              char *end;
              opts.dpi = strtol(dargv[++i], &end, 10);
              if (*end || opts.dpi<25 || opts.dpi>144000)
                usage();
            }
          else if (arg == "-q" && i+1<argc)
            {
              // Specify background quality
              parse_slice(dargv[++i], opts);
            }
          else 
            {
              // Process separation file
              GP<ByteStream> fbs=ByteStream::create(GURL::Filename::UTF8(arg),"rb");
              BufferByteStream ibs(*fbs);
              do {
                char pagename[16];
                sprintf(pagename, "p%04d.djvu", ++pageno);
                if (opts.verbose > 1)
                  DjVuPrintErrorUTF8("%s","-------------------------------------\n");
                // Compress page 
                goutputpage=ByteStream::create();
                ByteStream &outputpage=*goutputpage;
                csepdjvu_page(ibs, goutputpage, opts);
                if (opts.verbose) {
                  DjVuPrintErrorUTF8("csepdjvu: %d bytes for page %d",
                          outputpage.size(), pageno);
                  if (arg == "-")
                    DjVuPrintErrorUTF8("%s"," (from stdin)\n");
                  else
                    DjVuPrintErrorUTF8(" (from file '%s')\n", (const char*)arg);
                }
                // Insert page into document
                outputpage.seek(0);
                doc.insert_file(outputpage, DjVmDir::File::PAGE, pagename, pagename);
              } while (check_for_another_page(ibs, opts));
            }
        } 
      // Save file
      if (pageno == 1) 
        {
          ByteStream &outputpage=*goutputpage;
          // Save as a single page 
          outputpage.seek(0);
          ByteStream::create(outputurl,"wb")->copy(outputpage);
        }
      else if (pageno > 1)
        {
          // Save as a bundled file
          doc.write(ByteStream::create(outputurl,"wb"));
        }
      else 
        usage();
    }
  G_CATCH(ex)
    {
      ex.perror();
      exit(1);
    }
  G_ENDCATCH;
  return 0;
}

