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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#include "IW44Image.h"
#include "GOS.h"
#include "GString.h"
#include "DjVuDocEditor.h"
#include "DjVuDumpHelper.h"
#include "BSByteStream.h"
#include "DjVuText.h"
#include "DjVuAnno.h"
#include "DjVuInfo.h"
#include "IFFByteStream.h"
#include "DataPool.h"
#include "DjVuPort.h"
#include "DjVuFile.h"


GNativeString djvufile;
GP<ByteStream> cmdbs;
GP<DjVuDocEditor> doc;
bool modified = false;
bool verbose = false;
bool save = false;
bool nosave = false;

GP<DjVuFile> file = 0;
GUTF8String fileid;


// --------------------------------------------------
// PARSING BYTESTREAM
// --------------------------------------------------

// -- A bytestream that performs buffering and 
//    offers a stdio-like interface for reading files.

class ParsingByteStream : public ByteStream 
{
private:
  static const size_t bufsize = 512;
  const GP<ByteStream> &gbs;
  ByteStream &bs;
  unsigned char buffer[bufsize];
  int  bufpos;
  int  bufend;
  bool goteof;
  ParsingByteStream(const GP<ByteStream> &gbs);
public:
  static GP<ParsingByteStream> create(const GP<ByteStream> &gbs) 
  { return new ParsingByteStream(gbs); }
  size_t read(void *buffer, size_t size);
  size_t write(const void *buffer, size_t size);
  long int tell() const;
  int eof();
  int unget(int c);
  inline int get();
  int get_spaces(bool skipseparator=false);
  GUTF8String get_utf8_token(bool skipseparator=false);
  GUTF8String get_native_token(bool skipseparator=false);
};

ParsingByteStream::ParsingByteStream(const GP<ByteStream> &xgbs)
  : gbs(xgbs),bs(*gbs), bufpos(1), bufend(1), goteof(false)
{ 
}

int 
ParsingByteStream::eof() // aka. feof
{
  if (bufpos < bufend) 
    return false;
  if (goteof)
    return true;
  bufend = bufpos = 1;
  while (bs.read(buffer+bufend,1) && ++bufend<(int)bufsize)
    if (buffer[bufend-1]=='\r' || buffer[bufend-1]=='\n')
      break;
  if (bufend == bufpos)
    goteof = true;
  return goteof;
}

size_t 
ParsingByteStream::read(void *buf, size_t size)
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
ParsingByteStream::write(const void *, size_t )
{
  G_THROW("Cannot write() into a ParsingByteStream");
}

long int
ParsingByteStream::tell() const
{ 
  G_THROW("Cannot tell() a ParsingByteStream");
}

inline int 
ParsingByteStream::get() // aka. getc()
{
  if (bufpos < bufend || !eof())
    return buffer[bufpos++];
  return EOF;
}

int  
ParsingByteStream::unget(int c) // aka. ungetc()
{
  if (bufpos > 0 && c != EOF) 
    return buffer[--bufpos] = (unsigned char)c;
  return EOF;
}

int
ParsingByteStream::get_spaces(bool skipseparator)
{
   int c = get();
   while (c==' ' || c=='\t' || c=='\r' 
          || c=='\n' || c=='#' || c==';' )
     {
       if (c == '#')
         do { c=get(); } while (c!=EOF && c!='\n' && c!='\r');
       if (!skipseparator && (c=='\n' || c=='\r' || c==';'))
         break;
       c = get();
     }
   return c;
}
  
GUTF8String
ParsingByteStream::get_utf8_token(bool skipseparator)
{
  GUTF8String str;
  int c = get_spaces(skipseparator);
  if (c == EOF)
    {
      return str;
    }
  if (!skipseparator && (c=='\n' || c=='\r' || c==';'))
    {
      unget(c);
      return str;
    }
  if (c != '\"' && c != '\'') 
    {
      while (c!=' ' && c!='\t' && c!='\r' && c!=';'
             && c!='\n' && c!='#' && c!=EOF)
        {
          str += c;
          c = get();
        }
      unget(c);
    }
  else 
    {
      int delim = c;
      c = get();
      while (c != delim && c!=EOF) 
        {
          if (c == '\\') 
            {
              c = get();
              if (c>='0' && c<='7')
                {
                  int x = 0;
                  for (int i=0; i<3 && c>='0' && c<='7'; i++) 
                    {
                      x = x * 8 + c - '0';
                      c = get();
                    }
                  unget(c);
                  c = x;
                }
              else 
                {
                  char *tr1 = "tnrbfva";
                  char *tr2 = "\t\n\r\b\f\013\007";
                  for (int i=0; tr1[i]; i++)
                    if (c == tr1[i])
                      c = tr2[i];
                }
            }
          if (c != EOF)
            str += c;
          c = get();
        }
    }
  return str;
}

GUTF8String
ParsingByteStream::get_native_token(bool skipseparator)
{
  GUTF8String fake = get_utf8_token(skipseparator);
  GNativeString nat((const char*)fake);
  return GUTF8String(nat);
}


// --------------------------------------------------
// COMMANDS
// --------------------------------------------------


void
vprint(const char *fmt, ... )
#ifdef __GNUC__
  __attribute__ ((format (printf, 1, 2)));
void
vprint(const char *fmt, ... )
#endif
{
  if (verbose)
    {
      GNativeString msg("");
      va_list args;
      va_start(args, fmt);
      msg.vformat(fmt, args);
      fprintf(stderr,"djvused: %s\n", (const char*)msg);
    }
}

void
verror(const char *fmt, ... )
#ifdef __GNUC__
  __attribute__ ((format (printf, 1, 2)));
void
verror(const char *fmt, ... )
#endif
{
  GNativeString msg;
  va_list args;
  va_start(args, fmt);
  msg.vformat(fmt, args);
  G_THROW((const char*)msg);
}

