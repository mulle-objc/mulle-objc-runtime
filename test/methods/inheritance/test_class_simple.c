//
//  test_class_simple.c
//  mulle-objc-runtime
//
//  Created by Nat! on 10.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__
#define __MULLE_OBJC_FMC__

#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include "test_runtime_ids.h"

#include <errno.h>


void   test_class_simple( void)
{
   struct _mulle_objc_classpair    *pair;
   struct _mulle_objc_infraclass   *A_infra;
   struct _mulle_objc_metaclass    *A_meta;
   struct _mulle_objc_infraclass   *B_infra;
   struct _mulle_objc_metaclass    *B_meta;
   int                             rval;
   struct _mulle_objc_universe     *universe;

   universe = mulle_objc_register_universe();
   assert( universe);

   pair = mulle_objc_new_classpair_nofail( A_classid, "A", 0, NULL);
   assert( pair);
   A_infra = _mulle_objc_classpair_get_infraclass( pair);
   A_meta  = _mulle_objc_classpair_get_metaclass( pair);

   assert( (void *) A_infra != (void *) A_meta);
   assert( _mulle_objc_class_get_metaclass( _mulle_objc_infraclass_as_class( A_infra)) == (void *) A_meta);

   pair = mulle_objc_new_classpair_nofail( B_classid, "B", 0, A_infra);
   assert( pair);
   B_meta  = _mulle_objc_classpair_get_metaclass( pair);
   B_infra = _mulle_objc_classpair_get_infraclass( pair);

   // check that we can't add basic useless stuff
   rval = mulle_objc_universe_add_infraclass( universe, NULL);
   assert( rval == -1 && errno == EINVAL);

   // B_infra has no method lists
   rval = mulle_objc_universe_add_infraclass( universe, B_infra);
   assert( rval == -1 && errno == ECHILD);

   // now fix up classes with empty lists, so they are OK to be added
   mulle_objc_infraclass_add_methodlist_nofail( B_infra, NULL);
   mulle_objc_metaclass_add_methodlist_nofail( B_meta, NULL);
   mulle_objc_infraclass_add_ivarlist_nofail( B_infra, NULL);
   mulle_objc_infraclass_add_propertylist_nofail( B_infra, NULL);

   // check that we can't add, if superclass is not present
   rval = mulle_objc_universe_add_infraclass( universe, B_infra);
   assert( rval == -1 && errno == EFAULT);

   mulle_objc_infraclass_add_methodlist_nofail( A_infra, NULL);
   rval = mulle_objc_universe_add_infraclass( universe, A_infra);
   assert( rval == -1 && errno == ECHILD);

   mulle_objc_metaclass_add_methodlist_nofail( A_meta, NULL);
   rval = mulle_objc_universe_add_infraclass( universe, A_infra);
   assert( rval == -1 && errno == ECHILD);

   mulle_objc_infraclass_add_ivarlist_nofail( A_infra, NULL);
   rval = mulle_objc_universe_add_infraclass( universe, A_infra);
   assert( rval == -1 && errno == ECHILD);

   mulle_objc_infraclass_add_propertylist_nofail( A_infra, NULL);


   // check that we can add a class
   rval = mulle_objc_universe_add_infraclass( universe, A_infra);
   assert( rval == 0);

   // check double add not possible
   rval = mulle_objc_universe_add_infraclass( universe, A_infra);
   assert( rval == -1 && errno == EEXIST);

   rval = mulle_objc_universe_add_infraclass( universe, B_infra);
   assert( rval == 0);
}
