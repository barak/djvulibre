//C-  -*- mode: C++; c-file-style: "stroustrup" -*-
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "jb2cmp.h"

// Stuff for not using malloc in C++
#ifdef __cplusplus
# define MALLOC(Type)    new Type
# define FREE(p)         delete p
# define MALLOCV(Type,n) new Type[n]
# define FREEV(p)        delete [] p
#else
# define MALLOC(Type)    ((Type*)malloc(sizeof(Type)))
# define FREE(p)         do{if(p)free(p);}while(0)
# define MALLOCV(Type,n) ((Type*)malloc(sizeof(Type)*(n)))
# define FREEV(p)        do{if(p)free(p);}while(0)
#endif


// This is the unit that handles pattern matching.  Its task is only to
// compare pairs of images, not to classify a set of them.  And this has
// absolutely nothing to do with choosing a cross-coding prototype.

// This file is best viewed with Vim because of fold markers ("{{{" and "}}}")

// These are hand-tweaked parameters of this classifier.

static const double pithdiff_threshold             = 2.1;
static const double softdiff_threshold             = 800;
static const double shiftdiff1_threshold           = 60;
static const double shiftdiff2_threshold           = 80;
static const double shiftdiff3_threshold           = 200;

static const double pithdiff_veto_threshold        = 5; // tightly
static const double softdiff_veto_threshold        = 2500;
static const double shiftdiff1_veto_threshold      = 1000;
static const double shiftdiff2_veto_threshold      = 1500;
static const double shiftdiff3_veto_threshold      = 2000;

static const double size_difference_threshold = 10;
static const double pith_falloff              = .85;
static const double shiftdiff1_falloff        = .9;
static const double shiftdiff2_falloff        = 1;
static const double shiftdiff3_falloff        = 1.15;

#define SIGNATURE_SIZE 32

static const int maxint = ~(1 << (sizeof(int) * 8 - 1));

typedef unsigned char byte;

typedef struct ComparableImageData {
    byte **pixels;                   // 0:white, 255:black (unlike PGM)
    int width, height;
    byte signature[SIGNATURE_SIZE];  // for shiftdiff 1 and 3 tests
    byte signature2[SIGNATURE_SIZE]; // for shiftdiff 2 test
} Image;

// Each image pair undergoes a simple dimensions check and at most five tests.
// Each test may end up with three outcomes: veto (-1), doubt (0) and match(1).
// Images are equivalent if and only if
// - there was no `veto'
// - and there was at least one `match'.

// Dimensions checking {{{

// We check whether images' dimensions are different
//     no more than by size_difference_threshold percent.
// Return value is usual: veto or doubt, -1 or 0.

static int 
dimensions_test(Image *i1, Image *i2)
{
    int w1 = i1->width, h1 = i1->height;
    int w2 = i2->width, h2 = i2->height;
    
    if (w1 > (100. + size_difference_threshold) * w2 / 100) return -1;
    if (w2 > (100. + size_difference_threshold) * w1 / 100) return -1;
    if (h1 > (100. + size_difference_threshold) * h2 / 100) return -1;
    if (h2 > (100. + size_difference_threshold) * h1 / 100) return -1;
    
    return 0;
}

// Dimensions checking }}}

#define USE_PITHDIFF 1
#define USE_SOFTDIFF 1
#define USE_SHIFTDIFF_1 1
#define USE_SHIFTDIFF_2 1
#define USE_SHIFTDIFF_3 1


// Computing distance by comparing pixels {{{

#if USE_PITHDIFF || USE_SOFTDIFF