void
get_data_from_file(const char *cmd, ParsingByteStream &pbs, ByteStream &out)
{
  GUTF8String fname = pbs.get_native_token();

  if (! fname)
    {
      vprint("%s: enter data and terminate with a period on a single line", cmd);
      int c = pbs.get_spaces(true);
      pbs.unget(c);
      char skip[4];
      char term[4] = "\n.\n";
      char *s = skip;
      int state = 1;
      while (state < 3) 
        {
          c = pbs.get();
          if ( c == term[state] )
            {
              state += 1;
              *s++ = c;
            }
          else
            {
              for (char *m=skip; m<s; m++)
                out.write8(*m);
              s = skip;
              state = 0;
              if (c == '\n')
                pbs.unget(c);
              else if (c != EOF)
                out.write8(c);
            }
        }
      pbs.unget('\n');
    }
  else
    {
      GP<ByteStream> in=ByteStream::create(GURL::Filename::UTF8(fname),"rb");
      out.copy(*in);
    }
}

void
print_c_string(const char *data, int length, ByteStream &out)
{
  out.write("\"",1);
  while (*data && length>0) 
    {
      int span = 0;
      while (span<length && data[span]>=0x20 && 
             data[span]<0x7f && data[span]!='"' && data[span]!='\\' )
        span++;
      if (span > 0) 
        {
          out.write(data, span);
          data += span;
          length -= span;
        }
      else
        {
          char buffer[5];
          sprintf(buffer,"\\%03o", (int)*(unsigned char*)data);
          out.write(buffer,4);
          data += 1;
          length -= 1;
        }
    }
  out.write("\"",1);
}

void
command_ls(ParsingByteStream &)
{
  int pagenum = 0;
  GPList<DjVmDir::File> lst = doc->get_djvm_dir()->get_files_list();
  for (GPosition p=lst; p; ++p) 
    {
      GP<DjVmDir::File> f = lst[p];
      if (f->is_page())
        fprintf(stdout,"%4d P ", ++pagenum);
      else if (f->is_include())
        fprintf(stdout,"     I ");
      else if (f->is_thumbnails())
        fprintf(stdout,"     T ");
      else if (f->is_shared_anno())
        fprintf(stdout,"     A ");
      else
        fprintf(stdout,"     ? ");
      GUTF8String id = f->get_load_name();
      fprintf(stdout,"%8d  %s\n", f->size, (const char*)(GNativeString)id);
    }
  if (doc->get_thumbnails_num() == doc->get_pages_num())
    fprintf(stdout,"     T %8s  %s\n", "", "<thumbnails>");
}

void
command_n(ParsingByteStream &)
{
  int pagenum = 0;
  GPList<DjVmDir::File> lst = doc->get_djvm_dir()->get_files_list();
  for (GPosition p=lst; p; ++p) 
    {
      GP<DjVmDir::File> f = lst[p];
      if (f->is_page())
        ++pagenum;
    }
  fprintf(stdout,"%d\n", pagenum); 
}

void
command_dump(ParsingByteStream &)
{
  GP<DataPool> pool;
  if (file)
    pool = file->get_djvu_data(false, false);
  else
    pool = doc->get_init_data_pool();
  DjVuDumpHelper helper;
  GP<ByteStream> bs = helper.dump(pool);
  GP<ByteStream> obs=ByteStream::create("w");
  bs->seek(0);
  obs->copy(*bs);
}

void
print_size(const GP<DjVuFile> &file)
{
  const GP<ByteStream> pbs(file->get_djvu_bytestream(false, false));
  const GP<IFFByteStream> iff(IFFByteStream::create(pbs));
  GUTF8String chkid;
  if (! iff->get_chunk(chkid))
    verror("Selected file contains no data");
  if (chkid == "FORM:DJVU")
    {
      while (iff->get_chunk(chkid) && chkid!="INFO")
        iff->close_chunk();
      if (chkid == "INFO")
        {
          GP<DjVuInfo> info=DjVuInfo::create();
          info->decode(*iff->get_bytestream());
          fprintf(stdout,"width=%d height=%d\n", info->width, info->height);
        }
    }
  else if (chkid == "FORM:BM44" || chkid == "FORM:PM44")
    {
      while (iff->get_chunk(chkid) && chkid!="BM44" && chkid!="PM44")
        iff->close_chunk();
      if (chkid=="BM44" || chkid=="PM44")
        {
          GP<IW44Image> junk=IW44Image::create_decode(IW44Image::COLOR);
          junk->decode_chunk(iff->get_bytestream());
          fprintf(stdout,"width=%d height=%d\n", 
                  junk->get_width(), junk->get_height());
        }
    }
}

void
command_size(ParsingByteStream &)
{
  if (file)
    {
      print_size(file);
    }
  else 
    {
      GPList<DjVmDir::File> lst = doc->get_djvm_dir()->get_files_list();
       for (GPosition p=lst; p; ++p)
        {
          if (lst[p]->is_page())
            {
              const GP<DjVuFile> f(doc->get_djvu_file(doc->page_to_id(lst[p]->get_page_num())));
              print_size(f);
            }
        }
    }
}

void
command_select(ParsingByteStream &pbs)
{
  GUTF8String pagid = pbs.get_native_token();
  // Case of NULL
  if (pagid == "") 
    {
      file = 0;
      fileid = "";
      vprint("select: selecting entire document");
      return;
    } 
  // Case of page number
  if (pagid.is_int())
    {
      int pageno = atoi(pagid);
      if (pageno<=0 || pageno>doc->get_pages_num())
        verror("incorrect page number:  %d", pageno);
      pagid = doc->page_to_id(pageno-1);
    }
  // General case
  const GP<DjVuFile> f(doc->get_djvu_file(pagid));
  if (!f)
    verror("page \"%s\" not found", (const char*)(GNativeString)pagid);    
  file = f;
  fileid = pagid;
  vprint("select: selecting \"%s\"", (const char*)(GNativeString)pagid);
}

