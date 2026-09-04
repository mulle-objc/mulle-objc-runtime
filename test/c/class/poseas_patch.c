//
//  poseas_patch.c
//  test-runtime
//
//  Copyright (c) 2026 Mulle kybernetiK. All rights reserved.
//
//  Test that verifies:
//  1) The main thread does NOT retain the universe (retaincount_1 == 0)
//  2) mulle_objc_universe_patch_infraclass works in single-threaded mode
//
//  This test validates the correctness of using retaincount_1 as the
//  multi-threading discriminator for patch vs pose in
//  mulle_objc_universe_patch_infraclass.
//
#ifndef __MULLE_OBJC__
# define __MULLE_OBJC_NO_TPS__
# define __MULLE_OBJC_FCS__
# if defined( DEBUG) || ! defined( __OPTIMIZE__)
#  define __MULLE_OBJC_TAO__
# else
#  define __MULLE_OBJC_NO_TAO__
# endif
#endif

#define MULLE_OBJC_DEFINE_REGISTER_UNIVERSE

#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


MULLE_C_CONST_RETURN
struct _mulle_objc_universe  *
   __register_mulle_objc_universe( mulle_objc_universeid_t universeid,
                                   char *universename)
{
   struct _mulle_objc_universe    *universe;

   universe = __mulle_objc_global_get_universe( universeid, universename);
   if( ! _mulle_objc_universe_is_initialized( universe))
   {
      _mulle_objc_universe_bang( universe, 0, NULL, NULL);
      universe->config.ignore_ivarhash_mismatch = 1;
   }
   return( universe);
}


int   main( int argc, const char * argv[])
{
   struct _mulle_objc_infraclass   *base_infra;
   struct _mulle_objc_infraclass   *poser_infra;
   struct _mulle_objc_classpair    *base_pair;
   struct _mulle_objc_classpair    *poser_pair;
   struct _mulle_objc_universe     *universe;
   intptr_t                        rc;
   int                             rval;

   universe = mulle_objc_global_register_universe( 0, "");
   assert( universe);

   //
   // 1) Verify the main thread does NOT retain the universe.
   //    This is the key assumption: retaincount_1 == 0 means single-threaded.
   //    NSThread only retains for secondary threads (asserts thread != main).
   //
   rc = (intptr_t) _mulle_atomic_pointer_read( &universe->retaincount_1);
   if( rc != 0)
   {
      printf( "FAIL: retaincount_1 is %ld after init, expected 0\n", (long) rc);
      _mulle_objc_universe_release( universe);
      return( 1);
   }

   //
   // 2) Create base class "Bar" and register
   //
   base_pair = mulle_objc_universe_new_classpair( universe,
                                                  mulle_objc_classid_from_string( "Bar"),
                                                  "Bar",
                                                  0,
                                                  0,
                                                  NULL);
   assert( base_pair);
   base_infra = _mulle_objc_classpair_get_infraclass( base_pair);

   mulle_objc_infraclass_add_methodlist_nofail( base_infra, NULL);
   mulle_objc_metaclass_add_methodlist_nofail( _mulle_objc_classpair_get_metaclass( base_pair), NULL);
   mulle_objc_infraclass_add_ivarlist_nofail( base_infra, NULL);
   mulle_objc_infraclass_add_propertylist_nofail( base_infra, NULL);
   mulle_objc_universe_register_infraclass_nofail( universe, base_infra);

   //
   // 3) Create poser class "Foo : Bar"
   //
   poser_pair = mulle_objc_universe_new_classpair( universe,
                                                   mulle_objc_classid_from_string( "Foo"),
                                                   "Foo",
                                                   0,
                                                   0,
                                                   base_infra);
   assert( poser_pair);
   poser_infra = _mulle_objc_classpair_get_infraclass( poser_pair);

   mulle_objc_infraclass_add_methodlist_nofail( poser_infra, NULL);
   mulle_objc_metaclass_add_methodlist_nofail( _mulle_objc_classpair_get_metaclass( poser_pair), NULL);
   mulle_objc_infraclass_add_ivarlist_nofail( poser_infra, NULL);
   mulle_objc_infraclass_add_propertylist_nofail( poser_infra, NULL);
   mulle_objc_universe_register_infraclass_nofail( universe, poser_infra);

   //
   // 4) The universe IS initialized but we are single-threaded.
   //    patch_infraclass should use the cheap patch path (retaincount_1 == 0).
   //
   assert( _mulle_objc_universe_is_initialized( universe));
   rc = (intptr_t) _mulle_atomic_pointer_read( &universe->retaincount_1);
   if( rc != 0)
   {
      printf( "FAIL: retaincount_1 changed to %ld unexpectedly\n", (long) rc);
      _mulle_objc_universe_release( universe);
      return( 1);
   }

   //
   // 5) Pose Foo as Bar
   //
   rval = mulle_objc_universe_patch_infraclass( universe, poser_infra);
   if( rval)
   {
      printf( "FAIL: mulle_objc_universe_patch_infraclass returned %d (errno=%d)\n", rval, errno);
      _mulle_objc_universe_release( universe);
      return( 1);
   }

   //
   // 6) Verify: looking up "Bar" classid should now find the poser "Foo"
   //
   {
      struct _mulle_objc_infraclass  *found;

      found = _mulle_objc_universe_lookup_infraclass( universe,
                                                      mulle_objc_classid_from_string( "Bar"));
      if( found != poser_infra)
      {
         printf( "FAIL: lookup 'Bar' returned %p (%s), expected poser %p (%s)\n",
                  (void *) found, found ? _mulle_objc_infraclass_get_name( found) : "nil",
                  (void *) poser_infra, _mulle_objc_infraclass_get_name( poser_infra));
         _mulle_objc_universe_release( universe);
         return( 1);
      }
   }

   printf( "passed\n");

   _mulle_objc_universe_release( universe);
   return( 0);
}