// This function compares two images pixel by pixel.
// The exact way to compare pixels is defined by two functions,
//     compare_row and compare_with_white.
// Both functions take pointers to byte rows and their length.
static int 
distance_by_pixeldiff_functions(Image *i1, Image *i2,
                                int (*compare_row)(byte *, byte *, int),
                                int (*compare_with_white)(byte *, int), 
                                int ceiling)
{
    byte **p1, **p2;
    int w1, w2, h1, h2;
    int shift_x, shift_y; // of i1's coordinate system with respect to i2
    int s = 0, i, i_start, i_cap;
    int right_margin_start, right_margin_width;

    // make i1 to be narrower than i2
    if (i1->width > i2->width)
    {
        Image *img = i1;
        i1 = i2;
        i2 = img;
    }

    w1 = i1->width; h1 = i1->height; p1 = i1->pixels;
    w2 = i2->width; h2 = i2->height; p2 = i2->pixels;

    // (shift_x, shift_y) is what should be 
    // added to i1's coordinates to get i2's coordinates.
    shift_x = (w2 - w2/2) - (w1 - w1/2); // center favors right
    shift_y = h2/2 - h1/2;               // center favors top

    // Compute difference in the non-overlapping top margin
    if (shift_y < 0)
    {
        // i1 has top rows not covered by i2
        i_cap = -shift_y;
        for (i = 0; i < i_cap; i++)
        {
            assert(i >= 0 && i < h1);
            s += compare_with_white(p1[i], w1);
            if (s > ceiling) return maxint;
        }
        i_start = i_cap; // topmost overlapping row in i1's coords
    }
    else
    {
        // i2 has top rows not covered by i1
        for (i = 0; i < shift_y; i++)
        {
            assert(i >= 0 && i < h2);
            s += compare_with_white(p2[i], w2);
            if (s > ceiling) return maxint;
        }
        i_start = 0;
    }

    // Compute difference in the overlapping area
    i_cap = h2 - shift_y;
    if (h1 < i_cap) i_cap = h1;

    right_margin_start = shift_x + w1;
    right_margin_width = w2 - right_margin_start;

    for (i = i_start; i < i_cap; i++) // i is a coordinate in i1 system
    {
        int y = i + shift_y; // same row coordinate in i2 system
        assert(y >= 0 && y < h2);
        s += compare_with_white(p2[y], shift_x);
        if (s > ceiling) return maxint;
        assert(i >= 0 && i < h1);
        assert(shift_x + w1 <= w2);
        assert(i < h1);
        s += compare_row(p2[y] + shift_x, p1[i], w1);
        if (s > ceiling) return maxint;
        s += compare_with_white(p2[y] + right_margin_start, right_margin_width);
        if (s > ceiling) return maxint;
    }

    // Compute difference in the non-overlapping bottom margin
    if (i_cap == h1)
    {
        // i2 has bottom rows not covered by i1
        i_start = i_cap + shift_y;
        for (i = i_start; i < h2; i++)
        {
            assert(i >= 0 && i < h2);
            s += compare_with_white(p2[i], w2);
            if (s > ceiling) return maxint;
        }
    }
    else
    {
        // i1 has bottom rows not covered by i2
        i_start = i_cap;
        for (i = i_cap; i < h1; i++)
        {
            assert(i >= 0 && i < h1);
            s += compare_with_white(p1[i], w1);
            if (s > ceiling) return maxint;
        }
    }

    return s;
}

#endif

// Computing distance by comparing pixels }}}

// inscribed pith penalty counting {{{

// (See `finding pith' to see what it's all about)

#if USE_PITHDIFF

// If the pith of one letter is inscribed into another and vice versa, 
// then those letters are probably equivalent. That's the idea... 
// Counting penalty points here for any pixel 
// that's pith in one image and white in the other. 

static int 
pithdiff_compare_row(byte *row1, byte *row2, int n)
{
    int i, s = 0;
    for (i = 0; i < n; i++)
    {
        int k = row1[i], l = row2[i];
        if (k == 255)
            s += 255 - l;
        else if (l == 255)
            s += 255 - k;
    }
    return s;
}

static int 
pithdiff_compare_with_white(byte *row, int n)
{
    int i, s = 0;
    for (i = 0; i < n; i++) if (row[i] == 255) s += 255;
    return s;
}

static int 
pithdiff_distance(Image *i1, Image *i2, int ceiling)
{
    return distance_by_pixeldiff_functions(i1, i2,
                                           &pithdiff_compare_row, 
                                           &pithdiff_compare_with_white, 
                                           ceiling);
}

