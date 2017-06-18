//
//  test_message_forwarding.c
//  mulle-objc-runtime
//
//  Created by Nat! on 13.03.15.
//  Copyright (c) 2015 Mulle kybernetiK. All rights reserved.
//
#define __MULLE_OBJC_NO_TPS__
#define __MULLE_OBJC_NO_TRT__

#include <mulle_objc_runtime/mulle_objc_runtime.h>
#include "test_runtime_ids.h"

#include <stdio.h>

//
// this doesn't check inheritance, it just creates a class dynamically
// adds methods to it and checks that the cache is OK
//
static intptr_t  forward_global( void *obj, mulle_objc_methodid_t sel, void *params)
{
   return( (intptr_t) 0x1234);
}


static struct _mulle_objc_method   forward =
{
   {
      MULLE_OBJC_FORWARD_METHODID,
      "forward:",  // egal
      "@:^v",
      0
   },
   (void *) forward_global
};


static struct _mulle_objc_methodlist  forward_list =
{
   1,
   NULL,
   {
      {
         {
            MULLE_OBJC_FORWARD_METHODID,
            "forward:",  // egal
            "@:^v",
            0
         },
         (void *) forward_global
      }
   }
};



void   test_message_forwarding1()
{
   struct _mulle_objc_classpair    *pair;
   struct _mulle_objc_infraclass   *A_infra;
   struct _mulle_objc_metaclass    *A_meta;
   void                            *rval;
   struct _mulle_objc_universe      *universe;

   pair = mulle_objc_unfailing_new_classpair( A_classid, "A", 0, NULL);
   assert( pair);
   A_infra      = _mulle_objc_classpair_get_infraclass( pair);
   A_meta = _mulle_objc_class_get_metaclass( _mulle_objc_infraclass_as_class( A_infra));

      // now fix up classes with empty method lists, so they are OK to be added
   mulle_objc_infraclass_unfailing_add_methodlist( A_infra, NULL);
   mulle_objc_metaclass_unfailing_add_methodlist( A_meta, NULL);
   mulle_objc_infraclass_unfailing_add_ivarlist( A_infra, NULL);
   mulle_objc_infraclass_unfailing_add_propertylist( A_infra, NULL);

   universe = mulle_objc_get_or_create_universe();
   universe->classdefaults.forwardmethod = &forward_list.methods[ 0];

   mulle_objc_unfailing_add_infraclass( A_infra);

   assert( _mulle_objc_class_get_forwardmethod( (void *) A_infra) == NULL);
   assert( _mulle_objc_class_get_forwardmethod( (void *) A_meta) == NULL);

   rval = mulle_objc_object_call( A_infra, 0x1, NULL);
   assert( rval == (void *) 0x1234);

   // it's + here
   assert( _mulle_objc_class_get_forwardmethod( (void *) A_infra) == NULL);
   assert( _mulle_objc_class_get_forwardmethod( (void *) A_meta) != NULL);

   rval = mulle_objc_object_call( A_infra, 0x1, NULL);
   assert( rval == (void *) 0x1234);
}



void   test_message_forwarding2()
{
   struct _mulle_objc_classpair     *pair;
   struct _mulle_objc_infraclass    *A_infra;
   struct _mulle_objc_metaclass     *A_meta;
   struct _mulle_objc_universe       *universe;
   void                             *rval;

   pair = mulle_objc_unfailing_new_classpair( A_classid, "A", 0, NULL);
   assert( pair);
   A_infra      = _mulle_objc_classpair_get_infraclass( pair);
   A_meta = _mulle_objc_classpair_get_metaclass( pair);

   // now fix up classes with empty method lists, so they are OK to be added
   mulle_objc_infraclass_unfailing_add_methodlist( A_infra, NULL);
   mulle_objc_metaclass_unfailing_add_methodlist( A_meta, &forward_list);
   mulle_objc_infraclass_unfailing_add_ivarlist( A_infra, NULL);
   mulle_objc_infraclass_unfailing_add_propertylist( A_infra, NULL);

   mulle_objc_unfailing_add_infraclass( A_infra);

   universe = mulle_objc_get_or_create_universe();
   assert( universe->classdefaults.forwardmethod == NULL);

   assert( _mulle_objc_class_get_forwardmethod( (void *) A_infra) == NULL);
   assert( _mulle_objc_class_get_forwardmethod( (void *) A_meta) == NULL);

   rval = mulle_objc_object_call( A_infra, 0x1, NULL);
   assert( rval == (void *) 0x1234);

   // it's + here
   assert( _mulle_objc_class_get_forwardmethod( (void *) A_infra) == NULL);
   assert( _mulle_objc_class_get_forwardmethod( (void *) A_meta) != NULL);

   rval = mulle_objc_object_call( A_infra, 0x1, NULL);
   assert( rval == (void *) 0x1234);
}
