//
//  test_class_simple.c
//  mulle-objc-runtime
//
//  Created by Nat! on 10.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__

#include <mulle_objc/mulle_objc.h>

#include "test_runtime_ids.h"

#include <errno.h>


void   test_class_simple( void)
{
   struct _mulle_objc_classpair           *pair;
   struct _mulle_objc_infraclass          *A_infra;
   struct _mulle_objc_metaclass           *A_meta;
   struct _mulle_objc_infraclass          *B_infra;
   struct _mulle_objc_infraclass          *C_infra;
   int                                    rval;
   struct _mulle_objc_runtime             *runtime;

   runtime = __get_or_create_objc_runtime();
   assert( runtime);

   pair = mulle_objc_unfailing_new_classpair( A_classid, "A", 0, NULL);
   assert( pair);
   A_infra = _mulle_objc_classpair_get_infraclass( pair);
   A_meta  = _mulle_objc_classpair_get_metaclass( pair);

   assert( (void *) A_infra != (void *) A_meta);
   assert( _mulle_objc_class_get_metaclass( _mulle_objc_infraclass_as_class( A_infra)) == (void *) A_meta);

   pair = mulle_objc_unfailing_new_classpair( B_classid, "B", 0, A_infra);
   assert( pair);
   B_infra = _mulle_objc_classpair_get_infraclass( pair);

   // check that we can't add basic useless stuff
   rval = mulle_objc_runtime_add_infraclass( runtime, NULL);
   assert( rval == -1 && errno == EINVAL);

   // C_infra has no method lists

   pair    = mulle_objc_unfailing_new_classpair( C_classid, "C", 0, NULL);
   C_infra = _mulle_objc_classpair_get_infraclass( pair);

   rval = mulle_objc_runtime_add_infraclass( runtime, C_infra);
   assert( rval == -1 && errno == EINVAL);

   rval = mulle_objc_runtime_add_infraclass( runtime, B_infra);
   assert( rval == -1 && errno == ECHILD);

   // now fix up classes with empty lists, so they are OK to be added
   mulle_objc_infraclass_unfailing_add_methodlist( B_infra, NULL);
   mulle_objc_infraclass_unfailing_add_methodlist( B_infra, NULL);
   mulle_objc_infraclass_unfailing_add_ivarlist( B_infra, NULL);
   mulle_objc_infraclass_unfailing_add_propertylist( B_infra, NULL);

   // check that we can't add, if superclass is not present
   rval = mulle_objc_runtime_add_infraclass( runtime, B_infra);

   mulle_objc_infraclass_unfailing_add_methodlist( A_infra, NULL);
   rval = mulle_objc_runtime_add_infraclass( runtime, A_infra);

   mulle_objc_metaclass_unfailing_add_methodlist( A_meta, NULL);
   rval = mulle_objc_runtime_add_infraclass( runtime, A_infra);

   mulle_objc_infraclass_unfailing_add_ivarlist( A_infra, NULL);
   rval = mulle_objc_runtime_add_infraclass( runtime, A_infra);

   mulle_objc_infraclass_unfailing_add_propertylist( A_infra, NULL);


   // check that we can add a class
   rval = mulle_objc_runtime_add_infraclass( runtime, A_infra);
   assert( rval == 0);

   // check double add not possible
   rval = mulle_objc_runtime_add_infraclass( runtime, A_infra);
   assert( rval == -1 && errno == EEXIST);

   rval = mulle_objc_runtime_add_infraclass( runtime, B_infra);
   assert( rval == 0);
}