static int 
pithdiff_equivalence(Image *i1, Image *i2)
{
    int area = (i1->width * i1->height + i2->width * i2->height) / 2;
    double ceiling = pithdiff_veto_threshold * 255 * area / 100;
    int d = pithdiff_distance(i1, i2, (int) ceiling);
    if (d == maxint) return -1;
    if (d < pithdiff_threshold * 255 * area / 100)
        return 1;
    else
        return 0;
}

#endif // if USE_PITHDIFF

// inscribed pith penalty counting }}}

// soft penalty counting {{{

#if USE_SOFTDIFF

// This test scores penalty points for pixels that are different in both
// images.  Since every black pixel has a rating of importance, the penalty
// for a pair of corresponding pixels, one black, one white, is equal to the
// rating of the black pixel.

static int 
softdiff_compare_row(byte *row1, byte *row2, int n)
{
    int i, s = 0;
    for (i = 0; i < n; i++)
    {
        if (!row1[i])
            s += row2[i];
        else if (!row2[i])
            s += row1[i];
    }
    return s;
}

static int 
softdiff_compare_with_white(byte *row, int n)
{
    int i, s = 0;
    for (i = 0; i < n; i++) s += row[i];
    return s;
}

static int 
softdiff_distance(Image *i1, Image *i2, int ceiling)
{
    return distance_by_pixeldiff_functions(i1, i2,
                                           &softdiff_compare_row, 
                                           &softdiff_compare_with_white, 
                                           ceiling);
}

static int 
softdiff_equivalence(Image *i1, Image *i2)
{
    int area = (i1->width * i1->height + i2->width * i2->height) / 2;
    double ceiling = softdiff_veto_threshold * area / 100;
    int d = softdiff_distance(i1, i2, (int) ceiling);
    if (d == maxint) return -1;
    if (d < softdiff_threshold * area / 100) return 1;
    return 0;
}

#endif // if USE_SOFTDIFF

// soft penalty counting }}}

// finding shift signature {{{

// We cut an image horizontally in such a way that below and above the cut the
// blackness is roughly the same.  Than cutting each of the two pieces
// vertically in the same fashion.  Then horizontally, and so on until
// SIGNATURE_SIZE - 1 cuts.  The position of each cut is normalized into
// 0..255 and put into signature.

static int 
sum_column_gray(byte **pixels, int x, int y1, int y2)
{
    int sum = 0, y;
    for (y = y1; y <= y2; y++) sum += pixels[y][x];
    return sum;
}

static int 
sum_row_gray(byte *row, int x1, int x2)
{
    int sum = 0, x, n = x2 - x1;
    byte *p = row + x1;
    for (x = 0; x <= n; x++) sum += p[x];
    return sum;
}

static int 
sum_column_black_and_white(byte **pixels, int x, int y1, int y2)
{
    int sum = 0, y;
    for (y = y1; y <= y2; y++) if (pixels[y][x]) sum++;
    return sum;
}

static int 
sum_row_black_and_white(byte *row, int x1, int x2)
{
    int sum = 0, x, n = x2 - x1;
    byte *p = row + x1;
    for (x = 0; x <= n; x++) if (p[x]) sum++;
    return sum;
}

static void make_hcut(int a, int l, int w, int h, 
                      byte **pixels, byte *sig, int k,
                      int s_row(byte *, int, int), 
                      int s_col(byte **, int, int, int));

static void make_vcut(int a, int l, int w, int h, 
                      byte **pixels, byte *sig, int k,
                      int s_row(byte *, int, int), 
                      int s_col(byte **, int, int, int));

