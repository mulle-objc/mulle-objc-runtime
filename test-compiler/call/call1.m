#include <mulle-objc-runtime/mulle-objc-runtime.h>


//
// this test makes no sense in terms of being a test, but it is useful
// to observe cache usage and also with different .cflaqs how the compiler
// actually produces code.
//
static void   fprint_cache( FILE *fp, id foo)
{

   mulle_objc_cache_uint_t          offset;
   mulle_objc_cache_uint_t          mask;
   struct _mulle_objc_cacheentry   *p;
   struct _mulle_objc_cacheentry   *sentinel;
   struct _mulle_objc_cache        *cache;
   struct _mulle_objc_class        *cls;

   cls      = _mulle_objc_object_get_isa( foo);
   cache    = _mulle_objc_class_get_impcache_cache_atomic( cls);

   p        = cache->entries;
   sentinel = &p[ cache->size];
   mask     = cache->mask;
   offset   = 0;

   for( ; p < sentinel; p++)
   {
      fprintf( fp, "%x: %x", offset, p->key.uniqueid);
      if( p->key.uniqueid)
         fprintf( fp, "(%x)", (p->key.uniqueid & mask));
      fprintf( fp, "\n");

      offset += sizeof( struct _mulle_objc_cacheentry);
   }
}


@implementation Foo

+ (void) load
{
   _mulle_objc_class_set_state_bit( _mulle_objc_infraclass_as_class( self),
                                    MULLE_OBJC_CLASS_IS_NOT_THREAD_AFFINE);
}


+ (id) new
{
   return( mulle_objc_infraclass_alloc_instance( self));
}


- (void) dealloc
{
   _mulle_objc_instance_free( self);
}

#define a_x( x)  a__0 ## x ## 0 ## _


#define define_a_x( x)                    \
                                          \
- (void) a_x( x)                          \
{                                         \
   printf( "%s\n", __PRETTY_FUNCTION__);  \
}


define_a_x( 0)
define_a_x( 1)
define_a_x( 2)
define_a_x( 3)
define_a_x( 4)
define_a_x( 5)
define_a_x( 6)
define_a_x( 7)
define_a_x( 8)
define_a_x( 9)
define_a_x( a)
define_a_x( b)
define_a_x( c)
define_a_x( d)
define_a_x( e)

@end


static void  print_cache_stats( Foo *foo)
{
   struct _mulle_objc_cache  *cache;
   struct _mulle_objc_class  *cls;
   unsigned int              counts[ 4];
   unsigned int              percentage;
   unsigned int              n;
   unsigned int              size;

   cls        = _mulle_objc_object_get_isa( foo);
   cache      = _mulle_objc_class_get_impcache_cache_atomic( cls);

   percentage = mulle_objc_cache_calculate_fillpercentage( cache);

   n          = mulle_objc_cache_calculate_hits( cache, counts);
   size       = _mulle_objc_cache_get_size( cache);

   fprintf( stderr, "%u of %u (%u%%) %u,%u,%u,%u\n",
                        n, size, percentage,
                        counts[ 0], counts[ 1], counts[ 2], counts[ 3]);
}


#define call_a_x( foo, x)     [(foo) a_x( x)]; print_cache_stats( foo)



int   main( int argc, char *argv[])
{
   char   *s;
   Foo   *foo;

   s = mulle_objc_global_preprocessor_string( NULL);
   fprintf( stderr, "%s%s\n", argv[ 0], s);
   mulle_free( s);

   foo = [Foo new];

   // break thread affinity
   call_a_x(foo, 0);
   call_a_x(foo, 1);
   call_a_x(foo, 2);
   call_a_x(foo, 3);
   call_a_x(foo, 4);
   call_a_x(foo, 5);
   call_a_x(foo, 6);
   call_a_x(foo, 7);
   call_a_x(foo, 8);
   call_a_x(foo, 9);
   call_a_x(foo, a);
   call_a_x(foo, b);
   call_a_x(foo, c);
   call_a_x(foo, d);
   call_a_x(foo, e);

   fprint_cache( stderr, foo);

   [foo dealloc];
}
