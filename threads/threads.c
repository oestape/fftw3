/*
 * Copyright (c) 2002 Matteo Frigo
 * Copyright (c) 2002 Steven G. Johnson
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* threads.c: Portable thread spawning for loops, via the X(spawn_loop)
   function.  The first portion of this file is a set of macros to
   spawn and join threads on various systems. */

#include "threads.h"

/***********************************************************************/

/************************* Thread Glue *************************/

/* Adding support for a new shared memory thread API should be easy.  You
   simply do the following things (look at the POSIX and Solaris
   threads code for examples):

   * Invent a symbol of the form USING_FOO_THREADS to denote
     the use of your thread API, and add an 
              #elif defined(USING_FOO_THREADS)
      before the #else clause below.  This is where you will put
      your thread definitions.  In this #elif, insert the following:

      -- #include any header files needed to use the thread API.

      -- Typedef fftw_thr_function to be a function pointer
         of the type used as a argument to fftw_thr_spawn
	 (i.e. the entry function for a thread).

      -- Define fftw_thr_id, via a typedef, to be the type
         that is used for thread identifiers.

      -- #define fftw_thr_spawn(tid_ptr, proc, data) to
         call whatever function to spawn a new thread.  The
         new thread should call proc(data) as its starting point,
         and tid_ptr is a pointer to a fftw_thr_id that
         is set to an identifier for the thread.  You can also
         define this as a subroutine (put it in fftw_thrs.c)
	 if it is too complicated for a macro.  The prototype should
	 be:

	 void fftw_thr_spawn(fftw_thr_id *tid_ptr,
	                        fftw_thr_function proc,
				void *data);

      -- #define fftw_thr_wait(tid) to block until the thread
         whose identifier is tid has terminated.  You can also
         define this as a subroutine (put it in fftw_thrs.c) if
	 it is too complicated for a macro.  The prototype should be:
	
	 void fftw_thr_wait(fftw_thr_id tid);

   * If you need to perform any initialization before using threads,
     put your initialization code in the fftw_thrs_init() function
     in fftw_thrs.c, bracketed by the appropriate #ifdef of course.

   * Also, of course, you should modify config.h to #define
     USING_FOO_THREADS, or better yet modify config.h.in
     and configure.ac so that autoconf can automatically detect
     your threads library.

   * Finally, if you do implement support for a new threads API, be
     sure to let us know at fftw@fftw.org so that we can distribute
     your code to others!

*/

/************************** Solaris Threads ****************************/
      
#if defined(USING_SOLARIS_THREADS)

/* Solaris threads glue.  Tested. */

/* link with -lthread */

#include <thread.h>

/* Thread entry point: */
typedef void * (*fftw_thr_function) (void *);

typedef thread_t fftw_thr_id;

#define fftw_thr_spawn(tid_ptr, proc, data) \
     thr_create(0,0,proc,data,THR_BOUND,tid_ptr)

#define fftw_thr_wait(tid) thr_join(tid,0,0)

/************************** BeOS Threads ****************************/
      
#elif defined(USING_BEOS_THREADS)

/* BeOS threads glue.  Tested for DR8.2. */

#include <OS.h>

/* Thread entry point: */
typedef thread_entry fftw_thr_function;

typedef thread_id fftw_thr_id;

#define fftw_thr_spawn(tid_ptr, proc, data) { \
     *(tid_ptr) = spawn_thread(proc,"FFTW",B_NORMAL_PRIORITY,data); \
     resume_thread(*(tid_ptr)); \
}

/* wait_for_thread requires that we pass a valid pointer as the
   second argument, even if we're not interested in the result. */
#define fftw_thr_wait(tid) {long exit_val;wait_for_thread(tid, &exit_val);}

/************************** MacOS Threads ****************************/

#elif defined(USING_MACOS_THREADS)