static void 
make_hcut(int a, int l, int w, int h, 
          byte **pixels, byte *sig, int k,
          int s_row(byte *, int, int), 
          int s_col(byte **, int, int, int))
{
    int cut = 0; // how many rows are in the top part
    int up_weight = 0;

    if (k >= SIGNATURE_SIZE) return;

    if (a)
    {
        int last_row_weight = 0;

        assert(w && h);

        while ((up_weight << 1) < a)
        {
            last_row_weight = s_row(pixels[cut], l, l + w - 1);
            up_weight += last_row_weight;
            cut++;
        }
        cut--;
        up_weight -= last_row_weight;
        sig[k] = (256 *
                    (cut * w + w * ((a >> 1) - up_weight) / last_row_weight))
                 / (w * h);
        if (a - (up_weight << 1) > last_row_weight)
        {
            cut++;
            up_weight += last_row_weight;
        }
    }
    else
    {
        cut = h / 2;
        sig[k] = 128;
    }

    make_vcut(up_weight, l, w, cut, 
              pixels, sig, k << 1, 
              s_row, s_col);

    make_vcut(a - up_weight, l, w, h - cut, 
              pixels + cut, sig, (k << 1) | 1, 
              s_row, s_col);
}

static void 
make_vcut(int a, int l, int w, int h, 
          byte **pixels, byte *sig, int k,
          int s_row(byte *, int, int), 
          int s_col(byte **, int, int, int))
{
    int cut = 0; // how many columns are in the left part
    int left_weight = 0;

    if (k >= SIGNATURE_SIZE) return;

    if (a)
    {
        int last_col_weight = 0;

        assert(w && h);

        while ((left_weight << 1) < a)
        {
            last_col_weight = s_col(pixels, l + cut, 0, h-1);
            left_weight += last_col_weight;
            cut++;
        }
        cut--;
        left_weight -= last_col_weight;
        sig[k] = (256 *
                    (cut * h + h * ((a >> 1) - left_weight) / last_col_weight))
                 / (w * h);
        if (a - (left_weight << 1) > last_col_weight)
        {
            cut++; left_weight += last_col_weight;
        }
    }
    else
    {
        cut = w / 2;
        sig[k] = 128;
    }

    make_hcut(left_weight, l, cut, h, 
              pixels, sig, k << 1, 
              s_row, s_col);
    make_hcut(a - left_weight, l + cut, w - cut, h, 
              pixels, sig, (k << 1) | 1, 
              s_row, s_col);
}

static void 
get_signature(int width, int height, 
              byte **pixels, byte *sig,
              int s_row(byte *, int, int), 
              int s_col(byte **, int, int, int))
{
    int area = 0, i;
    for (i = 0; i < height; i++)
    {
        area += s_row(pixels[i], 0, width - 1);
    }
    // FIXME: sig[0] is wasted
    make_hcut(area, 0, width, height, 
              pixels, sig, 1, 
              s_row, s_col);
}

// finding shift signature }}}

// finding pith {{{

// This is a funny thing...
// The algorithm is to be commented yet.
// Here's a picture illustrating what is a pith.
//
/*  Original letter:        Its pith:
 * 
 .....@@@@@@@@........ .....................
 ...@@@@@@@@@@@@...... ......@@@@@..........
 ..@@@@@@@@@@@@@@..... .....@@...@@@........
 ..@@@@@...@@@@@@@.... ....@@......@@.......
 ..@@@@.....@@@@@@.... ....@........@.......
 .@@@@@.....@@@@@@.... ....@........@.......
 .@@@@@.....@@@@@@.... ....@........@.......
 ..@@@@.....@@@@@@.... ....@........@.......
 ..........@@@@@@@.... .............@.......
 .......@@@@@@@@@@.... .............@.......
 .....@@@@@@@@@@@@.... ........@@@@@@.......
 ...@@@@@@@@@@@@@@.... ......@@@....@.......
 ..@@@@@@@..@@@@@@.... .....@@......@.......
 .@@@@@@....@@@@@@.... ...@@@.......@.......
 .@@@@@.....@@@@@@.... ...@.........@.......
 @@@@@......@@@@@@.... ..@@.........@.......
 @@@@@......@@@@@@.... ..@..........@.......
 @@@@@.....@@@@@@@.... ..@..........@.......
 @@@@@@....@@@@@@@.@@@ ..@@.........@.......
 .@@@@@@@@@@@@@@@@@@@@ ...@@.....@@@@@......
 .@@@@@@@@@@@@@@@@@@@@ ....@@..@@@...@@@@@..
 ..@@@@@@@@@.@@@@@@@@. .....@@@@............
 ....@@@@@....@@@@@... .....................

    A letter is converted into grayshades, and a pith is the set of its purely
    black pixels after the transformation.  In the grayshade version of a
    letter, all pixels that were white remain absolutely white, the pith is
    black and the blackness falls down from the pith to the border.
*/

