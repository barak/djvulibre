//C-  -*- C++ -*-
//C- -----------------------------------------------------------
//C- DjVuLibre-3.5
//C- Copyright (c) 2002  Leon Bottou and Yann Le Cun.
//C- Copyright (c) 2001  AT&T
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- DjVuLibre-3.5 is derived from the DjVu (r) Reference Library.
//C- 
//C- DjVu (r) Reference Library (v. 3.5)
//C- Copyright (c) 1999-2001 LizardTech, Inc. All Rights Reserved.
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
//C- -----------------------------------------------------------
// 
// $Id$
// $Name$

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "MMX.h"
#include <stdio.h>
#include <stdlib.h>


// ----------------------------------------
// PRINTING MMX REGISTERS (Debug)


#if defined(MMX) && defined(DEBUG)
extern "C" void
mmx_show()
{
  /* This function can be called from a debugger
     in order to visualize the contents of the MMX registers. */
  int mmregs[16];
  MMXra( movq,  mm0, &mmregs[0]);
  MMXra( movq,  mm1, &mmregs[2]);
  MMXra( movq,  mm2, &mmregs[4]);
  MMXra( movq,  mm3, &mmregs[6]);
  MMXra( movq,  mm4, &mmregs[8]);
  MMXra( movq,  mm5, &mmregs[10]);
  MMXra( movq,  mm6, &mmregs[12]);
  MMXra( movq,  mm7, &mmregs[14]);
  MMXemms;
  for (int i=0; i<8; i++)
    DjVuPrintMessageUTF8("mm%d: %08x%08x\n", i, 
           mmregs[i+i+1], mmregs[i+i]);
  MMXar( movq,  &mmregs[0], mm0);
  MMXar( movq,  &mmregs[2], mm1);
  MMXar( movq,  &mmregs[4], mm2);
  MMXar( movq,  &mmregs[6], mm3);
  MMXar( movq,  &mmregs[8], mm4);
  MMXar( movq,  &mmregs[10], mm5);
  MMXar( movq,  &mmregs[12], mm6);
  MMXar( movq,  &mmregs[14], mm7);
}
#endif



// ----------------------------------------
// MMX ENABLE/DISABLE

// Default settings autodetect MMX.
// Use macro DISABLE_MMX to disable MMX by default.

#if defined(MMX) && !defined(DISABLE_MMX)
int MMXControl::mmxflag = -1;
#else
int MMXControl::mmxflag = 0;
#endif

int 
MMXControl::disable_mmx()
{
  mmxflag = 0;
  return mmxflag;
}

int 
MMXControl::enable_mmx()
{
  int cpuflags = 0;
#if defined(MMX) && defined(__GNUC__) && defined(__i386__)
  // Detection of MMX for GCC
  __asm__ volatile ("pushl %%ebx\n\t"
                    "pushfl\n\t"    
                    "popl %%ecx\n\t"
                    "xorl %%edx,%%edx\n\t"
                    // Check that CPUID exists
                    "movl %%ecx,%%eax\n\t"
                    "xorl $0x200000,%%eax\n\t"
                    "pushl %%eax\n\t"
                    "popfl\n\t"
                    "pushfl\n\t"
                    "popl %%eax\n\t"
                    "xorl %%ecx,%%eax\n\t"
                    "jz 1f\n\t"
                    "pushl %%ecx\n\t"
                    "popfl\n\t"
                    // Check that CR0:EM is clear
                    "smsw %%ax\n\t"
                    "andl $4,%%eax\n\t"
                    "jnz 1f\n\t"
                    // Execute CPUID
                    "movl $1,%%eax\n\t"
                    "cpuid\n"
                    "1:\tmovl %%edx, %0\n\t"
                    // EBX contains magic when -fPIC is on.
                    "popl %%ebx"
                    : "=m" (cpuflags) :
                    : "eax","ebx","ecx","edx");
#endif
#if defined(MMX) && defined(_MSC_VER) && defined(_M_IX86)
  // Detection of MMX for MSVC
  __asm {  pushfd
           pop     ecx
           xor     edx,edx
             ;// Check that CPUID exists
           mov     eax,ecx        
           xor     eax,0x200000
           push    eax
           popfd
           pushfd
           pop     eax
           xor     eax,ecx
           jz      fini
           push    ecx
           popfd
             ;// Check that CR0:EM is zero
           smsw    ax
           and     eax,4
           jnz     fini
             ;// Execute CPUID
           mov     eax,1
           _emit   0xf
           _emit   0xa2
         fini:
           mov     cpuflags,edx
             ;// MSVC determines clobbered registers by scanning the assembly code.
             ;// Since it does not know CPUID, it would not know that EBX is clobbered
             ;// without the dummy instruction below...
           xor     ebx,ebx
         }
#endif
  mmxflag = !!(cpuflags & 0x800000);
  return mmxflag;
}


