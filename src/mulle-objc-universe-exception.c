//
//  mulle-objc-universe-fail.h
//  mulle-objc-runtime
//
//  Created by Nat! on 16/11/14.
//  Copyright (c) 2014-2018 Nat! - Mulle kybernetiK.
//  Copyright (c) 2014-2018 Codeon GmbH.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  Redistributions of source code must retain the above copyright notice, this
//  list of conditions and the following disclaimer.
//
//  Redistributions in binary form must reproduce the above copyright notice,
//  this list of conditions and the following disclaimer in the documentation
//  and/or other materials provided with the distribution.
//
//  Neither the name of Mulle kybernetiK nor the names of its contributors
//  may be used to endorse or promote products derived from this software
//  without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
#include "mulle-objc-universe-exception.h"

#include <string.h>
#include <setjmp.h>

#include "mulle-objc-universe-struct.h"
#include "mulle-objc-universe.h"
#include "mulle-objc-universe-fail.h"
#include "mulle-objc-universe-class.h"
#include "mulle-objc-object-convenience.h"

# pragma mark - "exceptions"



/* from clang:

  A catch buffer is a setjmp buffer plus:
    - a pointer to the exception that was caught
    - a pointer to the previous exception data buffer
    - two pointers of reserved storage
  Therefore catch buffers form a stack, with a pointer to the top
  of the stack kept in thread-local storage.

  objc_exception_throw pops the top of the EH stack, writes the
    thrown exception into the appropriate field, and longjmps
    to the setjmp buffer.  It crashes the process (with a printf
    and an abort()) if there are no catch buffers on the stack.
  objc_exception_extract just reads the exception pointer out of the
    catch buffer.
*/

struct _mulle_objc_exceptionstackentry
{
   void                                     *exception;
   struct _mulle_objc_exceptionstackentry   *previous;
   void                                     *unused[ 2];
   jmp_buf                                  buf;
};


void   _mulle_objc_universe_throw( struct _mulle_objc_universe *universe, void *exception)
{
   struct _mulle_objc_exceptionstackentry  *entry;
   struct _mulle_objc_threadinfo           *config;

   config = _mulle_objc_thread_get_threadinfo( universe);
   entry  = config->exception_stack;
   if( ! entry)
   {
      if( universe->failures.uncaughtexception)
         (*universe->failures.uncaughtexception)( exception);

      fprintf( stderr, "mulle-objc-universe %p: Uncaught exception %p", universe, exception);
      abort();
   }

   entry->exception        = exception;
   config->exception_stack = entry->previous;

   // from Apple objc_universe.mm
#if defined( _WIN32) || defined( __linux__)
    longjmp( entry->buf, 1);
#else
    _longjmp( entry->buf, 1);
#endif
}


//
//  objc_exception_tryenter pushes a catch buffer onto the EH stack.
//
void   _mulle_objc_universe_tryenter( struct _mulle_objc_universe *universe,
                                      void *data)
{
   struct _mulle_objc_exceptionstackentry  *entry = data;
   struct _mulle_objc_exceptionstackentry  *top;
   struct _mulle_objc_threadinfo         *config;

   config                  = _mulle_objc_thread_get_threadinfo( universe);
   top                     = config->exception_stack;
   entry->exception        = NULL;
   entry->previous         = top;
   config->exception_stack = entry;
}


//
//  objc_exception_tryexit pops the given catch buffer, which is
//    required to be the top of the EH stack.
//
void   _mulle_objc_universe_tryexit( struct _mulle_objc_universe *universe,
                                     void *data)
{
   struct _mulle_objc_exceptionstackentry *entry = data;
   struct _mulle_objc_threadinfo        *config;

   config                  = _mulle_objc_thread_get_threadinfo( universe);
   config->exception_stack = entry->previous;
}


void   *_mulle_objc_universe_extract_exception( struct _mulle_objc_universe *universe,
                                                void *data)
{
   struct _mulle_objc_exceptionstackentry *entry = data;

   return( entry->exception);
}


int  _mulle_objc_universe_match_exception( struct _mulle_objc_universe *universe,
                                           mulle_objc_classid_t classid,
                                           void *exception)
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_class        *exceptionCls;

   assert( classid != MULLE_OBJC_NO_CLASSID && classid != MULLE_OBJC_INVALID_CLASSID);
   assert( exception);

   infra        = _mulle_objc_universe_lookup_infraclass_nofail( universe, classid);
   exceptionCls = __mulle_objc_object_get_isa_notps( exception);

   do
   {
      if( _mulle_objc_infraclass_as_class( infra) == exceptionCls)
         return( 1);
      exceptionCls = _mulle_objc_class_get_superclass( exceptionCls);
   }
   while( exceptionCls);

   return( 0);
}


void   _mulle_objc_universe_init_exception( struct _mulle_objc_universe  *universe)
{
   universe->exceptionvectors.throw     = _mulle_objc_universe_throw;
   universe->exceptionvectors.try_enter = _mulle_objc_universe_tryenter;
   universe->exceptionvectors.try_exit  = _mulle_objc_universe_tryexit;
   universe->exceptionvectors.extract   = _mulle_objc_universe_extract_exception;
   universe->exceptionvectors.match     = _mulle_objc_universe_match_exception;
}