void
command_select_shared_ant(ParsingByteStream &)
{
  const GP<DjVuFile> f(doc->get_shared_anno_file());
  if (!f) 
    verror("select-shared-ant: no shared annotation file");
  file = f;
  fileid = "<shared_ant>";
  vprint("select-shared-ant: selecting shared annotation");
}

void
command_create_shared_ant(ParsingByteStream &)
{
  GP<DjVuFile> f(doc->get_shared_anno_file());
  if (!f) 
    {
      vprint("create-shared-ant: creating shared annotation file");
      doc->create_shared_anno_file();
      f = doc->get_shared_anno_file();
      if (!f) G_THROW("internal error");
    }
  file = f;
  fileid = "<shared_ant>";
  vprint("create-shared-ant: selecting shared annotation");
}

void
print_ant(IFFByteStream &iff, ByteStream &out)
{
  GUTF8String chkid;
  while (iff.get_chunk(chkid))
    {
      if (chkid == "ANTa") 
        {
          out.copy(*iff.get_bytestream());
          out.write8('\n');
        }
      else if (chkid == "ANTz") 
        {
          GP<ByteStream> bsiff=BSByteStream::create(iff.get_bytestream());
          out.copy(*bsiff);
          out.write8('\n');
        }
      iff.close_chunk();
    }
}

void
command_print_ant(ParsingByteStream &)
{
  if (!file)
    verror("you must first select a page");
  GP<ByteStream> out=ByteStream::create("w");
  GP<ByteStream> anno = file->get_anno();
  if (! (anno && anno->size())) return;
  GP<IFFByteStream> iff=IFFByteStream::create(anno);
  print_ant(*iff, *out);
}

void
command_print_merged_ant(ParsingByteStream &)
{
  if (!file)
    verror("you must first select a page");
  GP<ByteStream> out=ByteStream::create("w");
  GP<ByteStream> anno = file->get_merged_anno();
  if (! (anno && anno->size())) return;
  GP<IFFByteStream> iff=IFFByteStream::create(anno);
  print_ant(*iff, *out);
}


static void
modify_ant(const GP<DjVuFile> &f, 
           const char *newchunkid,
           const GP<ByteStream> newchunk )
{
  const GP<ByteStream> anno(ByteStream::create());
  if (newchunkid && newchunk && newchunk->size())
    {
      const GP<IFFByteStream> out(IFFByteStream::create(anno));
      newchunk->seek(0);
      out->put_chunk(newchunkid);
      out->copy(*newchunk);
      out->close_chunk();
    }
  f->anno = anno;
  if (! anno->size())
    f->remove_anno();
  f->set_modified(true);
  modified = true;
}

void
file_remove_ant(const GP<DjVuFile> &f, const char *id)
{
  if (!f) return;
  modify_ant(f, 0, 0);
  vprint("remove_ant: modified \"%s\"", id);
}

void
command_remove_ant(ParsingByteStream &)
{
  if (file) 
    {
      file_remove_ant(file, fileid);
    }
  else 
    {
      GPList<DjVmDir::File> lst = doc->get_djvm_dir()->get_files_list();      
      for (GPosition p=lst; p; ++p)
        {
          GUTF8String id = lst[p]->get_load_name();
          const GP<DjVuFile> f(doc->get_djvu_file(id));
          file_remove_ant(f, id);
        }
    }
}

void
command_set_ant(ParsingByteStream &pbs)
{
  if (! file)
    verror("must select a page first");
  const GP<ByteStream> ant=ByteStream::create();
  { 
    GP<ByteStream> bsant=BSByteStream::create(ant,100);
    get_data_from_file("set-ant", pbs, *bsant);
  }
  modify_ant(file, "ANTz", ant);
  vprint("set-ant: modified \"%s\"", (const char*)(GNativeString)fileid);
}

void
print_meta(IFFByteStream &iff, ByteStream &out)
{
  GUTF8String chkid;  
  while (iff.get_chunk(chkid))
    {
      bool ok = false;
      GP<DjVuANT> ant=DjVuANT::create();
      if (chkid == "ANTz") {
          GP<ByteStream> bsiff=BSByteStream::create(iff.get_bytestream());
          ant->decode(*bsiff);
          ok = true;
      } else if (chkid == "ANTa") {
        ant->decode(*iff.get_bytestream());
        ok = true;
      }
      if (ok)
        {
          for (GPosition pos=ant->metadata; pos; ++pos)
            { 
              GUTF8String tmp;
              tmp=ant->metadata.key(pos);
              out.writestring(tmp); 
              out.write8('\t');
              tmp=ant->metadata[pos];
              print_c_string((const char*)tmp, tmp.length(), out);
              out.write8('\n');
            }
        }
      iff.close_chunk();
    }
}

void 
command_print_meta(ParsingByteStream &)
{
  if (!file)
    verror("you must first select a page");
  GP<ByteStream> out=ByteStream::create("w");
  GP<ByteStream> anno = file->get_anno();
  if (! (anno && anno->size())) return;
  GP<IFFByteStream> iff=IFFByteStream::create(anno); 
  print_meta(*iff,*out);
}

