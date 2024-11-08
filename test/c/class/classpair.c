//
//  main.c
//  test-runtime-2
//
//  Copyright (c) 2024 Mulle kybernetiK. All rights reserved.
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

#include <mulle-objc-runtime/mulle-objc-runtime.h>

#include <stdio.h>


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
   fprintf( stderr, "__register_mulle_objc_universe done");
   return( universe);
}


int   main( int argc, const char * argv[])
{
   struct _mulle_objc_infraclass   *infra;
   struct _mulle_objc_metaclass    *meta;
   struct _mulle_objc_classpair    *pair;
   struct _mulle_objc_universe     *universe;

   universe = mulle_objc_global_register_universe( 0, "");
   assert( universe);

   pair = mulle_objc_universe_new_classpair( universe,
                                             0xc7e16770,
                                             "Foo",
                                             0,
                                             0,
                                             NULL);
   assert( pair);

   infra = _mulle_objc_classpair_get_infraclass( pair);
   if( pair != _mulle_objc_infraclass_get_classpair( infra))
      return( 1);

   meta = _mulle_objc_classpair_get_metaclass( pair);
   if( pair != _mulle_objc_metaclass_get_classpair( meta))
      return( 1);

   if( meta != _mulle_objc_infraclass_get_metaclass( infra))
      return( 1);

   if( infra != _mulle_objc_metaclass_get_infraclass( meta))
      return( 1);

   return 0;
}

