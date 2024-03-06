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


/* TODO:

   NSProxy              Object
   +-----------+       +------------+
   | forward:  |<----->|            |  [NSNotificationCenter self]
   +-----------+       +------------+

   NSProxy       
   +------------+       
   | forward:   |
   +------------+
   |            | 
   +------------+
   |            | [super ]
   +------------+

*/



#define N_CACHES  2

struct demo_class
{
   struct _mulle_objc_methodcachepivot   cachepivot[ N_CACHES];
};


static void  demo_class_print_cache( struct demo_class *cls, unsigned int index)
{
   struct _mulle_objc_cache        *cache;
   mulle_objc_cache_uint_t         i, n;

   cache = _mulle_objc_cachepivot_atomicget_cache( &cls->cachepivot[ index].pivot);
   n     = _mulle_objc_cache_get_size( cache);

   fprintf( stderr, "%d:\n", index);
   for( i = 0; i < n; i++)
   {
      fprintf( stderr, "   % 2d: [ 0x%08x, %p ]\n", 
                        i,
                        cache->entries[ i].key.uniqueid, 
                        (void *) cache->entries[ i].value.functionpointer);
   }
}


static void   *imp0( void *self, mulle_objc_methodid_t _cmd, void *_params)
{
   printf( "%s\n", __PRETTY_FUNCTION__);
   return( self);
}


static void   *imp1( void *self, mulle_objc_methodid_t _cmd, void *_params)
{
   printf( "%s\n", __PRETTY_FUNCTION__);
   return( self);
}


static void   *imp2( void *self, mulle_objc_methodid_t _cmd, void *_params)
{
   printf( "%s\n", __PRETTY_FUNCTION__);
   return( self);
}


static void   *imp3( void *self, mulle_objc_methodid_t _cmd, void *_params)
{
   printf( "%s\n", __PRETTY_FUNCTION__);
   return( self);
}


#define N_METHODS  4

static struct _mulle_objc_method   methods[ N_METHODS] = 
{
   {
      .descriptor =
      {
         0x10100,             // descriptor.methodid
         "@:",                // descriptor.signature
         "m0",                // descriptor.name
         0                    // descriptor.bits
      },
      .value = imp0
   },
   {
      .descriptor =
      {
         0x10110,             // descriptor.methodid
         "@:",                // descriptor.signature
         "m1",                // descriptor.name
         0                    // descriptor.bits
      },
      .value = imp1
   },
   {
      .descriptor =
      {
         0x10120,             // descriptor.methodid
         "@:",                // descriptor.signature
         "m2",                // descriptor.name
         0                    // descriptor.bits
      },
      .value = imp2
   },
   {
      .descriptor =
      {
         0x10130,             // descriptor.methodid
         "@:",                // descriptor.signature
         "m3",                // descriptor.name
         0                    // descriptor.bits
      },
      .value = imp3
   }
};


struct _mulle_objc_universe  *
   __register_mulle_objc_universe( mulle_objc_universeid_t universeid,
                                  char *universename)
{
   struct _mulle_objc_universe    *universe;

   universe = __mulle_objc_global_get_universe( universeid, universename);
   if( ! _mulle_objc_universe_is_initialized( universe))
   {
      _mulle_objc_universe_bang( universe, 0, NULL, NULL);
   }
   return( universe);
}


int  main( int argc, char *argv[])
{
   struct demo_class                cls = { 0 };
   unsigned int                     i;
   unsigned int                     j;
   unsigned int                     k;
   struct _mulle_objc_methodcache   *mcache;
   struct mulle_allocator           *allocator;
   mulle_objc_implementation_t      imp;

   // use this because the test allocator doesn't do abafree well , why ?
   allocator = &mulle_stdlib_allocator;

   assert( allocator->abafree);

   for( i = 0; i < N_CACHES; i++)
   {
      mcache = mulle_objc_methodcache_new( 4, allocator);

      // we don't have these callbacks
      mcache->call         = 0;
      mcache->call2        = 0;
      mcache->superlookup  = 0;  
      mcache->superlookup2 = 0;

      // MEMO: as the cache will grow, first two methods will be evicted
      //       again
      if( _mulle_objc_methodcachepivot_swap( &cls.cachepivot[ i],
                                             mcache,
                                             NULL,
                                             allocator))
         abort();

      for( j = 0; j < N_METHODS; j++)
          _mulle_objc_methodcachepivot_add_method( &cls.cachepivot[ i],
                                                   &methods[ j],
                                                   methods[ j].descriptor.methodid,
                                                   0,
                                                   allocator);      
   }

   for( i = 0; i < N_CACHES; i++)
   {
      for( j = 0; j < N_METHODS; j++)
      {
retry:
        // demo_class_print_cache( &cls, i);

         imp = mulle_objc_methodcachepivot_lookup_inline_null( &cls.cachepivot[ i],
                                                               methods[ j].descriptor.methodid);
         if( imp)
            (*imp)( NULL, methods[ j].descriptor.methodid, NULL);
         else
         {
            for( k = 0; k < N_METHODS; k++)
               if( methods[ k].descriptor.methodid == methods[ j].descriptor.methodid)
               {
                  _mulle_objc_methodcachepivot_add_method( &cls.cachepivot[ i],
                                                           &methods[ k],
                                                           methods[ k].descriptor.methodid,
                                                           0,
                                                           allocator); 
                  goto retry;     
               }
            // lookup method somehow and add to methodcache
         }
      }
   }

   // get rid of it
   for( i = 0; i < N_CACHES; i++)
   {
      mcache = _mulle_objc_methodcachepivot_get_methodcache( &cls.cachepivot[ i]);
      _mulle_objc_methodcachepivot_swap( &cls.cachepivot[ i],
                                         NULL,
                                         mcache,
                                         allocator);
   }

   return( 0);
}