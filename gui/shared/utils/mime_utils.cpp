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


#include "Arrays.h"
#include "GString.h"
#include "debug.h"

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

static DArray<GUTF8String>
getRecords(FILE * file_in, FILE * file_out)
{
   signed char ch;
   DArray<GUTF8String> records;
   
      // Skip (while copying!) leading spaces and comments
   while(true)
   {
      if ((ch=fgetc(file_in))==EOF) return records;
      
      if (isspace(ch)) fputc(ch, file_out);
      else if (ch=='#')
      {
	 fputc(ch, file_out);
	 while(true)
	 {
	    if ((ch=fgetc(file_in))==EOF) return records;
	    fputc(ch, file_out);
	    if (ch=='\n') break;
	 }
      } else break;
   }

      // No more comments allowed. If we encounter a comment, we will
      // assume, that the record is over (it's impossible to add '\' after
      // a comment after all).
   while(true)
   {
      GUTF8String term;
      bool end_of_rec=false;
      while(true)
      {
	 if (ch=='"')
	 {
	    term+=ch;
	    while(true)
	    {
	       if ((ch=fgetc(file_in))==EOF) G_THROW("Unexpected EOF.");
	       term+=ch;
	       if (ch=='"') break;
	       else if (ch=='\n')
	       {
		  end_of_rec=true;
		  break;
	       }
	    }
	 } else if (ch=='\\')
	 {
	    if ((ch=fgetc(file_in))==EOF) G_THROW("Unexpected EOF.");
	    if (ch=='\n') break;
	    else G_THROW("Malformed file.");
	 } else if (ch=='#')
	 {
	    while((ch=fgetc(file_in))!=EOF && ch!='\n');
	    end_of_rec=true;
	 } else if (ch=='\n') end_of_rec=true;
	 else if (isspace(ch)) break;
	 else term+=ch;
	 if (end_of_rec) break;
	 if ((ch=fgetc(file_in))==EOF) break;
      }

      records.resize(records.size());
      records[records.size()-1]=term;

      if (end_of_rec) break;
      
	 // Skip white spaces
      while(true)
      {
	 if ((ch=fgetc(file_in))==EOF) return records;

	 if (ch=='#')
	 {
	    while((ch=fgetc(file_in))!=EOF && ch!='\n');
	    return records;
	 } else if (ch=='\\')
	 {
	    if ((ch=fgetc(file_in))==EOF) G_THROW("Unexpected EOF.");
	    if (ch!='\n') G_THROW("EOL expected.");
	 } else if (ch=='\n') return records;
	 else if (!isspace(ch)) break;
      }
   }
   return records;
}

bool
fixMimeTypes(const char * name_in, const char * name_out)
      // The function will scan file 'name_in' with MIME types description
      // and will output possibly corrected data into name_out
      // If smth has been changed, TRUE will be returned. and 'name_out'
      // file will be preserved. Otherwise it will be destroyed and
      // FALSE will be returned.
{
   typedef const char * string;
   const int mime_types=8;
   const int mime_fields=3;
   static string mime_type[mime_types][mime_fields]=
   {
      { "type=image/x-djvu", "exts=\"djvu,djv\"", "desc=\"DjVu File\"" },
      { "type=image/x.djvu", "exts=\"\"", "desc=\"DjVu File\"" },
      { "type=image/djvu", "exts=\"\"", "desc=\"DjVu File\"" },
      { "type=image/dejavu", "exts=\"\"", "desc=\"DjVu File (obsolete mime type)\"" },
      { "type=image/x-dejavu", "exts=\"\"", "desc=\"DjVu File (obsolete mime type)\"" },
      { "type=image/x-iw44", "exts=\"iw44,iw4\"", "desc=\"DjVu File (obsolete mime type)\"" },
      { "type=image/x-jb2", "exts=\"\"", "desc=\"DjVu File (obsolete mime type)\"" },
      { "type=image/jb2", "exts=\"\"", "desc=\"DjVu File (obsolete mime type)\"" }
   };
   const char title[]="#--Netscape Communications Corporation MIME Information\n";

   FILE * file_in=0, * file_out=0;
   G_TRY {
      file_in=fopen(name_in, "r");
      if (!file_in)
      {
	    // The ~/.mime.types doesn't exist => create it
	 FILE * f=fopen(name_out, "w");
	 fprintf(f, "%s", title);
	 for(int type_num=0;type_num<mime_types;type_num++)
	 {
	    for(int field_num=0;field_num<mime_fields;field_num++)
	       fprintf(f, "%s ", mime_type[type_num][field_num]);
	    fprintf(f, "\n");
	 }
	 fclose(f);
	 return true;
      }

	 // Fine. The input file exist. Check that it has the correct
	 // starting string and proceed...
      file_out=fopen(name_out, "w");

      bool done=false;
	 
      char buffer[1024];
      fgets(buffer, 1024, file_in);
      if (strcmp(buffer, title))
      {
	 done=true;
	 fprintf(file_out, "%s", title);
      }
      fseek(file_in, 0, SEEK_SET);

      bool has_type[mime_types]={ 0, 0, 0, 0, 0, 0, 0, 0 };
      DArray<GUTF8String> records;
      while((records=getRecords(file_in, file_out)).size())
      {
	 GUTF8String term=records[0];
	 DEBUG_MSG("records=" << records.size() << ", [0]='" << term << "'\n");
	 int type_num;
	 for(type_num=0;type_num<mime_types;type_num++)
	    if (term==mime_type[type_num][0]) break;
	 if (type_num<mime_types)
	 {
	       // One of our MIME types
	    has_type[type_num]=true;
	    bool has_field[mime_fields]={ 0, 0, 0 };
	    for(int i=0;i<records.size();i++)
	    {
	       GUTF8String term=records[i];
	       const char * ptr;
	       for(ptr=term;*ptr;ptr++)
		  if (*ptr=='=') break;
	       if (*ptr=='=')
	       {
		  int symbols=ptr-term+1;
		  int field_num;
		  for(field_num=0;field_num<mime_fields;field_num++)
		     if (!strncmp(mime_type[type_num][field_num], term, symbols)) break;
		  if (field_num<mime_fields)
		  {
		     if (strcmp(mime_type[type_num][field_num], term)) done=true;
		     if (!has_field[field_num])
		     {
			has_field[field_num]=true;
			fprintf(file_out, "%s ", mime_type[type_num][field_num]);
		     }
		     continue;
		  }
	       }
	       fprintf(file_out, "%s ", (const char *) term);
	    }
	    for(int field_num=0;field_num<mime_fields;field_num++)
	       if (!has_field[field_num])
	       {
		  done=true;
		  fprintf(file_out, "%s ", mime_type[type_num][field_num]);
	       }
	 } else
	 {
	       // Not our MIME type
	    for(int i=0;i<records.size();i++)
	       fprintf(file_out, "%s ", (const char *) records[i]);
	 }
	 fprintf(file_out, "\n");
      } // while(getRecords())

      for(int type_num=0;type_num<mime_types;type_num++)
	 if (!has_type[type_num])
	 {
	    done=true;
	    for(int field_num=0;field_num<mime_fields;field_num++)
	       fprintf(file_out, "%s ", mime_type[type_num][field_num]);
	    fprintf(file_out, "\n");
	 }

      fclose(file_in);
      fclose(file_out);

      if (!done) unlink(name_out);
      return done;
   } G_CATCH(exc) {
      if (file_in) fclose(file_in);
      if (file_out) fclose(file_out);
      unlink(name_out);
      exc.get_line();
   } G_ENDCATCH;
   return false;
}
