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

#include "DjVuDocument.h"
#include "DjVmDir.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
#include "DjVuText.h"
#include "DjVuImage.h"
#include "GString.h"
#include "GOS.h"
#include "DjVuMessage.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>



bool cgi = false;
bool head = false;
GUTF8String pathtranslated;
GUTF8String requestmethod;


static void
usage(void)
{
   DjVuPrintErrorUTF8(
#ifdef DJVULIBRE_VERSION
          "DJVUSERVE --- DjVuLibre-" DJVULIBRE_VERSION "\n"
#endif
          "Extracts hidden text from Djvu files\n"
          "\n"
          "Usage: djvuserve [<djvufile>[/<djvmid>]\n"
          "Outputs the specified <djvufile> with valid Content-Type,\n"
          "Content-Length, and Expire HTTP headers.  Multipage DjVu\n"
          "documents can be accessed as indirect document using the <djvmid>\n"
          "syntax. Specifying a <djvmid> of <index> generates an indirect page\n"
          "directory pointing to the other component files.\n"
          "This program is designed to be used as a CGI executable.\n"
          "It uses environment variable PATH_TRANSLATED when executed\n"
          "without arguments.\n\n" );
   exit(10);
}

void
fprintdate(FILE *f, const char *fmt, const time_t *tim)
{
#ifdef HAVE_STRFTIME
  char ctim[128];
  struct tm *ttim = gmtime(tim);
  strftime(ctim, sizeof(ctim)-1, "%a, %d %b %Y %H:%M:%S GMT", ttim);
  fprintf(stdout, fmt, ctim);
#else
  fprintf(stdout,fmt, asctime(ttim));
#endif
}

void
headers(const struct stat *statbuf)
{
  fprintf(stdout,"Content-Type: image/x.djvu\n");
  fprintf(stdout,"Content-Length: %ld\n", statbuf->st_size);
  time_t tim = time(0) + 360 * 24 * 3600;
  fprintdate(stdout, "Last-Modified: %s\n", &statbuf->st_mtime);
  fprintdate(stdout, "Expires: %s\n", &tim);
}

void 
djvuserver_file(GURL pathurl)
{
  GNativeString fname = pathurl.NativeFilename();
  struct stat statbuf;
  if (stat((const char *)fname, &statbuf) < 0)
    G_THROW(strerror(errno));

  headers(&statbuf);
  if (head) 
    return;

  GP<ByteStream> in = ByteStream::create(pathurl,"rb");
  fprintf(stdout,"\n");
  GP<ByteStream> out = ByteStream::get_stdout();
  out->copy(*in);
}

void 
djvuserver_directory(GURL pathurl)
{
  GNativeString fname = pathurl.NativeFilename();
  struct stat statbuf;
  if (stat((const char *)fname, &statbuf) < 0)
    G_THROW(strerror(errno));
  // Find the DIRM chunk directly (save time)
  GP<ByteStream> temp;
  GP<ByteStream> bsin = ByteStream::create(pathurl,"rb");
  GP<DjVmDir> dir = DjVmDir::create();
  {
    GP<IFFByteStream> iffin = IFFByteStream::create(bsin);
    GUTF8String chkid;
    iffin->get_chunk(chkid);
    if (chkid != "FORM:DJVM")
      G_THROW( "This is not a multipage DjVu document" );
    while (iffin->get_chunk(chkid) && chkid!="DIRM")
      iffin->close_chunk();
    if (chkid != "DIRM")
      G_THROW( "This is not a new style bundled DjVu document" );
    temp = iffin->get_bytestream();
    dir->decode(temp);
    if (! dir->is_bundled())
      G_THROW( "This is not a bundled DjVu document" );
  }
  // Assemble index of indirect multipage file
  GP<ByteStream> bsdir = ByteStream::create();
  {
    GP<IFFByteStream> iff = IFFByteStream::create(bsdir);
    iff->put_chunk("FORM:DJVM",1);
    iff->put_chunk("DIRM");
    temp = iff->get_bytestream();
    dir->encode(temp, false, false);
    iff->close_chunk();
    iff->close_chunk();
  }
  // HTTP output
  statbuf.st_size = bsdir->tell();
  headers(&statbuf);
  if (head) 
    return;
  bsdir->seek(0);
  fprintf(stdout,"\n");
  GP<ByteStream> out = ByteStream::get_stdout();
  out->copy(*bsdir);
}

