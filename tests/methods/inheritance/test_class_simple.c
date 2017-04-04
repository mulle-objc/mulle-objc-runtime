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
   struct _mulle_objc_classpair      *pair;
   struct _mulle_objc_class          *A_cls;
   struct _mulle_objc_class          *B_cls;
   static struct _mulle_objc_class   C_cls;
   int                               rval;
   struct _mulle_objc_runtime        *runtime;

   runtime = __get_or_create_objc_runtime();
   assert( runtime);

   pair = mulle_objc_unfailing_new_classpair( A_classid, "A", 0, NULL);
   assert( pair);
   A_cls = _mulle_objc_classpair_get_infraclass( pair);

   assert( _mulle_objc_class_get_metaclass( A_cls));
   assert( _mulle_objc_class_get_metaclass( A_cls) != A_cls);

   pair = mulle_objc_unfailing_new_classpair( B_classid, "B", 0, A_cls);
   assert( pair);
   B_cls = _mulle_objc_classpair_get_infraclass( pair);

   // check that we can't add basic useless stuff
   rval = mulle_objc_runtime_add_class( runtime, NULL);
   assert( rval == -1 && errno == EINVAL);

   // C_cls has no method lists
   rval = mulle_objc_runtime_add_class( runtime, &C_cls);
   assert( rval == -1 && errno == EINVAL);

   rval = mulle_objc_runtime_add_class( runtime, B_cls);
   assert( rval == -1 && errno == ECHILD);

   // now fix up classes with empty lists, so they are OK to be added
   rval = mulle_objc_class_add_instancemethodlist( B_cls, NULL);
   assert( rval == 0);
   rval = mulle_objc_class_add_classmethodlist( B_cls, NULL);
   assert( rval == 0);
   rval = mulle_objc_class_add_ivarlist( B_cls, NULL);
   assert( rval == 0);
   rval = mulle_objc_class_add_propertylist( B_cls, NULL);
   assert( rval == 0);

   // check that we can't add, if superclass is not present
   rval = mulle_objc_runtime_add_class( runtime, B_cls);
   assert( rval == -1 && errno == EFAULT);

   rval = mulle_objc_class_add_instancemethodlist( A_cls, NULL);
   assert( rval == 0);
   rval = mulle_objc_runtime_add_class( runtime, A_cls);
   assert( rval == -1 && errno == ECHILD);

   rval = mulle_objc_class_add_classmethodlist( A_cls, NULL);
   assert( rval == 0);
   rval = mulle_objc_runtime_add_class( runtime, A_cls);
   assert( rval == -1 && errno == ECHILD);

   rval = mulle_objc_class_add_ivarlist( A_cls, NULL);
   assert( rval == 0);
   rval = mulle_objc_runtime_add_class( runtime, A_cls);
   assert( rval == -1 && errno == ECHILD);

   rval = mulle_objc_class_add_propertylist( A_cls, NULL);
   assert( rval == 0);


   // check that we can add a class
   rval = mulle_objc_runtime_add_class( runtime, A_cls);
   assert( rval == 0);

   // check double add not possible
   rval = mulle_objc_runtime_add_class( runtime, A_cls);
   assert( rval == -1 && errno == EEXIST);

   rval = mulle_objc_runtime_add_class( runtime, B_cls);
   assert( rval == 0);
}
