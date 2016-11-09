//
//  mulle_standalone_objc_runtime.c
//  mulle-objc
//
//  Created by Nat! on 21.01.16.
//  Copyright (c) 2016 Nat! - Mulle kybernetiK.
//  Copyright (c) 2016 Codeon GmbH.
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
#include "mulle_objc.h"

#include <mulle_test_allocator/mulle_test_allocator.h>



static void  tear_down()
{
   // no autoreleasepools here
   
   mulle_objc_release_runtime();
   
   if( getenv( "MULLE_OBJC_TEST_ALLOCATOR"))
      mulle_test_allocator_reset();
}



MULLE_C_CONST_RETURN  // always returns same value (in same thread)
struct _mulle_objc_runtime  *__get_or_create_objc_runtime( void)
{
   struct _mulle_objc_runtime      *runtime;
   struct mulle_allocator          *allocator;
   int                             is_test;
   int                             is_pedantic;
   
   runtime = __mulle_objc_get_runtime();
   if( ! _mulle_objc_runtime_is_initalized( runtime))
   {
      allocator = NULL;
      is_test = getenv( "MULLE_OBJC_TEST_ALLOCATOR") != NULL;
      if( is_test)
      {
         // call this because we are probably also in +load here
         mulle_test_allocator_initialize();
         allocator = &mulle_test_allocator;
#if DEBUG
         fprintf( stderr, "mulle_objc_runtime uses \"mulle_test_allocator\" to detect leaks.\n");
#endif
      }
      __mulle_objc_runtime_setup( runtime, allocator);

      is_pedantic = getenv( "MULLE_OBJC_PEDANTIC_EXIT") != NULL;
      if( is_test || is_pedantic)
         if( atexit( tear_down))
            perror( "atexit:");
   }
   return( runtime);
}