static int 
donut_connectivity_test(byte *upper, byte *row, byte *lower)
/*{{{*/
{
    /*(on the pictures below 0 is white, 1 is black or gray)
     *
     * 01.
     * 1 . -> 1
     * ...
     *
     * .0.
     * 1.1 ->  1
     * .0.
     *
     * all others -> 0
     */

    int sum, l, u, d, r;

    sum = (u = *upper ? 1 : 0) + (d = *lower ? 1 : 0) +
          (l = row[-1] ? 1 : 0) + (r = row[1] ? 1 : 0);

    switch(sum)
    {
        case 3:/*{{{*/
        {
            int x = 6 - (u + (l << 1) + d + (d << 1));
            switch(x)
            {
                case 0: // l
                    return upper[-1] && lower[-1] ? 0 : 1;
                case 1: // d
                    return lower[-1] && lower[1] ? 0 : 1;
                case 2: // r
                    return upper[1] && lower[1] ? 0 : 1;
                case 3: // u
                    return upper[-1] && upper[1] ? 0 : 1;
                default: assert(0); return 0;
            }
        }
        break;/*}}}*/
        case 2:/*{{{*/
        {
            int s = l + r;
            if (s & 1)
            {
                /*   A1.
                 *   1.0 - should be !A (2x2 square extermination)
                 *   .0.
                 */
                if (l)
                {
                    if (u)
                        return upper[-1] ? 0 : 1;
                    else
                        return lower[-1] ? 0 : 1;
                }
                else // r
                {
                    if (u)
                        return upper[1] ? 0 : 1;
                    else
                        return lower[1] ? 0 : 1;
                }
            }
            else
            {
                /*   .0.
                 *   1.1 - surely should be 1 to preserve connection
                 *   .0.
                 */
                return 1;
            }
        }
        break;/*}}}*/
        case 0: case 4:
            return 1;
        case 1:
            return 0;
        default: assert(0); return 0;
    }
}
/*}}}*/

static byte 
donut_transform_pixel(byte *upper, byte *row, byte *lower)
/*{{{*/
{
    /* (center pixel should be gray in order for this to work)
     * (on the pictures below 0 is white, 1 is black or gray)
     *
     * 01.
     * 1 . -> center will become 1
     * ...
     *
     * .0.
     * 1 1 -> center will become 1
     * .0.
     *
     * 00.
     * 1 0 -> center will become 1
     * .0.
     *
     * 1..
     * 1 0 -> center will become 0
     * 1..
     *
     * 11.
     * 1 0 -> center will become 0
     * .0.
     *
     * .A.
     * A A -> center will become 1
     * .A.
     */

    int sum, l, u, d, r;
    if (!*row) return 0;

    sum = (u = *upper ? 1 : 0) + (d = *lower ? 1 : 0) +
          (l = row[-1] ? 1 : 0) + (r = row[1] ? 1 : 0);

    switch(sum)
    {
        case 1: case 3:/*{{{*/
        {
            int x = u + (l << 1) + d + (d << 1);
            if (sum == 3) x = (6 - x) ^ 2;
            switch(x)
            {
                case 0: // r
                    return upper[1] && lower[1] ? 0 : 1;
                case 1: // u
                    return upper[-1] && upper[1] ? 0 : 1;
                case 2: // l
                    return upper[-1] && lower[-1] ? 0 : 1;
                case 3: // d
                    return lower[-1] && lower[1] ? 0 : 1;
                default: assert(0); return 0;
            }
        }
        break;/*}}}*/
        case 2:/*{{{*/
        {
            int s = l + r;
            if (s & 1)
            {
                /*   A1.
                 *   1 0 - should be !A (2x2 square extermination)
                 *   .0.
                 */
                if (l)
                {
                    if (u)
                        return upper[-1] ? 0 : 1;
                    else
                        return lower[-1] ? 0 : 1;
                }
                else // r
                {
                    if (u)
                        return upper[1] ? 0 : 1;
                    else
                        return lower[1] ? 0 : 1;
                }
            }
            else
            {
                /*   .0.
                 *   1 1 - surely should be 1 to preserve connection
                 *   .0.
                 */
                return 1;
            }
        }
        break;/*}}}*/
        case 0: case 4:
            return 1; // lone pixels are NOT omitted
        default: assert(0); return 0;
    }
}
/*}}}*/

