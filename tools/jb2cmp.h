//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C- 
//C-  ------------------------------------------------------------------
//
// ------------------------------------------
// This file is directly derived from cjb2++.
// Copyright (c) 2004 Ilya Mezhirov.
// With thanks to Alexander Shen.
// License: GPL
// Conditions: usual (no warranty etc)
//-------------------------------------------
//
// $Id$
//

#ifndef JB2CMP_H
# define JB2CMP_H
# ifdef HAVE_CONFIG_H
#  include "config.h"
# endif
# if NEED_GNUG_PRAGMAS
#  pragma interface
# endif

# ifdef __cplusplus
extern "C" {
#  ifdef NOT_DEFINED // Just to fool emacs c++ mode
}
#  endif
# endif

// To get an image ready for comparisons, one have to `prepare' it.  A
// prepared image is promoted to a ComparableImage.  I don't want to bother
// here with structures and strong typing.  (precise structure definition
// would require keeping some of the parameters here, and now they all live in
// `compare.cpp')

typedef struct ComparableImageData *ComparableImage;

// Allocate a ComparableImage chunk and calculate all necessary info.  Memory
// consumption is around byte per pixel.  ComparableImage would be completely
// independent on the bitmap given.  (that is, you can free() `pixels'
// immediately) Lines in `pixels' array go top down, but in fact that does not
// matter.

ComparableImage prepare_comparable_image(unsigned char **pixels, int w, int h);

// Compare images. Returns
// +1 if images are considered equivalent,
// -1 if they are considered totally different (just to speed up things),
//  0 if unknown, but probably different.
// Exchanging the order of arguments should not change the outcome.
// If you have found that A ~ B and B ~ C,
// then you may assume A ~ C regardless of this function's result.

int compare_images(ComparableImage, ComparableImage);

// Frees memory associated with the ComparableImage.

void free_comparable_image(ComparableImage);

// Classifies a bunch of ComparableImages.
// Variable result then contains an array of tags, 
// which range from 1 to the return value of this function.
// It contains 0 for those cells which were NULL.
// Every tag has at least one image to which it's attached.
// Equally tagged images are classified equivalent.

unsigned classify_images(ComparableImage *, 
                         unsigned int *result, 
                         unsigned int n);

# ifdef __cplusplus
}
# endif
#endif // JB2CMP_H
