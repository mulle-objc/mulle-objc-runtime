#include <mulle-objc-runtime/mulle-objc-runtime.h>


@interface Foo

- (long long) dontDoit:(double) value;
- (long long) doit:(double) value;

@end


@implementation Foo

+ (id) new
{
   return( mulle_objc_infraclass_alloc_instance( self));
}


- (void) dealloc
{
   _mulle_objc_instance_free( self);
}


- (long long) doit:(double) value
{
   return( (long long) (value + 1847.4));
}


- (void *) forward:(void *) param
{
   /* caveat: the selector of dontDoit: is not known to the
              runtime, only to the compiler. That's because there is
              no actual method implemented so nothing gets compiled and
              loaded into the runtime.
              It would be an error in any case to forward
              _cmd here as the selector to 'doit:'!
    */
   if( _cmd == @selector( dontDoit:))
      return( mulle_objc_object_call( self, @selector( doit:), param));
   fprintf( stderr, "FAIL");
   return( (void *) 12345678);
}

@end


main()
{
   Foo  *foo;

   foo = [Foo new];
   printf( "%lld\n", [foo dontDoit:0.6]);
   [foo dealloc];
}
