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

#define BUILD_LIBDJVU 1
#include "DjVmNav.h"
#include "BSByteStream.h"
#include "GURL.h"
#include "debug.h"

#include <ctype.h>

#ifdef _SPEEDTRACER
#include "speedtracer.h"
#endif

GP<DjVmNav::DjVuBookMark>
DjVmNav::DjVuBookMark::create(void)
{
  return new DjVuBookMark();
}

GP<DjVmNav::DjVuBookMark>
DjVmNav::DjVuBookMark::create(const unsigned short count,
     const GUTF8String &displayname, const GUTF8String &url)
{
  DjVuBookMark *pvm=new DjVuBookMark();
  GP<DjVuBookMark> bookmark=pvm;
  pvm->count=count;
  pvm->displayname=displayname;
  pvm->url=url;
  return bookmark;
}   

DjVmNav::DjVuBookMark::DjVuBookMark(void) : count(0), displayname(), url(){ }

GP<DjVmNav>
DjVmNav::create(void)
{
  return new DjVmNav;
}

// Decode the input bytestream and populate this object
void 
DjVmNav::DjVuBookMark::decode(const GP<ByteStream> &gstr)
{
  int textsize=0, readsize=0;
  char *buffer=0;
  ByteStream &bs=*gstr;

  count = bs.read8();

  displayname.empty();
  textsize = bs.read24();
  if (textsize)
  {
	buffer = displayname.getbuf(textsize);
	readsize = bs.read(buffer,textsize);
	buffer[readsize] = 0;
  }

  url.empty();
  textsize = bs.read24();
  if (textsize)
  {
	 buffer = url.getbuf(textsize);
	 readsize = bs.read(buffer,textsize);
     buffer[readsize] = 0;
  }
}


// Serialize this object to the output bytestream
void  
DjVmNav::DjVuBookMark::encode(const GP<ByteStream> &gstr) 
{
  int textsize=0, readsize=0;
  char *buffer=0;
  ByteStream &bs=*gstr;
  bs.write8(count);

  textsize = displayname.length();
  bs.write24( textsize );
  bs.writestring(displayname);
  
  textsize = url.length();
  bs.write24( textsize );
  bs.writestring(url);
}
// Text dump of this object to the output bytestream
void 
DjVmNav::DjVuBookMark::dump(const GP<ByteStream> &gstr) 
{
  int textsize=0, readsize=0;
  char *buffer=0;
  ByteStream &bs=*gstr;
  bs.format("\n  count=%d\n",count);

  textsize = displayname.length();
  bs.format("  (%d) %s\n",textsize, displayname.getbuf());
  
  textsize = url.length();
  bs.format("  (%d) %s\n",textsize, url.getbuf());

}





// Decode the input bytestream and populate this object
void 
DjVmNav::decode(const GP<ByteStream> &gstr)
{
   //ByteStream &str=*gstr;
   GP<ByteStream> gpBSByteStream = BSByteStream::create(gstr);
   GCriticalSectionLock lock(&class_lock);
   bookmark_list.empty();
   //   int nbookmarks=str.read16();
   int nbookmarks=gpBSByteStream->read16();
   if (nbookmarks)
   {
      for(int bookmark=0;bookmark<nbookmarks;bookmark++)
      {
		  GP<DjVuBookMark> pBookMark=DjVuBookMark::create();
		  //pBookMark->decode(&str);
		  pBookMark->decode(gpBSByteStream);
		  bookmark_list.append(pBookMark);  
	  }
   }
}

// Serialize this object to the output stream
void 
DjVmNav::encode(const GP<ByteStream> &gstr)
{
   //ByteStream &str=*gstr;
   GP<ByteStream> gpBSByteStream = BSByteStream::create(gstr, 1024);
   GCriticalSectionLock lock(&class_lock);
   int nbookmarks=bookmark_list.size();
   //str.write16(nbookmarks);
   gpBSByteStream->write16(nbookmarks);
   if (nbookmarks)
   {
	  GPosition pos;
	  int cnt=0;
      for (pos = bookmark_list; pos; ++pos)
	  {
		  //bookmark_list[pos]->encode(&str);
		  bookmark_list[pos]->encode(gpBSByteStream);
		  cnt++;
	  }
	  if (nbookmarks != cnt)
	  {
		  GUTF8String msg;
		  msg.format("Corrupt bookmarks found during encode: %d of %d \n",cnt,nbookmarks);
		  G_THROW (msg);
	  }
   }
}
int DjVmNav::getBookMarkCount()
{
	return(bookmark_list.size());
}

void DjVmNav::append (const GP<DjVuBookMark> &gpBookMark) 
{
	bookmark_list.append(gpBookMark);
}

bool DjVmNav::getBookMark(GP<DjVuBookMark> &gpBookMark, int iPos)
{
	GPosition pos = bookmark_list.nth(iPos);
	if (pos)
		gpBookMark = bookmark_list[pos];
	else
		gpBookMark = 0;

	return (gpBookMark?true:false);
}


// A text dump of this object
void 
DjVmNav::dump(const GP<ByteStream> &gstr)
{
   ByteStream &str=*gstr;
   GCriticalSectionLock lock(&class_lock);
   int nbookmarks=bookmark_list.size();
   str.format("%d bookmarks:\n",nbookmarks);
   if (nbookmarks)
   {
	  GPosition pos;
	  int cnt=0;
      for (pos = bookmark_list; pos; ++pos)
	  {
		  bookmark_list[pos]->dump(&str);
		  cnt++;
	  }
	  if (nbookmarks != cnt)
	  {
		  GUTF8String msg;
		  msg.format("Corrupt bookmarks found during encode: %d of %d \n",cnt,nbookmarks);
		  G_THROW (msg);
	  }
   }

}

bool DjVmNav::isValidBookmark()
{
	//test if the bookmark is properly given
	//for example: (4, "A", urla)
	//			   (0, "B", urlb)
	//             (0, "C", urlc)
	//is not a bookmark since A suppose to have 4 decendents, it only get one.
	int bookmark_totalnum=getBookMarkCount();
	GP<DjVuBookMark> gpBookMark;
	int* count_array=(int*)malloc(sizeof(int)*bookmark_totalnum);
	for(int i=0;i<bookmark_totalnum;i++)
	{
		getBookMark(gpBookMark, i);
		count_array[i]=gpBookMark->count;
	}

	int index=0;
	int trees=0;
	int* treeSizes=(int*)malloc(sizeof(int)*bookmark_totalnum);
	while(index<bookmark_totalnum)
	{
		int treeSize=get_tree(index,count_array,bookmark_totalnum);
		if(treeSize>0) //is a tree
		{
			index+=treeSize;
			treeSizes[trees++]=treeSize;
		}
		else //not a tree
			break;
	}

	free(count_array);
	free(treeSizes);

	return true;
}


int DjVmNav::get_tree(int index, int* count_array, int count_array_size)
{
	int i=index;
	int accumulate_count=0;
	while(i<count_array_size)
	{
		accumulate_count+=count_array[i];
		if(accumulate_count==0)
			return 1;
		else if(accumulate_count == i-index) //get a tree
			return accumulate_count;

		i++;
	}
	return 0;
}


