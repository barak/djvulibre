/* -*- C -*-
// -------------------------------------------------------------------
// Atomic primitives
// Copyright (c) 2008  Leon Bottou. All rights reserved
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// ------------------------------------------------------------------- */

#include "atomic.h"

#ifndef ATOMIC_MACROS

// #include "GThreads.h"
#include <pthread.h>
// #include <QMutex>
// #include <QWaitCondition>

/* Select synchronization primitives */

# if defined(WIN32) || defined(_WIN32) || defined(__WIN64)

static LONG ini = 0;
static CRITICAL_SECTION cs;
# define MUTEX_LEAVE LeaveCriticalSection(&cs)
# define MUTEX_ENTER mutex_enter()
static inline void mutex_enter() {
  if (!InterlockedExchange(&ini, 1))
    {
      InitializeCriticalSection(&cs);
      ev = CreateEvent(NULL, FALSE, FALSE, NULL);
      assert(ev);
    }
  EnterCriticalSection(&cs);
}

# elif defined (PTHREAD_MUTEX_INITIALIZER)

static pthread_mutex_t ptm = PTHREAD_MUTEX_INITIALIZER;
# define MUTEX_ENTER  pthread_mutex_lock(&ptm)
# define MUTEX_LEAVE  pthread_mutex_unlock(&ptm)

# elif defined(__cplusplus) && defined(_GTHREADS_H_)

static GMonitor m;
# define MUTEX_ENTER  m.enter()
# define MUTEX_LEAVE  m.leave()

# elif defined(__cplusplus) && defined(QMUTEX_H)

static QMutex qtm;
# define MUTEX_ENTER  qtm.lock()
# define MUTEX_LEAVE  qtm.unlock()

# endif


/* atomic primitive emulation */

int
atomicIncrement(int volatile *var)
{
  int res;
  MUTEX_ENTER;
  res = ++(*var);
  MUTEX_LEAVE;
  return res;
}

int 
atomicDecrement(int volatile *var)
{
  int res;
  MUTEX_ENTER;
  res = --(*var);
  MUTEX_LEAVE;
  return res;
}

int 
atomicCompareAndSwap(int volatile *var, int oldval, int newval)
{
  int ret;
  MUTEX_ENTER;
  ret = *var;
  if (ret == oldval)
    *var = newval;
  MUTEX_LEAVE;
  return ret;
}

int 
atomicExchange(int volatile *var, int newval)
{
  int ret;
  MUTEX_ENTER;
  ret = *var;
  *var = newval;
  MUTEX_LEAVE;
  return ret;
}

void* 
atomicExchangePointer(void* volatile *var, void* newval)
{
  void *ret;
  MUTEX_ENTER;
  ret = *var;
  *var = newval;
  MUTEX_LEAVE;
  return ret;
}

#endif 


