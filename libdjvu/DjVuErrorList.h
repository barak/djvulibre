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

#ifndef _DJVUERRORLIST_H
#define _DJVUERRORLIST_H
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "DjVuPort.h"

class ByteStream;

/** @name DjVuErrorList.h
    This file implements a very simple class for redirecting port caster
    messages that would normally end up on stderr to a double linked list.

    @memo DjVuErrorList class.
    @author Bill C Riemers <bcr@lizardtech.com>
    @version #$Id$#
*/

//@{

/** #DjVuErrorList# provides a convenient way to redirect error messages
    from classes derived from DjVuPort to a list that can be accessed at
    any time. */

class DjVuErrorList : public DjVuSimplePort
{
protected:
     /// The normal port caster constructor. 
  DjVuErrorList(void);
public:
  static GP<DjVuErrorList> create(void) {return new DjVuErrorList();}

     /// This constructor allows the user to specify the ByteStream.
  GURL set_stream(GP<ByteStream>);

     /// Append all error messages to the list
  virtual bool notify_error(const DjVuPort * source, const GUTF8String & msg);

     /// Append all status messages to the list
  virtual bool notify_status(const DjVuPort * source, const GUTF8String & msg);

     /// Add a new class to have its messages redirected here.
  inline void connect( const DjVuPort &port);

     /// Get the listing of errors, and clear the list.
  inline GList<GUTF8String> GetErrorList(void);

     /// Just clear the list.
  inline void ClearError(void);

     /// Get one error message and clear that message from the list.
  GUTF8String GetError(void);

     /// Check if there are anymore error messages.
  inline bool HasError(void) const;

     /// Get the listing of status messages, and clear the list.
  inline GList<GUTF8String> GetStatusList(void);

     /// Just clear the list.
  inline void ClearStatus(void);

     /// Get one status message and clear that message from the list.
  GUTF8String GetStatus(void);

     /// Check if there are any more status messages.
  inline bool HasStatus(void) const;

     /** This gets the data.  We can't use the simple port's request
       data since we want to allow the user to specify the ByteStream. */
  virtual GP<DataPool> request_data (
    const DjVuPort * source, const GURL & url );

private:
  GURL pool_url;
  GP<DataPool> pool;
  GList<GUTF8String> Errors;
  GList<GUTF8String> Status;
private: //dummy stuff
  static GURL set_stream(ByteStream *);
};

inline void
DjVuErrorList::connect( const DjVuPort &port )
{ get_portcaster()->add_route(&port, this); }

inline GList<GUTF8String>
DjVuErrorList::GetErrorList(void)
{
  GList<GUTF8String> retval=(const GList<GUTF8String>)Errors;
  Errors.empty();
  return retval;
}

inline void
DjVuErrorList::ClearError(void)
{ Errors.empty(); }

inline GList<GUTF8String>
DjVuErrorList::GetStatusList(void)
{
  GList<GUTF8String> retval=(const GList<GUTF8String>)Status;
  Status.empty();
  return retval;
}

inline void
DjVuErrorList::ClearStatus(void)
{ Status.empty(); }

inline bool
DjVuErrorList::HasError(void) const
{ return !Errors.isempty(); }

inline bool
DjVuErrorList::HasStatus(void) const
{ return !Status.isempty(); }

#endif