/* MacOS (old! old!) MP threads glue. Experimental, untested! I do not
   have an MP MacOS system available to me...I just read the
   documentation.  There is actually a good chance that this will work
   (since the code below is so short), but I make no guarantees.
   Consider it to be a starting point for your own implementation.

   I also had to insert some code in fftw_thrs.c. 

   MacOS X has real SMP support, thank goodness; I'm leaving this
   code here mainly for historical purposes. */

/* Using this code in the MacOS: (See the README file for general
   documenation on the FFTW threads code.)  To use this code, you have
   to do two things.  First of all, you have to #define the symbol
   USING_MACOS_THREADS. This can be done at the top of this file
   or perhaps in your compiler options.  Second, you have to weak-link
   your project to the MP library.

   In your code, you should check at run-time with MPLibraryIsLoaded()
   to see if the MP library is available. If it is not, it is
   still safe to call the fftw_thrs routines...in this case,
   however, you must always pass 1 for the nthreads parameter!
   (Otherwise, you will probably want to pass the value of
   MPProcessors() for the nthreads parameter.) */

#include <MP.h>

typedef TaskProc fftw_thr_function;

typedef MPQueueID fftw_thr_id;

#define fftw_thr_spawn(tid_ptr, proc, data) { \
     MPTaskID task; \
     MPCreateQueue(tid_ptr); \
     MPCreateTask(proc,data,kMPUseDefaultStackSize,*(tid_ptr),0,0, \
		  kMPNormalTaskOptions,&task); \
}

#define fftw_thr_wait(tid) { \
     void *param1,*param2,*param3; \
     MPWaitOnQueue(tid,&param1,&param2,&param3,kDurationForever); \
     MPDeleteQueue(tid); \
}

/************************** Win32 Threads ****************************/

#elif defined(USING_WIN32_THREADS)

/* Win32 threads glue.  We have not tested this code!  (I just implemented
   it by looking at a Win32 threads manual.)  Users have reported that this
   code works under NT using Microsoft compilers.
   
   To use it, you should #define the symbol USING_WIN32_THREADS. */

#include <windows.h>

typedef LPTHREAD_START_ROUTINE fftw_thr_function;
typedef HANDLE fftw_thr_id;

#define fftw_thr_spawn(tid_ptr, proc, data) { \
     DWORD thrid; \
     *(tid_ptr) = CreateThread((LPSECURITY_ATTRIBUTES) NULL, 0, \
			       (fftw_thr_function) proc, (LPVOID) data, \
			       0, &thrid); \
}

#define fftw_thr_wait(tid) { \
     WaitForSingleObject(tid, INFINITE); \
     CloseHandle(tid); \
}

/************************** Mach cthreads ****************************/

#elif defined(USING_MACH_THREADS)

#ifdef HAVE_MACH_CTHREADS_H
#include <mach/cthreads.h>
#elif defined(HAVE_CTHREADS_H)
#include <cthreads.h>
#elif defined(HAVE_CTHREAD_H)
#include <cthread.h>
#endif

typedef cthread_fn_t fftw_thr_function;

typedef cthread_t fftw_thr_id;

#define fftw_thr_spawn(tid_ptr, proc, data) \
     *(tid_ptr) = cthread_fork(proc, (any_t) (data))

#define fftw_thr_wait(tid) cthread_join(tid)

/************************** MP directive Threads ****************************/

#elif defined(USING_OPENMP_THREADS) || defined(USING_SGIMP_THREADS)

/* Use MP compiler directives to induce parallelism, in which case
   we don't need any of the thread spawning/waiting macros: */

typedef void * (*fftw_thr_function) (void *);

typedef char fftw_thr_id;  /* dummy */

#define fftw_thr_spawn(tid_ptr, proc, data) ((proc)(data))
#define fftw_thr_wait(tid) (0) /* do nothing */

#define USING_COMPILER_THREADS 1

/************************** POSIX Threads ****************************/

#elif defined(USING_POSIX_THREADS) /* use the default, POSIX threads: */

/* POSIX threads glue.  Tested. */

#ifndef USING_POSIX_THREADS
#define USING_POSIX_THREADS
#endif