static bool
filter_meta_out(GP<ByteStream> in, GP<ByteStream> out)
{
  int c;
  int plevel = 0;
  bool copy = true;
  bool unchanged = true;
  GP<ParsingByteStream> inp = ParsingByteStream::create(in);
  while ((c = inp->get()) != EOF)
    if (c!=' ' && c!='\t' && c!='\r' && c!='\n')
      break;
  inp->unget(c);
  while ((c = inp->get()) != EOF)
    {
      if (plevel == 0)
        if (c !=' ' && c!='\t' && c!='\r' && c!='\n')
          copy = true;
      if (c == '#')
        {
          while (c!=EOF && c!='\n' && c!='\r') {
            if (copy) 
              out->write8(c);
            c = inp->get();
          }
        }
      else if (c == '\"')
        {
          inp->unget(c);
          GUTF8String token = inp->get_utf8_token();
          if (copy)
            print_c_string(token, token.length(), *out);
        }
      else if (c == '(')
        {
          if (plevel == 0)
            {
              GUTF8String token = inp->get_utf8_token();
              if (token == "metadata")
                copy = unchanged = false;
              if (copy) {
                out->write8('(');
                out->write((const char*)token, token.length());
              }
            }
          else if (copy) 
            out->write8(c);
          plevel += 1;
        }
      else if (c == ')')
        {
          if (copy) 
            out->write8(c);
          if ( --plevel < 0)
            plevel = 0;
        }
      else if (copy)
        out->write8(c);
    }
  return !unchanged;
}

static bool
modify_meta(const GP<DjVuFile> &f,
            GMap<GUTF8String,GUTF8String> *newmeta)
{
  bool changed = false;
  GP<ByteStream> newant = ByteStream::create();
  if (newmeta && !newmeta->isempty())
    {
      newant->writestring(GUTF8String("(metadata"));
      for (GPosition pos=newmeta->firstpos(); pos; ++pos)
        {
          GUTF8String key = newmeta->key(pos); 
          GUTF8String val = (*newmeta)[pos];
          newant->write("\n\t(",3);
          newant->writestring(key);
          newant->write(" ",1);
          print_c_string((const char*)val, val.length(), *newant);
          newant->write(")",1);
        }
      newant->write(" )\n",3);
      changed = true;
    }
  GP<ByteStream> anno = f->get_anno();
  if (anno && anno->size()) 
    {
      GP<IFFByteStream> iff=IFFByteStream::create(anno);
      GP<ByteStream> oldant = ByteStream::create();
      print_ant(*iff,*oldant);
      oldant->seek(0);
      if (filter_meta_out(oldant, newant))
        changed = true;
    }
  const GP<ByteStream> newantz=ByteStream::create();
  if (changed)
    {
      newant->seek(0);
      { GP<ByteStream> bzz = BSByteStream::create(newantz,100); bzz->copy(*newant); }
      newantz->seek(0);
      modify_ant(f, "ANTz", newantz);
    }
  return changed;
}

void
file_remove_meta(const GP<DjVuFile> &f, const char *id)
{
  if (modify_meta(f, 0))
    vprint("remove_meta: modified \"%s\"", id);
}

void 
command_remove_meta(ParsingByteStream &)
{
  if (file) 
    {
      file_remove_meta(file, fileid);
    }
  else 
    {
      GPList<DjVmDir::File> lst = doc->get_djvm_dir()->get_files_list();      
      for (GPosition p=lst; p; ++p)
        {
          GUTF8String id = lst[p]->get_load_name();
          const GP<DjVuFile> f(doc->get_djvu_file(id));
          file_remove_meta(f, id);
        }
    }
}

void
command_set_meta(ParsingByteStream &pbs)
{
  if (!file)
    verror("you must first select a page");
  // get metadata
  GP<ByteStream> metastream = ByteStream::create();
  get_data_from_file("set-meta", pbs, *metastream);
  metastream->seek(0);
  // parse metadata
  GMap<GUTF8String,GUTF8String> metadata;
  GP<ParsingByteStream> inp = ParsingByteStream::create(metastream);
  int c;
  while ( (c = inp->get_spaces(true)) != EOF )
    {
      GUTF8String key, val;
      inp->unget(c);
      key = inp->get_utf8_token();
      c = inp->get_spaces(false);
      if (c == '\"') {
        inp->unget(c);
        val = inp->get_utf8_token();
      } else {
        while (c!='\n' && c!='\r' && c!=EOF) {
          val += c;
          c = inp->get();
        }
      }
      if (key.length()>0 && val.length()>0)
        metadata[key] = val;
    }
  // set metadata
  if (modify_meta(file, &metadata))
    vprint("set-meta: modified \"%s\"", (const char*)(GNativeString)fileid);
}

static struct 
{ 
  const char *name;
  DjVuTXT::ZoneType ztype;
  const char separator;
} 
zone_names[] = 
{
  { "page",   DjVuTXT::PAGE,      0 },
  { "column", DjVuTXT::COLUMN,    DjVuTXT::end_of_column },
  { "region", DjVuTXT::REGION,    DjVuTXT::end_of_region },
  { "para",   DjVuTXT::PARAGRAPH, DjVuTXT::end_of_paragraph },
  { "line",   DjVuTXT::LINE,      DjVuTXT::end_of_line },
  { "word",   DjVuTXT::WORD,      ' ' },
  { "char",   DjVuTXT::CHARACTER, 0 },
  { 0, (DjVuTXT::ZoneType)0 ,0 }
};

GP<DjVuTXT>
get_text(const GP<DjVuFile> &file)
{ 
  GUTF8String chkid;
  const GP<ByteStream> bs(file->get_text());
  if (bs) 
    {
      long int i=0;
      const GP<IFFByteStream> iff(IFFByteStream::create(bs));
      while (iff->get_chunk(chkid))
        {
          i++;
          if (chkid == GUTF8String("TXTa")) 
            {
              GP<DjVuTXT> txt = DjVuTXT::create();
              txt->decode(iff->get_bytestream());
              return txt;
            }
          else if (chkid == GUTF8String("TXTz")) 
            {
              GP<DjVuTXT> txt = DjVuTXT::create();
              GP<ByteStream> bsiff=BSByteStream::create(iff->get_bytestream());
              txt->decode(bsiff);
              return txt;
            }
          iff->close_chunk();
        }
    }
  return 0;
}

