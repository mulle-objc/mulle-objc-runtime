#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface Foo

- (unsigned long long) doit;
- (unsigned long long) doitnow;

@end


@implementation Foo

+ (id) new
{
   return( mulle_objc_infraclass_alloc_instance( self));
}


- (void) dealloc
{
   _mulle_objc_object_free( self);
}


- (unsigned long long) doit
{
   unsigned long long   value;

   // sez hi bit and lo bit,
   value = 0x1ULL << (sizeof( unsigned long long) * 8 - 1) | 0x1;
//   fprintf( stderr, "0x%llx\n", value);
   return( value);
}


- (void *) forward:(void *) param
{
   if( _cmd == @selector( doitnow))
      return( mulle_objc_object_call( self, @selector( doit), param));
   fprintf( stderr, "FAIL");
   return( (void *) 12345678);
}

@end


main()
{
   Foo                  *foo;
   unsigned long long   value;
   unsigned long long   org;

   foo   = [Foo new];
   org   = [foo doitnow];
   value = org;
   if( value & 1)
   {
      printf( "lo set\n");
      value &= ~1ULL;
   }
   if( value && ! (value << 1))
   {
      printf( "hi set\n");
      value <<= 1;
      value >>= 1;
   }
   if( value)
      printf( "more bits set 0x%llx (FAIL)\n", org);

   [foo dealloc];
}