/* link with -lpthread, or better yet use ACX_PTHREAD in autoconf */

#include <pthread.h>

/* Thread entry point: */
typedef void * (*fftw_thr_function) (void *);

extern pthread_attr_t *fftw_pthread_attributes_p;

typedef pthread_t fftw_thr_id;

#define fftw_thr_spawn(tid_ptr, proc, data) { \
     CK(!pthread_create(tid_ptr,fftw_pthread_attributes_p,proc,data)); \
}

#define fftw_thr_wait(tid) { CK(!pthread_join(tid,0)); }

#elif defined(HAVE_THREADS)
#  error HAVE_THREADS is defined without any USING_*_THREADS
#endif

/***********************************************************************/

#ifdef HAVE_THREADS

/* Distribute a loop from 0 to loopmax-1 over nthreads threads.
   proc(d) is called to execute a block of iterations from d->min
   to d->max-1.  d->thr_num indicate the number of the thread
   that is executing proc (from 0 to nthreads-1), and d->data is
   the same as the data parameter passed to fftw_thr_spawn_loop.

   This function returns only when all the threads have completed. */
void X(spawn_loop)(uint loopmax, uint nthr,
		   spawn_function proc, void *data)
{
     uint block_size;

     A(loopmax > 0);
     A(nthr > 0);

     /* Choose the block size and number of threads in order to (1)
        minimize the critical path and (2) use the fewest threads that
        achieve the same critical path (to minimize overhead).
        e.g. if loopmax is 5 and nthr is 4, we should use only 3
        threads with block sizes of 2, 2, and 1. */
     block_size = (loopmax + nthr - 1) / nthr;
     nthr = (loopmax + block_size - 1) / block_size;

     if (nthr <= 1) {
	  spawn_data d;
	  d.min = 0; d.max = loopmax;
	  d.thr_num = 0;
	  d.data = data;
	  proc(&d);
     }
     else {
#ifdef USING_COMPILER_THREADS
	  spawn_data d;
#else
	  spawn_data *d;
	  fftw_thr_id *tid;
#endif
	  uint i;
	  
	  THREAD_ON;

#ifdef USING_COMPILER_THREADS
	  
#if defined(USING_SGIMP_THREADS)
#pragma parallel local(d,i)
	  {
#pragma pfor iterate(i=0; nthr; 1)
#elif defined(USING_OPENMP_THREADS)
#pragma omp parallel for private(d)
#endif	
	       for (i = 0; i < nthr; ++i) {
		    d.max = (d.min = i * block_size) + block_size;
                    if (d.max > loopmax)
                         d.max = loopmax;
		    d.thr_num = i;
		    d.data = data;
		    proc(&d);
	       }
#if defined(USING_SGIMP_THREADS)
	  }
#endif

#else /* ! USING_COMPILER_THREADS, i.e. explicit thread spawning: */

	  STACK_MALLOC(spawn_data *, d, sizeof(spawn_data) * nthr);
	  STACK_MALLOC(fftw_thr_id *, tid, sizeof(fftw_thr_id) * (--nthr));

	  for (i = 0; i < nthr; ++i) {
	       d[i].max = (d[i].min = i * block_size) + block_size;
	       d[i].thr_num = i;
	       d[i].data = data;
	       fftw_thr_spawn(&tid[i],
			      (fftw_thr_function) proc, (void *) &d[i]);
	  }
	  d[i].min = i * block_size;
	  d[i].max = loopmax;
	  d[i].thr_num = i;
	  d[i].data = data;
	  proc(&d[i]);
	  
	  for (i = 0; i < nthr; ++i)
	       fftw_thr_wait(tid[i]);

	  STACK_FREE(tid);
	  STACK_FREE(d);

#endif /* ! USING_COMPILER_THREADS */

	  THREAD_OFF;
     }
}