void
print_txt_sub(const GP<DjVuTXT> &txt, DjVuTXT::Zone &zone, ByteStream &out, int indent)
{
  // Indentation
  if (indent)
    {
      out.write("\n",1);
      static const char spaces[] = "                        ";
      if (indent > (int)sizeof(spaces))
        indent = sizeof(spaces);
      out.write(spaces, indent);
    }
  // Zone header
  int zinfo;
  for (zinfo=0; zone_names[zinfo].name; zinfo++)
    if (zone.ztype == zone_names[zinfo].ztype)
      break;
  GUTF8String message = "(bogus";
  if (zone_names[zinfo].name)
    message.format("(%s %d %d %d %d", zone_names[zinfo].name,
                   zone.rect.xmin, zone.rect.ymin, 
                   zone.rect.xmax, zone.rect.ymax);
  out.write((const char*)message, message.length());
  // Zone children
  if (zone.children.isempty()) 
    {
      const char *data = txt->textUTF8.getbuf() + zone.text_start;
      int length = zone.text_length;
      if (data[length-1] == zone_names[zinfo].separator)
        length -= 1;
      out.write(" ",1);
      print_c_string(data, length, out);
    }
  else
    {
      for (GPosition pos=zone.children; pos; ++pos)
        print_txt_sub(txt, zone.children[pos], out, indent + 1);
    }
  // Finish
  out.write(")",1);
  if (!indent)
    out.write("\n", 1);
}

void
print_txt(const GP<DjVuTXT> &txt, ByteStream &out)
{
  if (txt)
    print_txt_sub(txt, txt->page_zone, out, 0);
}

void
command_print_txt(ParsingByteStream &)
{
  const GP<ByteStream> out(ByteStream::create("w"));
  if (file)
    {
      print_txt(get_text(file), *out);
    }
  else
    {
      GPList<DjVmDir::File> lst = doc->get_djvm_dir()->get_files_list();
      for (GPosition p=lst; p; ++p)
        {
          GUTF8String id = lst[p]->get_load_name();
          const GP<DjVuFile> f(doc->get_djvu_file(id));
          print_txt(get_text(f), *out);
        }
    }
}

void
command_print_pure_txt(ParsingByteStream &)
{
  const GP<ByteStream> out(ByteStream::create("w"));
  GP<DjVuTXT> txt;
  if (file)
    {
      if ((txt = get_text(file)))
        {
          GUTF8String ntxt = txt->textUTF8;
          out->write((const char*)ntxt, ntxt.length());
        }
    }
  else
    {
      GPList<DjVmDir::File> lst = doc->get_djvm_dir()->get_files_list();
      for (GPosition p=lst; p; ++p)
        {
          GUTF8String id = lst[p]->get_load_name();
          const GP<DjVuFile> f(doc->get_djvu_file(id));
          if ((txt = get_text(f)))
            {
              GUTF8String ntxt = txt->textUTF8;
              out->write((const char*)ntxt, ntxt.length());
            }
          out->write("\f",1);
        }
    }
}

static void
modify_txt(const GP<DjVuFile> &f, 
           const char *newchunkid,
           const GP<ByteStream> newchunk )
{
  const GP<ByteStream> text(ByteStream::create());
  if (newchunkid && newchunk && newchunk->size())
    {
      const GP<IFFByteStream> out(IFFByteStream::create(text));
      newchunk->seek(0);
      out->put_chunk(newchunkid);
      out->copy(*newchunk);
      out->close_chunk();
    }
  f->text = text;
  if (! text->size())
    f->remove_text();
  f->set_modified(true);
  modified = true;
}

void
file_remove_txt(const GP<DjVuFile> &f, const char *id)
{
  if (! f) return;
  modify_txt(f, 0, 0);
  vprint("remove-txt: modified \"%s\"", id);
}

void
command_remove_txt(ParsingByteStream &)
{
  if (file) 
    {
      file_remove_txt(file, fileid);
    }
  else 
    {
      GPList<DjVmDir::File> lst = doc->get_djvm_dir()->get_files_list();      
      for (GPosition p=lst; p; ++p)
        {
          GUTF8String id = lst[p]->get_load_name();
          const GP<DjVuFile> f(doc->get_djvu_file(id));
          file_remove_txt(f, id);
        }
    }
}

