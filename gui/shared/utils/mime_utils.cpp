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
// $Id: mime_utils.cpp,v 1.10 2007/03/25 20:48:28 leonb Exp $
// $Name: release_3_5_22 $

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#if NEED_GNUG_PRAGMAS
# pragma implementation
#endif

#include "GContainer.h"
#include "GString.h"
#include "debug.h"
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>


static GUTF8String
readRecord(FILE *f)
{
  GArray<char> buf;
  int state = 0;
  bool escape = false;
  int c;
  int pos = 0;
  while ((c = fgetc(f)) != EOF)
    {
      buf.touch(pos);
      buf[pos++] = c;
      if (escape)
        escape = false;
      else if ((c=='\\') && (state!='#')) 
        escape = true;
      else if ((c=='\'' || c=='\"') && (state==0))
        state = c;
      else if ((c==state) && (state=='\'' || state=='\"'))
        state = 0;
      else if ((c=='\n') && (state=='#' || state==0))
        break;
    }
  return GUTF8String((char*)buf, pos);
}

static int
readFile(FILE *f, GList<GUTF8String> &fdata)
{
  int lines = 0;
  fdata.empty();
  while (!feof(f)) {
    fdata.append(readRecord(f));
    lines += 1;
  }
  return lines;
}

static void
writeFile(FILE *f, GList<GUTF8String> &fdata)
{
  for (GPosition pos=fdata; pos; ++pos)
    fprintf(f,"%s",(const char*)fdata[pos]);
}

static bool
line_is_djvu(const char *l)
{
  // skip spaces
  while (isspace(*l))
    l += 1;
  // skip "type="
  if (! strncmp(l,"type=", 5))
    l += 5;
  // require "image/"
  if (strncmp(l,"image/", 6))
    return false;
  l += 6;
  // skip vnd and x prefixes
  if (! strncmp(l, "vnd.", 4))
    l += 4;
  else if (! strncmp(l, "x.", 2))
    l += 2;
  else if (! strncmp(l, "x-", 2))
    l += 2;
  // require djvu, dejavu, iw44 or jb2
  if (! strncmp(l, "djvu", 4))
    l += 4;
  else if (! strncmp(l, "dejavu", 6))
    l += 6;
  else if (! strncmp(l, "iw44", 4))
    l += 4;
  else if (! strncmp(l, "jb2", 3))
    l += 3;
  else
    return false;
  // check remaining char
  if (*l==0 || *l==';' || isspace(*l))
    return true;
  return false;
}

static bool
line_contains(const char *l, const char *s)
{
  int n = strlen(s);
  while (*l)
    {
      if (*l == *s)
        if (! strncmp(l,s,n))
          return true;
      l += 1;
    }
  return false;
}

static bool
line_is_ns_comment(const char *l)
{
  while (isspace(*l))
    l += 1;
  if (l[0]=='#' && line_contains(l,"Netscape Helper"))
    return true;
  return false;
}

static bool
line_matches(const char *l, const char *m)
{
  while (isspace(*l) || (l[0]=='\\' && isspace(l[1])))
    l += 1;
  while (isspace(*m))
    m += 1;
  while (*m && *l)
    {
      if (isspace(*m) && (isspace(*l) || (l[0]=='\\' && isspace(l[1]))))
        {
          while (isspace(*l) || (l[0]=='\\' && isspace(l[1])))
            l += 1;
          while (isspace(*m))
            m += 1;
        }
      else if (*l != *m)
        return false;
      l += 1;
      m += 1;
    }
  while (isspace(*l) || (l[0]=='\\' && isspace(l[1])))
    l += 1;
  while (isspace(*m))
    m += 1;
  if (*m || *l)
    return false;
  return true;
}

