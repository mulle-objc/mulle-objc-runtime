//
//  test_method.c
//  mulle-objc-runtime
//
//  Created by Nat! on 16.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__
#define __MULLE_OBJC_FMC__

#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include "test_runtime_ids.h"
#include "test_simple_inheritance.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <setjmp.h>


static jmp_buf  crashbuf;

static void   crash_catch( char *format, va_list args)
{
   vfprintf( stderr, format, args);
   fprintf( stderr, "\n");
   longjmp( crashbuf, 1);
}

void   test_method( void)
{
   int    rval;
   struct _mulle_objc_descriptor   clone;
   struct _mulle_objc_universe     *universe;
   void                            (*oldfail)( char *, va_list);

   universe = mulle_objc_register_universe();
   assert( universe);

   rval = _mulle_objc_universe_add_descriptor( universe, &A_foo_method->descriptor);
   assert( ! rval);

   // duplicate add with same name is OK
   rval = _mulle_objc_universe_add_descriptor( universe, &A_foo_method->descriptor);
   assert( ! rval);

// this is now hard to test, would need longjmp setjmp here
   oldfail                 = universe->failures.fail;
   universe->failures.fail = crash_catch;

   if( ! setjmp( crashbuf))
   {
      // check that duplicate ID is wrong if name doesn't match
      clone           = A_foo_method->descriptor;
      clone.name      = "x";

#if DEBUG
      fprintf( stderr, "possibly following warning about x's different method id is expected\n");
#endif

      rval = _mulle_objc_universe_add_descriptor( universe, &clone);
      assert( 0 && "must not reach this");
   }
   universe->failures.fail = oldfail;

   // different signature is harmless
   clone           = A_foo_method->descriptor;
   clone.signature = "@:@";

#if DEBUG
   fprintf( stderr, "possibly following warning about x's different signature is expected\n");
#endif
   rval = _mulle_objc_universe_add_descriptor( universe, &clone);
   assert( ! rval);

   // different id for same name will be caught in debug only
   clone          = A_foo_method->descriptor;
   clone.methodid = B_bar_method->descriptor.methodid;

#if DEBUG
   fprintf( stderr, "possibly following warning about foo's different method id is expected\n");
#endif
   rval = _mulle_objc_universe_add_descriptor( universe, &clone);
}