// pixels should have a margin of 1 pixel at each side
// returns true if the image was changed
static int 
donut_transform_graymap(byte **pixels, int w, int h, 
                        int rank, int **ranks)
/*{{{*/
{
    int i, j, result = 0;

    byte *buf = MALLOCV(byte,w*h);

    assert(pixels);
    assert(w);
    assert(h);

    for (i = 0; i < h; i++) for (j = 0; j < w; j++)
    {
        buf[w * i + j] =
            donut_transform_pixel(pixels[i-1] + j, pixels[i] + j, pixels[i+1] + j);
    }

    for (i = 0; i < h; i++)
    {
        byte *up = pixels[i-1], *row = pixels[i], *dn = pixels[i+1];
        byte *buf_row = buf + w * i;
        int *rank_row = NULL;
        int j;
        if (ranks) rank_row = ranks[i];
        for (j = 0; j < w; j++)
        {
            if (row[j] && !buf_row[j])
            {
                if (!donut_connectivity_test(up + j, row + j, dn + j))
                {
                    row[j] = buf_row[j];
                    if (rank) rank_row[j] = rank;
                    result = 1;
                }
            }
            else
                row[j] = buf_row[j];
        }
    }

    FREEV(buf);
    return result;
}
/*}}}*/

// TODO: use less temporary buffers and silly copyings
static void 
find_pith_and_signature(Image *img)
/*{{{*/
{
    byte **pixels = img->pixels;
    int w = img->width, h = img->height;
    byte *r = MALLOCV(byte, (h + 2) * (w + 2));
    byte **pointers = MALLOCV(byte*, h + 2);
    int *ranks_buf = MALLOCV(int, w * h);
    int **ranks = MALLOCV(int*, h);
    int i, j, passes = 1;
    double level = 1;
    byte *colors;

    memset(r, 0, sizeof(byte) * (w + 2) * (h + 2));
    memset(ranks_buf, 0, sizeof(int) * w * h);

    for (i = 0; i < h + 2; i++)
        pointers[i] = r + (w + 2) * i + 1;

    for (i = 0; i < h; i++)
        memcpy(pointers[i+1], pixels[i], w);

    for (i = 0; i < h; i++)
        ranks[i] = ranks_buf + w * i;

    while(donut_transform_graymap(pointers + 1, w, h, passes, ranks)) passes++;

    colors = MALLOCV(byte, passes + 1);

    for (i = 0; i < passes; i++)
    {
        colors[i] = (byte) (level * 255);
        level *= pith_falloff;
    }
    colors[passes] = 0;

    for (i = 1; i <= h; i++)
    {
        for (j = 0; j < w; j++)
        {
            if (pointers[i][j])
            {
                pixels[i-1][j] = 255;
            }
            else
            {
                pixels[i-1][j] = colors[passes - ranks[i - 1][j]];
            }
        }
    }

    get_signature(w, h, pixels, img->signature, 
                  sum_row_gray, sum_column_gray);
    get_signature(w, h, pixels, img->signature2, 
                  sum_row_black_and_white, sum_column_black_and_white);

    FREEV(colors);
    FREEV(ranks);
    FREEV(ranks_buf);
    FREEV(r);
    FREEV(pointers);
}/*}}}*/