void
construct_djvutxt_sub(ParsingByteStream &pbs, 
                      const GP<DjVuTXT> &txt, DjVuTXT::Zone &zone,
                      int mintype, bool exact)
{
  int c;
  GUTF8String token;
  // Get zone type
  c = pbs.get_spaces(true);
  if (c != '(')
    verror("syntax error in txt data: got '%c', expecting '('", c);
  token = pbs.get_utf8_token(true);
  int zinfo;
  for (zinfo=0; zone_names[zinfo].name; zinfo++)
    if (token == zone_names[zinfo].name)
      break;
  if (! zone_names[zinfo].name)
    verror("Syntax error in txt data: undefined token '%s'",
           (const char*)(GNativeString)token);
  zone.ztype = zone_names[zinfo].ztype;
  if (zone.ztype<mintype || (exact && zone.ztype>mintype))
    verror("Syntax error in txt data: illegal zone token '%s'",
           (const char*)(GNativeString)token);           
  // Get zone rect
  GUTF8String str;
  str = pbs.get_utf8_token(true);
  if (!str || !str.is_int()) 
    nerror: verror("Syntax error in txt data: number expected, got '%s'",
                   (const char*)(GNativeString)str);  
  zone.rect.xmin = atoi(str);
  str = pbs.get_utf8_token(true);
  if (!str || !str.is_int()) 
    goto nerror;
  zone.rect.ymin = atoi(str);
  str = pbs.get_utf8_token(true);
  if (!str || !str.is_int()) 
    goto nerror;
  zone.rect.xmax = atoi(str);
  str = pbs.get_utf8_token(true);
  if (!str || !str.is_int()) 
    goto nerror;
  zone.rect.ymax = atoi(str);
  if (zone.rect.xmin > zone.rect.xmax) 
    {
      int tmp = zone.rect.xmin; 
      zone.rect.xmin=zone.rect.xmax; 
      zone.rect.xmax=tmp; 
    }
  if (zone.rect.ymin > zone.rect.ymax)
    {
      int tmp = zone.rect.ymin; 
      zone.rect.ymin=zone.rect.ymax; 
      zone.rect.ymax=tmp; 
    }
  // Continue processing
  c = pbs.get_spaces(true);
  pbs.unget(c);
  if (c == '"') 
    {
      // This is a terminal
      str = pbs.get_utf8_token(true);
      zone.text_start = txt->textUTF8.length();
      zone.text_length = str.length();
      txt->textUTF8 += str;
      
    }
  else 
    {
      // This is a non terminal
      while (c != ')')
        {
          if (c != '(')
            verror("Syntax error in txt data: expecting subzone");
          DjVuTXT::Zone *nzone = zone.append_child();
          construct_djvutxt_sub(pbs, txt, *nzone, zone.ztype+1, false);
          c = pbs.get_spaces(true);
          pbs.unget(c);
        }
    }
  // Skip last parenthesis
  c = pbs.get_spaces(true);
  if (c != ')')
    verror("Syntax error in txt data: missing parenthesis");
}

GP<DjVuTXT>
construct_djvutxt(ParsingByteStream &pbs)
{
  GP<DjVuTXT> txt(DjVuTXT::create());
  int c = pbs.get_spaces(true);
  if (c == EOF)
    return 0;
  pbs.unget(c);
  construct_djvutxt_sub(pbs, txt, txt->page_zone, DjVuTXT::PAGE, true);
  if (pbs.get_spaces(true) != EOF)
    verror("Syntax error in txt data: garbage after data");
  txt->normalize_text();
  return txt;
}

void
command_set_txt(ParsingByteStream &pbs)
{
  if (! file)
    verror("must select a page first");
  const GP<ByteStream> txtbs(ByteStream::create());
  get_data_from_file("set-txt", pbs, *txtbs);
  txtbs->seek(0);
  GP<ParsingByteStream> txtpbs(ParsingByteStream::create(txtbs));
  const GP<DjVuTXT> txt(construct_djvutxt(*txtpbs));
  GP<ByteStream> txtobs=ByteStream::create();
  if (txt)
    {
      const GP<ByteStream> bsout(BSByteStream::create(txtobs,1000));
      txt->encode(bsout);
    }
  txtobs->seek(0);
  modify_txt(file, "TXTz", txtobs);
  vprint("set-txt: modified \"%s\"", (const char*)(GNativeString)fileid);
}

void
output(const GP<DjVuFile> &f, ByteStream &out, int flag, const char *id=0)
{
  if (f)
    {
      const GP<ByteStream> ant(ByteStream::create());
      const GP<ByteStream> txt(ByteStream::create());
      if (flag & 1) 
        { 
          const GP<ByteStream> anno(f->get_anno());
          if (anno && anno->size()) 
            {
              const GP<IFFByteStream> iff(IFFByteStream::create(anno)); 
              print_ant(*iff,*ant); 
              ant->seek(0); 
            }
        }
      if (flag & 2)
        { 
          print_txt(get_text(f),*txt); 
          txt->seek(0); 
        }
      if (id && ant->size() + txt->size())
        {
          GUTF8String msg;
          msg.format("# -------------------------------------\n"
                     "select '%s'\n", id);
          out.write((const char*)msg, msg.length());
        }
      if (ant->size()) 
        {
          out.write("set-ant\n", 8);
          out.copy(*ant);
          out.write(".\n", 2);
        }
      if (txt->size()) 
        {
          out.write("set-txt\n", 8);
          out.copy(*txt);
          out.write(".\n", 2);
        }
    }
}

void
command_output_ant(ParsingByteStream &)
{
  const GP<ByteStream> out(ByteStream::create("w"));
  if (file) 
    {
      output(file, *out, 1);
    }
  else 
    {
      const char *pre = "select; remove-ant\n";
      out->write(pre, strlen(pre));
      GPList<DjVmDir::File> lst = doc->get_djvm_dir()->get_files_list();
      for (GPosition p=lst; p; ++p)
        {
          GUTF8String id = lst[p]->get_load_name();
          const GP<DjVuFile> f(doc->get_djvu_file(id));
          output(f, *out, 1, id);
        }
    }
}

void
command_output_txt(ParsingByteStream &)
{
  const GP<ByteStream> out(ByteStream::create("w"));
  if (file) 
    {
      output(file, *out, 2);
    }
  else 
    {
      const char *pre = "select; remove-txt\n";
      out->write(pre, strlen(pre));
      GPList<DjVmDir::File> lst = doc->get_djvm_dir()->get_files_list();
      for (GPosition p=lst; p; ++p)
        {
          GUTF8String id = lst[p]->get_load_name();
          const GP<DjVuFile> f(doc->get_djvu_file(id));
          output(f, *out, 2, id);
        }
    }
}