void 
djvuserver_component(GURL pathurl, GUTF8String id)
{
  GNativeString fname = pathurl.NativeFilename();
  struct stat statbuf;
  if (stat((const char *)fname, &statbuf) < 0)
    G_THROW(strerror(errno));
  // Find the DIRM chunk directly (save time)
  GP<ByteStream> temp;
  GP<ByteStream> bsin = ByteStream::create(pathurl,"rb");
  GP<DjVmDir> dir = DjVmDir::create();
  {
    GP<IFFByteStream> iffin = IFFByteStream::create(bsin);
    GUTF8String chkid;
    iffin->get_chunk(chkid);
    if (chkid != "FORM:DJVM")
      G_THROW( "This is not a multipage DjVu document" );
    while (iffin->get_chunk(chkid) && chkid!="DIRM")
      iffin->close_chunk();
    if (chkid != "DIRM")
      G_THROW( "This is not a new style bundled DjVu document" );
    temp = iffin->get_bytestream();
    dir->decode(temp);
    if (! dir->is_bundled())
      G_THROW( "This is not a bundled DjVu document" );
  }
  // Find the file record
  GP<DjVmDir::File> frec = dir->id_to_file(id);
  if (!frec)
    G_THROW( "Cannot locate requested component file" );
  if (!frec->size || !frec->offset)
    G_THROW( "Corrupted DjVu directory" );

  // HTTP output
  statbuf.st_size = frec->size + 4;
  headers(&statbuf);
  if (head) 
    return;
  fprintf(stdout,"\n");
  GP<ByteStream> out = ByteStream::get_stdout();
  out->writall("AT&T", 4);
  bsin->seek(frec->offset);
  out->copy(*bsin, frec->size);
}

int
main(int argc, char ** argv)
{
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);

  G_TRY 
    {
      // Obtain path
      if (argc == 1)
        {
          cgi = true;
          pathtranslated = GNativeString(getenv("PATH_TRANSLATED"));
          if (! pathtranslated)
            usage();
          requestmethod = GNativeString(getenv("REQUEST_METHOD"));
        }
      else if (argc == 2)
        {
          cgi = false;
          pathtranslated = GNativeString(argv[1]);
          requestmethod = "GET";
        }
      if (! pathtranslated)
        usage();
      head = false;
      if (requestmethod == "HEAD")
        head = true;
      else if (requestmethod != "GET")
        G_THROW("Only serve HEAD and GET requests");
      // Do it.
      GURL pathurl = GURL::Filename::UTF8(pathtranslated);
      if (pathurl.is_file())
        {
          djvuserver_file(pathurl);
        }
      else
        {
          GUTF8String id = pathurl.name();
          pathurl = pathurl.base();
          if (! pathurl.is_file())
            G_THROW("File not found");
          if (id == "index" || id == "index.djvu")
            djvuserver_directory(pathurl);
          else
            djvuserver_component(pathurl, id);
        }
    }
  G_CATCH(ex)
    {
      if (cgi)
        {
          fprintf(stdout,"Content-Type: text/html\n\n");
          fprintf(stdout,"<html><body><h2>Error executing djvuserve</h2><p><hr><pre>\n");
          fprintf(stdout,"*** %s\n",(const char *)DjVuMessageLite::LookUpUTF8(ex.get_cause()));
          fprintf(stdout,"*** REQUEST_METHOD=%s\n",(const char*)requestmethod);
          fprintf(stdout,"*** PATH_TRANSLATED=%s\n",(const char*)pathtranslated);
          fprintf(stdout,"</pre><hr></body></html>\n");
        }
      else
        {
          ex.perror();
        }
      exit(10);
    }
  G_ENDCATCH;
  exit(0);
}
