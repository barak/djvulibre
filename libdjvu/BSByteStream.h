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

#ifndef _BSBYTESTREAM_H
#define _BSBYTESTREAM_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif

/** @name BSByteStream.h
    
    Files #"BSByteStream.h"# and #"BSByteStream.cpp"# implement a very compact
    general purpose compressor based on the Burrows-Wheeler transform.  The
    utility program \Ref{bzz} provides a front-end for this class. Although
    this compression model is not currently used in DjVu files, it may be used
    in the future for encoding textual data chunks.

    {\bf Algorithms} --- The Burrows-Wheeler transform (also named Block-Sorting)
    is performed using a combination of the Karp-Miller-Rosenberg and the
    Bentley-Sedgewick algorithms. This is comparable to (Sadakane, DCC 98)
    with a slightly more flexible ranking scheme. Symbols are then ordered
    according to a running estimate of their occurrence frequencies.  The
    symbol ranks are then coded using a simple fixed tree and the
    \Ref{ZPCodec} binary adaptive coder.

    {\bf Performances} --- The basic algorithm is mostly similar to those
    implemented in well known compressors like #bzip# or #bzip2#
    (\URL{http://www.muraroa.demon.co.uk}).  The adaptive binary coder however
    generates small differences. The adaptation noise may cost up to 5\% in
    file size, but this penalty is usually offset by the benefits of
    adaptation.  This is good when processing large and highly structured
    files like spreadsheet files.  Compression and decompression speed is
    about twice slower than #bzip2# but the sorting algorithms is more
    robust. Unlike #bzip2# (as of August 1998), this code can compress half a
    megabyte of "abababab...." in bounded time.
    
    Here are some comparative results (in bits per character) obtained on the
    Canterbury Corpus (\URL{http://corpus.canterbury.ac.nz}) as of August
    1998. The BSByteStream performance on the single spreadsheet file #Excl#
    moves #bzz#'s weighted average ahead of much more sophisticated methods,
    like Suzanne Bunton's #fsmxBest# system
    \URL{http://corpus.canterbury.ac.nz/methodinfo/fsmx.html}.  This result
    will not last very long.

    {\footnotesize
    \begin{tabular}{lccccccccccccc}
      & text & fax & Csrc & Excl & SPRC & tech 
      & poem & html & lisp & man & play & Weighted & Average \\
      compress 
      & 3.27 & 0.97 & 3.56 & 2.41 & 4.21 & 3.06 
      & 3.38 & 3.68 & 3.90 & 4.43 & 3.51 
      & 2.55 & 3.31 \\
      gzip -9
      & 2.85 & 0.82 & 2.24 & 1.63 & 2.67 & 2.71 
      & 3.23 & 2.59 & 2.65 & 3.31 & 3.12 
      & 2.08 & 2.53 \\  
      bzip2 -9
      & 2.27 & 0.78 & 2.18 & 1.01 & 2.70 & 2.02 
      & 2.42 & 2.48 & 2.79 & 3.33 & 2.53 
      & 1.54 & 2.23 \\
      ppmd
      & 2.31 & 0.99 & 2.11 & 1.08 & 2.68 & 2.19 
      & 2.48 & 2.38 & 2.43 & 3.00 & 2.53 
      & 1.65 & 2.20 \\
      fsmx
      & {\bf 2.10} & 0.79 & {\bf 1.89} & 1.48 & {\bf 2.52} & {\bf 1.84} 
      & {\bf 2.21} & {\bf 2.24} & {\bf 2.29} & {\bf 2.91} & {\bf 2.35} 
      & 1.63 & {\bf 2.06} \\
      {\bf bzz}
      & 2.25 & {\bf 0.76} & 2.13 & {\bf 0.78} & 2.67 & 2.00
      & 2.40 & 2.52 & 2.60 & 3.19 & 2.52 
      & {\bf 1.44} & 2.16
    \end{tabular}
    }

    Note that the DjVu people have several entries in this table.  Program
    #compress# was written some time ago by Joe Orost
    (\URL{http://www.research.att.com/info/orost}). The #ppmc# method, (a
    precursor of #ppmd#) was created by Paul Howard
    (\URL{http://www.research.att.com/info/pgh}). The #bzz# program is just
    below your eyes.

    @author
    L\'eon Bottou <leonb@research.att.com> -- Initial implementation\\
    Andrei Erofeev <eaf@geocities.com> -- Improved Block Sorting algorithm.
    @memo
    Simple Burrows-Wheeler general purpose compressor.
    @version
    #$Id$# */
