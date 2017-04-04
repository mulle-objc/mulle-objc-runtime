//
//  test_method.c
//  mulle-objc-runtime
//
//  Created by Nat! on 16.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__

#include <mulle_objc/mulle_objc.h>

#include "test_runtime_ids.h"
#include "test_simple_inheritance.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>


static void   _test_method( struct abc_classes  *classes)
{
   int    rval;
   struct _mulle_objc_methoddescriptor   clone;
   struct _mulle_objc_runtime             *runtime;

   runtime = __get_or_create_objc_runtime();
   assert( runtime);

   create_ABC_classes( classes);

   rval = _mulle_objc_runtime_add_methoddescriptor( runtime, &A_foo_method->descriptor);
   assert( ! rval);

   // duplicate add with same name is OK
   rval = _mulle_objc_runtime_add_methoddescriptor( runtime, &A_foo_method->descriptor);
   assert( ! rval);

   // check that duplicate ID is wrong if name doesn't match
   clone       = A_foo_method->descriptor;
   clone.name = "x";

#if DEBUG
   fprintf( stderr, "possibly following warning about x's different method id is expected\n");
#endif
   rval = _mulle_objc_runtime_add_methoddescriptor( runtime, &clone);
   assert( rval && errno == EEXIST);

   // different signature is harmless
   clone           = A_foo_method->descriptor;
   clone.signature = "@:@";
#if DEBUG
   fprintf( stderr, "possibly following warning about x's different signature is expected\n");
#endif
   rval = _mulle_objc_runtime_add_methoddescriptor( runtime, &clone);
   assert( ! rval);

   // different id for same name will not be caught, but will warn in debug
   clone           = A_foo_method->descriptor;
   clone.methodid = B_bar_method->descriptor.methodid;

#if DEBUG
   fprintf( stderr, "possibly following warning about foo's different method id is expected\n");
#endif
   rval = _mulle_objc_runtime_add_methoddescriptor( runtime, &clone);
   assert( ! rval);
}


void   test_method( void)
{
   struct abc_classes   classes;

   _test_method( &classes);
}