// finding pith }}}

// shift signature comparison {{{

// Just finding the square of a normal Euclidean 
// distance between vectors (but with falloff)

#if USE_SHIFTDIFF_1 || USE_SHIFTDIFF_2 || USE_SHIFTDIFF_3

static int 
shiftdiff_equivalence(byte *s1, byte *s2, 
                      double falloff, double veto, double threshold)
{
    int i, delay_before_falloff = 1, delay_counter = 1;
    double penalty = 0;
    double weight = 1;

    for (i = 1; i < SIGNATURE_SIZE; i++) // kluge: ignores the first byte
    {
        int difference = s1[i] - s2[i];
        penalty += difference * difference * weight;
        if (!--delay_counter)
        {
            weight *= falloff;
            delay_counter = delay_before_falloff <<= 1;
        }
    }

    if (penalty >= veto * SIGNATURE_SIZE) return -1;
    if (penalty <= threshold * SIGNATURE_SIZE) return 1; 
    return 0;
}

#endif

// shift signature comparison }}}

// Exported functions without that classification stuff {{{

ComparableImage 
prepare_comparable_image(byte **pixels, int w, int h)
/*{{{*/
{
    int i;
    Image *img = MALLOC(Image);
    byte *pool = MALLOCV(byte, w * h);

    memset(pool, 0, w * h * sizeof(byte));

    img->width = w;
    img->height = h;
    img->pixels = MALLOCV(byte*, h);
    for (i = 0; i < h; i++)
        img->pixels[i] = pool + i * w;

    for (i = 0; i < h; i++)
    {
        int j;
        for (j = 0; j < w; j++) 
            if (pixels[i][j]) 
                img->pixels[i][j] = 255;
    }
    find_pith_and_signature(img);

    return img;
}
/*}}}*/

int 
compare_images(ComparableImage i1, ComparableImage i2)
/*{{{*/
{
    int i, state = 0; // 0 - unsure, 1 - equal unless veto

    if (dimensions_test(i1, i2)) return -1;

#if USE_SHIFTDIFF_1
    i = shiftdiff_equivalence(i1->signature, i2->signature,
                              shiftdiff1_falloff, 
                              shiftdiff1_veto_threshold, 
                              shiftdiff1_threshold);
        if (i == -1) return -1;
        state |= i;
#endif
        
#if USE_SHIFTDIFF_2
        i = shiftdiff_equivalence(i1->signature2, i2->signature2,
                                  shiftdiff2_falloff, 
                                  shiftdiff2_veto_threshold, 
                                  shiftdiff2_threshold);
        if (i == -1) return -1;
        state |= i;
#endif

#if USE_SHIFTDIFF_3
        i = shiftdiff_equivalence(i1->signature, i2->signature,
                                  shiftdiff3_falloff, 
                                  shiftdiff3_veto_threshold, 
                                  shiftdiff3_threshold);
        if (i == -1) return -1;
        state |= i;
#endif
        
#if USE_PITHDIFF
        i = pithdiff_equivalence(i1, i2);
        if (i == -1) return 0; // pithdiff has no right to veto at upper level
        state |= i;
#endif
        
#if USE_SOFTDIFF
        i = softdiff_equivalence(i1, i2);
        if (i == -1) return 0;  // softdiff has no right to veto at upper level
        state |= i;
#endif

        return state;
}
/*}}}*/

void 
free_comparable_image(ComparableImage img)
/*{{{*/
{
    FREEV(img->pixels[0]);
    FREEV(img->pixels);
    FREE(img);
}
/*}}}*/

// Exported functions without that classification stuff }}}

// Classification {{{

// Classes are single-linked lists with an additional pointer to the last
// node.  This is an class item.
typedef struct ClassNode {
    ComparableImage ptr;
    struct ClassNode *next; // == NULL if this node is the last one
    struct ClassNode *global_next; // next among all nodes to classify
    int tag; // filled before the final dumping
} ClassNode;