bool 
FixMailCap(bool really=false)
{
  bool modified = false;
  // Locate home directory
  const char *home = getenv("HOME");
  if (! home)
    return modified;
  // Locate mailcap file
  GUTF8String fname = GUTF8String(home) + "/.mailcap";
  FILE *f = fopen((const char*)fname, "r");
  if (! f)
    return modified;
  // Read mailcap file
  GList<GUTF8String> fdata;
  readFile(f, fdata);
  fclose(f);
  // Eliminate dubious records
  bool again = true;
  while (again)
    {
      again = false;
      bool previous_line_is_comment = false;
      for (GPosition pos = fdata; pos; ++pos)
        {
          bool zap = false;
          const char *l = (const char*) fdata[pos];
          if (line_is_ns_comment(l))
            {
              zap = previous_line_is_comment;
              previous_line_is_comment = true;
            }
          else
            {
              previous_line_is_comment = false;
              if (line_is_djvu(l))
                if (! line_contains(l, "x-mozilla-flags=plugin:DjVu 3.5"))
                  zap = true;
            }
          if (zap)
            {
              again = modified = true;
              fdata.del(pos);
              break;
            }
        }
    }
  // Save
  if (modified && really)
    {
      FILE *out = fopen((const char*)fname, "w");
      writeFile(out, fdata);
      fclose(out);
    }
  // Return modified
  return modified;
}


static const char *ns_comment = "#mime types added by Netscape Helper";

static const char *mime_types[] = {
  "type=image/x.djvu desc=\"DjVu File\"",
  "type=image/x-djvu desc=\"DjVu File\" exts=\"djvu,djv\"",
  "type=image/x-dejavu desc=\"DjVu File (obsolete mime type)\"",
  "type=image/x-iw44 desc=\"DjVu File (obsolete mime type)\" exts=\"iw44,iw4\"",
  "type=image/vnd.djvu desc=\"DjVu File\"",
  "type=image/djvu desc=\"DjVu File\"",
};

static const char *ns_start[] = {
  "#--Netscape Communications Corporation MIME Information\n",
  "#Do not delete the above line. It is used to identify the file type.\n",
  "#\n"
};

#define N_MIME_TYPES ((int)(sizeof(mime_types)/sizeof(mime_types[0])))

bool 
FixMimeTypes(bool really)
{
  bool modified = false;
  // Locate home directory
  const char *home = getenv("HOME");
  if (! home)
    return modified;
  // Locate mailcap file
  GUTF8String fname = GUTF8String(home) + "/.mime.types";
  FILE *f = fopen((const char*)fname, "r");
  // Read mailcap file
  GList<GUTF8String> fdata;
  if (f) {
    readFile(f, fdata);
    fclose(f);
  }
  // Check mailcap file
  if (! fdata.size() )
    {
      modified = true;
      fdata.append(ns_start[0]);
      fdata.append(ns_start[1]);
      fdata.append(ns_start[2]);
    }
  else
    {
      GPosition first = fdata;
      if ( (! line_contains((const char*)fdata[first], "Netscape")) ||
           (! line_contains((const char*)fdata[first], "MIME Information")) )
        return false;
    }
  // Initialize array of observed entries
  int i;
  bool seen[N_MIME_TYPES];
  for(i=0; i<N_MIME_TYPES; i++)
    seen[i] = false;
  // Eliminate dubious records
  bool again = true;
  while (again)
    {
      again = false;
      bool previous_line_is_comment = false;
      for (GPosition pos = fdata; pos; ++pos)
        {
          bool zap = false;
          const char *l = (const char*) fdata[pos];
          if (line_is_ns_comment(l)) 
            {
              zap = previous_line_is_comment;
              previous_line_is_comment = true;
            } 
          else
            {
              previous_line_is_comment = false;
              if (line_is_djvu(l))
                {
                  for (i=0; i<N_MIME_TYPES; i++)
                    if (line_matches(l, mime_types[i]))
                      break;
                  if (i<N_MIME_TYPES)
                    seen[i] = true;
                  else
                    zap = true;
                }
            }
          if (zap)
            {
              again = modified = true;
              fdata.del(pos);
              break;
            }
        }
    }
  // Check that all required lines are present
  for (i=0; i<N_MIME_TYPES; i++)
    if (! seen[i])
      modified = true;
  // Save
  if (modified && really)
    {
      FILE *out = fopen((const char*)fname, "w");
      writeFile(out, fdata);
      for (i=0; i<N_MIME_TYPES; i++)
        if (! seen[i])
          fprintf(out, "%s\n%s\n", ns_comment, mime_types[i]);
      fclose(out);
    }
  // Return
  return modified;
}