void
command_output_all(ParsingByteStream &)
{
  const GP<ByteStream> out(ByteStream::create("w"));
  if (file) 
    {
      output(file, *out, 3);
    }
  else 
    {
      const char *pre = "select; remove-ant; remove-txt\n";
      out->write(pre, strlen(pre));
      GPList<DjVmDir::File> lst = doc->get_djvm_dir()->get_files_list();
      for (GPosition p=lst; p; ++p)
        {
          GUTF8String id = lst[p]->get_load_name();
          const GP<DjVuFile> f(doc->get_djvu_file(id));
          output(f, *out, 3, id);
        }
    }
}

static bool
callback_thumbnails(int page_num, void *)
{
  vprint("set-thumbnails: processing page %d", page_num+1);
  return false;
}

void
command_set_thumbnails(ParsingByteStream &pbs)
{
  GUTF8String sizestr = pbs.get_native_token();
  if (! sizestr)
    sizestr = "128";
  if (! sizestr.is_int() )
    verror("expecting integer argument");
  int size = atoi(sizestr);
  if (size<32 || size >256) 
    verror("size should be between 32 and 256 (e.g. 128)");
  doc->generate_thumbnails(size, callback_thumbnails, NULL);
  modified = true;
}

void
command_remove_thumbnails(ParsingByteStream &)
{
  doc->remove_thumbnails();
  modified = true;
}

void
command_save_page(ParsingByteStream &pbs)
{
  GUTF8String fname = pbs.get_native_token();
  if (! fname) 
    verror("empty filename");
  if (! file)
    verror("must select a page first");
  if (nosave)
    vprint("save_page: not saving anything (-n was specified)");
  if (nosave)
    return;
  const GP<ByteStream> bs(file->get_djvu_bytestream(false, false));
  const GP<ByteStream> out(ByteStream::create(GURL::Filename::UTF8(fname), "wb"));
  out->writall("AT&T",4);
  out->copy(*bs);
  vprint("saved \"%s\" as \"%s\"  (without inserting included files)",
         (const char*)(GNativeString)fileid, (const char*)fname);
}

void
command_save_page_with(ParsingByteStream &pbs)
{
  GUTF8String fname = pbs.get_native_token();
  if (! fname) 
    verror("empty filename");
  if (! file)
    verror("must select a page first");
  if (nosave)
    vprint("save-page-with: not saving anything (-n was specified)");
  if (nosave)
    return;
  const GP<ByteStream> bs(file->get_djvu_bytestream(true, false));
  const GP<ByteStream> out(ByteStream::create(GURL::Filename::UTF8(fname), "wb"));
  out->writall("AT&T",4);
  out->copy(*bs);
  vprint("saved \"%s\" as \"%s\"  (inserting included files)",
         (const char*)(GNativeString)fileid, (const char*)fname);
}

void
command_save_bundled(ParsingByteStream &pbs)
{
  GUTF8String fname = pbs.get_native_token();
  if (! fname) 
    verror("empty filename");
  if (nosave) 
    vprint("save-bundled: not saving anything (-n was specified)");
  else
    doc->save_as(GURL::Filename::UTF8(fname), true);
  modified = false;
}

void
command_save_indirect(ParsingByteStream &pbs)
{
  GUTF8String fname = pbs.get_native_token();
  if (! fname) 
    verror("empty filename");
  if (nosave) 
    vprint("save-indirect: not saving anything (-n was specified)");
  else
    doc->save_as(GURL::Filename::UTF8(fname), false);
  modified = false;
}

void
command_save(void)
{
  if (!doc->can_be_saved())
    verror("cannot save old format (use save-bundled or save-indirect)");
  if (nosave)
    vprint("save-indirect: not saving anything (-n was specified)");
  else if (!modified)
    vprint("save: document was not modified");
  else 
    doc->save();
  modified = false;
}

void
command_save(ParsingByteStream &)
{
  command_save();
}

void
command_help(void)
{
  fprintf(stderr,
          "\n"
          "Commands\n"
          "--------\n"
          "The following commands can be separated by newlines or semicolons.\n"
          "Comment lines start with '#'.  Commands usually operate on pages and files\n"
          "specified by the \"select\" command.  All pages and files are initially selected.\n"
          "A single page must be selected before executing commands marked with a period.\n"
          "Commands marked with an underline do not use the selection\n"
          "\n"
          "   ls                     -- list all pages/files\n"
          "   n                      -- list pages count\n"
          "   dump                   -- shows IFF structure\n"
          "   size                   -- prints page width and height in html friendly way\n"
          "   select                 -- selects the entire document\n"
          "   select <id>            -- selects a single page/file by name or page number\n"
          "   select-shared-ant      -- selects the shared annotations file\n"
          "   create-shared-ant      -- creates and select the shared annotations file\n"
          " . print-ant              -- prints annotations\n"
          " . print-merged-ant       -- prints annotations including the shared annotations\n"
          " . print-meta             -- prints file metadatas (a subset of the annotations\n"
          "   print-txt              -- prints hidden text using a lisp syntax\n"
          "   print-pure-txt         -- print hidden text without coordinates\n"
          "   output-ant             -- dumps ant as a valid cmdfile\n"
          "   output-txt             -- dumps text as a valid cmdfile\n"
          "   output-all             -- dumps ant and text as a valid cmdfile\n"
          " . set-ant [<antfile>]    -- copies <antfile> into the annotation chunk\n"
          " . set-meta [<metafile>]  -- copies <metafile> into the metadata part of the annotations\n"
          " . set-txt [<txtfile>]    -- copies <txtfile> into the hidden text chunk\n"
          " _ set-thumbnails [<sz>]  -- generates all thumbnails with given size\n"
          "   remove-ant             -- removes annotations\n"
          "   remove-meta            -- removes metadatas without changing other annotations\n"
          "   remove-txt             -- removes hidden text\n"
          " _ remove-thumbnails      -- removes all thumbnails\n"
          " . save-page <name>       -- saves selected page/file as is\n"
          " . save-page-with <name>  -- saves selected page/file, inserting all included files\n"
          " _ save-bundled <name>    -- saves as bundled document under fname\n"
          " _ save-indirect <name>   -- saves as indirect document under fname\n"
          " _ save                   -- saves in-place\n"
          " _ help                   -- prints this message\n"
          "\n"
          "Interactive example:\n"
          "--------------------\n"
          "  Type\n"
          "    %% djvused -v file.djvu\n"
          "  and play with the commands above\n"
          "\n"
          "Command line example:\n"
          "---------------------\n"
          "  Save all text and annotation chunks as a djvused script with\n"
          "    %% djvused file.djvu -e output-all > file.dsed\n"
          "  Then edit the script with any text editor.\n"
          "  Finally restore the modified text and annotation chunks with\n"
          "    %% djvused file.djvu -f file.dsed -s\n"
          "  You may use option -v to see more messages\n"
          "\n" );
}

