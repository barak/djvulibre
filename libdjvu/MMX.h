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

#ifndef _MMX_H_
#define _MMX_H_
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef __GNUG__
#pragma interface
#endif

#include "DjVuGlobal.h"


/** @name MMX.h
    Files #"MMX.h"# and #"MMX.cpp"# implement basic routines for
    supporting the MMX instructions on x86.  Future instruction sets
    for other processors may be supported in this file as well.

    Macro #MMX# is defined if the compiler supports the X86-MMX instructions.
    It does not mean however that the processor supports the instruction set.
    Variable #MMXControl::mmxflag# must be used to decide whether MMX.
    instructions can be executed.  MMX instructions are entered in the middle
    of C++ code using the following macros.  Examples can be found in
    #"IWTransform.cpp"#.

    \begin{description}
    \item[MMXrr( insn, srcreg, dstreg)] 
       Encode a register to register MMX instruction 
       (e.g. #paddw# or #punpcklwd#).
    \item[MMXar( insn, addr, dstreg )]
       Encode a memory to register MMX instruction 
       (e.g. #moveq# from memory).
    \item[MMXra( insn, srcreg, addr )]
       Encode a register to memory MMX instruction 
       (e.g. #moveq# to memory).
    \item[MMXir( insn, imm, dstreg )]
       Encode a immediate to register MMX instruction 
       (e.g #psraw#).
    \item[MMXemms]
       Execute the #EMMS# instruction to reset the FPU state.
    \end{description}

    @memo
    Essential support for MMX.
    @version 
    #$Id$#
    @author: 
    L\'eon Bottou <leonb@research.att.com> -- initial implementation */
//@{


/** MMX Control. 
    Class #MMXControl# encapsulates a few static functions for 
    globally enabling or disabling MMX support. */

class MMXControl
{
 public:
  // MMX DETECTION
  /** Detects and enable MMX or similar technologies.  This function checks
      whether the CPU supports a vectorial instruction set (such as Intel's
      MMX) and enables them.  Returns a boolean indicating whether such an
      instruction set is available.  Speedups factors may vary. */
  static int enable_mmx();
  /** Disables MMX or similar technologies.  The transforms will then be
      performed using the baseline code. */
  static int disable_mmx();
  /** Contains a value greater than zero if the CPU supports vectorial
      instructions. A negative value means that you must call \Ref{enable_mmx}
      and test the value again. Direct access to this member should only be
      used to transfer the instruction flow to the vectorial branch of the
      code. Never modify the value of this variable.  Use #enable_mmx# or
      #disable_mmx# instead. */
  static int mmxflag;  // readonly
};

//@}




// ----------------------------------------
// GCC MMX MACROS

#ifndef NO_MMX

#if defined(__GNUC__) && defined(__i386__)
#define MMXemms \
  __asm__ volatile("emms" : : : "memory" ) 
#define MMXrr(op,src,dst) \
  __asm__ volatile( #op " %%" #src ",%%" #dst : : : "memory") 
#define MMXir(op,imm,dst) \
  __asm__ volatile( #op " %0,%%" #dst : : "i" (imm) : "memory") 
#define MMXar(op,addr,dst) \
  __asm__ volatile( #op " %0,%%" #dst : : "rm" (*(int*)(addr)) : "memory") 
#define MMXra(op,src,addr) \
  __asm__ volatile( #op " %%" #src ",%0" : : "rm" (*(int*)(addr)) : "memory") 
#define MMX 1
#endif


// ----------------------------------------
// MSVC MMX MACROS

#if defined(_MSC_VER) && defined(_M_IX86)
// Compiler option /GM is required
#pragma warning( disable : 4799 )
#define MMXemms \
  __asm { emms }
#define MMXrr(op,src,dst) \
  __asm { op dst,src }
#define MMXir(op,imm,dst) \
  __asm { op dst,imm }
#define MMXar(op,addr,dst) \
  { register __int64 var=*(__int64*)(addr); __asm { op dst,var } }
#define MMXra(op,src,addr) \
  { register __int64 var; __asm { op [var],src };  *(__int64*)addr = var; } 
// Probably not as efficient as GCC macros
#define MMX 1
#endif

#endif

// -----------
#endif
