//
//  waitqueue_hash.c
//  test-runtime
//
//  Copyright (c) 2026 Mulle kybernetiK. All rights reserved.
//
//  Verify the class-ID normalization used by the loader wait queues. Positive
//  mulle_concurrent_hashtable mode reserves the top intptr_t bit, so the two
//  IDs below alias on 32-bit systems and remain distinct on 64-bit systems.
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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>


int   main( void)
{
   struct mulle_concurrent_hashtable   map;
   mulle_objc_classid_t                classid;
   mulle_objc_classid_t                alias;
   intptr_t                            hash;
   intptr_t                            alias_hash;
   void                               *old;
   void                               *first;
   void                               *second;

   classid     = (mulle_objc_classid_t) 0x12345678u;
   alias       = classid | (mulle_objc_classid_t) 0x80000000u;
   hash        = _mulle_objc_classid_to_waitqueue_hash( classid);
   alias_hash  = _mulle_objc_classid_to_waitqueue_hash( alias);
   first       = (void *) 0x10;
   second      = (void *) 0x20;

#if UINTPTR_MAX == UINT32_MAX
   // The top bit is reserved by positive-mode hashtable migration.
   assert( hash == alias_hash);
#else
   assert( hash != alias_hash);
#endif

   mulle_aba_init( NULL);
   mulle_aba_register();
   mulle_concurrent_hashtable_init_positive( &map, 4, NULL);

   assert( mulle_concurrent_hashtable_insert( &map, hash, first) == 0);
#if UINTPTR_MAX == UINT32_MAX
   // This is the loader's real collision: both missing IDs share one list.
   assert( mulle_concurrent_hashtable_register( &map, alias_hash, second, &old) == 0);
   assert( old == first);
   assert( mulle_concurrent_hashtable_lookup( &map, alias_hash) == first);

   // Removing the list and registering the alias reuses the same queue slot;
   // this is exactly the lifecycle exercised by the loader's requeue path.
   assert( mulle_concurrent_hashtable_remove( &map, hash, first) == 0);
   assert( mulle_concurrent_hashtable_register( &map, alias_hash, second, &old) == 0);
   assert( old == NULL);
   assert( mulle_concurrent_hashtable_lookup( &map, hash) == second);
#else
   assert( mulle_concurrent_hashtable_insert( &map, alias_hash, second) == 0);
   assert( mulle_concurrent_hashtable_lookup( &map, hash) == first);
   assert( mulle_concurrent_hashtable_lookup( &map, alias_hash) == second);
#endif

   mulle_concurrent_hashtable_done( &map);
   mulle_aba_unregister();
   mulle_aba_done();

   printf( "passed\n");
   return( 0);
}