//@{


#include "ByteStream.h"
#include "GException.h"
#include "ZPCodec.h"


/** Performs bzz compression/decompression.
    
    Class #BSByteStream# defines a \Ref{ByteStream} which transparently
    performs the BZZ compression/decompression. The constructor of class
    \Ref{BSByteStream} takes another \Ref{ByteStream} as argument.  Any data
    written to the BSByteStream is compressed and written to this second
    ByteStream. Any data read from the BSByteStream is internally generated by
    decompressing data read from the second ByteStream.

    Program \Ref{bzz} demonstrates how to use this class.  All the hard work
    is achieved by a simple ByteStream to ByteStream copy, as shown below.
    \begin{verbatim}
      GP<ByteStream> in=ByteStream::create(infile,"rb");
      GP<ByteStream> out=ByteStream::create(outfile,"wb");
      if (encoding) {
          BSByteStream bsb(out, blocksize);
          bsb.copy(*in);
      } else {
          BSByteStream bsb(in);
          out->copy(bsb);
      }
    \end{verbatim}
    Due to the block oriented nature of the Burrows-Wheeler transform, there
    is a very significant latency between the data input and the data output.
    You can use function #flush# to force data output at the expense of
    compression efficiency.

    You should never directly access a ByteStream object connected to a valid
    BSByteStream object. The ByteStream object can be accessed again after the
    destruction of the BSByteStream object.  Note that the encoder always
    flushes its internal buffers and writes a few final code bytes when the
    BSByteStream object is destroyed.  Note also that the decoder often reads
    a few bytes beyond the last code byte written by the encoder.  This lag
    means that you must reposition the ByteStream after the destruction of the
    BSByteStream object and before re-using the ByteStream object (see
    \Ref{IFFByteStream}.)
*/
class BSByteStream : public ByteStream
{
public:
// Limits on block sizes
  enum { MINBLOCK=10, MAXBLOCK=4096 };

// Sorting tresholds
  enum { FREQMAX=4, CTXIDS=3 };

  class Decode;
  class Encode;
protected:
  BSByteStream(GP<ByteStream> bs);

public:
  /** Creates a BSByteStream.
      The BSByteStream will be used for decompressing data.
      \begin{description}
      \item[Decompression]
      The BSByteStream is created and the decompressor initializes.  Chunks of
      data will be read from ByteStream #bs# and decompressed into an internal
      buffer. Function #read# can be used to access the decompressed data.
      \end{description} */
  static GP<ByteStream> create(GP<ByteStream> bs);

  /** Constructs a BSByteStream.
      The BSByteStream will be used for compressing data.
      \begin{description}
      \item[Compression]
      Set #blocksize# to a positive number smaller than 4096 to 
      initialize the compressor.  Data written to the BSByteStream will be
      accumulated into an internal buffer.  The buffered data will be
      compressed and written to ByteStream #bs# whenever the buffer sizes
      reaches the maximum value specified by argument #blocksize# (in
      kilobytes).  Using a larger block size usually increases the compression
      ratio at the expense of computation time.  There is no need however to
      specify a block size larger than the total number of bytes to compress.
      Setting #blocksize# to #1024# is a good starting point.  A minimal block
      size of 10 is silently enforced.
      \end{description} */
  static GP<ByteStream> create(GP<ByteStream> bs, const int blocksize);

  // ByteStream Interface
  ~BSByteStream();
  virtual long tell(void) const;
  virtual void flush(void) = 0;
protected:
  // Data
  long            offset;
  int             bptr;
  unsigned int    blocksize;
  int             size;
  ByteStream *bs;
  GP<ByteStream> gbs;
  unsigned char  *data;
  GPBuffer<unsigned char> gdata;
  // Coder
  GP<ZPCodec> gzp;
  BitContext ctx[300];
private:  
  // Cancel C++ default stuff
  BSByteStream(const BSByteStream &);
  BSByteStream & operator=(const BSByteStream &);
  BSByteStream(ByteStream *);
  BSByteStream(ByteStream *, int);
};

//@}

#endif
