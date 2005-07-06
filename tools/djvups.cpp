//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002-2003  Leon Bottou and Yann Le Cun.
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
//C- DjVuLibre-3.5 is derived from the DjVu(r) Reference Library
//C- distributed by Lizardtech Software.  On July 19th 2002, Lizardtech 
//C- Software authorized us to replace the original DjVu(r) Reference 
//C- Library notice by the following text (see doc/lizard2002.djvu):
//C-
//C-  ------------------------------------------------------------------
//C- | DjVu (r) Reference Library (v. 3.5)
//C- | Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- | The DjVu Reference Library is protected by U.S. Pat. No.
//C- | 6,058,214 and patents pending.
//C- |
//C- | This software is subject to, and may be distributed under, the
//C- | GNU General Public License, Version 2. The license should have
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
// $Id$

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "libdjvu/ddjvuapi.h"

#if defined(WIN32) || defined(__CYGWIN32__)
# include <io.h>
#endif

/* Some day we'll redo i18n right. */
#ifndef i18n
# define i18n(x) (x)
# define I18N(x) (x)
#endif

bool verbose = false;
bool tryhelp = false;
ddjvu_context_t *ctx;
ddjvu_document_t *doc;
ddjvu_job_t *job;

void
progress(int p)
{
  if (verbose)
    {
      int i=0;
      char buffer[52];
      for (; p>0; p-=2)
        buffer[i++]='#';
      for (; i<50;)
        buffer[i++]=' ';
      buffer[i] = 0;
      fprintf(stderr,"\r[%s]",buffer);
    }        
}

void
handle(int wait)
{
  const ddjvu_message_t *msg;
  if (!ctx)
    return;
  if (wait)
    msg = ddjvu_message_wait(ctx);
  while ((msg = ddjvu_message_peek(ctx)))
    {
      switch(msg->m_any.tag)
        {
        case DDJVU_ERROR:
          if (verbose)
            fprintf(stderr,"\n");
          fprintf(stderr,"djvups: %s\n", msg->m_error.message);
          if (msg->m_error.filename)
            fprintf(stderr,"djvups: '%s:%d'\n", 
                    msg->m_error.filename, msg->m_error.lineno);
          if (tryhelp)
            fprintf(stderr,"djvups: %s\n", i18n("Try option --help."));
          exit(10);
        case DDJVU_PROGRESS:
          if (verbose)
            progress(msg->m_progress.percent);
          break;
        default:
          break;
        }
      ddjvu_message_pop(ctx);
    }
}

void 
die(const char *fmt, ...)
{
  /* Handling messages might give a better error message */
  tryhelp = true;
  handle(FALSE);
  /* Print */
  va_list args;
  fprintf(stderr,"ddjvu: ");
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr,"\n");
  /* Terminates */
  exit(10);
}

void
usage(void)
{
#ifdef DJVULIBRE_VERSION
  fprintf(stderr, "DJVUPS --- DjVuLibre-" DJVULIBRE_VERSION "\n");
#endif
  fprintf(stderr, "%s",
     i18n("DjVu to PostScript conversion utility\n\n"
          "Usage: djvups [<options>] [<infile.djvu> [<outfile.ps>]]\n\n"
          "Options:\n"
          "  -help\n"
          "  -verbose\n"
          "  -page=<pagelists>                   (default: print all)\n"
          "  -format=<ps|eps>                    (default: ps)\n"
          "  -level=<1|2|3>                      (default: 2)\n"
          "  -orient=<auto|portrait|landscape>   (default: auto)\n"
          "  -mode=<color|bw|fore|back>          (default: color)\n"
          "  -zoom=<auto|25...2400)              (default: auto)\n"
          "  -color=<yes|no>                     (default: yes)\n"
          "  -gray                               (same as -color=no)\n"
          "  -colormatch=<yes|no>                (default: yes)\n"
          "  -gamma=<0.3...5.0>                  (default: 2.2)\n"
          "  -copies=<1...999999>                (default: 1)\n"
          "  -frame=<yes|no>                     (default: no)\n"
          "  -cropmarks=<yes|no>                 (default: no)\n"
          "  -text=<yes|no>                      (default: no)\n"
          "  -booklet=<no|recto|verso|yes>       (default: no)\n"
          "  -bookletmax=<n>                     (default: 0)\n"
          "  -bookletalign=<n>                   (default: 0)\n"
          "  -bookletfold=<n>[+<m>]              (default: 18+200)\n"
          "\n"));
  exit(1);
}

int
main(int argc, char **argv)
{
  int i;
  int optc = 0;
  char **optv;
  char *infile = 0;
  char *outfile = 0;
  FILE *fout;

  /* Sort options */
  if (! (optv = (char**)malloc(argc*sizeof(char*))))
    die(i18n("Out of memory"));
  for (i=1; i<argc; i++)
    {
      char *s = argv[i];
      if (s[0]=='-' && s[1]=='-')
        s = s+1;
      if (!strcmp(s,"-help") && !strcmp(s,"-h"))
        usage();
      else if (!strcmp(s,"-verbose"))
        verbose = true;
      else if (s[0]=='-' && s[1])
        optv[optc++] = s;
      else if (s[0] && !infile)
        infile = s;
      else if (s[0] && !outfile)
        outfile = s;
      else
        die(i18n("Incorrect arguments. Try option --help."));
    }
  if (! infile)
    infile = "-";
  if (! outfile)
    outfile = "-";
  /* Open document */
  if (! (ctx = ddjvu_context_create(argv[0])))
    die(i18n("Cannot create djvu context."));
  if (! (doc = ddjvu_document_create_by_filename(ctx, infile, TRUE)))
    die(i18n("Cannot open djvu document '%s'."), infile);
  while (! ddjvu_document_decoding_done(doc))
    handle(TRUE);
  /* Open output file */
  if (! strcmp(outfile,"-")) 
    {
      fout = stdout;
#if defined(__CYGWIN32__)
      setmode(fileno(fout), O_BINARY);
#elif defined(WIN32)
      _setmode(_fileno(fout), _O_BINARY);
#endif
    } 
  else if (! (fout = fopen(outfile, "wb")))
    die(i18n("Cannot open output file '%s'."), outfile);
  /* Create printing job */
  if (! (job = ddjvu_document_print(doc, fout, optc, optv)))
    die(i18n("Cannot create print job."));
  /* Wait until completion and cleanup */
  while (! ddjvu_job_done(job))
    handle(TRUE);
  if (verbose)
    fprintf(stderr,"\n");
  fclose(fout);
  if (job)
    ddjvu_job_release(job);
  if (doc)
    ddjvu_document_release(doc);
  if (ctx)
    ddjvu_context_release(ctx);
  return 0;
}