void
command_help(ParsingByteStream &)
{
  command_help();
}

typedef void (*CommandFunc)(ParsingByteStream &pbs);
GMap<GUTF8String,CommandFunc> command_map;

void
init_command_map(void)
{
  command_map["ls"] = command_ls;
  command_map["n"] = command_n;
  command_map["dump"] = command_dump;
  command_map["size"] = command_size;
  command_map["select"] = command_select;
  command_map["select-shared-ant"] = command_select_shared_ant;
  command_map["create-shared-ant"] = command_create_shared_ant;
  command_map["print-ant"] = command_print_ant;  
  command_map["print-merged-ant"] = command_print_merged_ant;
  command_map["print-meta"] = command_print_meta;
  command_map["print-txt"] = command_print_txt;
  command_map["print-pure-txt"] = command_print_pure_txt;
  command_map["output-ant"] = command_output_ant;
  command_map["output-txt"] = command_output_txt;
  command_map["output-all"] = command_output_all;
  command_map["set-ant"] = command_set_ant;
  command_map["set-meta"] = command_set_meta;
  command_map["set-txt"] = command_set_txt;
  command_map["set-thumbnails"] = command_set_thumbnails;
  command_map["remove-ant"] = command_remove_ant;
  command_map["remove-meta"] = command_remove_meta;
  command_map["remove-txt"] = command_remove_txt;
  command_map["remove-thumbnails"] = command_remove_thumbnails;
  command_map["save-page"] = command_save_page;
  command_map["save-page-with"] = command_save_page_with;
  command_map["save-bundled"] = command_save_bundled;
  command_map["save-indirect"] = command_save_indirect;
  command_map["save"] = command_save;
  command_map["help"] = command_help;
}

void
usage()
{
  fprintf(stderr,"Usage: djvused [options] djvufile\n"
          "Executes scripting commands on djvufile.\n"
          "Script command come either from a script file (option -f),\n"
          "from the command line (option -e), or from stdin (default).\n"
          "\n"
          "Options are\n"
          "  -v               -- verbose\n"
          "  -f <scriptfile>  -- take commands from a file\n"
          "  -e <script>      -- take commands from the command line\n"
          "  -s               -- save after execution\n"
          "  -n               -- do not save anything\n"
          "\n"
          );
  command_help();
  exit(10);
}



// --------------------------------------------------
// MAIN
// --------------------------------------------------

void 
execute()
{
  if (!cmdbs)
    cmdbs = ByteStream::create("r");
  const GP<ParsingByteStream> gcmd(ParsingByteStream::create(cmdbs));
  ParsingByteStream &cmd=*gcmd;
  GUTF8String token;
  init_command_map();
  vprint("type \"help\" to see available commands.");
  vprint("ok.");
  while (!! (token = cmd.get_native_token(true)))
    {
      CommandFunc func = command_map[token];
      G_TRY
        {
          if (!func) 
            verror("unrecognized command");
          // Cautious execution
          (*func)(cmd);
          // Skip extra arguments
          int c = cmd.get_spaces();
          if (c!=';' && c!='\n' && c!='\r' && c!=EOF)
            {
              while (c!=';' && c!='\n' && c!='\r' && c!=EOF)
                c = cmd.get();
              verror("too many arguments");
            }
          cmd.unget(c);
        }
      G_CATCH(ex)
        {

          vprint("Error (%s): %s",
                 (const char*)(GNativeString)token, ex.get_cause());
          if (! verbose)
            G_RETHROW;
        }
      G_ENDCATCH;
      vprint("ok.");
    }
}


int 
main(int argc, char **argv)
{
  G_TRY
     {
      for (int i=1; i<argc; i++)
        if (!strcmp(argv[i],"-v"))
          verbose = true;
        else if (!strcmp(argv[i],"-s"))
          save = true; 
        else if (!strcmp(argv[i],"-n"))
          nosave = true;
        else if (!strcmp(argv[i],"-f") && i+1<argc && !cmdbs) 
          cmdbs = ByteStream::create(GURL::Filename::UTF8(argv[++i]), "r");
        else if (!strcmp(argv[i],"-e") && !cmdbs && ++i<argc) 
          cmdbs = ByteStream::create_static(argv[i],strlen(argv[i]));
        else if (argv[i][0] != '-' && !djvufile)
          djvufile = argv[i];
        else
          usage();
      if (!djvufile)
        usage();
      // Open file
      doc = DjVuDocEditor::create_wait(GURL::Filename::UTF8(djvufile));
      // Execute
      execute();
      if (modified)
        if (save)
          command_save();
        else
          fprintf(stderr,"djvused: (warning) file was modified but not saved\n");
    }
  G_CATCH(ex)
    {
      ex.perror();
      return 10;
    }
  G_ENDCATCH;
  return 0;
}