#else /* ! HAVE_THREADS */
void X(spawn_loop)(uint loopmax, uint nthr,
		   spawn_function proc, void *data)
{
     spawn_data d;
     UNUSED(nthr);
     d.min = 0; d.max = loopmax;
     d.thr_num = 0;
     d.data = data;
     proc(&d);
}
#endif

#ifdef HAVE_THREADS
void kdft_dit_register_hook(planner *p, kdft_dit k, const ct_desc *d)
{
     REGISTER_SOLVER(p, X(mksolver_dft_ct_dit_thr)(k, d));
}
void khc2hc_dit_register_hook(planner *p, khc2hc k, const hc2hc_desc *d)
{
     REGISTER_SOLVER(p, X(mksolver_rdft_hc2hc_dit_thr)(k, d));
}
void khc2hc_dif_register_hook(planner *p, khc2hc k, const hc2hc_desc *d)
{
     REGISTER_SOLVER(p, X(mksolver_rdft_hc2hc_dif_thr)(k, d));
}
#endif /* HAVE_THREADS */

#ifdef USING_POSIX_THREADS
static pthread_attr_t fftw_pthread_attributes; /* attrs for POSIX threads */
pthread_attr_t *fftw_pthread_attributes_p = NULL;
#endif /* USING_POSIX_THREADS */

/* fftw_thrs_init does any initialization that is necessary to use
   threads.  It must be called before calling fftw_thrs or
   fftwnd_threads. 
   
   Returns 0 if successful, and non-zero if there is an error.
   Do not call any fftw_thrs routines if fftw_thrs_init
   is not successful! */

int X(threads_init)(void)
{
#ifdef USING_POSIX_THREADS
     /* Set the thread creation attributes as necessary.  If we don't
	change anything, just use the default attributes (NULL). */
     int err, attr, attr_changed = 0;

     err = pthread_attr_init(&fftw_pthread_attributes); /* set to defaults */
     if (err) return err;

     /* Make sure that threads are joinable!  (they aren't on AIX) */
     err = pthread_attr_getdetachstate(&fftw_pthread_attributes, &attr);
     if (err) return err;
     if (attr != PTHREAD_CREATE_JOINABLE) {
	  err = pthread_attr_setdetachstate(&fftw_pthread_attributes,
					    PTHREAD_CREATE_JOINABLE);
	  if (err) return err;
	  attr_changed = 1;
     }

     /* Make sure threads parallelize (they don't by default on Solaris) */
     err = pthread_attr_getscope(&fftw_pthread_attributes, &attr);
     if (err) return err;
     if (attr != PTHREAD_SCOPE_SYSTEM) {
	  err = pthread_attr_setscope(&fftw_pthread_attributes,
				      PTHREAD_SCOPE_SYSTEM);
	  if (err) return err;
	  attr_changed = 1;
     }

     if (attr_changed)  /* we aren't using the defaults */
	  fftw_pthread_attributes_p = &fftw_pthread_attributes;
     else {
	  fftw_pthread_attributes_p = NULL;  /* use default attributes */
	  err = pthread_attr_destroy(&fftw_pthread_attributes);
	  if (err) return err;
     }
#endif /* USING_POSIX_THREADS */

#ifdef USING_MACOS_THREADS
     /* FIXME: don't have malloc hooks (yet) in fftw3 */
     /* Must use MPAllocate and MPFree instead of malloc and free: */
     if (MPLibraryIsLoaded()) {
	  fftw_malloc_hook = MPAllocate;
	  fftw_free_hook = MPFree;
     }
#endif /* USING_MACOS_THREADS */

#if defined(USING_OPENMP_THREADS) && ! defined(_OPENMP)
#error OpenMP enabled but not using an OpenMP compiler
#endif

#ifdef HAVE_THREADS
     X(kdft_dit_register_hook) = kdft_dit_register_hook;
     X(khc2hc_dit_register_hook) = khc2hc_dit_register_hook;
     X(khc2hc_dif_register_hook) = khc2hc_dif_register_hook;
     return 0; /* no error */
#else
     return -31416; /* no threads */
#endif
}