// Classes themselves are composed in double-linked list.
typedef struct Class {
    ClassNode *first, *last;
    struct Class *prev_class;
    struct Class *next_class;
} Class;

typedef struct Classification {
    Class *first_class;
    ClassNode *first_node, *last_node;
} Classification;

// Creates an empty class and links it to the list of classes.
static Class *
new_class(Classification *cl)
{
    Class *c = MALLOC(Class);
    c->first = c->last = NULL;
    c->prev_class = NULL;
    c->next_class = cl->first_class;
    if (cl->first_class) cl->first_class->prev_class = c;
    cl->first_class = c;
    return c;
}

// Unlinks a class and deletes it. Its nodes are not deleted.
static void 
delete_class(Classification *cl, Class *c)
{
    Class *prev = c->prev_class, *next = c->next_class;
    if (prev)
        prev->next_class = next;
    else
        cl->first_class = next;
    if (next)
        next->prev_class = prev;
    FREE(c);
}

// Creates a new node and adds it to the given class.
static ClassNode *
new_node(Classification *cl, Class *c, ComparableImage ptr)
{
    ClassNode *n = MALLOC(ClassNode);
    n->ptr = ptr;
    n->next = c->first;
    c->first = n;
    if (!c->last) c->last = n;
    n->global_next = NULL;
    if (cl->last_node)
        cl->last_node->global_next = n;
    else
        cl->first_node = n;
    cl->last_node = n;
    return n;
}

// Merges two classes and deletes one of them.
static Class *
merge(Classification *cl, Class *c1, Class *c2)
{
    if (!c1->first)
    {
        delete_class(cl, c1);
        return c2;
    }
    if (c2->first)
    {
        c1->last->next = c2->first;
        c1->last = c2->last;
    }
    delete_class(cl, c2);
    return c1;
}

// Puts a tag on each node corresponding to its class.
static unsigned int
put_tags(Classification *cl)
{
    int tag = 1;
    Class *c = cl->first_class;
    while (c)
    {
        ClassNode *n = c->first;
        while (n)
        {
            n->tag = tag;
            n = n->next;
        }
        c = c->next_class;
        tag++;
    }
    return tag - 1;
}

// Deletes all classes; nodes are untouched.
static void 
delete_all_classes(Classification *cl)
{
    Class *c = cl->first_class;
    while (c)
    {
        Class *t = c;
        c = c->next_class;
        FREE(t);
    }
}

// Compares p with nodes from c until a meaningful result.
static int 
compare_to_class(ComparableImage p, Class *c)
{
    int r = 0;
    ClassNode *n = c->first;
    while(n)
    {
        r = compare_images(p, n->ptr);
        if (r) break;
        n = n->next;
    }
    return r;
}

static void 
classify(Classification *cl, ComparableImage p)
{
    Class *class_of_this = NULL;
    Class *c, *next_c = NULL;
    for (c = cl->first_class; c; c = next_c)
    {
        next_c = c->next_class; // That's because c may be deleted in merging

        if (class_of_this == c) continue;
        if (compare_to_class(p, c) != 1) continue;
            
        if (class_of_this)
            class_of_this = merge(cl, class_of_this, c);
        else
            class_of_this = c;
    }
    if (!class_of_this) class_of_this = new_class(cl);
    new_node(cl, class_of_this, p);
}

unsigned int 
classify_images(ComparableImage *b, 
                unsigned int *r, unsigned int n)
{
    unsigned int i, max_tag;
    ClassNode *node;
    Classification cl;

    cl.first_class = NULL;
    cl.first_node = cl.last_node = NULL;

    for (i = 0; i < n; i++) if (b[i]) classify(&cl, b[i]);

    max_tag = put_tags(&cl);
    delete_all_classes(&cl);

    i = 0;
    node = cl.first_node;
    while (node)
    {
        ClassNode *t;
        while (!b[i]) r[i++] = 0;
        r[i++] = node->tag;
        t = node;
        node = node->global_next;
        FREE(t);
    }
    if (i < n) while (i < n) r[i++] = 0;
    return max_tag;
}
// Classification }}}
